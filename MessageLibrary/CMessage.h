#pragma once

template <class DATA>
class CLockFreeObjectFreeList;

class CExceptionObject
{
public:

	CExceptionObject(char* buffer, int bufferSize, const WCHAR* data)
	{	
		mBufferSize = bufferSize;

		memcpy_s(mMessageLog, sizeof(mMessageLog), buffer, bufferSize);

		memcpy_s(mErrorDataLog, sizeof(mErrorDataLog), data, wcslen(data) * 2);
	}

	CExceptionObject(const CExceptionObject &exceptionObject)
	{
		mBufferSize = exceptionObject.mBufferSize;

		memcpy_s(mMessageLog, sizeof(mMessageLog), exceptionObject.mMessageLog, exceptionObject.mBufferSize);

		memcpy_s(mErrorDataLog, sizeof(mErrorDataLog), exceptionObject.mErrorDataLog, wcslen(exceptionObject.mErrorDataLog) * 2);
	}

	~CExceptionObject()
	{

	}

	void PrintExceptionData(void)
	{		
		AcquireSRWLockExclusive(&mSrwLock);

		FILE* fp = nullptr;

		// 에러 파일 오푼
		_wfopen_s(&fp, L"ErrorDump.txt", L"a+t");
		if (fp != nullptr)
		{
			for (int iCnt = 0; iCnt < mBufferSize; ++iCnt)
			{
				// 메시지 로그
				fprintf_s(fp, "%02x ", mMessageLog[iCnt]);
			}

			// 에러 함수의 인자 데이터 타입 
			fwprintf_s(fp, mErrorDataLog);

			fclose(fp);
		}

		ReleaseSRWLockExclusive(&mSrwLock);
	}


public:

	static SRWLOCK mSrwLock;

private:

	int mBufferSize;

	 char mMessageLog[2000] = { 0, };

	 WCHAR mErrorDataLog[200] = { 0, };

};


class  CMessage
{
private:

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

public:

	// 헤더 셋팅을 위한 friend
	friend class CNetServer;

	// 헤더 셋팅을 위한 friend
	friend class CLanServer;

	// CObjectFreeList에서 생성자 및 소멸자를 위한 friend
	template <class DATA>
	friend class CLockFreeObjectFreeList;

	template <class DATA>
	friend class CTLSLockFreeObjectFreeList;

	template <class DATA>
	friend class CChunk;

	void Release(void)
	{
		free(mpBufferPtr);
	}

	void Clear(void)
	{
		mFront = -1;
		mRear = -1;
		mDataSize = 0;
		mpHeaderPtr = mpBufferPtr + sizeof(stNetHeader);
	}

	int GetBufferSize(void)
	{
		return mBufferLen;
	}

	int GetUseSize(void)
	{
		return mDataSize;
	}

	char* GetMessagePtr(void)
	{
		return mpHeaderPtr;
	}

	int MoveWritePos(int iSize)
	{
		mDataSize += iSize;

		mRear += iSize;

		return iSize;
	}

	int MoveReadPos(int iSize)
	{
		mDataSize -= iSize;

		mFront += iSize;

		return iSize;
	}

	int GetPayload(char* chpDest, int iSize)
	{
		memcpy(chpDest, &mpPayloadPtr[mFront + 1], iSize);

		return iSize;
	}

	int PutPayload(char* chpSrc, int iSize)
	{
		memcpy(&mpPayloadPtr[mRear + 1], chpSrc, iSize);

		return iSize;
	}

	void AddReferenceCount(void)
	{
		InterlockedIncrement(&mReferenceCount);
	}

	long SubReferenceCount(void)
	{
		return InterlockedDecrement(&mReferenceCount);
	}

	CMessage& operator << (char chValue)
	{
		mpPayloadPtr[mRear + 1] = chValue;

		mRear += sizeof(char);

		mDataSize += sizeof(char);

		return *this;
	}

	CMessage& operator << (unsigned char uchValue)
	{
		mpPayloadPtr[mRear + 1] = (unsigned char)uchValue;

		mRear += sizeof(char);

		mDataSize += sizeof(char);

		return *this;
	}

	CMessage& operator << (short sValue)
	{
		short* shortBuffer = (short*)&mpPayloadPtr[mRear + 1];

		*shortBuffer = sValue;

		mRear += sizeof(short);

		mDataSize += sizeof(short);

		return *this;
	}

