#include "stdafx.h"


#include "CLanServer.h"
#include "CMemoryPoolTLS.h"
#include "CSession.h"
#include "SerializationBuffer.h"
#include "LogManager.h"
#include "RingBufferEx.h"
#include "CPacket.h"


const int WSABUFFSIZE =100;//wsabuff�� �ִ� ũ��

#ifdef _TEST


#pragma region TestWSASendLog
enum class EWSASendLogDataType
{
    REQSEND,
    COMPLETSEND_1,
    COMPLETSEND_2,
};
struct WSASendLogData
{
    DWORD threadID;
    unsigned long reqcurSendCount;
    unsigned long reqreadIdx;

    unsigned long comcurSendCount;
    unsigned long comreadIdx;

    unsigned long comcurSendCount2;
    unsigned long comreadIdx2;
};
unsigned long idx_WSASendLogDataReqSendIdx = 0;
unsigned long idx_WSASendLogDataCompSendIdx = 0;
unsigned long idx_WSASendLogDataCompSendIdx2 = 0;
WSASendLogData* WSASendLogDataArr;
bool WSASendLog(DWORD _threadID, EWSASendLogDataType type, unsigned long _readidx, unsigned long _curSendCont)
{
    unsigned long idx = 0;
    switch (type)
    {
    case EWSASendLogDataType::REQSEND:
        idx = _InterlockedIncrement(&idx_WSASendLogDataReqSendIdx);
        WSASendLogDataArr[idx].threadID = _threadID;
        WSASendLogDataArr[idx].reqreadIdx = _readidx;
        WSASendLogDataArr[idx].reqcurSendCount = _curSendCont;
        break;
    case EWSASendLogDataType::COMPLETSEND_1:
        idx = _InterlockedIncrement(&idx_WSASendLogDataCompSendIdx);
        WSASendLogDataArr[idx].threadID = _threadID;
        WSASendLogDataArr[idx].comreadIdx = _readidx;
        WSASendLogDataArr[idx].comcurSendCount = _curSendCont;

        break;
    case EWSASendLogDataType::COMPLETSEND_2:
        idx = _InterlockedIncrement(&idx_WSASendLogDataCompSendIdx2);
        WSASendLogDataArr[idx].threadID = _threadID;
        WSASendLogDataArr[idx].comreadIdx2 = _readidx;
        WSASendLogDataArr[idx].comcurSendCount2 = _curSendCont;
        break;

    default:
        break;
    }
    return true;
}
#pragma endregion



#pragma region TestPacketLog
enum class EPacketLogDataType
{
    RECV,
    SEND,
    REMOVE
};
struct PacketLogData
{
    DWORD threadID;
    unsigned char recvData;
    unsigned char sendData;
    unsigned char removeData;
    unsigned char temp;
};
unsigned long idx_PacketLogDataRecvIdx = 0;
unsigned long idx_PacketLogDataSendIdx = 0;
unsigned long idx_PacketLogDataRemoveIdx = 0;
PacketLogData* PacketLogDataArr;

bool PacketLog(DWORD _threadID, EPacketLogDataType type, unsigned char _data)
{
    unsigned long idx = 0;
    switch (type)
    {
    case EPacketLogDataType::RECV:
        idx = _InterlockedIncrement(&idx_PacketLogDataRecvIdx);
        PacketLogDataArr[idx].threadID = _threadID;
        PacketLogDataArr[idx].recvData = _data;
        break;
    case EPacketLogDataType::SEND:
        idx = _InterlockedIncrement(&idx_PacketLogDataSendIdx);
        PacketLogDataArr[idx].temp = PacketLogDataArr[idx].sendData;
        PacketLogDataArr[idx].sendData = _data;
        if (PacketLogDataArr[idx].recvData != PacketLogDataArr[idx].sendData)
        {
            int a = 10;
            return false;
        }
        break;
    case EPacketLogDataType::REMOVE:
        idx = _InterlockedIncrement(&idx_PacketLogDataRemoveIdx);
        PacketLogDataArr[idx].removeData = _data;
        break;
    default:
        break;
    }

    return true;
}

#pragma endregion

#pragma region TestMemoryLog

unsigned long long test_AllocIdx = 0;
static const int MEMLOGSIZE = 100000;
struct MemoryLogData
{
    DWORD threadID;// ������ ���̵�
    int repeatCount; // �ݺ��ߴ� ����
    const char* funcName;//�Լ���
    long long sessionId; // sessionID 
    int sub;//�ش� �Լ��� ���°����
    unsigned long long writeidx;
    unsigned long long readidx;
    int enqueResult;
    unsigned long long ioCount;
};

