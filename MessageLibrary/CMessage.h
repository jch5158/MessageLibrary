#pragma once

#include <iostream>
#include <Windows.h>

#include "DumpLibrary/DumpLibrary/CCrashDump.h"
#include "SystemLogLibrary/SystemLogLibrary/CSystemLog.h"
#include "CMessage.h"
//#include "LockFreeObjectFreeList/ObjectFreeListLib/CLockFreeObjectFreeList.h"
#include "LockFreeObjectFreeList/ObjectFreeListLib/CTLSLockFreeObjectFreeList.h"

template <class DATA>
class CLockFreeObjectFreeList;

class CExceptionObject
{
public:

	CExceptionObject(char* buffer, int bufferSize, const WCHAR* data)
	{	
		mBufferSize = bufferSize;

		mpMessageLog = buffer;

		wcscpy_s(mErrorTextLog, _countof(mErrorTextLog), data);
	
	}

	CExceptionObject(const CExceptionObject &exceptionObject)
	{
		mBufferSize = exceptionObject.mBufferSize;

		mpMessageLog = exceptionObject.mpMessageLog;

		wcscpy_s(mErrorTextLog, _countof(mErrorTextLog), exceptionObject.mErrorTextLog);
	}

	~CExceptionObject()
	{

	}

	void PrintExceptionData(void)
	{		
		CSystemLog::GetInstance()->LogHex(true, CSystemLog::eLogLevel::LogLevelError, L"ExceptionObject", (const WCHAR*)mErrorTextLog, (BYTE*)mpMessageLog, mBufferSize);
	}

private:

	 int mBufferSize;

	 char *mpMessageLog;

	 WCHAR mErrorTextLog[200];
};


class CMessage
{
private:

	#pragma pack(1)
	struct stLanHeader
	{
		WORD payloadSize;
	};

	struct stNetHeader
	{
		BYTE code;
		WORD payloadSize;
		BYTE randomKey;
		BYTE checkSum;
	};
	#pragma pack()

public:

	// 헤더 셋팅을 위한 friend
	friend class CNetClient;

	friend class CNetServer;

	// CObjectFreeList에서 생성자 및 소멸자를 위한 friend
	template <class DATA>
	friend class CLockFreeObjectFreeList;

	template <class DATA>
	friend class CTLSLockFreeObjectFreeList;

	template <class DATA>
	friend class CChunk;

	inline void Release(void)
	{
		free(mpBufferPtr);
	}

	inline void Clear(void)
	{
		//mReferenceCount = 0;
		mbEndcodingFlag = false;
		mFront = -1;
		mRear = -1;
		mDataSize = 0;
		mpHeaderPtr = mpBufferPtr + sizeof(stNetHeader);
	}

	inline int GetBufferSize(void)
	{
		return mBufferLen;
	}

	inline int GetUseSize(void)
	{
		return mDataSize;
	}

	inline char* GetMessagePtr(void)
	{
		return mpHeaderPtr;
	}

	inline int MoveWritePos(int iSize)
	{
		mDataSize += iSize;

		mRear += iSize;

		return iSize;
	}

	inline int MoveReadPos(int iSize)
	{
		mDataSize -= iSize;

		mFront += iSize;

		return iSize;
	}

	inline int GetPayload(char* chpDest, int iSize)
	{
		memcpy(chpDest, &mpPayloadPtr[mFront + 1], iSize);

		return iSize;
	}

	inline int PutPayload(char* chpSrc, int iSize)
	{
		memcpy(&mpPayloadPtr[mRear + 1], chpSrc, iSize);

		return iSize;
	}

	inline void AddReferenceCount(void)
	{
		InterlockedIncrement(&mReferenceCount);
	}

	inline long SubReferenceCount(void)
	{
		return InterlockedDecrement(&mReferenceCount);
	}

	inline CMessage& operator << (bool value)
	{
		mpPayloadPtr[mRear + 1] = value;

		mRear += sizeof(bool);

		mDataSize += sizeof(bool);

		return *this;
	}


	inline CMessage& operator << (char value)
	{
		mpPayloadPtr[mRear + 1] = value;

		mRear += sizeof(char);

		mDataSize += sizeof(char);

		return *this;
	}

	inline CMessage& operator << (unsigned char value)
	{
		mpPayloadPtr[mRear + 1] = (unsigned char)value;

		mRear += sizeof(unsigned char);

		mDataSize += sizeof(unsigned char);

		return *this;
	}