	CMessage& operator << (unsigned short usValue)
	{
		unsigned short* unShortBuffer = (unsigned short*)&mpPayloadPtr[mRear + 1];

		*unShortBuffer = usValue;

		mRear += sizeof(short);

		mDataSize += sizeof(short);

		return *this;
	}

	CMessage& operator << (int iValue)
	{
		int* intBuffer = (int*)&mpPayloadPtr[mRear + 1];

		*intBuffer = iValue;

		mRear += sizeof(int);

		mDataSize += sizeof(int);

		return *this;
	}

	CMessage& operator << (unsigned int uiValue)
	{
		int* unIntBuffer = (int*)&mpPayloadPtr[mRear + 1];

		*unIntBuffer = uiValue;

		mRear += sizeof(int);

		mDataSize += sizeof(int);

		return *this;
	}

	CMessage& operator << (long lValue)
	{
		long* longBuffer = (long*)&mpPayloadPtr[mRear + 1];

		*longBuffer = lValue;

		mRear += sizeof(long);

		mDataSize += sizeof(long);

		return *this;
	}

	CMessage& operator << (unsigned long ulValue)
	{
		unsigned long* unLongBuffer = (unsigned long*)&mpPayloadPtr[mRear + 1];

		*unLongBuffer = ulValue;

		mRear += sizeof(long);

		mDataSize += sizeof(long);

		return *this;
	}

	CMessage& operator << (long long llValue)
	{
		long long* llBuffer = (long long*)&mpPayloadPtr[mRear + 1];

		*llBuffer = llValue;

		mRear += sizeof(long long);

		mDataSize += sizeof(long long);

		return *this;
	}

	CMessage& operator << (unsigned long long ullValue)
	{
		unsigned long long* ullBuffer = (unsigned long long*)&mpPayloadPtr[mRear + 1];

		*ullBuffer = ullValue;

		mRear += sizeof(unsigned long long);

		mDataSize += sizeof(unsigned long long);

		return *this;
	}



	CMessage& operator >> (char& chValue)
	{
		if (mDataSize < sizeof(chValue))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" char\n");