unsigned long long memoryLogIdx = 0;
unsigned long long memoryLogRepeatCount = 0;
MemoryLogData* MemoryLogArr;
void MemoryLog(DWORD _threadID, const char* funcName, int _sub, long long sessionId, unsigned long long readidx, unsigned long long writeidx, int enqueResult, unsigned long long ioCount)
{
    unsigned long long idx = 0;
    idx = _InterlockedIncrement(&memoryLogIdx);
    if (idx >= 100000)
    {
        _InterlockedExchange(&memoryLogIdx, 0);
        _InterlockedIncrement(&memoryLogRepeatCount);
    }
    memset(&MemoryLogArr[idx], 0, sizeof(MemoryLogData));
    MemoryLogArr[idx].threadID = _threadID;
    MemoryLogArr[idx].funcName = funcName;
    MemoryLogArr[idx].repeatCount = (int)memoryLogRepeatCount;
    MemoryLogArr[idx].sub = _sub;
    MemoryLogArr[idx].sessionId = sessionId;
    MemoryLogArr[idx].writeidx = writeidx;
    MemoryLogArr[idx].readidx = readidx;
    MemoryLogArr[idx].enqueResult = enqueResult;
    MemoryLogArr[idx].ioCount = ioCount;
}
#pragma endregion




unsigned long long SendPacketCount = 0;
#endif




unsigned __stdcall AcceptThread(LPVOID arg)
{
    CLanServer* self = (CLanServer*)arg;
    if (self == nullptr)
    {
        return 0;
    }
    self->AcceptThreadFunc();
    return 0;
}

unsigned __stdcall WorkerThread(LPVOID arg)
{
    CLanServer* self = (CLanServer*)arg;
    if (self == nullptr)
    {
        return 0;
    }

    self->WorkerThreadFunc();
    InterlockedDecrement(&self->curWorkerThreadCnt);
    return 0;
}


CLanServer::CLanServer()
{
    maxConnectClient = 0;
    sessionArr = nullptr;
    runingWorkerThreadCnt = 0;
    waitWorkerThreadCnt = 0;
    isDoNagle = false;
    ip = nullptr;
    port = -1;
    hCCP = 0;
    bShutdown = false;
    pPacketPool = new CMemoryPoolTLS<SerializableBuffer>(10000, true);
#ifdef _TEST
    MemoryLogArr = new MemoryLogData[MEMLOGSIZE];
    PacketLogDataArr = new PacketLogData[MEMLOGSIZE];
    WSASendLogDataArr = new WSASendLogData[MEMLOGSIZE];
#endif // _TEST
}

CLanServer::~CLanServer()
{
    //���� �޸� ����
    if (sessionArr != nullptr)
    {
        delete[] sessionArr;
    }

}


bool CLanServer::Start(const char* _ip, int _port, int _runingWorkerThreadCnt, int _waitWorkerThreadCnt, bool _isDoNagle, unsigned long long _maxConnectClient)
{
    port = _port;
    runingWorkerThreadCnt = _runingWorkerThreadCnt;
    waitWorkerThreadCnt = _waitWorkerThreadCnt;
    isDoNagle = _isDoNagle;

    //���� ���� �Ҵ�
    maxConnectClient = _maxConnectClient;
    sessionArr = new CSession[maxConnectClient];
    if (sessionArr == nullptr)
    {
        return false;
    }
    //���� �ε��� �Ҵ�
    for (int i = 0; i < maxConnectClient; ++i)
    {
        CSession* session = sessionArr + i;
        session->index = i;
        sessionIdx.push(sessionArr + i);
    }

    //�ʱ�ȭ
    int result = Initialize();
    if (result == false)
    {
        return false;
    }

    //����� �Ϸ� ��Ʈ ����
    hCCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (hCCP == NULL) return false;

    //Create Accept Thread
    HANDLE hAcceptThread;
    hAcceptThread = (HANDLE)_beginthreadex(0, 0, AcceptThread, this, 0, nullptr);
    if (hAcceptThread == NULL)
    {
        return false;
    }
    CloseHandle(hAcceptThread);

    //Create Worker thread
    HANDLE hThread;
    for (int i = 0; i < runingWorkerThreadCnt; ++i)
    {
        hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, this, 0, NULL);
        if (hThread == NULL) return false;
        CloseHandle(hThread);
        ++curWorkerThreadCnt;
    }
    return true;
}

void CLanServer::Stop(bool _isShutdown)
{
    //�����ִ� ���� ���̵� �α׷� �����
    for (int i = 0; i < maxConnectClient; ++i)
    {
        CSession* session = sessionArr + i;
        if (session == nullptr)
        {
            break;
        }

        if (session->sessionID != -1)
        {
            _LOGW(ELogLevel::Level_DEBUG, L"���� ����: %d \n", session->sessionID);
        }
    }

    //��Ŀ�����忡�� ���� �޼��� ������
    //post...
    //��� ��Ŀ ������ ���� ���� Ȯ��.

    //accept ������ ���� ��Ű��
    bShutdown = true;
    closesocket(listen_sock); //accept ������ ���̱� ���ؼ�.
}

int CLanServer::GetClientCount()
{
    return 0;
}

bool CLanServer::Disconnect(unsigned long long _sessionID)
{
    return false;
}


