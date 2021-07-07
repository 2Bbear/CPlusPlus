#pragma once

#include "stdafx.h"
using namespace std;

//���漱��
class CSession;
class SerializableBuffer;
template<typename T> class CMemoryPoolTLS;
/// @brief ��������
class CLanServer
{
public:
	enum class ECLanServerErrorCode : int
	{
		Default = 0,
	};

private:
	CMemoryPoolTLS<SerializableBuffer> * pPacketPool;
	unsigned long long maxConnectClient; // �ִ� ���� ������ Ŭ���̾�Ʈ ��
	CSession * sessionArr; // ������ ������ ���� ���� �迭
	stack<CSession*>  sessionIdx;
	int runingWorkerThreadCnt; // ���� ������ ��Ŀ ������ ��
	int waitWorkerThreadCnt; //��� ������ ��Ŀ ������ ��
	bool isDoNagle; //���̱� �ɼǿ� ���� ����
	const char* ip;
	int port;
	HANDLE hCCP;// �Ϸ� ��ƾ�� ���� �ڵ�
	SOCKET listen_sock;
	int ringBufferSize = 20000;
public:
	unsigned long long curWorkerThreadCnt;//���� ��Ŀ������ ���� ��Ÿ��
	unsigned long long curSessionCount; // ���� ����� ���� ��
	unsigned long long curDisconnectSessionCount = 0;//������ ���� ������ ��
	unsigned long long curRecvCount = 0;
	unsigned long long curSendCount = 0;
private:
	bool bShutdown; //�� ������ true �϶� ��� ������� ���� ������ ����ȴ�.
	CRITICAL_SECTION cs_sessionArr; //session �迭�� ���� ����ȭ ��ü
public:		
	CLanServer();
	~CLanServer();
//==============================�ܺ� ���� �Լ�========================================
public:
	/// @brief ���� ���ۿ� �Լ� , ���� ����ۿ� ���� ����� ���� ����.
	/// @param _ip 
	/// @param _port 
	/// @param _runingWorkerThreadCnt 
	/// @param _waitWorkerThreadCnt 
	/// @param _isDoNagle 
	/// @param _maxConnectClient 
	/// @return 
	bool Start(const char* _ip, int _port, int _runingWorkerThreadCnt, int _waitWorkerThreadCnt, bool _isDoNagle, unsigned long long _maxConnectClient);
	/// @brief ���� ����� �Լ�
	/// Ŭ���̾�Ʈ ����, DB �� �α� ������� �����ϰ� �ȴ�.
	/// @param _isShutdown 
	void Stop(bool _isShutdown);
	/// @brief ���� ������ Ŭ���̾�Ʈ�� ���� ��ȯ�Ѵ�.
	/// @return 
	int GetClientCount();
public:
	/// @brief �ش� ���ǿ� ���� ���� ���� ��ûs
	/// @param SessionID ���� ����� ���� ���̵�
	/// @return ���� ����
	bool Disconnect(unsigned long long _sessionID);
	/// @brief �ش� ���ǿ� ��Ŷ�� ���� �ش޶�� ��û
	/// �ش� ������ SendQ�� ��Ŷ�� �ִ� �۾��� �ϴ� ����
	/// @param SessionID 
	/// @return 
	bool SendPacket(unsigned long long _sessionID, SerializableBuffer* _paload);
	bool InputSendPacket(unsigned long long _sessionID, SerializableBuffer* _packet);

//==================================���� ������ ��� �Լ�===================================================
public:
	/// @brief IOCP ��Ʈ��ũ �����ϱ� �� ������ ó���� ���� �Լ�
	/// @param _sessionID 
	virtual void BegineClientJoin(unsigned long long _sessionID) = 0;
	/// @brief IOCP ��Ʈ��ũ ���� �� ������ ó���� ���� �Լ�
	/// @param _sessionID 
	virtual void OnClientJoin(unsigned long long _sessionID) = 0;
	/// @brief Release �� ȣ��Ǵ� �Լ�
	/// @param _sessionID 
	virtual void OnClientLeave(unsigned long long _sessionID) = 0;
	/// @brief accept ���� �Ҹ��� �Լ�, ��� ���� �׽��z ���ؼ� �׽�Ʈ�� ������ �������� ���� ���� �Լ���.
	/// @param _ip 
	/// @param _port 
	/// @return false �� Ŭ���̾�Ʈ �ź��� ��, true �� ���� ���
	virtual bool OnConnectionRequest(const char* _ip, int _port) = 0;
	/// @brief ��Ŷ ���� �Ϸ��� �Ҹ� �Լ�
	/// @param _sessionID 
	/// @param _packet 
	virtual void OnRecv(unsigned long long _sessionID, SerializableBuffer* _packet) = 0;

	virtual void OnSend(unsigned long long _sessionID, int sendsize) = 0; //          < ��Ŷ �۽� �Ϸ� ��
	//	virtual void OnWorkerThreadBegin() = 0;                    < ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
	//	virtual void OnWorkerThreadEnd() = 0;                      < ��Ŀ������ 1���� ���� ��

	/// @brief ������ ������ �ܺη� �˷��ִ� �뵵�̴�.
	/// @param errorcode 
	/// @param _out 
	virtual void OnError(ECLanServerErrorCode errorcode, wchar_t* _out) = 0;


//=====================================================================================
private:
	
	void SendPost(CSession* _session);
	void RecvPost(CSession* _session);
private:
	//�ش� ���ǰ��� ������ ������
	void Release(unsigned long long _sessionId);
public:
	void WorkerThreadFunc();
	void AcceptThreadFunc();
private:
	/// @brief ���ϰ� ��Ÿ �α� �ɼǵ��� ���� �ʱ�ȭ �Լ�
	/// @return 
	bool Initialize();
public:
	CSession* FindSession(unsigned long long _sessionID);
	/*void SetSessionLogin(bool isLogin);
	bool GetSessionLogin(unsigned long long _sessionID);*/
public:
	//��Ŷ ���� �ּҿ�, �� ũ�⸦ �ָ� ���������� ��Ŷ�� �� �ִ��� Ȯ�����ִ� �Լ�
	void TestFunc(char* packetPtr, DWORD _size);
	

};
