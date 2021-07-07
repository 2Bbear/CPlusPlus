#include "CPacket.h"

#include <iostream>
using namespace std;

#include "CPacketPtr.h"

CPacketPtr::CPacketPtr(CPacket* p)
{
	ptr = p;
	ptr->addRef();

}

CPacketPtr::~CPacketPtr()
{
	if (ptr == nullptr)
	{
		return;
	}
	if (ptr->subRef() == 0)
	{
		delete ptr;
	}

}

CPacketPtr::CPacketPtr(CPacketPtr& pref)
{
	ptr = pref.ptr;
	ptr->addRef();
}

long CPacketPtr::use_count()
{
	if (ptr == nullptr)
	{
		return 0;
	}

	return ptr->useCount;
}

void CPacketPtr::reset()
{
	if (ptr == nullptr)
	{
		return;
	}
	if (ptr->subRef() == 0)
	{
		delete ptr;
	}
	ptr = nullptr;
}

const char* CPacketPtr::operator*()
{
	return (char*)ptr->GetBufferPtr();
}

const CPacket* CPacketPtr::operator->()
{
	return ptr;
}