bool CLanServer::SendPacket(unsigned long long _sessionID, SerializableBuffer* _paload)
{
    int curThreadID = GetCurrentThreadId();
    CSession* session = FindSession(_sessionID);
    if (session == nullptr)
    {
        return false;
    }
    if (session->sessionID == -1)
    {
        return false;
    }

    //��Ŷ�� ��� �ٿ��ֱ�
    SerializableBuffer* packet = pPacketPool->Alloc();
    packet->Init(sizeof(short) + _paload->GetUseSize());//�޸�Ǯ�� ���ø��̱� ������ �ʱ�ȭ�� �ڿ��� ȣ��������ϴٴ� ����ġ ���ߴ�.
    
    //SerializableBuffer* packet =new SerializableBuffer(sizeof(short)+ _paload->GetUseSize());
  
    *packet << (short)_paload->GetUseSize();
    *packet << *_paload;

    if (session->sendRing->GetFreeSize() <= packet->GetUseSize())
    {
        _LOGW(ELogLevel::Level_ERROR, L"SendPacket - send �� ���۰� ����á���ϴ�.\n");
        return false;
    }
    int enqueResult = packet->GetBufferSize();
#ifdef _TEST
    MemoryLog(GetCurrentThreadId(), "SendPacket", 1, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, enqueResult, session->ioCount);
#endif // _TEST
    //�̷��Ÿ� �ѹ��� ��ť ���� ���Ϸ� ��������.
    memcpy(session->sendRing->GetWritebufferPtr(), &packet, sizeof(SerializableBuffer*));
    session->sendRing->MoveWriteIdxEx(&session->sendRing->writeIdx, sizeof(SerializableBuffer*));

#ifdef _TEST
    _InterlockedIncrement(&SendPacketCount);
    MemoryLog(GetCurrentThreadId(), "SendPacket", 2, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, enqueResult, session->ioCount);
#endif // _TEST

    SendPost(session);

    return true;
}

bool CLanServer::InputSendPacket(unsigned long long _sessionID, SerializableBuffer* _packet)
{
    int curThreadID = GetCurrentThreadId();
    CSession* session = FindSession(_sessionID);
    if (session == nullptr)
    {
        return false;
    }
    if (session->sessionID == -1)
    {
        return false;
    }

    if (session->sendRing->GetFreeSize() <= _packet->GetUseSize())
    {
        _LOGW(ELogLevel::Level_ERROR, L"InputSendPacket - send �� ���۰� ����á���ϴ�.\n");
        return false;
    }
    int enqueResult = _packet->GetBufferSize();
#ifdef _TEST
    MemoryLog(GetCurrentThreadId(), "InputSendPacket", 1, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, enqueResult, session->ioCount);
#endif // _TEST
    memcpy(session->sendRing->GetWritebufferPtr(), _packet->GetBufferPtr(), _packet->GetBufferSize());
    session->sendRing->MoveWriteIdxEx(&session->sendRing->writeIdx, enqueResult);

    if (enqueResult != _packet->GetBufferSize())
    {
        _LOGW(ELogLevel::Level_ERROR, L"InputSendPacket - send �� ���� Enque ����.\n");
        return false;
    }
#ifdef _TEST
    _InterlockedIncrement(&SendPacketCount);
    MemoryLog(GetCurrentThreadId(), "InputSendPacket", 2, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, enqueResult, session->ioCount);
#endif // _TEST

    return true;
}


