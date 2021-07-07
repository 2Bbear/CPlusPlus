
#include <iostream>
using namespace std;
#include "CPacket.h"

CPacket::CPacket(int _bufSize) : SerializableBuffer(_bufSize + HEADERSIZE) //��� ũ�� ��ŭ ���� ���ܿ�
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
	
	//�� ��� �ڵ� �ֱ�
	memset(buf, 'c', 1);
	//���鶧 �� ����� ���� Ű �ֱ�
	srand((unsigned int)time(0));
	unsigned char randKey = 22;//rand() % 256; //�����̾�� �ϴµ� �ϴ� �׽�Ʈ�� ���� �������� ����
	memset(buf+3, randKey, 1);
	//������� ������ �� �κ� 5�� �����
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
	if (_header==nullptr) // ���� ��� ����ϱ�
	{
		//üũ�� �����
		unsigned char checksum = 0;
		for (int i = HEADERSIZE; i < HEADERSIZE + GetUseSize(); ++i)
		{
			checksum += buf[i];
		}
		checksum %= 256;
		memcpy(buf+4,&checksum,sizeof(unsigned char));

		//�� ��� 5����Ʈ �о����
		buf = buf - 5;
		stHeader header;
		memcpy(&header.code, buf, 1);
		memcpy(&header.len, buf + 1, 2);
		memcpy(&header.randKey, buf + 3, 1);
		memcpy(&header.checkSum, buf + 4, 1);

		Encrypt(&header);
		isEncrypt = true;

		//�ٽ� 5����Ʈ ���ܿ�
		buf = buf + 5;
	}
	else // �ܺ� �ش� ����ϱ�
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
	# ���� ������ ����Ʈ ����  D1 D2 D3 D4
	----------------------------------------------------------------------------------------------------------
	|          D1           |            D2             |            D3             |             D4            |
	----------------------------------------------------------------------------------------------------------
		D1 ^ (RK + 1) = P1   |  D2 ^ (P1 + RK + 2) = P2  |  D3 ^ (P2 + RK + 3) = P3  |  D4 ^ (P3 + RK + 4) = P4  |
		P1 ^ (K + 1) = E1   |   P2 ^ (E1 + K + 2) = E2  |   P3 ^ (E2 + K + 3) = E3  |   P4 ^ (E3 + K + 4) = E4  |

	# ��ȣ ������ ����Ʈ ����  E1 E2 E3 E4
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
	//üũ������ ��ȣȭ
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
		p = buf[idx] ^ (e+stickyKey + count);//p���ϱ�
		d = p ^ (psub+header.randKey + count); // d ���ϱ�
		psub = p;//p ����
		e = buf[idx];//e���ܳ���
		buf[idx] = d;
		count++;
	}
}

