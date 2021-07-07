
#include "stdafx.h"

#include "SerializationBuffer.h"
#include "LogManager.h"
#include "RingBufferEx.h"
#include "CSession.h"

#include "CNetServer.h"


namespace CNetServerSpace
{
    unsigned __stdcall AcceptThread(LPVOID arg)
    {
        CNetServer* self = (CNetServer*)arg;
        if (self == nullptr)
        {
            return 0;
        }
        self->AcceptThreadFunc();
        return 0;
    }

    unsigned __stdcall WorkerThread(LPVOID arg)
    {
        CNetServer* self = (CNetServer*)arg;
        if (self == nullptr)
        {
            return 0;
        }

        self->WorkerThreadFunc();
        InterlockedDecrement(&self->curWorkerThreadCnt);
        return 0;
    }
#ifdef _TEST
    unsigned long long test_AllocIdx = 0;
    static const int MEMLOGSIZE = 100000;
    struct MemoryLogData_T
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
    MemoryLogData_T* MemoryLogArr;
    void MemoryLog(DWORD _threadID, const char* funcName, int _sub, long long sessionId, unsigned long long readidx, unsigned long long writeidx, int enqueResult, unsigned long long ioCount)
    {
        unsigned long long idx = 0;
        idx = _InterlockedIncrement(&memoryLogIdx);
        if (idx >= 100000)
        {
            _InterlockedExchange(&memoryLogIdx, 0);
            _InterlockedIncrement(&memoryLogRepeatCount);
        }
        memset(&MemoryLogArr[idx], 0, sizeof(MemoryLogData_T));
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
    unsigned long long SendPacketCount = 0;
#endif
}

using namespace CNetServerSpace;


CNetServer::CNetServer()
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
#ifdef _TEST
    MemoryLogArr = new MemoryLogData_T[MEMLOGSIZE];
#endif // _TEST
}

CNetServer::~CNetServer()
{
    //���� �޸� ����
    if (sessionArr != nullptr)
    {
        delete[] sessionArr;
    }

}


bool CNetServer::Start(const char* _ip, int _port, int _runingWorkerThreadCnt, int _waitWorkerThreadCnt, bool _isDoNagle, unsigned long long _maxConnectClient)
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
    hAcceptThread = (HANDLE)_beginthreadex(0, 0, CNetServerSpace::AcceptThread, this, 0, nullptr);
    if (hAcceptThread == NULL)
    {
        return false;
    }
    CloseHandle(hAcceptThread);

    //Create Worker thread
    HANDLE hThread;
    for (int i = 0; i < runingWorkerThreadCnt; ++i)
    {
        hThread = (HANDLE)_beginthreadex(NULL, 0, CNetServerSpace::WorkerThread, this, 0, NULL);
        if (hThread == NULL) return false;
        CloseHandle(hThread);
        ++curWorkerThreadCnt;
    }
    return true;
}

void CNetServer::Stop(bool _isShutdown)
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

int CNetServer::GetClientCount()
{
    return 0;
}

bool CNetServer::Disconnect(unsigned long long _sessionID)
{
    return false;
}


bool CNetServer::SendPacket(unsigned long long _sessionID, SerializableBuffer* _packet)
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
        _LOGW(ELogLevel::Level_ERROR, L"SendPacket - send �� ���۰� ����á���ϴ�.\n");
        return false;
    }
    int enqueResult = _packet->GetBufferSize();
#ifdef _TEST
    MemoryLog(GetCurrentThreadId(), "SendPacket", 1, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, enqueResult, session->ioCount);
#endif // _TEST
    memcpy(session->sendRing->GetWritebufferPtr(), _packet->GetBufferPtr(), _packet->GetBufferSize());
    session->sendRing->MoveWriteIdxEx(&session->sendRing->writeIdx, enqueResult);



    if (enqueResult != _packet->GetBufferSize())
    {
        _LOGW(ELogLevel::Level_ERROR, L"SendPacket - send �� ���� Enque ����.\n");
        return false;
    }
#ifdef _TEST
    _InterlockedIncrement(&SendPacketCount);
    MemoryLog(GetCurrentThreadId(), "SendPacket", 2, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, enqueResult, session->ioCount);
#endif // _TEST

    SendPost(session);

    return true;
}


void CNetServer::WorkerThreadFunc()
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
                        int a = 10;
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
                OnRecv(ptr->sessionID, &recvPacket);
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
#endif // _TEST
                InterlockedIncrement(&curSendCount);
                //send ��ġ �����̱� ================================================
                ptr->sendRing->MoveReadIdxEx(&ptr->sendRing->readIdx, cbTransferred);
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

void CNetServer::AcceptThreadFunc()
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



void CNetServer::SendPost(CSession* session)
{
    if (_InterlockedExchange(&session->isSend, true) == (LONG)true)
    {
        return;
    }

    int curThreadID = GetCurrentThreadId();
    DWORD sendbytes = 0;
    InterlockedIncrement(&session->ioCount);
    //������ ������
    ZeroMemory(&session->sendOverlapped, sizeof(session->sendOverlapped));

    WSABUF wsabuff[2];

#ifdef _TEST
    MemoryLog(GetCurrentThreadId(), "SendPost", 1, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, wsabuff[0].len + wsabuff[1].len, session->ioCount);
#endif // _TEST

    memset(&wsabuff[0], 0, sizeof(WSABUF) * 2);
    wsabuff[0].buf = session->sendRing->GetReadBufferPtr();
    int len = 0;
    if (session->sendRing->GetDirectDequeSize((int)session->sendRing->readIdx, (int)session->sendRing->writeIdx) >= session->sendRing->GetUseSize())
    {
        len = session->sendRing->GetUseSize();
    }
    else
    {
        len = session->sendRing->GetDirectDequeSize((int)session->sendRing->readIdx, (int)session->sendRing->writeIdx);
    }
    wsabuff[0].len = len;
    wsabuff[1].buf = session->sendRing->buffer;
    int remainSize = session->sendRing->GetUseSize() - session->sendRing->GetDirectDequeSize((int)session->sendRing->readIdx, (int)session->sendRing->writeIdx);
    wsabuff[1].len = (remainSize) > 0 ? remainSize : 0;

    if ((wsabuff[0].len + wsabuff[1].len) == 0)
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
    int retval = WSASend(session->sock, wsabuff, 2, &sendbytes, 0, &session->sendOverlapped, NULL);
#ifdef _TEST
    MemoryLog(GetCurrentThreadId(), "SendPost", 2, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, wsabuff[0].len + wsabuff[1].len, session->ioCount);
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

void CNetServer::RecvPost(CSession* _session)
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

void CNetServer::Release(unsigned long long _sessionId)
{

    EnterCriticalSection(&cs_sessionArr);
    CSession* session = FindSession(_sessionId);
#ifdef _TEST
    MemoryLog(GetCurrentThreadId(), "Release", 1, session->sessionID, session->sendRing->readIdx, session->sendRing->writeIdx, 0, session->ioCount);
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
bool CNetServer::Initialize()
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

CSession* CNetServer::FindSession(unsigned long long _sessionID)
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

void CNetServer::TestFunc(char* packetPtr, DWORD _size)
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