void CLanServer::WorkerThreadFunc()
{
    int curThreadID = GetCurrentThreadId();
    _LOGW(ELogLevel::Level_DEBUG, L"��Ŀ������ : %d \n", curThreadID);

    int retval;
    while (true)
    {
        //�񵿱� ����� �Ϸ� ��ٸ���
        DWORD cbTransferred = 0;
        CSession* ptr = NULL;
        OVERLAPPED* pOverlapped = NULL;
        retval = -1;
        retval = GetQueuedCompletionStatus(hCCP, &cbTransferred, (PULONG_PTR)&ptr, &pOverlapped, INFINITE);
        if (retval == 0 && cbTransferred == NULL && pOverlapped == NULL)
        {
            //���� ó��
            if (InterlockedDecrement(&ptr->ioCount) == 0)
            {
                Release(ptr->sessionID);

                InterlockedDecrement(&curSessionCount);
                InterlockedIncrement(&curDisconnectSessionCount);
            }
#ifdef _TEST
            MemoryLog(GetCurrentThreadId(), "workThread_Decrement", 1, ptr->sessionID, ptr->sendRing->readIdx, ptr->sendRing->writeIdx, cbTransferred, ptr->ioCount);
#endif // _TEST
            return;
        }
        //�񵿱� ����� ��� Ȯ��
        if (retval == 0)
        {
            int errorCode = WSAGetLastError();
            if (errorCode == ERROR_NETNAME_DELETED) //������ RST�� �����Ѱ��. �� HardClose�� �� ����� �� ���� ���� ���� ������ ��������� �ϴ� ����...
            {
                long long result = InterlockedDecrement(&ptr->ioCount);
#ifdef _TEST
                MemoryLog(GetCurrentThreadId(), "workThread_Decrement", 2, ptr->sessionID, ptr->sendRing->readIdx, ptr->sendRing->writeIdx, cbTransferred, ptr->ioCount);
#endif // _TEST
                if (result < 0)
                {
                    _LOGW(ELogLevel::Level_ERROR, L"sessionID : %d, WorkerThread IOCount is Error!!!%d \n", ptr->sessionID, result);
                }
                if (result == 0)
                {
#ifdef _TEST
                    if (&(ptr->recvOverlapped) == pOverlapped)
                    {
                        int a = 10;
                    }
                    else if (&(ptr->sendOverlapped) == pOverlapped)
                    {
                        int a = 10; // ��¥ send�ϴٰ� �߻��ұ�?
                    }
                    else
                    {
                        int a = 10;
                    }
#endif // _TEST
                    _LOGW(ELogLevel::Level_ERROR, L"thID : %d , sessionID : %d, GetQueueCompletionStatus - socket ���� ����\n", curThreadID, ptr->sessionID);
                    Release(ptr->sessionID);
                    InterlockedDecrement(&curSessionCount);
                    InterlockedIncrement(&curDisconnectSessionCount);
                }
            }
            else
            {
                _LOGW(ELogLevel::Level_DEBUG, L"sessionID : %d, WorkerThread retval == 0 ErrorCode : %d \n", ptr->sessionID, errorCode);
            }
        }
        if (cbTransferred == 0)
        {
            //_LOGW(ELogLevel::Level_DEBUG, L"sessionID : %d, WorkerThread retval == 0, transffer : %d  \n", ptr->sessionID, cbTransferred);
#ifdef _TEST
            MemoryLog(GetCurrentThreadId(), "transferZero", 1, ptr->sessionID, ptr->sendRing->readIdx, ptr->sendRing->writeIdx, cbTransferred, ptr->ioCount);
#endif // _TEST
            continue;
        }
        else
        {
            //�� ����� �������� ���ú����� Ȯ���ϱ�
            if (&(ptr->recvOverlapped) == pOverlapped) //recv �۾��� �����ٴ� �ǹ�//=============================================================================
            {
                _InterlockedDecrement(&ptr->isRecv);
#ifdef _TEST
                MemoryLog(GetCurrentThreadId(), "recv", 1, ptr->sessionID, ptr->sendRing->readIdx, ptr->sendRing->writeIdx, cbTransferred, ptr->ioCount);
#endif // _TEST
                InterlockedIncrement(&curRecvCount);
                //================================================
                //recv �� ��ġ ����
                //���� ��ŭ recv ��ġ �̵�, �ϴ� ������ ũ�⸦ �Ѿ� ���Ŷ�� ������ ����. wsabuf�� len �����Ҷ� �ݵ�� �Ѿ�� ���� ũ�⸸ ������ ���̱� ����.
                ptr->recvRing->MoveWriteIdxEx(&ptr->recvRing->writeIdx, cbTransferred);
#ifdef _TEST
                TestFunc(ptr->recvRing->GetReadBufferPtr(), cbTransferred);
#endif // _TEST
                //���� ������ ����ȭ ���ۿ� �ֱ�==================
                SerializableBuffer recvPacket(cbTransferred);
                //peek�� ���� �� �ִ��� Ȯ���ϱ�
                int reulstPeek = ptr->recvRing->Peek(recvPacket.GetBufferPtr(), cbTransferred);
                if (reulstPeek != cbTransferred)
                {
                    _LOGW(ELogLevel::Level_ERROR, L"recv - peek ����\n");
                }
                recvPacket.MoveWritePos(reulstPeek);
#ifdef _TEST
                TestFunc((char*)recvPacket.GetBufferPtr(), cbTransferred);
#endif // _TEST
                //�� ��Ŷ�� ����� �������� �۾�
                while(true)
                {
                    unsigned short packSize = 0;
                    int recvPackUseSize = recvPacket.GetUseSize();
                    if (recvPackUseSize < sizeof(short))
                    {
                        break;
                    }
                    recvPacket >> packSize;
                    recvPackUseSize = recvPacket.GetUseSize();
                    if (recvPackUseSize < packSize)
                    {
                        break;
                    }
                    SerializableBuffer::stCHAR paload(packSize);
                    recvPacket >> paload;
                    //=====
                    SerializableBuffer sendPacket(packSize );
                    sendPacket << paload;
#ifdef _TEST
                    int test = paload.data[0];
                    //PacketLog(GetCurrentThreadId(),EPacketLogDataType::RECV, test);
                    //MemoryLog(GetCurrentThreadId(), "ForRecv", 1111, ptr->sessionID, ptr->sendRing->readIdx, ptr->sendRing->writeIdx, test, ptr->ioCount);
#endif // _TEST
                    OnRecv(ptr->sessionID, &sendPacket);
                }
#ifdef _TEST
                TestFunc(ptr->recvRing->GetReadBufferPtr(), cbTransferred);
#endif // _TEST

                //OnRecv���� ���� ���� �������� �ϴ� �˾Ƽ� ó���ϰ� ���ΰ� ���Ѱ��� �ִٸ� �װ�
                //������ å���̴�. ������ ������ ����� �ƴϴ�.
                ptr->recvRing->MoveReadIdxEx(&ptr->recvRing->readIdx, reulstPeek);
                RecvPost(ptr);
            }
            else if (&(ptr->sendOverlapped) == pOverlapped) //send �۾��� �����ٴ� �ǹ�.======================================================
            {
#ifdef _TEST
                MemoryLog(GetCurrentThreadId(), "send", 1, ptr->sessionID, ptr->sendRing->readIdx, ptr->sendRing->writeIdx, cbTransferred, ptr->ioCount);
                if ((cbTransferred / 10) != ptr->curSendCount)
                {
                    int a = 10;
                }
#endif // _TEST
                InterlockedIncrement(&curSendCount);
                //send ��ġ �����̱� ================================================
                SerializableBuffer* packetArr[WSABUFFSIZE] = { 0 };
                //WSASendLog(GetCurrentThreadId(), EWSASendLogDataType::COMPLETSEND_1,ptr->sendRing->readIdx, ptr->curSendCount);
                ptr->sendRing->Deque((char*)packetArr, sizeof(SerializableBuffer*)* ptr->curSendCount);
                //WSASendLog(GetCurrentThreadId(), EWSASendLogDataType::COMPLETSEND_2, ptr->sendRing->readIdx, ptr->curSendCount);
                //memcpy(packetArr, ptr->sendRing->GetReadBufferPtr(), sizeof(SerializableBuffer*)* WSABUFFSIZE);
                //ptr->sendRing->MoveReadIdxEx(&ptr->sendRing->readIdx,ptr->curSendCount*sizeof(SerializableBuffer*));
                for (unsigned long ide=0;ide< ptr->curSendCount;++ide)
                {
                    SerializableBuffer* packet = nullptr;
                    packet = packetArr[ide];
#ifdef _TEST
                    //PacketLog(GetCurrentThreadId(), EPacketLogDataType::REMOVE, packet->GetBufferPtr()[2]);
#endif // _TEST
                    //delete packet;
                    pPacketPool->Free(&packet);
                }
                //ptr->sendRing->MoveReadIdxEx(&ptr->sendRing->readIdx, ptr->curSendCount*sizeof(CPacket*));
                //===================================================================
#ifdef _TEST
                MemoryLog(GetCurrentThreadId(), "send", 2, ptr->sessionID, ptr->sendRing->readIdx, ptr->sendRing->writeIdx, cbTransferred, ptr->ioCount);
#endif // _TEST
                OnSend(ptr->sessionID, cbTransferred);
                _InterlockedExchange(&ptr->isSend, false);
                if (ptr->sendRing->GetUseSize() > 0)
                {
                    SendPost(ptr);
                }
                else //�� ���¾�. recv�� �ؾ� �ұ�?
                {
                    if (ptr->isRecv <= 0)
                    {

                    }
                }
            }
            else
            {
                _LOGW(ELogLevel::Level_ERROR, L"thid  %d: sessionID : %d GQCS Overlapped ��ü�� �ٸ�!!!  \n", curThreadID, ptr->sessionID);
            }
        }

        long long result = InterlockedDecrement(&ptr->ioCount);
#ifdef _TEST
        MemoryLog(GetCurrentThreadId(), "workThread_Decrement", 3, ptr->sessionID, ptr->sendRing->readIdx, ptr->sendRing->writeIdx, cbTransferred, ptr->ioCount);
#endif // _TEST
        if (result < 0)
        {
            _LOGW(ELogLevel::Level_ERROR, L"sessionID : %d, WorkerThread IOCount is Error!!!%d \n", ptr->sessionID, result);
        }
        if (result == 0)
        {
            _LOGW(ELogLevel::Level_ERROR, L"thID : %d , sessionID : %d, GetQueueCompletionStatus - socket ���� ����\n", curThreadID, ptr->sessionID);
            Release(ptr->sessionID);
            InterlockedDecrement(&curSessionCount);
            InterlockedIncrement(&curDisconnectSessionCount);
        }
    }
    InterlockedDecrement(&curWorkerThreadCnt);
    return;
}

