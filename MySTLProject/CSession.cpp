#include "stdafx.h"

#include "RingBufferEx.h"
#include "CSession.h"


CSession::CSession()
{
	sendRing = nullptr;// new RingBufferEx(_buffsize);
	recvRing = nullptr;// new RingBufferEx(_buffsize);
	sessionID = - 1;
	ioCount = 0;
	index = 0;
	InitializeCriticalSection(&cs_session);
	isSend = false;
	isRecv = false;
}

CSession::~CSession()
{
	if (sendRing != nullptr)
	{
		delete sendRing;
	}
	if (recvRing != nullptr)
	{
		delete recvRing;
	}
}
