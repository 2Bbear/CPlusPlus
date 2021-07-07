#pragma once

class CPacket;
class CPacketPtr
{
public:
	CPacket* ptr;
	CPacketPtr(CPacket* p);
	~CPacketPtr();
	/// @brief 복사생성자
	/// @param pref 
	/// @return 
	CPacketPtr(CPacketPtr& pref);
public:
	long use_count();
	/// @brief 다 없애버리기.
	/// @return 
	void reset();
public:
	const char* operator * ();
	const CPacket* operator -> ();

};