void CLanServer::AcceptThreadFunc()
{
    //���� ���� ����
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET)
    {
        _LOGW(ELogLevel::Level_ERROR, L"���� ���� ����\n");
        return;
    }
    //bind
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(port);
    int retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) _LOGW(ELogLevel::Level_ERROR, L"���ε� ����\n");

    //listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) _LOGW(ELogLevel::Level_ERROR, L"listen ����\n");

    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;

    while (true)
    {
        //accept
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen); //���⼭ ��� �ɸ� ����.
        if (client_sock == INVALID_SOCKET)
        {
            _LOGW(ELogLevel::Level_ERROR, L"accept ���� ����\n");
            break;
        }
        char clientIp[32] = { 0 };
        inet_ntop(AF_INET, &clientaddr.sin_addr, clientIp, sizeof(clientIp));
        int clientPort = ntohs(clientaddr.sin_port);

        //ȭ��Ʈ����Ʈ ��� ip���� Ȯ��
        bool retval = OnConnectionRequest(clientIp, clientPort);
        if (retval == false)
        {
            closesocket(client_sock);
            _LOGW(ELogLevel::Level_ERROR, L"Accept Error �߸��� Client ���� IP : %s, Port : %d\n", clientIp, clientPort);
            continue;
        }
        //�ִ� ������ �� ����
        if (maxConnectClient <= curSessionCount)
        {
            closesocket(client_sock);
            //_LOGW(ELogLevel::Level_ERROR, L"Accept Error Client �ִ� �� �ʰ� IP : %s, Port : %d curSessionCount : %d\n", clientIp, clientPort, curSessionCount);
            continue;
        }
        //Securing Session...
        CSession* session = sessionIdx.top();
        sessionIdx.pop();
        if (session == nullptr)
        {
            closesocket(client_sock);
            _LOGW(ELogLevel::Level_ERROR, L"Accept Error ���� Ȯ�� ���� IP : %s, Port : %d curSessionCount : %d\n", clientIp, clientPort, curSessionCount);
            continue;
        }
        //Setting Session...
        ZeroMemory(&session->recvOverlapped, sizeof(OVERLAPPED));
        ZeroMemory(&session->sendOverlapped, sizeof(OVERLAPPED));
        if (session->sendRing == nullptr)
        {
            session->sendRing = new RingBufferEx(ringBufferSize);
        }
        else
        {
            session->sendRing->ClearAll();
        }
        if (session->recvRing == nullptr)
        {
            session->recvRing = new RingBufferEx(ringBufferSize);
        }
        else
        {
            session->recvRing->ClearAll();
        }
        static unsigned long long sessionIDCount = 0;
        sessionIDCount++;
        session->sessionID = sessionIDCount;
        session->sock = client_sock;
        InterlockedIncrement(&curSessionCount);
