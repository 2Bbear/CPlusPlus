#pragma once

class CPacket;
class CPacketPtr
{
public:
	CPacket* ptr;
	CPacketPtr(CPacket* p);
	~CPacketPtr();
	/// @brief ���������
	/// @param pref 
	/// @return 
	CPacketPtr(CPacketPtr& pref);
public:
	long use_count();
	/// @brief �� ���ֹ�����.
	/// @return 
	void reset();
public:
	const char* operator * ();
	const CPacket* operator -> ();

};
