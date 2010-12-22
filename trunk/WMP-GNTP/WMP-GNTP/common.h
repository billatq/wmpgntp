// Copyright (c) 2010 William Reading
// Available under the terms of the Microsoft Public License (Ms-PL) 

#pragma once

#include "targetver.h"

#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <atlenc.h>

extern CComModule _Module;

#include <atlcom.h>
#include <ocidl.h>

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#endif

#include <shlobj.h>
#include <wininet.h>