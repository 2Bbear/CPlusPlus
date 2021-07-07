#pragma once

#include "stdafx.h"
//전방 선언
class RingBufferEx;

class CSession
{
public:
	OVERLAPPED sendOverlapped;
	OVERLAPPED recvOverlapped;
	SOCKET sock;
	RingBufferEx* sendRing;
	unsigned long curSendCount;
	RingBufferEx* recvRing;
	WSABUF wsabuf[2];
	long long sessionID;
	unsigned long long ioCount;
	CRITICAL_SECTION cs_session;
	int index;
	unsigned long isSend;
	long isRecv;
	long isLogin;
	
	CSession();
	~CSession();
};

