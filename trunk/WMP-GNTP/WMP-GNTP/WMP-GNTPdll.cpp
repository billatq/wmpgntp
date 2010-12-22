// Copyright (c) 2010 William Reading
// Available under the terms of the Microsoft Public License (Ms-PL) 

#include "common.h"
#include "resource.h"
#include <initguid.h>

#include "WMP-GNTP.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_WMPGNTP, CWMPGNTP)
END_OBJECT_MAP()

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;    // ok
}

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _Module.RegisterServer();

    // Notify WMP that plugin has been added
    WMPNotifyPluginAddRemove();

    return hr;
}


STDAPI DllUnregisterServer(void)
{
    HRESULT hr = _Module.UnregisterServer();

    // Notify WMP that plugin has been removed
    WMPNotifyPluginAddRemove();

    return hr;
}


