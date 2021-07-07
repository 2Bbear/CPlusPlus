#pragma once
#include "stdafx.h"
using namespace std;
//#define BUFFSIZE  1400
#define SERIALIZATION_ERRTHROW(_errorType,msg) (PacketException(_errorType,msg,__FILE__,__LINE__))

	/// @brief Packet Ŭ������ ���� ����ó���� �����մϴ�.
	class PacketException :public exception
	{
	public:
		enum class EExceptionType :int
		{
			Default = 0,
			ErrorOfInsufficientInsertSpace = 1,//���� ���� ����
			ErrorOfNoRemainData=2, //���� ������ ����.
			ErrorOfInsertDataInObject, // ��ü�� ������ �ִٰ� ����
			ErrorOfGetObject,//��ü ������ �̴ٰ� ����
		};
	public:
		PacketException(EExceptionType _type,const char * _errorMsg,const char * _fileName, int _line)noexcept
		{
			errorType = _type;
			fileName = _fileName;
			line = _line;
			errorMsg = _errorMsg;
		}
		~PacketException()noexcept
		{
		}

	public:
		EExceptionType errorType;
		const char* errorMsg = nullptr;
		const char* fileName = nullptr;
		int line;
	};
	/// @brief �� Ŭ������ ��� ������ ����ȭ �ڵ忡 ������ ���������ϴ�. �� �����ڿ� �ݵ�� ����ȭ�� �Ѱ��� �� �������� ���� �־��־�� �մϴ�.
	/// ���� �ݵ��  ���� �Լ��� ToSerialize�� UnSerialize�� �������־�� �մϴ�.
	/*
	����
	int netDirection=0;
	int iCurX=200;
	int iCury=300;
	MyPacket::SerializablePacket seriPaload(sizeof(STPacketMessage_CS_Attack2));
			seriPaload << netDirection << iCurX << iCurY;
			MySend(headerMsPacket, seriPaload.GetBufferPtr(), headerMsPacket->bySize);
	*/
	class ISerialize
	{
	protected:
		int serializeSize = 0;
	public:
		ISerialize(int _serializeSize)
		{
			serializeSize = _serializeSize;
		}
	public:
		/// @brief �ش� Ŭ������ ����ȭ�� �����մϴ�.
		/// @param _out ����ȭ �� �����͸� ���� �������Դϴ�.
		/// @param _writePos �ش� ����ȭ �������� ���� ��ġ �Դϴ�.
		/// @return ����ȭ�� ������ byte�� ���Դϴ�. -1�� ��ȯ�� ��� ����ȭ�� ������ ���Դϴ�.
		virtual int ToSerialize(unsigned char* _out, int _writePos) = 0;
		/// @brief �ش� Ŭ������ ������ȭ�� �����մϴ�.
		/// @param _out ����ȭ �� �����͸� ���� ���Դϴ�.
		/// @param _readPos �ش� ����ȭ �������� �б� ��ġ�Դϴ�.
		/// @return ������ true�� ��ȯ�մϴ�.
		virtual bool UnSerialize(unsigned char* _out, int _readPos) = 0;
		int GetSerializeSize()
		{
			return serializeSize;
		}
	};

	class TestPlayerObject :public ISerialize
	{
	public:
		int x;
		int y;
	public:
		TestPlayerObject():ISerialize(8)
		{
			x = 0;
			y = 0;
		}
		~TestPlayerObject()
		{

		}
	public:
		virtual int ToSerialize(unsigned char * _out,int _writePos) override
		{
			int sucSize = -1;
			memcpy(_out + _writePos, &x, sizeof(int));
			_writePos += sizeof(int);
			sucSize += sizeof(int);

			memcpy(_out + _writePos, &y, sizeof(int));
			_writePos += sizeof(int);
			sucSize += sizeof(int);

			return sucSize;
		}
		virtual bool UnSerialize(unsigned char* _out, int _readPos) override
		{
			//x ������
			memcpy(&x, _out + _readPos, sizeof(int));
			_readPos += sizeof(int);
			//y ������
			memcpy(&y, _out + _readPos, sizeof(int));
			_readPos += sizeof(int);

			return true;
		}
	};
	

	/*
	!.	���ԵǴ� �����ʹ� FIFO ������ �����ȴ�.
		ȯ�� ť�� �ƴϹǷ�, �ֱ�(<<).����(>>) �� ȥ���ؼ� ������� �ʵ��� �Ѵ�
	*/
	class SerializableBuffer
	{
		//Ư���� Ÿ�Ե��� �޾Ƴֱ� ����
	public:
		struct stWCHAR
		{
			stWCHAR(unsigned int _stringLength, bool _isAddEndZero = false)
			{
				if (_isAddEndZero==true)
				{
					stringLength = _stringLength;
					data = new wchar_t[stringLength+1];
					size_t arg = (size_t)((stringLength+1) * 2);
					memset(data, L'\0', arg);
				}
				else
				{
					stringLength = _stringLength;
					data = new wchar_t[stringLength];
					size_t arg = (size_t)(stringLength * 2);
					memset(data, 0, arg);
				}
			}
			stWCHAR(unsigned int _stringLength, wchar_t* _data)
			{
				stringLength = _stringLength;
				data = new wchar_t[stringLength];
				size_t arg = (size_t)(stringLength * 2);
				memset(data, 0, arg);
				memcpy(data, _data, arg);
			}
			~stWCHAR()
			{
				delete data;
			}
			unsigned int stringLength;
			wchar_t *data;
		};
		struct stCHAR
		{
			stCHAR(unsigned int _stringLength, bool _isAddEndZero=false)
			{
				if (_isAddEndZero == true)
				{
					stringLength = _stringLength;
					data = new unsigned char[stringLength + 1];
					memset(data, '\0', stringLength + 1);
				}
				else
				{
					stringLength = _stringLength;
					data = new unsigned char[stringLength];
					memset(data, '\0', stringLength);
				}
			}
			stCHAR(unsigned int _stringLength, unsigned char* _data)
			{
				stringLength = _stringLength;
				data = new unsigned char[stringLength];
				memset(data, 0, stringLength);
				memcpy(data, _data, stringLength);
			}
			~stCHAR()
			{
				delete data;
			}
			unsigned int stringLength;
			unsigned char* data;
		};
	public:
		SerializableBuffer(unsigned int _bufSize=100)
		{ 
			buf = (unsigned char*)malloc(_bufSize);
			BUFFSIZE = _bufSize;
			if (buf != nullptr)
			{
				memset(buf, 0, BUFFSIZE);
			}
		}
		virtual ~SerializableBuffer() 
		{
			if (buf!=nullptr)
			{
				free(buf);
				buf = nullptr;
			}
		}
		/// @brief �ʱ�ȭ�� ���� �Լ� ���� ���۰� �ִٸ� ���۸� �������� �ʰ� ���θ� û���� 
		/// _bufSize�� ���� ���� ������ ���� Ŭ ��� ���� ���۸� �����ϰ� ���ο� ���۸� �Ҵ���
		/// @param _bufSize 
		void Init(unsigned int _bufSize)
		{
			if (buf!=nullptr)
			{
				if (_bufSize > (unsigned int)BUFFSIZE)
				{
					free(buf);

					buf = (unsigned char*)malloc(_bufSize);
					BUFFSIZE = _bufSize;
					if (buf != nullptr)
					{
						memset(buf, 0, BUFFSIZE);
					}
				}
				else
				{
					BUFFSIZE = _bufSize;
					memset(buf, 0, _bufSize);
				}
			}
			else
			{
				buf = (unsigned char*)malloc(_bufSize);
				BUFFSIZE = _bufSize;
				if (buf != nullptr)
				{
					memset(buf, 0, BUFFSIZE);
				}
			}
			writePos = 0;
			readPos = 0;
		}
	protected:
		unsigned char* buf;//[BUFFSIZE];
		int BUFFSIZE = 0;
		int writePos = 0;
		int readPos = 0;
	protected:
		/// @brief ���� �����͸� �Ҵ� ���� ���� ���� ������ ������ ��ȯ��
		/// @return 
		int GetFreeSize()
		{
			return BUFFSIZE - writePos;
		}
		
		/// @brief ���� ��� �ִ��� ���θ� ��ȯ��
		/// @return 
		bool IsEmpty()
		{
			if (writePos== readPos)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

	public:
		bool Copy(SerializableBuffer * _age)
		{
			bool result = false;



			return result;
		}
		/// @brief ���� ����ϰ� �ִ� ������ ���� ������ ��ȯ��
		/// @return 
		int GetUseSize()
		{
			return writePos - readPos;
		}
		/// @brief ������ ����
		/// @param chpSrc ���� ������
		/// @param iSrcSize ���� �� ������ ũ��
		/// @return ������ ������
		int	PutData(char* chpSrc, int iSrcSize)
		{
			if (GetFreeSize() < iSrcSize)
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "�ִٰ� �����߾��");
			}
			memcpy(buf + writePos, chpSrc, iSrcSize);
			writePos += iSrcSize;
			return iSrcSize;
		}
		/// @brief ������ ���
		/// @param chpDest ���� ������ ��ġ
		/// @param iSize ���� ������ ��ġ ũ��
		/// @return 
		int	GetData(char* chpDest, int iSize)
		{
			if (GetUseSize() < iSize)
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			memcpy(chpDest, buf + readPos, iSize);
			readPos += iSize;

			return readPos;
		}
		int MoveWritePos(int iSize)
		{
			writePos += iSize;
			return readPos;
		}
		/// @brief ���� ������ ���
		/// @param  
		/// @return ���� ������
		unsigned char* GetBufferPtr(void)
		{ 
			return buf;
		}
		int GetBufferSize()
		{
			return BUFFSIZE;
		}
		bool Clear()
		{
			writePos = 0;
			readPos = 0;
			return true;
		}
		/*
		���� ���ø��� ������� �ʴ� ����. ���ø� �پ��� Ÿ���� ���� �� �ְ� �ߴٰ�,
		����ġ ���� Ÿ���� ��� ������ ������ �߻��� �� ����.

		��Ŷ�ʿ��� ����ȭ ���Ǹ� �ϴ� ����. ���� ��...
		*/
	public:
		SerializableBuffer& operator << (BYTE data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(BYTE))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "�ִٰ� �����߾��");
			}
			memcpy(buf + writePos, &data, sizeof(BYTE));
			writePos += sizeof(BYTE);
			return *(this);
		}
		SerializableBuffer& operator << (char data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(char))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "�ִٰ� �����߾��");
			}
			memcpy(buf + writePos, &data, sizeof(char));
			writePos += sizeof(char);
			return *(this);
		}
		SerializableBuffer& operator << (short data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(short))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "�ִٰ� �����߾��");
			}
			memcpy(buf + writePos, &data, sizeof(short));
			writePos += sizeof(short);
			return *(this);
		}
		SerializableBuffer& operator << (WORD data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(WORD))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "�ִٰ� �����߾��");
			}
			memcpy(buf + writePos, &data, sizeof(WORD));
			writePos += sizeof(WORD);
			return *(this);
		}
		SerializableBuffer& operator << (int data) throw(PacketException)
		{
			if (GetFreeSize()<sizeof(int))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace,"�ִٰ� �����߾��");
			}
			memcpy(buf + writePos, &data, sizeof(int));
			writePos += sizeof(int);
			return *(this);
		}
		SerializableBuffer& operator << (DWORD data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(DWORD))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "�ִٰ� �����߾��");
			}
			memcpy(buf + writePos, &data, sizeof(DWORD));
			writePos += sizeof(DWORD);
			return *(this);
		}
		SerializableBuffer& operator << (float data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(float))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "�ִٰ� �����߾��");
			}
			memcpy(buf + writePos, &data, sizeof(float));
			writePos += sizeof(float);
			return *(this);
		}
		SerializableBuffer& operator << (__int64 data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(__int64))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "�ִٰ� �����߾��");
			}
			memcpy(buf + writePos, &data, sizeof(__int64));
			writePos += sizeof(__int64);
			return *(this);
		}
		SerializableBuffer& operator << (double data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(double))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "�ִٰ� �����߾��");
			}
			memcpy(buf + writePos, &data, sizeof(double));
			writePos += sizeof(double);
			return *(this);
		}
		SerializableBuffer& operator << (unsigned long long data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(unsigned long long))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "�ִٰ� �����߾��");
			}
			memcpy(buf + writePos, &data, sizeof(unsigned long long));
			writePos += sizeof(unsigned long long);
			return *(this);
		}
		SerializableBuffer& operator <<(stWCHAR& iValue) throw(PacketException)
		{
			if (GetFreeSize() < int(iValue.stringLength * 2))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "�ִٰ� �����߾��");
			}
			memcpy(buf + writePos,iValue.data, (int)(iValue.stringLength * 2));
			writePos += iValue.stringLength * 2;
			return *(this);
		}
		SerializableBuffer& operator <<(stCHAR& iValue) throw(PacketException)
		{
			if (GetFreeSize() < int(iValue.stringLength))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "�ִٰ� �����߾��");
			}
			memcpy(buf + writePos, iValue.data, int(iValue.stringLength) );
			writePos += iValue.stringLength ;
			return *(this);
		}

		SerializableBuffer& operator << (ISerialize& data) throw(PacketException)
		{
			if (GetFreeSize() < data.GetSerializeSize())
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "�ִٰ� �����߾��");
			}
			if (data.ToSerialize(buf, writePos) == -1)
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsertDataInObject, "��ü �����͸� �ִٰ� �����߾��");
			}
			writePos += data.GetSerializeSize();
			return *(this);
		}
		SerializableBuffer& operator <<(SerializableBuffer& iValue) throw(PacketException)
		{
			if (GetFreeSize() < int(iValue.GetUseSize()))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "�ִٰ� �����߾��");
			}
			memcpy(buf + writePos, iValue.GetBufferPtr(), int(iValue.GetUseSize()));
			writePos += iValue.GetUseSize();
			return *(this);
		}
		//=============================================================================================
		SerializableBuffer& operator >>(BYTE& iValue)
		{
			if (GetUseSize()<sizeof(BYTE))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData,"���ٰ� �����߾��");
			}
			memcpy(&iValue, buf+ readPos,sizeof(BYTE));
			readPos += sizeof(BYTE);
			return *(this);
		}
		SerializableBuffer& operator >>(char& iValue)
		{
			if (GetUseSize() < sizeof(char))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			memcpy(&iValue, buf + readPos, sizeof(char));
			readPos += sizeof(char);
			return *(this);
		}
		SerializableBuffer& operator >>(short& iValue)
		{
			if (GetUseSize() < sizeof(short))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			memcpy(&iValue, buf + readPos, sizeof(short));
			readPos += sizeof(short);
			return *(this);
		}
		SerializableBuffer& operator >>(WORD& iValue)
		{
			if (GetUseSize() < sizeof(WORD))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			memcpy(&iValue, buf + readPos, sizeof(WORD));
			readPos += sizeof(WORD);
			return *(this);
		}
		SerializableBuffer& operator >>(int& iValue)
		{
			if (GetUseSize() < sizeof(int))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			memcpy(&iValue, buf + readPos, sizeof(int));
			readPos += sizeof(int);
			return *(this);
		}
		SerializableBuffer& operator >>(DWORD& iValue)
		{
			if (GetUseSize() < sizeof(DWORD))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			memcpy(&iValue, buf + readPos, sizeof(DWORD));
			readPos += sizeof(DWORD);
			return *(this);
		}
		SerializableBuffer& operator >>(float& iValue)
		{
			if (GetUseSize() < sizeof(float))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			memcpy(&iValue, buf + readPos, sizeof(float));
			readPos += sizeof(float);
			return *(this);
		}
		SerializableBuffer& operator >>(__int64& iValue)
		{
			if (GetUseSize() < sizeof(__int64))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			memcpy(&iValue, buf + readPos, sizeof(__int64));
			readPos += sizeof(__int64);
			return *(this);
		}
		SerializableBuffer& operator >>(double& iValue)
		{
			if (GetUseSize() < sizeof(double))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			memcpy(&iValue, buf + readPos, sizeof(double));
			readPos += sizeof(double);
			return *(this);
		}
		SerializableBuffer& operator >>(unsigned long long& iValue)
		{
			if (GetUseSize() < sizeof(unsigned long long))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			memcpy(&iValue, buf + readPos, sizeof(unsigned long long));
			readPos += sizeof(unsigned long long);
			return *(this);
		}
		SerializableBuffer& operator >>(stWCHAR &iValue)
		{
			if (GetUseSize() < int(iValue.stringLength * 2))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			memcpy(iValue.data, buf + readPos, int(iValue.stringLength*2));
			readPos += iValue.stringLength * 2;;
			return *(this);
		}
		SerializableBuffer& operator >>(stCHAR& iValue)
		{
			if (GetUseSize() < int(iValue.stringLength) )
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			memcpy(iValue.data, buf + readPos, int(iValue.stringLength ));
			readPos += iValue.stringLength ;
			return *(this);
		}



		SerializableBuffer& operator >>(ISerialize& _data)
		{
			if (GetUseSize() < _data.GetSerializeSize())
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			if (_data.UnSerialize(buf, readPos) == false)
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfGetObject, "��ü �����Ϳ� ���ٰ� �����߾��");
			}
			readPos += _data.GetSerializeSize();
			return *(this);
		}
		SerializableBuffer& operator >>(SerializableBuffer& iValue)
		{
			if (GetUseSize() < iValue.GetFreeSize())
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "���ٰ� �����߾��");
			}
			memcpy(&iValue, buf + readPos, iValue.GetFreeSize());
			readPos += iValue.GetFreeSize();
			return *(this);
		}
	};