	//CMessage& operator << (BYTE value)
	//{
	//	mpPayloadPtr[mRear + 1] = (BYTE)value;

	//	mRear += sizeof(BYTE);

	//	mDataSize += sizeof(BYTE);

	//	return *this;
	//}

	inline CMessage& operator << (short value)
	{
		*((short*)&mpPayloadPtr[mRear + 1]) = value;

		mRear += sizeof(short);

		mDataSize += sizeof(short);

		return *this;
	}

	inline CMessage& operator << (unsigned short value)
	{
		*((unsigned short*)&mpPayloadPtr[mRear + 1]) = value;

		mRear += sizeof(unsigned short);

		mDataSize += sizeof(unsigned short);

		return *this;
	}

	//CMessage& operator << (WORD value)
	//{
	//	*((WORD*)&mpPayloadPtr[mRear + 1]) = value;

	//	mRear += sizeof(WORD);

	//	mDataSize += sizeof(WORD);

	//	return *this;
	//}

	inline CMessage& operator << (int value)
	{		
		*((int*)&mpPayloadPtr[mRear + 1]) = value;

		mRear += sizeof(int);

		mDataSize += sizeof(int);

		return *this;
	}

	inline CMessage& operator << (unsigned int value)
	{	
		*((unsigned int*)&mpPayloadPtr[mRear + 1]) = value;

		mRear += sizeof(unsigned int);

		mDataSize += sizeof(unsigned int);

		return *this;
	}

	inline CMessage& operator << (long value)
	{
		*((long*)&mpPayloadPtr[mRear + 1]) = value;

		mRear += sizeof(long);

		mDataSize += sizeof(long);

		return *this;
	}

	inline CMessage& operator << (unsigned long value)
	{		
		*((unsigned long*)&mpPayloadPtr[mRear + 1]) = value;

		mRear += sizeof(long);

		mDataSize += sizeof(long);

		return *this;
	}

	//CMessage& operator << (DWORD value)
	//{
	//	*((DWORD*)&mpPayloadPtr[mRear + 1]) = value;

	//	mRear += sizeof(DWORD);

	//	mDataSize += sizeof(DWORD);

	//	return *this;
	//}

	inline CMessage& operator << (long long value)
	{
		*((long long*)&mpPayloadPtr[mRear + 1]) = value;

		mRear += sizeof(long long);

		mDataSize += sizeof(long long);

		return *this;
	}

	//CMessage& operator << (INT64 value)
	//{
	//	*((INT64*)&mpPayloadPtr[mRear + 1]) = value;

	//	mRear += sizeof(INT64);

	//	mDataSize += sizeof(INT64);

	//	return *this;
	//}

	inline CMessage& operator << (unsigned long long value)
	{
		*((unsigned long long*) & mpPayloadPtr[mRear + 1]) = value;

		mRear += sizeof(unsigned long long);

		mDataSize += sizeof(unsigned long long);

		return *this;
	}


	//CMessage& operator << (UINT64 value)
	//{
	//	*((UINT64*) & mpPayloadPtr[mRear + 1]) = value;

	//	mRear += sizeof(UINT64);

	//	mDataSize += sizeof(UINT64);

	//	return *this;
	//}

	inline CMessage& operator >> (bool& value)
	{
		if (mDataSize < sizeof(bool))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" char\n");

