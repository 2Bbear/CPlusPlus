
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
        DWORD threadID;// 스레드 아이디
        int repeatCount; // 반복했던 숫자
        const char* funcName;//함수명
        long long sessionId; // sessionID 
        int sub;//해당 함수의 몇번째인지
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
    //세션 메모리 정리
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

    //세션 공간 할당
    maxConnectClient = _maxConnectClient;
    sessionArr = new CSession[maxConnectClient];
    if (sessionArr == nullptr)
    {
        return false;
    }
    //세션 인덱스 할당
    for (int i = 0; i < maxConnectClient; ++i)
    {
        CSession* session = sessionArr + i;
        session->index = i;
        sessionIdx.push(sessionArr + i);
    }

    //초기화
    int result = Initialize();
    if (result == false)
    {
        return false;
    }

    //입출력 완료 포트 생성
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
    //남아있는 세션 아이디 로그로 남기기
    for (int i = 0; i < maxConnectClient; ++i)
    {
        CSession* session = sessionArr + i;
        if (session == nullptr)
        {
            break;
        }

        if (session->sessionID != -1)
        {
            _LOGW(ELogLevel::Level_DEBUG, L"남은 세션: %d \n", session->sessionID);
        }
    }

    //워커스레드에게 종료 메세지 보내기
    //post...
    //모든 워커 스레드 종료 상태 확인.

    //accept 스레드 종료 시키기
    bShutdown = true;
    closesocket(listen_sock); //accept 스레드 죽이기 위해서.
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
        _LOGW(ELogLevel::Level_ERROR, L"SendPacket - send 링 버퍼가 가득찼습니다.\n");
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
        _LOGW(ELogLevel::Level_ERROR, L"SendPacket - send 링 버퍼 Enque 에러.\n");
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
    _LOGW(ELogLevel::Level_DEBUG, L"워커스레드 : %d \n", curThreadID);

    int retval;
    while (true)
    {
        //비동기 입출력 완료 기다리기
        DWORD cbTransferred = 0;
        CSession* ptr = NULL;
        OVERLAPPED* pOverlapped = NULL;
        retval = -1;
        retval = GetQueuedCompletionStatus(hCCP, &cbTransferred, (PULONG_PTR)&ptr, &pOverlapped, INFINITE);
        if (retval == 0 && cbTransferred == NULL && pOverlapped == NULL)
        {
            //종료 처리
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
        //비동기 입출력 결과 확인
        if (retval == 0)
        {
            int errorCode = WSAGetLastError();
            if (errorCode == ERROR_NETNAME_DELETED) //상대방이 RST를 전송한경우. 즉 HardClose를 한 경우임 이 경우는 볼것 없이 강제로 끊어버려야 하는 것이...
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

                    _LOGW(ELogLevel::Level_ERROR, L"thID : %d , sessionID : %d, GetQueueCompletionStatus - socket 연결 끊기\n", curThreadID, ptr->sessionID);
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
            //이 결과가 센드인지 리시브인지 확인하기
            if (&(ptr->recvOverlapped) == pOverlapped) //recv 작업이 끝났다는 의미//=============================================================================
            {
                _InterlockedDecrement(&ptr->isRecv);
#ifdef _TEST
                MemoryLog(GetCurrentThreadId(), "recv", 1, ptr->sessionID, ptr->sendRing->readIdx, ptr->sendRing->writeIdx, cbTransferred, ptr->ioCount);
#endif // _TEST
                InterlockedIncrement(&curRecvCount);
                //================================================
                //recv 링 위치 수정
                //받은 만큼 recv 위치 이동, 일단 여분의 크기를 넘어 설거라는 생각은 안함. wsabuf에 len 설정할때 반드시 넘어서지 않을 크기만 전달할 것이기 때문.
                ptr->recvRing->MoveWriteIdxEx(&ptr->recvRing->writeIdx, cbTransferred);
#ifdef _TEST
                TestFunc(ptr->recvRing->GetReadBufferPtr(), cbTransferred);
#endif // _TEST
                //받은 데이터 직렬화 버퍼에 넣기==================
                SerializableBuffer recvPacket(cbTransferred);

                //peek로 넣을 수 있는지 확인하기
                int reulstPeek = ptr->recvRing->Peek(recvPacket.GetBufferPtr(), cbTransferred);
                if (reulstPeek != cbTransferred)
                {
                    _LOGW(ELogLevel::Level_ERROR, L"recv - peek 에러\n");
                }
                recvPacket.MoveWritePos(reulstPeek);
#ifdef _TEST
                TestFunc((char*)recvPacket.GetBufferPtr(), cbTransferred);
#endif // _TEST
                OnRecv(ptr->sessionID, &recvPacket);
#ifdef _TEST
                TestFunc(ptr->recvRing->GetReadBufferPtr(), cbTransferred);
#endif // _TEST
                //OnRecv에서 무슨 짓이 벌어지든 일단 알아서 처리하게 냅두고 못한것이 있다면 그건
                //컨텐츠 책임이다. 서버가 갑섭할 대상이 아니다.
                ptr->recvRing->MoveReadIdxEx(&ptr->recvRing->readIdx, reulstPeek);
                RecvPost(ptr);
            }
            else if (&(ptr->sendOverlapped) == pOverlapped) //send 작업이 끝났다는 의미.======================================================
            {
#ifdef _TEST
                MemoryLog(GetCurrentThreadId(), "send", 1, ptr->sessionID, ptr->sendRing->readIdx, ptr->sendRing->writeIdx, cbTransferred, ptr->ioCount);
#endif // _TEST
                InterlockedIncrement(&curSendCount);
                //send 위치 움직이기 ================================================
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
                else //다 보냈어. recv를 해야 할까?
                {
                    if (ptr->isRecv <= 0)
                    {
                    }
                }

            }
            else
            {
                _LOGW(ELogLevel::Level_ERROR, L"thid  %d: sessionID : %d GQCS Overlapped 개체가 다름!!!  \n", curThreadID, ptr->sessionID);
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
            _LOGW(ELogLevel::Level_ERROR, L"thID : %d , sessionID : %d, GetQueueCompletionStatus - socket 연결 끊기\n", curThreadID, ptr->sessionID);
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
    //리슨 소켓 생성
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET)
    {
        _LOGW(ELogLevel::Level_ERROR, L"소켓 생성 에러\n");
        return;
    }
    //bind
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(port);
    int retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) _LOGW(ELogLevel::Level_ERROR, L"바인드 에러\n");

    //listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) _LOGW(ELogLevel::Level_ERROR, L"listen 에러\n");

    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;

    while (true)
    {
        //accept
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen); //여기서 블락 걸릴 것임.
        if (client_sock == INVALID_SOCKET)
        {
            _LOGW(ELogLevel::Level_ERROR, L"accept 에러 종료\n");
            break;
        }
        char clientIp[32] = { 0 };
        inet_ntop(AF_INET, &clientaddr.sin_addr, clientIp, sizeof(clientIp));
        int clientPort = ntohs(clientaddr.sin_port);

        //화이트리스트 대상 ip인지 확인
        bool retval = OnConnectionRequest(clientIp, clientPort);
        if (retval == false)
        {
            closesocket(client_sock);
            _LOGW(ELogLevel::Level_ERROR, L"Accept Error 잘못된 Client 접근 IP : %s, Port : %d\n", clientIp, clientPort);
            continue;
        }

        //최대 접속자 수 제한
        if (maxConnectClient <= curSessionCount)
        {
            closesocket(client_sock);
            //_LOGW(ELogLevel::Level_ERROR, L"Accept Error Client 최대 수 초과 IP : %s, Port : %d curSessionCount : %d\n", clientIp, clientPort, curSessionCount);
            continue;
        }

        //Securing Session...
        CSession* session = sessionIdx.top();
        sessionIdx.pop();

        if (session == nullptr)
        {
            closesocket(client_sock);
            _LOGW(ELogLevel::Level_ERROR, L"Accept Error 세션 확보 실패 IP : %s, Port : %d curSessionCount : %d\n", clientIp, clientPort, curSessionCount);
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

        //세션 접속 처리 완료! =======================
        BegineClientJoin(session->sessionID);

        //세션에서 부터 네트워크 통신 시작!===========
        //소켓과 IOCP 연결
        CreateIoCompletionPort((HANDLE)client_sock, hCCP, (ULONG_PTR)session, 0); // 연결하는 순간 이 소켓은 넌 블락 소켓이 됨.

        //비동기 입출력 시작시 미리 걸어야 IOcount가 0이 되는 것을 막을 수 있음.
        RecvPost(session);

        //접속 후 컨텐츠 처리 진행
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
    //데이터 보내기
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
        _InterlockedExchange(&session->isSend, false);//결국 실패 했으니 강제로 돌려 놓음.
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
                _LOGW(ELogLevel::Level_DEBUG, L"WSASend Error 호스트또는 원격지 컴퓨터가 접속을 강제 종료 시켰습니다 \n");
            }
            else if (errorCode == WSAENOTSOCK)
            {
                _LOGW(ELogLevel::Level_DEBUG, L"지정한 소켓은 정상적이지 않습니다. 별도의 종료처리 없이 소켓을 닫았습니다.\n");
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
    int retval = WSARecv(_session->sock, wsabuff, 2, &recvbytes, &flag, &_session->recvOverlapped, NULL); // 일단 워커에 넣기 위해서...
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
                //_LOGW(ELogLevel::Level_DEBUG, L"WSARecv Error 호스트또는 원격지 컴퓨터가 접속을 강제 종료 시켰습니다 \n");
            }
            else if (errorCode == WSAENOTSOCK)
            {
                //_LOGW(ELogLevel::Level_DEBUG, L"지정한 소켓은 정상적이지 않습니다. 별도의 종료처리 없이 소켓을 닫았습니다.\n");
            }
            else if (errorCode == WSAECONNABORTED)
            {
                _LOGW(ELogLevel::Level_DEBUG, L"시간초과 또는 기타 오류로 인해 발생한 오류. \n");
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
    //유니코드 출력을 위한 지역설정
    _wsetlocale(LC_ALL, L"korean");
    //타이머 해상도 높이기
    timeBeginPeriod(1);
    //로그 설정
    LogManager::GetInstance().SetLogLevel(ELogLevel::Level_DEBUG);
    LogManager::GetInstance().SetSaveFile(true);
    LogManager::GetInstance().SetlogFileIntervalSec(600);
    //윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;
    //락 초기화
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
        //_mm_mfence();//왜 한거지?
        memcpy(&payloadsize, packetPtr + idx, 2);// 왜 2번 한거지?
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