#ifdef _TEST
        _LOGW(ELogLevel::Level_DEBUG, L"AcceptThread SessionID : %d\n", session->sessionID);
#endif // _TEST

        //���� ���� ó�� �Ϸ�! =======================
        BegineClientJoin(session->sessionID);
        //���ǿ��� ���� ��Ʈ��ũ ��� ����!===========
        //���ϰ� IOCP ����
        CreateIoCompletionPort((HANDLE)client_sock, hCCP, (ULONG_PTR)session, 0); // �����ϴ� ���� �� ������ �� ��� ������ ��.
        //�񵿱� ����� ���۽� �̸� �ɾ�� IOcount�� 0�� �Ǵ� ���� ���� �� ����.
        RecvPost(session);
        //���� �� ������ ó�� ����
        OnClientJoin(session->sessionID);
        //============================================
    }
}



void CLanServer::SendPost(CSession* session)
{
    unsigned long retIsSend = _InterlockedExchange(&session->isSend, true);
    if (retIsSend == (unsigned long)true)
    {
        return;
    }
    
    int curThreadID = GetCurrentThreadId();
    DWORD sendbytes = 0;
    InterlockedIncrement(&session->ioCount);
    //������ ������
    ZeroMemory(&session->sendOverlapped, sizeof(session->sendOverlapped));
    WSABUF wsabuff[WSABUFFSIZE];//���� 2 �����µ�, 100�� �ø��� ���� ���� �� 0���� ó���� ����.
    memset(&wsabuff[0], 0, sizeof(WSABUF) * WSABUFFSIZE);
#ifdef _TEST
    MemoryLog(GetCurrentThreadId(), "SendPost", 1, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, wsabuff[0].len + wsabuff[1].len, session->ioCount);
#endif // _TEST
    int requireSendSize = 0;
    //wsaBuffSize��ŭ ����� ������
    SerializableBuffer* pPacketArray[WSABUFFSIZE] = { 0 };
    //peek ���� ������ ���ϱ�
    int peekAbleCount = (session->sendRing->GetUseSize() / sizeof(SerializableBuffer*));
    peekAbleCount = (peekAbleCount > WSABUFFSIZE) ? (WSABUFFSIZE) : (peekAbleCount);
    session->sendRing->Peek((unsigned char*)pPacketArray,sizeof(SerializableBuffer*)* peekAbleCount);
    session->curSendCount = peekAbleCount;
    //WSASendLog(GetCurrentThreadId(), EWSASendLogDataType::REQSEND, session->sendRing->readIdx, peekAbleCount);
    int testSize = 0;
    for (int idxj=0; idxj < WSABUFFSIZE;++idxj)
    {
        if (idxj >= peekAbleCount)
        {
            break;
        }
        if ( pPacketArray[idxj]==nullptr)
        {
            break;
        }
       
        wsabuff[idxj].buf = (char*)pPacketArray[idxj]->GetBufferPtr();
        if (wsabuff[idxj].buf==nullptr)
        {
            break;
        }
       
        wsabuff[idxj].len = pPacketArray[idxj]->GetUseSize();
        if (wsabuff[idxj].len==0)
        {
            break;
        }
        testSize += wsabuff[idxj].len;
        requireSendSize += wsabuff[idxj].len;
#ifdef _TEST
       
        int test = (unsigned char)wsabuff[idxj].buf[2];
        //PacketLog(GetCurrentThreadId(), EPacketLogDataType::SEND, test);
        //MemoryLog(GetCurrentThreadId(), "ForSend", 1111, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, test, session->ioCount);
#endif // _TEST
    }
    if (requireSendSize == 0)
    {
        _InterlockedExchange(&session->isSend, false);//�ᱹ ���� ������ ������ ���� ����.
        long long result = InterlockedDecrement(&session->ioCount);
#ifdef _TEST
        MemoryLog(GetCurrentThreadId(), "SendPost_Decrement", 1, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, wsabuff[0].len + wsabuff[1].len, session->ioCount);
        
#endif // _TEST
        _LOGW(ELogLevel::Level_DEBUG, L"thid  %d: send Decrement result : %d \n", curThreadID, result);
        if (result < 0)
        {
            _LOGW(ELogLevel::Level_ERROR, L"sessionID : %d, WSASend IOCount is Error!!! %d \n", session->sessionID, result);
        }
        if (result == 0)
        {
            Release(session->sessionID);
            InterlockedDecrement(&curSessionCount);
            InterlockedIncrement(&curDisconnectSessionCount);
        }
        return;
    }
    int retval = WSASend(session->sock, wsabuff, WSABUFFSIZE, &sendbytes, 0, &session->sendOverlapped, NULL);
#ifdef _TEST
    MemoryLog(GetCurrentThreadId(), "SendPost", 3, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, wsabuff[0].len + wsabuff[1].len, session->ioCount);
#endif // _TEST
    if (retval == SOCKET_ERROR)
    {
        int errorCode = WSAGetLastError();
        if (errorCode != WSA_IO_PENDING && errorCode != 0)
        {
            _LOGW(ELogLevel::Level_ERROR, L"sessionID : %d, WSASend Error Code : %d , IOCount %d \n", session->sessionID, errorCode, session->ioCount, session->ioCount);
            if (errorCode == WSAECONNRESET)
            {
                _LOGW(ELogLevel::Level_DEBUG, L"WSASend Error ȣ��Ʈ�Ǵ� ������ ��ǻ�Ͱ� ������ ���� ���� ���׽��ϴ� \n");
            }
            else if (errorCode == WSAENOTSOCK)
            {
                _LOGW(ELogLevel::Level_DEBUG, L"������ ������ ���������� �ʽ��ϴ�. ������ ����ó�� ���� ������ �ݾҽ��ϴ�.\n");
            }
            else
            {
                _LOGW(ELogLevel::Level_ERROR, L"WSASend Error - ErrorCode : %d \n", errorCode);
            }
            long long result = InterlockedDecrement(&session->ioCount);
#ifdef _TEST
            MemoryLog(GetCurrentThreadId(), "SendPost_Decrement", 2, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, wsabuff[0].len + wsabuff[1].len, session->ioCount);
#endif // _TEST
            _LOGW(ELogLevel::Level_DEBUG, L"thid  %d: send Decrement result : %d \n", curThreadID, result);
            if (result < 0)
            {
                _LOGW(ELogLevel::Level_ERROR, L"sessionID : %d, WSASend IOCount is Error!!! %d \n", session->sessionID, result);
            }
            if (result == 0)
            {
                Release(session->sessionID);
                InterlockedDecrement(&curSessionCount);
                InterlockedIncrement(&curDisconnectSessionCount);
            }
        }
    }
}