			throw exception;
		}

		value = *((bool*)&mpPayloadPtr[mFront + 1]);

		mFront += sizeof(bool);

		mDataSize -= sizeof(bool);

		return *this;
	}


	inline CMessage& operator >> (char& value)
	{
		if (mDataSize < sizeof(char))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" char\n");

			throw exception;
		}

		value = *((char*)&mpPayloadPtr[mFront + 1]);

		mFront += sizeof(char);

		mDataSize -= sizeof(char);

		return *this;
	}

	inline CMessage& operator >> (unsigned char& value)
	{
		if (mDataSize < sizeof(unsigned char))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" unsigned char\n");

			throw exception;
		}

		value = *((unsigned char*)&mpPayloadPtr[mFront + 1]);

		mFront += sizeof(char);

		mDataSize -= sizeof(char);

		return *this;
	}

	inline CMessage& operator >> (short& value)
	{
		if (mDataSize < sizeof(short))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" short\n");

			throw exception;
		}

		value = *((short*)&mpPayloadPtr[mFront + 1]);

		mFront += sizeof(short);

		mDataSize -= sizeof(short);

		return *this;
	}

	inline CMessage& operator >> (unsigned short& value)
	{
		if (mDataSize < sizeof(unsigned short))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" unsigned short\n");

			throw exception;
		}

		value = *((unsigned short*)&mpPayloadPtr[mFront + 1]);

		mFront += sizeof(short);

		mDataSize -= sizeof(unsigned short);

		return *this;
	}

	inline CMessage& operator >> (int& value)
	{
		if (mDataSize < sizeof(int))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" int\n");

			throw exception;
		}

		value = *((int*)&mpPayloadPtr[mFront + 1]);

		mFront += sizeof(int);

		mDataSize -= sizeof(int);

		return *this;
	}

	inline CMessage& operator >> (unsigned int& value)
	{
		if (mDataSize < sizeof(unsigned int))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" unsigned int\n");

			throw exception;
		}

		value = *((unsigned int*)&mpPayloadPtr[mFront + 1]);

		mFront += sizeof(int);

		mDataSize -= sizeof(int);

		return *this;
	}

	inline CMessage& operator >> (long& value)
	{

		if (mDataSize < sizeof(long))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" long\n");

			throw exception;
		}

		value = *((long*)&mpPayloadPtr[mFront + 1]);

		mFront += sizeof(long);

		mDataSize -= sizeof(long);

		return *this;
	}

	inline CMessage& operator >> (unsigned long& value)
	{
		if (mDataSize < sizeof(unsigned long))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" unsigned long\n");

			throw exception;
		}

		value = *((unsigned long*)&mpPayloadPtr[mFront + 1]);

		mFront += sizeof(long);

		mDataSize -= sizeof(long);

		return *this;
	}

	inline CMessage& operator >> (long long& value)
	{
		if (mDataSize < sizeof(long long))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" long long\n");

			throw exception;
		}

		value = *((long long*)&mpPayloadPtr[mFront + 1]);

		mFront += sizeof(long long);

		mDataSize -= sizeof(long long);

		return *this;
	}

	inline CMessage& operator >> (unsigned long long& value)
	{
		if (mDataSize < sizeof(unsigned long long))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" unsigned long long\n");

			throw exception;
		}

		value = *((unsigned long long*)&mpPayloadPtr[mFront + 1]);

		mFront += sizeof(unsigned long long);

		mDataSize -= sizeof(unsigned long long);

		return *this;
	}

	
	static inline CMessage* Alloc(void)
	{
		//CMessage* pMessage = mMessageFreeList.Alloc();

		CMessage* pMessage = mTlsMessageFreeList.Alloc();

		pMessage->Clear();

		pMessage->AddReferenceCount();

		return pMessage;
	}


	inline bool Free()
	{
		if (SubReferenceCount() == 0)
		{
			//if (mMessageFreeList.Free(this) == false)				

			if (mTlsMessageFreeList.Free(this) == false)
			{
				return false;
			}
		}

		return true;
	}