			throw exception;
		}

		chValue = *(char*)&mpPayloadPtr[mFront + 1];

		mFront += sizeof(char);

		mDataSize -= sizeof(char);

		return *this;
	}

	CMessage& operator >> (unsigned char& uchValue)
	{
		if (mDataSize < sizeof(uchValue))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" unsigned char\n");

			throw exception;
		}

		uchValue = *(unsigned char*)&mpPayloadPtr[mFront + 1];

		mFront += sizeof(char);

		mDataSize -= sizeof(char);

		return *this;
	}

	CMessage& operator >> (short& sValue)
	{

		if (mDataSize < sizeof(sValue))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" short\n");

			throw exception;
		}

		sValue = *(short*)&mpPayloadPtr[mFront + 1];

		mFront += sizeof(short);

		mDataSize -= sizeof(short);

		return *this;
	}

	CMessage& operator >> (unsigned short& usValue)
	{
		if (mDataSize < sizeof(usValue))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" unsigned short\n");

			throw exception;
		}

		usValue = *(unsigned short*)&mpPayloadPtr[mFront + 1];

		mFront += sizeof(short);

		mDataSize -= sizeof(unsigned short);

		return *this;
	}

	CMessage& operator >> (int& iValue)
	{

		if (mDataSize < sizeof(iValue))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" int\n");

			throw exception;
		}

		iValue = *(int*)&mpPayloadPtr[mFront + 1];

		mFront += sizeof(int);

		mDataSize -= sizeof(int);

		return *this;
	}

	CMessage& operator >> (unsigned int& uiValue)
	{
		if (mDataSize < sizeof(uiValue))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" unsigned int\n");

			throw exception;
		}

		uiValue = *(unsigned int*)&mpPayloadPtr[mFront + 1];

		mFront += sizeof(int);

		mDataSize -= sizeof(int);

		return *this;
	}

	CMessage& operator >> (long& lValue)
	{

		if (mDataSize < sizeof(lValue))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" long\n");

			throw exception;
		}

		lValue = *(long*)&mpPayloadPtr[mFront + 1];

		mFront += sizeof(long);

		mDataSize -= sizeof(long);

		return *this;
	}

	CMessage& operator >> (unsigned long& ulValue)
	{

		if (mDataSize < sizeof(ulValue))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" unsigned long\n");

			throw exception;
		}

		ulValue = *(unsigned long*)&mpPayloadPtr[mFront + 1];

		mFront += sizeof(long);

		mDataSize -= sizeof(long);

		return *this;
	}

	CMessage& operator >> (long long& llValue)
	{
		if (mDataSize < sizeof(long long))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" long long\n");

			throw exception;
		}

		llValue = *(long long*)&mpPayloadPtr[mFront + 1];

		mFront += sizeof(long long);

		mDataSize -= sizeof(long long);

		return *this;
	}

	CMessage& operator >> (unsigned long long& ullValue)
	{
		if (mDataSize < sizeof(unsigned long long))
		{
			CExceptionObject exception(mpPayloadPtr, mRear + 1, L" unsigned long long\n");

			throw exception;
		}

		ullValue = *(unsigned long long*)&mpPayloadPtr[mFront + 1];

		mFront += sizeof(unsigned long long);

		mDataSize -= sizeof(unsigned long long);

		return *this;
	}

	static CMessage* Alloc();

	bool Free();

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

	void endcoding()
	{
		if (mbEndcodingFlag == true)
		{
			return;
		}

		stNetHeader netHeader;

		netHeader.code = 0x89;
		netHeader.payloadSize = mDataSize;
		netHeader.randomKey = rand();
		netHeader.checkSum = makeCheckSum();

		setNetHeader(&netHeader);

		BYTE encodeKey = 0;
		BYTE encodeData = 0;

		BYTE* pEncodeData = (BYTE*)(mpPayloadPtr - 1);

		for (int index = 0; index < mDataSize + 1; ++index)
		{
			encodeKey = pEncodeData[index] ^ (encodeKey + netHeader.randomKey + index + 1);

			encodeData = encodeKey ^ (encodeData + mStaticKey + index + 1);

			pEncodeData[index] = encodeData;
		}

		mbEndcodingFlag = true;
	}

	bool decoding()
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

		if (pDecodeData[0] != makeCheckSum())
		{
			return false;
		}

		return true;

	}

	bool decoding(stNetHeader* pNetHeader)
	{
		BYTE randomKey = pNetHeader->randomKey;

		BYTE decodeKey = NULL;

		BYTE encodeKey = NULL;

		BYTE decodeData = NULL;

		BYTE encodeData = NULL;

		BYTE* pDecodeData = (BYTE*)(mpPayloadPtr - 1);

		int length = mDataSize + 1;
		for (int index = 0; index < length; ++index)
		{
			decodeKey = pDecodeData[index] ^ (encodeKey + randomKey + index + 1);
			decodeData = decodeKey ^ (encodeData + mStaticKey + index + 1);

			pDecodeData[index] = decodeData;

			encodeKey = decodeData ^ (encodeKey + randomKey + index + 1);
			encodeData = encodeKey ^ (encodeData + mStaticKey + index + 1);
		}

		if (pDecodeData[0] != makeCheckSum())
		{
			return false;
		}

		return true;
	}

	BYTE makeCheckSum()
	{
		BYTE checkSum = 0;
		
		for (int offset = 0; offset < mDataSize; ++offset)
		{
			checkSum += *(mpPayloadPtr + offset);
		}

		return checkSum;
	}

	void setLanHeader(stLanHeader* pLanHeader)
	{			
		mpHeaderPtr -= sizeof(stLanHeader);

		*((stLanHeader*)mpHeaderPtr) = *pLanHeader;

		mDataSize += sizeof(stLanHeader);
	}

	void setNetHeader(stNetHeader* pNetHeader)
	{
		mpHeaderPtr -= sizeof(pNetHeader);

		*((stNetHeader*)mpHeaderPtr) = *pNetHeader;

		mDataSize += sizeof(stNetHeader);
	}	

	static CLockFreeObjectFreeList<CMessage> mMessageFreeList;

	static CTLSLockFreeObjectFreeList<CMessage> mTlsMessageFreeList;
	
	const static BYTE mStaticKey = 0xa9;

	bool mbEndcodingFlag;
	
	int mBufferLen;

	int mFront;

	int mRear;

	int mDataSize;

	long mReferenceCount;

	// 헤더 이후 데이터 부터 가리킴
	char* mpPayloadPtr;

	// 메시지의 헤더를 가리키는 포인터입니다.
	char* mpHeaderPtr;

	// 동적할당 받은 버퍼 가리키는 포인터
	char* mpBufferPtr;
};