void CLanServer::RecvPost(CSession* _session)
{
    int curThreadID = GetCurrentThreadId();
    DWORD recvbytes;
    DWORD flag = 0;
    InterlockedIncrement(&_session->ioCount);

    ZeroMemory(&_session->recvOverlapped, sizeof(_session->recvOverlapped));

    WSABUF wsabuff[2];
    memset(&wsabuff[0], 0, sizeof(WSABUF) * 2);

    wsabuff[0].buf = _session->recvRing->GetWritebufferPtr();
    wsabuff[0].len = _session->recvRing->GetDirectEnqueSize((int)_session->recvRing->readIdx, (int)_session->recvRing->writeIdx);
    wsabuff[1].buf = _session->recvRing->buffer;
    wsabuff[1].len = (int)_session->recvRing->readIdx;
    _InterlockedIncrement(&_session->isRecv);

#ifdef _TEST
    if ((wsabuff[0].len + wsabuff[1].len) <= 0)
    {
        int a = 10;
    }
#endif // _TEST
    int retval = WSARecv(_session->sock, wsabuff, 2, &recvbytes, &flag, &_session->recvOverlapped, NULL); // �ϴ� ��Ŀ�� �ֱ� ���ؼ�...
#ifdef _TEST
    MemoryLog(GetCurrentThreadId(), "RecvPost", 1, _session->sessionID, _session->sendRing->readIdx, _session->sendRing->writeIdx, wsabuff[0].len + wsabuff[1].len, _session->ioCount);
#endif // _TEST
    if (retval == SOCKET_ERROR)
    {
        int errorCode = WSAGetLastError();
        if (errorCode != ERROR_IO_PENDING && errorCode != 0)
        {
            //_LOGW(ELogLevel::Level_ERROR, L"sessionID : %d, WSARecv Error Code : %d \n", _session->sessionId, errorCode);

            if (errorCode == WSAECONNRESET)
            {
                //_LOGW(ELogLevel::Level_DEBUG, L"WSARecv Error ȣ��Ʈ�Ǵ� ������ ��ǻ�Ͱ� ������ ���� ���� ���׽��ϴ� \n");
            }
            else if (errorCode == WSAENOTSOCK)
            {
                //_LOGW(ELogLevel::Level_DEBUG, L"������ ������ ���������� �ʽ��ϴ�. ������ ����ó�� ���� ������ �ݾҽ��ϴ�.\n");
            }
            else if (errorCode == WSAECONNABORTED)
            {
                _LOGW(ELogLevel::Level_DEBUG, L"�ð��ʰ� �Ǵ� ��Ÿ ������ ���� �߻��� ����. \n");
            }
            else
            {
                _LOGW(ELogLevel::Level_DEBUG, L"WSARecv Error - ErrorCode : %d \n", errorCode);
            }

            long long result = InterlockedDecrement(&_session->ioCount);
#ifdef _TEST
            MemoryLog(GetCurrentThreadId(), "RecvPost_Decrement", 1, _session->sessionID, _session->sendRing->readIdx, _session->sendRing->writeIdx, wsabuff[0].len + wsabuff[1].len, _session->ioCount);
#endif // _TEST
            //_LOGW(ELogLevel::Level_DEBUG, L"thid  %d: recv Decrement result : %d \n", curThreadID, result);
            if (result < 0)
            {
                _LOGW(ELogLevel::Level_ERROR, L"sessionID : %d, WSARecv IOCount is Error!!! %d \n", _session->sessionID, result);
            }
            else if (result == 0)
            {
                Release(_session->sessionID);
                InterlockedDecrement(&curSessionCount);
                InterlockedIncrement(&curDisconnectSessionCount);
            }
        }
    }
}