private:

	CMessage(int size = 2000)
		: mbEndcodingFlag(false)
		, mFront(-1)
		, mRear(-1)
		, mDataSize(0)
		, mBufferLen(size)
		, mReferenceCount(0)
		, mpBufferPtr(nullptr)
		, mpHeaderPtr(nullptr)
		, mpPayloadPtr(nullptr)
	{				
		mpBufferPtr = (char*)malloc(size);

		mpHeaderPtr = mpBufferPtr + sizeof(stNetHeader);

		mpPayloadPtr = mpHeaderPtr;
	}

	~CMessage()
	{
		Release();
	}

	//static void SetStaticKey(BYTE staticKey)
	//{
	//	mStaticKey = staticKey;
	//}

	//static void SetHeaderCode(BYTE headerCode)
	//{
	//	mHeaderCode = headerCode;
	//}



	inline void encode(void)
	{
		if (mbEndcodingFlag == true)
		{
			return;
		}

		stNetHeader netHeader;

		netHeader.code = mHeaderCode;
		netHeader.payloadSize = mDataSize;
		netHeader.randomKey = rand();
		netHeader.checkSum = makeCheckSum();

		setNetHeader(&netHeader);

		BYTE encodeKey = NULL;
		BYTE encodeData = NULL;

		BYTE* pEncodeData = (BYTE*)(mpPayloadPtr - 1);

		int length = mDataSize - 4;
		for (int index = 0; index < length; ++index)
		{
			encodeKey = pEncodeData[index] ^ (encodeKey + netHeader.randomKey + index + 1);

			encodeData = encodeKey ^ (encodeData + mStaticKey + index + 1);

			pEncodeData[index] = encodeData;
		}

		mbEndcodingFlag = true;
	}


	inline bool decode(void)
	{
		BYTE randomKey =  ((stNetHeader*)GetMessagePtr())->randomKey;

		BYTE decodeKey = NULL;

		BYTE encodeKey = NULL;

		BYTE decodeData = NULL;

		BYTE encodeData = NULL;

		BYTE* pDecodeData = (BYTE*)(mpPayloadPtr - 1);

		int length = mDataSize - 4;
		for (int index = 0; index < length; ++index)
		{
			decodeKey = pDecodeData[index] ^ (encodeKey + randomKey + index + 1);
			decodeData = decodeKey ^ (encodeData + mStaticKey + index + 1);

			pDecodeData[index] = decodeData;

			encodeKey = decodeData ^ (encodeKey + randomKey + index + 1);
			encodeData = encodeKey ^ (encodeData + mStaticKey + index + 1);
		}
		
		removeNetHeader();

		if (pDecodeData[0] != makeCheckSum())
		{
			return false;
		}

		return true;
	}


	inline bool decode(stNetHeader* pNetHeader)
	{
		BYTE randomKey = pNetHeader->randomKey;

		BYTE decodeKey = NULL;

		BYTE encodeKey = NULL;

		BYTE decodeData = NULL;

		BYTE encodeData = NULL;

		decodeKey = pNetHeader->checkSum ^ (encodeKey + randomKey + 1);
		decodeData = decodeKey ^ (encodeData + mStaticKey + 1);

		pNetHeader->checkSum = decodeData;

		encodeKey = decodeData ^ (encodeKey + randomKey + 1);
		encodeData = encodeKey ^ (encodeData + mStaticKey + 1);

		int length = mDataSize;
		for (int index = 0; index < length; ++index)
		{
			decodeKey = mpPayloadPtr[index] ^ (encodeKey + randomKey + index + 2);
			decodeData = decodeKey ^ (encodeData + mStaticKey + index + 2);

			mpPayloadPtr[index] = decodeData;

			encodeKey = decodeData ^ (encodeKey + randomKey + index + 2);
			encodeData = encodeKey ^ (encodeData + mStaticKey + index + 2);
		}

		if (pNetHeader->checkSum != makeCheckSum())
		{
			return false;
		}

		return true;
	}

	inline BYTE makeCheckSum(void)
	{
		int checkSum = 0;
		
		for (int offset = 0; offset < mDataSize; ++offset)
		{
			checkSum += *(mpPayloadPtr + offset);
		}
		
		return checkSum & 0x000000ff;
	}

	inline void setLanHeader(stLanHeader* pLanHeader)
	{			
		mpHeaderPtr -= sizeof(stLanHeader);

		*((stLanHeader*)mpHeaderPtr) = *pLanHeader;

		mDataSize += sizeof(stLanHeader);
	}

	inline void setNetHeader(stNetHeader* pNetHeader)
	{
		mpHeaderPtr -= sizeof(stNetHeader);

		*((stNetHeader*)mpHeaderPtr) = *pNetHeader;

		mDataSize += sizeof(stNetHeader);
	}	

	inline void removeLanHeader()
	{
		mpHeaderPtr += sizeof(stLanHeader);

		mDataSize -= sizeof(stLanHeader);
	}

	inline void removeNetHeader()
	{
		mpHeaderPtr += sizeof(stNetHeader);

		mDataSize -= sizeof(stNetHeader);
	}

	//inline static CLockFreeObjectFreeList<CMessage> mMessageFreeList = { 0,false };

	inline static CTLSLockFreeObjectFreeList<CMessage> mTlsMessageFreeList = { 100,false };
	
	inline static BYTE mHeaderCode = 0;

	inline static BYTE mStaticKey = 0;

	bool mbEndcodingFlag;	

	int mFront;

	int mRear;

	int mBufferLen;

	int mDataSize;

	long mReferenceCount;

	// 헤더 이후 데이터 부터 가리킴
	char* mpPayloadPtr;

	// 메시지의 헤더를 가리키는 포인터입니다.
	char* mpHeaderPtr;

	// 동적할당 받은 버퍼 가리키는 포인터
	char* mpBufferPtr;
};
