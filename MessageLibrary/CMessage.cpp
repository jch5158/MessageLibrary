#include "stdafx.h"


SRWLOCK CExceptionObject::mSrwLock;

CLockFreeObjectFreeList<CMessage> CMessage::mMessageFreeList(0, false);

CTLSLockFreeObjectFreeList<CMessage> CMessage::mTlsMessageFreeList(0, false);

CMessage* CMessage::Alloc()
{	
	//CMessage* pMessage = mMessageFreeList.Alloc();

	CMessage* pMessage = mTlsMessageFreeList.Alloc();

	pMessage->Clear();

	pMessage->AddReferenceCount();

	return pMessage;
}


bool CMessage::Free()
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