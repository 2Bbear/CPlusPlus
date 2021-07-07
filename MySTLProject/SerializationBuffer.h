#pragma once
#include "stdafx.h"
using namespace std;
//#define BUFFSIZE  1400
#define SERIALIZATION_ERRTHROW(_errorType,msg) (PacketException(_errorType,msg,__FILE__,__LINE__))

	/// @brief Packet 클래스에 대한 예외처리를 정의합니다.
	class PacketException :public exception
	{
	public:
		enum class EExceptionType :int
		{
			Default = 0,
			ErrorOfInsufficientInsertSpace = 1,//삽입 공간 부족
			ErrorOfNoRemainData=2, //남은 데이터 없음.
			ErrorOfInsertDataInObject, // 객체에 데이터 넣다가 실패
			ErrorOfGetObject,//객체 데이터 뽑다가 실패
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
	/// @brief 이 클래스를 상속 받으면 직렬화 코드에 삽입이 가능해집니다. 단 생성자에 반드시 직렬화로 넘겨줄 총 데이터의 값을 넣어주어야 합니다.
	/// 또한 반드시  가상 함수인 ToSerialize와 UnSerialize를 구현해주어야 합니다.
	/*
	사용법
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
		/// @brief 해당 클래스의 직렬화를 수행합니다.
		/// @param _out 직렬화 된 데이터를 넣을 목적지입니다.
		/// @param _writePos 해당 직렬화 데이터의 쓰기 위치 입니다.
		/// @return 직렬화에 성공한 byte의 수입니다. -1을 반환할 경우 직렬화에 실패한 것입니다.
		virtual int ToSerialize(unsigned char* _out, int _writePos) = 0;
		/// @brief 해당 클래스의 역직렬화를 수행합니다.
		/// @param _out 직렬화 된 데이터를 빼올 곳입니다.
		/// @param _readPos 해당 직렬화 데이터의 읽기 위치입니다.
		/// @return 성공시 true를 반환합니다.
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
			//x 빼오기
			memcpy(&x, _out + _readPos, sizeof(int));
			_readPos += sizeof(int);
			//y 빼오기
			memcpy(&y, _out + _readPos, sizeof(int));
			_readPos += sizeof(int);

			return true;
		}
	};
	

	/*
	!.	삽입되는 데이터는 FIFO 순서로 관리된다.
		환형 큐는 아니므로, 넣기(<<).빼기(>>) 를 혼합해서 사용하지 않도록 한다
	*/
	class SerializableBuffer
	{
		//특수한 타입들을 받아넣기 위함
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
		/// @brief 초기화를 위한 함수 기존 버퍼가 있다면 버퍼를 해제하지 않고 내부만 청소함 
		/// _bufSize가 기존 버퍼 사이즈 보다 클 경우 기존 버퍼를 해제하고 새로운 버퍼를 할당함
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
		/// @brief 현재 데이터를 할당 하지 않은 남은 공간의 갯수를 반환함
		/// @return 
		int GetFreeSize()
		{
			return BUFFSIZE - writePos;
		}
		
		/// @brief 현재 비어 있는지 여부를 반환함
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
		/// @brief 현재 사용하고 있는 데이터 공간 갯수를 반환함
		/// @return 
		int GetUseSize()
		{
			return writePos - readPos;
		}
		/// @brief 데이터 삽입
		/// @param chpSrc 넣을 데이터
		/// @param iSrcSize 삽입 할 데이터 크기
		/// @return 삽입한 사이즈
		int	PutData(char* chpSrc, int iSrcSize)
		{
			if (GetFreeSize() < iSrcSize)
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "넣다가 실패했어요");
			}
			memcpy(buf + writePos, chpSrc, iSrcSize);
			writePos += iSrcSize;
			return iSrcSize;
		}
		/// @brief 데이터 얻기
		/// @param chpDest 넣을 데이터 위치
		/// @param iSize 넣을 데이터 위치 크기
		/// @return 
		int	GetData(char* chpDest, int iSize)
		{
			if (GetUseSize() < iSize)
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
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
		/// @brief 버퍼 포인터 얻기
		/// @param  
		/// @return 버퍼 포인터
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
		굳이 템플릿을 사용하지 않는 이유. 괜시리 다양한 타입을 받을 수 있게 했다가,
		예상치 못한 타입이 들어 왔을때 문제가 발생할 수 있음.

		패킷쪽에서 직렬화 정의를 하는 이유. 딱히 없...
		*/
	public:
		SerializableBuffer& operator << (BYTE data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(BYTE))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "넣다가 실패했어요");
			}
			memcpy(buf + writePos, &data, sizeof(BYTE));
			writePos += sizeof(BYTE);
			return *(this);
		}
		SerializableBuffer& operator << (char data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(char))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "넣다가 실패했어요");
			}
			memcpy(buf + writePos, &data, sizeof(char));
			writePos += sizeof(char);
			return *(this);
		}
		SerializableBuffer& operator << (short data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(short))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "넣다가 실패했어요");
			}
			memcpy(buf + writePos, &data, sizeof(short));
			writePos += sizeof(short);
			return *(this);
		}
		SerializableBuffer& operator << (WORD data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(WORD))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "넣다가 실패했어요");
			}
			memcpy(buf + writePos, &data, sizeof(WORD));
			writePos += sizeof(WORD);
			return *(this);
		}
		SerializableBuffer& operator << (int data) throw(PacketException)
		{
			if (GetFreeSize()<sizeof(int))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace,"넣다가 실패했어요");
			}
			memcpy(buf + writePos, &data, sizeof(int));
			writePos += sizeof(int);
			return *(this);
		}
		SerializableBuffer& operator << (DWORD data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(DWORD))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "넣다가 실패했어요");
			}
			memcpy(buf + writePos, &data, sizeof(DWORD));
			writePos += sizeof(DWORD);
			return *(this);
		}
		SerializableBuffer& operator << (float data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(float))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "넣다가 실패했어요");
			}
			memcpy(buf + writePos, &data, sizeof(float));
			writePos += sizeof(float);
			return *(this);
		}
		SerializableBuffer& operator << (__int64 data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(__int64))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "넣다가 실패했어요");
			}
			memcpy(buf + writePos, &data, sizeof(__int64));
			writePos += sizeof(__int64);
			return *(this);
		}
		SerializableBuffer& operator << (double data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(double))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "넣다가 실패했어요");
			}
			memcpy(buf + writePos, &data, sizeof(double));
			writePos += sizeof(double);
			return *(this);
		}
		SerializableBuffer& operator << (unsigned long long data) throw(PacketException)
		{
			if (GetFreeSize() < sizeof(unsigned long long))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "넣다가 실패했어요");
			}
			memcpy(buf + writePos, &data, sizeof(unsigned long long));
			writePos += sizeof(unsigned long long);
			return *(this);
		}
		SerializableBuffer& operator <<(stWCHAR& iValue) throw(PacketException)
		{
			if (GetFreeSize() < int(iValue.stringLength * 2))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "넣다가 실패했어요");
			}
			memcpy(buf + writePos,iValue.data, (int)(iValue.stringLength * 2));
			writePos += iValue.stringLength * 2;
			return *(this);
		}
		SerializableBuffer& operator <<(stCHAR& iValue) throw(PacketException)
		{
			if (GetFreeSize() < int(iValue.stringLength))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "넣다가 실패했어요");
			}
			memcpy(buf + writePos, iValue.data, int(iValue.stringLength) );
			writePos += iValue.stringLength ;
			return *(this);
		}

		SerializableBuffer& operator << (ISerialize& data) throw(PacketException)
		{
			if (GetFreeSize() < data.GetSerializeSize())
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsufficientInsertSpace, "넣다가 실패했어요");
			}
			if (data.ToSerialize(buf, writePos) == -1)
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfInsertDataInObject, "객체 데이터를 넣다가 실패했어요");
			}
			writePos += data.GetSerializeSize();
			return *(this);
		}
		SerializableBuffer& operator <<(SerializableBuffer& iValue) throw(PacketException)
		{
			if (GetFreeSize() < int(iValue.GetUseSize()))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "넣다가 실패했어요");
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
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData,"빼다가 실패했어요");
			}
			memcpy(&iValue, buf+ readPos,sizeof(BYTE));
			readPos += sizeof(BYTE);
			return *(this);
		}
		SerializableBuffer& operator >>(char& iValue)
		{
			if (GetUseSize() < sizeof(char))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
			}
			memcpy(&iValue, buf + readPos, sizeof(char));
			readPos += sizeof(char);
			return *(this);
		}
		SerializableBuffer& operator >>(short& iValue)
		{
			if (GetUseSize() < sizeof(short))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
			}
			memcpy(&iValue, buf + readPos, sizeof(short));
			readPos += sizeof(short);
			return *(this);
		}
		SerializableBuffer& operator >>(WORD& iValue)
		{
			if (GetUseSize() < sizeof(WORD))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
			}
			memcpy(&iValue, buf + readPos, sizeof(WORD));
			readPos += sizeof(WORD);
			return *(this);
		}
		SerializableBuffer& operator >>(int& iValue)
		{
			if (GetUseSize() < sizeof(int))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
			}
			memcpy(&iValue, buf + readPos, sizeof(int));
			readPos += sizeof(int);
			return *(this);
		}
		SerializableBuffer& operator >>(DWORD& iValue)
		{
			if (GetUseSize() < sizeof(DWORD))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
			}
			memcpy(&iValue, buf + readPos, sizeof(DWORD));
			readPos += sizeof(DWORD);
			return *(this);
		}
		SerializableBuffer& operator >>(float& iValue)
		{
			if (GetUseSize() < sizeof(float))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
			}
			memcpy(&iValue, buf + readPos, sizeof(float));
			readPos += sizeof(float);
			return *(this);
		}
		SerializableBuffer& operator >>(__int64& iValue)
		{
			if (GetUseSize() < sizeof(__int64))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
			}
			memcpy(&iValue, buf + readPos, sizeof(__int64));
			readPos += sizeof(__int64);
			return *(this);
		}
		SerializableBuffer& operator >>(double& iValue)
		{
			if (GetUseSize() < sizeof(double))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
			}
			memcpy(&iValue, buf + readPos, sizeof(double));
			readPos += sizeof(double);
			return *(this);
		}
		SerializableBuffer& operator >>(unsigned long long& iValue)
		{
			if (GetUseSize() < sizeof(unsigned long long))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
			}
			memcpy(&iValue, buf + readPos, sizeof(unsigned long long));
			readPos += sizeof(unsigned long long);
			return *(this);
		}
		SerializableBuffer& operator >>(stWCHAR &iValue)
		{
			if (GetUseSize() < int(iValue.stringLength * 2))
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
			}
			memcpy(iValue.data, buf + readPos, int(iValue.stringLength*2));
			readPos += iValue.stringLength * 2;;
			return *(this);
		}
		SerializableBuffer& operator >>(stCHAR& iValue)
		{
			if (GetUseSize() < int(iValue.stringLength) )
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
			}
			memcpy(iValue.data, buf + readPos, int(iValue.stringLength ));
			readPos += iValue.stringLength ;
			return *(this);
		}



		SerializableBuffer& operator >>(ISerialize& _data)
		{
			if (GetUseSize() < _data.GetSerializeSize())
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
			}
			if (_data.UnSerialize(buf, readPos) == false)
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfGetObject, "객체 데이터에 빼다가 실패했어요");
			}
			readPos += _data.GetSerializeSize();
			return *(this);
		}
		SerializableBuffer& operator >>(SerializableBuffer& iValue)
		{
			if (GetUseSize() < iValue.GetFreeSize())
			{
				throw SERIALIZATION_ERRTHROW(PacketException::EExceptionType::ErrorOfNoRemainData, "빼다가 실패했어요");
			}
			memcpy(&iValue, buf + readPos, iValue.GetFreeSize());
			readPos += iValue.GetFreeSize();
			return *(this);
		}
	};