void CLanServer::Release(unsigned long long _sessionId)
{

    EnterCriticalSection(&cs_sessionArr);
    CSession* session = FindSession(_sessionId);
#ifdef _TEST
    MemoryLog(GetCurrentThreadId(), "Release", 1, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx,0, session->ioCount);
#endif // _TEST
    if (session != nullptr)
    {
        EnterCriticalSection(&session->cs_session);
        LeaveCriticalSection(&session->cs_session);

        session->sessionID = -1;
        session->ioCount = 0;

        closesocket(session->sock);

        sessionIdx.push(session);
    }
    LeaveCriticalSection(&cs_sessionArr);
}
bool CLanServer::Initialize()
{
    //�����ڵ� ����� ���� ��������
    _wsetlocale(LC_ALL, L"korean");
    //Ÿ�̸� �ػ� ���̱�
    timeBeginPeriod(1);
    //�α� ����
    LogManager::GetInstance().SetLogLevel(ELogLevel::Level_DEBUG);
    LogManager::GetInstance().SetSaveFile(true);
    LogManager::GetInstance().SetlogFileIntervalSec(600);
    //���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;
    //�� �ʱ�ȭ
    InitializeCriticalSection(&cs_sessionArr);

    return true;
}

CSession* CLanServer::FindSession(unsigned long long _sessionID)
{
    for (int i = 0; i < maxConnectClient; ++i)
    {
        CSession* session = sessionArr + i;
        if (session == nullptr)
        {
            break;
        }

        if (session->sessionID == _sessionID)
        {
            return session;
        }
    }


    return nullptr;
}
//
//void CLanServer::SetSessionLogin(bool isLogin)
//{
//    
//}
//
//bool CLanServer::GetSessionLogin(unsigned long long _sessionID)
//{
//    CSession* session = FindSession(_sessionID);
//    if (session==nullptr)
//    {
//        return false;
//    }
//    
//    bool result =session->isLogin;
//
//    return result;
//}

void CLanServer::TestFunc(char* packetPtr, DWORD _size)
{
    unsigned long long idx = 0;
    while (idx < _size)
    {
        unsigned short payloadsize = 40;
        char test = *packetPtr;
        //memcpy(&payloadsize, packetPtr+ idx, 2);
        //_mm_mfence();//�� �Ѱ���?
        memcpy(&payloadsize, packetPtr + idx, 2);// �� 2�� �Ѱ���?
        if (payloadsize != 8)
        {
            int a = 10;
        }
        idx += 2;

        long long payload = -1;
        memcpy(&payload, packetPtr + idx, 8);
        if (payload == -1)
        {
            int a = 10;
        }
        idx += 8;
    }
}
