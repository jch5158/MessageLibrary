#pragma once

#pragma comment(lib, "DbgHelp.Lib")

#include <iostream>
#include <Windows.h>

#include <psapi.h>
#include <dbghelp.h>

#include "DumpLibrary/DumpLibrary/CCrashDump.h"
#include "CMessage.h"
#include "LockFreeObjectFreeList/ObjectFreeListLib/CLockFreeObjectFreeList.h"
#include "LockFreeObjectFreeList/ObjectFreeListLib/CTLSLockFreeObjectFreeList.h"