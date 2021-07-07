#pragma once
#include "SerializationBuffer.h"
class CPacket: public SerializableBuffer
{
public:
	/// @brief 단순히 외부에서 헤더를 받기 위한 용도일뿐. 내부에서 이것을 가지고 처리하지 않습니다.
	struct stHeader
	{
		unsigned char code;// + 패딩 1
		unsigned short len;
		unsigned char randKey;
		unsigned char checkSum;
	};
	static const int HEADERSIZE = 5;
public:
	CPacket(int _bufSize = 1400);
	~CPacket();
public:
	/// @brief 초기화
	/// @return 초기화 성공 여부
	bool init();
public:
	long useCount = 0; // 이게 이곳에 있는게 맞는건가?
	void Dispose();
	void addRef();
	int subRef();
public:
	/// @brief 갖고 있는 데이터를 전송할 수 있게 패킹하는 함수
	/// @param _header 암호화에 들어갈 헤드를 주입하는 매개변수
	/// @return 
	bool PackageData(stHeader* _header = nullptr);
	/// @brief 갖고 있는 데이터를 풀수 있게하는 함수
	/// @param _header 암호화에 들어갈 헤드를 주입하는 매개변수
	/// @return 실패했다면 nullptr을 반환함. 
	/// 성공했다면 데이터를 뽑을 수 있는 패키지 포인터를 반환함. 성공한 포인터는 자기 자신 포인터임
	CPacket* UnPackageData();
private:
	//=========암호화 기능==================//
	static const char stickyKey=87;
	bool isEncrypt = false;
	/// @brief 헤더부분을 설정할 수 있는 함수
	/// @param _header 
	void SetHeader(stHeader* _header);
	/// @brief 현재 갖고 있는 데이터를 자체 헤더에 맞춰 암호화 하는 기능
	/// 자동으로 랜덤키를 잡아줍니다.
	/// 암호화 대상은 checksum과 payload 입니다.
	void Encrypt(stHeader* _header);
	/// @brief 암호화를 푸는 함수
	void Decrypt();
public:
	SerializableBuffer& operator <<(stCHAR& iValue) throw(PacketException)
	{
		SerializableBuffer& result = SerializableBuffer::operator<< (iValue);//부모 연산자 정의 호출

		//Packet 클래스에서 추가로 해줘야 할 것
		//헤더에 len 길이를 넣어주기
		unsigned short len=0;
		len = GetUseSize();
		memcpy(buf + 1, &len, sizeof(unsigned short));

		return result;
	}
};


/* 암호화 테스트
* void Test()
{
	char dataArr[8];
	for (int i=0;i<8;++i)
	{
		dataArr[i] = i + 1;
	}
	CPacket::stCHAR inputData(8,dataArr);
	CPacket packet(8);
	packet << inputData;
	packet.PackageData();

	packet.UnPackageData();
}


*/