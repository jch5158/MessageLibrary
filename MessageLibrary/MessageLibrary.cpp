
#include "CMessage.h"

int wmain()
{
	CMessage* pMessage = CMessage::Alloc();

	pMessage->Free();

}
