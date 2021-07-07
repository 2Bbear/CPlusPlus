
#include <iostream>
using namespace std;
#include "CPacket.h"

CPacket::CPacket(int _bufSize) : SerializableBuffer(_bufSize + HEADERSIZE) //헤더 크기 만큼 앞을 땡겨옴
{
	init();
}

CPacket::~CPacket()
{
	buf = buf - 5;
	free(buf);
	buf = nullptr;
}

bool CPacket::init()
{
	useCount = 0;
	isEncrypt = false;
	
	//앞 헤더 코드 넣기
	memset(buf, 'c', 1);
	//만들때 앞 헤더에 랜덤 키 넣기
	srand((unsigned int)time(0));
	unsigned char randKey = 22;//rand() % 256; //랜덤이어야 하는데 일단 테스트를 위해 고정값을 넣음
	memset(buf+3, randKey, 1);
	//만들어진 버퍼의 앞 부분 5개 땡기기
	buf = buf + 5;
	return true;
}

void CPacket::Dispose()
{
	free(buf);
}

void CPacket::addRef()
{
	_InterlockedIncrement(&useCount);
}

int CPacket::subRef()
{
	long ret = _InterlockedDecrement(&useCount);
	return ret;
}

bool CPacket::PackageData(stHeader* _header)
{
	if (_header==nullptr) // 내부 헤더 사용하기
	{
		//체크섬 만들기
		unsigned char checksum = 0;
		for (int i = HEADERSIZE; i < HEADERSIZE + GetUseSize(); ++i)
		{
			checksum += buf[i];
		}
		checksum %= 256;
		memcpy(buf+4,&checksum,sizeof(unsigned char));

		//앞 헤더 5바이트 읽어오기
		buf = buf - 5;
		stHeader header;
		memcpy(&header.code, buf, 1);
		memcpy(&header.len, buf + 1, 2);
		memcpy(&header.randKey, buf + 3, 1);
		memcpy(&header.checkSum, buf + 4, 1);

		Encrypt(&header);
		isEncrypt = true;

		//다시 5바이트 땡겨옴
		buf = buf + 5;
	}
	else // 외부 해더 사용하기
	{
		Encrypt(_header);
		isEncrypt = true;
	}
	
	return true;
}

CPacket* CPacket::UnPackageData()
{
	buf = buf - 5;
	Decrypt();
	buf = buf +5;
	return this;
}

void CPacket::SetHeader(stHeader* _header)
{
	memcpy(buf, &_header->code,1);
	memcpy(buf, &_header->len, 2);
	memcpy(buf, &_header->randKey, 1);
	memcpy(buf, &_header->checkSum, 1);
}
void CPacket::Encrypt(stHeader* _header)
{
	//====================================================
	/*
	# 원본 데이터 바이트 단위  D1 D2 D3 D4
	----------------------------------------------------------------------------------------------------------
	|          D1           |            D2             |            D3             |             D4            |
	----------------------------------------------------------------------------------------------------------
		D1 ^ (RK + 1) = P1   |  D2 ^ (P1 + RK + 2) = P2  |  D3 ^ (P2 + RK + 3) = P3  |  D4 ^ (P3 + RK + 4) = P4  |
		P1 ^ (K + 1) = E1   |   P2 ^ (E1 + K + 2) = E2  |   P3 ^ (E2 + K + 3) = E3  |   P4 ^ (E3 + K + 4) = E4  |

	# 암호 데이터 바이트 단위  E1 E2 E3 E4
	----------------------------------------------------------------------------------------------------------
				E1                      E2                          E3                           E4 
	----------------------------------------------------------------------------------------------------------
	*/
	/*
	* 
	e1 ^ (k+1) = p1      e2 ^ (e1+k+2) = p2
	p1 ^(rk +1) = d1     p2 ^ (p1+rk+2)= d2
	
	*/
	//====================================================
	//체크섬부터 암호화
	int count = 1;
	unsigned char p = 0;
	unsigned char e = 0;
	for (int idx = 4; idx <= 4 + _header->len; ++idx)
	{
		p = buf[idx] ^ (p + _header->randKey + count);
		e = p ^ (e + stickyKey + count);
		buf[idx] = e;
		count++;
	}
}

void CPacket::Decrypt()
{
	stHeader header;
	memcpy(&header.code, buf, 1);
	memcpy(&header.len, buf+1, 2);
	memcpy(&header.randKey, buf+3, 1);
	memcpy(&header.checkSum, buf+4, 1);
	int count = 1;
	unsigned char p = 0;
	unsigned char psub = 0;
	unsigned char d = 0;
	unsigned char e = 0;
	for (int idx = 4; idx <= 4 + header.len; ++idx)
	{
		p = buf[idx] ^ (e+stickyKey + count);//p구하기
		d = p ^ (psub+header.randKey + count); // d 구하기
		psub = p;//p 보존
		e = buf[idx];//e남겨놓기
		buf[idx] = d;
		count++;
	}
}

