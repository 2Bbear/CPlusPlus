#pragma once

#include "stdafx.h"
using namespace std;

//전방선언
class CSession;
class SerializableBuffer;
template<typename T> class CMemoryPoolTLS;
/// @brief 서버역할
class CLanServer
{
public:
	enum class ECLanServerErrorCode : int
	{
		Default = 0,
	};

private:
	CMemoryPoolTLS<SerializableBuffer> * pPacketPool;
	unsigned long long maxConnectClient; // 최대 연결 가능한 클라이언트 수
	CSession * sessionArr; // 접속한 세션을 담을 세션 배열
	stack<CSession*>  sessionIdx;
	int runingWorkerThreadCnt; // 동작 가능한 워커 스레드 수
	int waitWorkerThreadCnt; //대기 가능한 워커 스레드 수
	bool isDoNagle; //네이글 옵션에 대한 여부
	const char* ip;
	int port;
	HANDLE hCCP;// 완료 루틴에 대한 핸들
	SOCKET listen_sock;
	int ringBufferSize = 20000;
public:
	unsigned long long curWorkerThreadCnt;//현재 워커스레드 수를 나타냄
	unsigned long long curSessionCount; // 현재 연결된 세션 수
	unsigned long long curDisconnectSessionCount = 0;//연결을 끊은 세션의 수
	unsigned long long curRecvCount = 0;
	unsigned long long curSendCount = 0;
private:
	bool bShutdown; //이 변수가 true 일때 모든 스레드는 종료 절차로 진행된다.
	CRITICAL_SECTION cs_sessionArr; //session 배열을 위한 동기화 객체
public:		
	CLanServer();
	~CLanServer();
//==============================외부 노출 함수========================================
public:
	/// @brief 서버 시작용 함수 , 현재 재시작에 대한 고려를 하지 않음.
	/// @param _ip 
	/// @param _port 
	/// @param _runingWorkerThreadCnt 
	/// @param _waitWorkerThreadCnt 
	/// @param _isDoNagle 
	/// @param _maxConnectClient 
	/// @return 
	bool Start(const char* _ip, int _port, int _runingWorkerThreadCnt, int _waitWorkerThreadCnt, bool _isDoNagle, unsigned long long _maxConnectClient);
	/// @brief 서버 종료용 함수
	/// 클라이언트 정리, DB 에 로그 저장등을 수행하게 된다.
	/// @param _isShutdown 
	void Stop(bool _isShutdown);
	/// @brief 현재 접속한 클라이언트의 수를 반환한다.
	/// @return 
	int GetClientCount();
public:
	/// @brief 해당 세션에 대해 연결 끊기 요청s
	/// @param SessionID 끊을 대상의 세션 아이디
	/// @return 성공 여부
	bool Disconnect(unsigned long long _sessionID);
	/// @brief 해당 세션에 패킷을 전송 해달라고 요청
	/// 해당 세션의 SendQ에 패킷을 넣는 작업을 하는 것임
	/// @param SessionID 
	/// @return 
	bool SendPacket(unsigned long long _sessionID, SerializableBuffer* _paload);
	bool InputSendPacket(unsigned long long _sessionID, SerializableBuffer* _packet);

//==================================하위 재정의 대상 함수===================================================
public:
	/// @brief IOCP 네트워크 연결하기 전 컨텐츠 처리를 위한 함수
	/// @param _sessionID 
	virtual void BegineClientJoin(unsigned long long _sessionID) = 0;
	/// @brief IOCP 네트워크 연결 후 컨텐츠 처리를 위한 함수
	/// @param _sessionID 
	virtual void OnClientJoin(unsigned long long _sessionID) = 0;
	/// @brief Release 후 호출되는 함수
	/// @param _sessionID 
	virtual void OnClientLeave(unsigned long long _sessionID) = 0;
	/// @brief accept 직후 불리는 함수, 운영시 서비스 테스틑 위해서 테스트를 제외한 유저들을 막기 위한 함수다.
	/// @param _ip 
	/// @param _port 
	/// @return false 시 클라이언트 거부한 것, true 시 접속 허용
	virtual bool OnConnectionRequest(const char* _ip, int _port) = 0;
	/// @brief 패킷 수신 완료후 불릴 함수
	/// @param _sessionID 
	/// @param _packet 
	virtual void OnRecv(unsigned long long _sessionID, SerializableBuffer* _packet) = 0;

	virtual void OnSend(unsigned long long _sessionID, int sendsize) = 0; //          < 패킷 송신 완료 후
	//	virtual void OnWorkerThreadBegin() = 0;                    < 워커스레드 GQCS 바로 하단에서 호출
	//	virtual void OnWorkerThreadEnd() = 0;                      < 워커스레드 1루프 종료 후

	/// @brief 에러가 났을때 외부로 알려주는 용도이다.
	/// @param errorcode 
	/// @param _out 
	virtual void OnError(ECLanServerErrorCode errorcode, wchar_t* _out) = 0;


//=====================================================================================
private:
	
	void SendPost(CSession* _session);
	void RecvPost(CSession* _session);
private:
	//해당 세션과의 연결을 해제함
	void Release(unsigned long long _sessionId);
public:
	void WorkerThreadFunc();
	void AcceptThreadFunc();
private:
	/// @brief 소켓과 기타 로그 옵션들을 위한 초기화 함수
	/// @return 
	bool Initialize();
public:
	CSession* FindSession(unsigned long long _sessionID);
	/*void SetSessionLogin(bool isLogin);
	bool GetSessionLogin(unsigned long long _sessionID);*/
public:
	//패킷 시작 주소와, 총 크기를 주면 정상적으로 패킷이 들어가 있는지 확인해주는 함수
	void TestFunc(char* packetPtr, DWORD _size);
	

};
