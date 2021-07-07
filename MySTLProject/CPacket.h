#pragma once
#include "SerializationBuffer.h"
class CPacket: public SerializableBuffer
{
public:
	/// @brief �ܼ��� �ܺο��� ����� �ޱ� ���� �뵵�ϻ�. ���ο��� �̰��� ������ ó������ �ʽ��ϴ�.
	struct stHeader
	{
		unsigned char code;// + �е� 1
		unsigned short len;
		unsigned char randKey;
		unsigned char checkSum;
	};
	static const int HEADERSIZE = 5;
public:
	CPacket(int _bufSize = 1400);
	~CPacket();
public:
	/// @brief �ʱ�ȭ
	/// @return �ʱ�ȭ ���� ����
	bool init();
public:
	long useCount = 0; // �̰� �̰��� �ִ°� �´°ǰ�?
	void Dispose();
	void addRef();
	int subRef();
public:
	/// @brief ���� �ִ� �����͸� ������ �� �ְ� ��ŷ�ϴ� �Լ�
	/// @param _header ��ȣȭ�� �� ��带 �����ϴ� �Ű�����
	/// @return 
	bool PackageData(stHeader* _header = nullptr);
	/// @brief ���� �ִ� �����͸� Ǯ�� �ְ��ϴ� �Լ�
	/// @param _header ��ȣȭ�� �� ��带 �����ϴ� �Ű�����
	/// @return �����ߴٸ� nullptr�� ��ȯ��. 
	/// �����ߴٸ� �����͸� ���� �� �ִ� ��Ű�� �����͸� ��ȯ��. ������ �����ʹ� �ڱ� �ڽ� ��������
	CPacket* UnPackageData();
private:
	//=========��ȣȭ ���==================//
	static const char stickyKey=87;
	bool isEncrypt = false;
	/// @brief ����κ��� ������ �� �ִ� �Լ�
	/// @param _header 
	void SetHeader(stHeader* _header);
	/// @brief ���� ���� �ִ� �����͸� ��ü ����� ���� ��ȣȭ �ϴ� ���
	/// �ڵ����� ����Ű�� ����ݴϴ�.
	/// ��ȣȭ ����� checksum�� payload �Դϴ�.
	void Encrypt(stHeader* _header);
	/// @brief ��ȣȭ�� Ǫ�� �Լ�
	void Decrypt();
public:
	SerializableBuffer& operator <<(stCHAR& iValue) throw(PacketException)
	{
		SerializableBuffer& result = SerializableBuffer::operator<< (iValue);//�θ� ������ ���� ȣ��

		//Packet Ŭ�������� �߰��� ����� �� ��
		//����� len ���̸� �־��ֱ�
		unsigned short len=0;
		len = GetUseSize();
		memcpy(buf + 1, &len, sizeof(unsigned short));

		return result;
	}
};


/* ��ȣȭ �׽�Ʈ
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