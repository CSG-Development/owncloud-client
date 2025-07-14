/**
* Copyright (c) 2015 Daniel Molkentin <danimo@owncloud.com>. All rights reserved.
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*/

#include <windows.h>
#include <Guiddef.h>
#include "CUContextMenuRegHandler.h"
#include "CUContextMenuFactory.h"

#include <algorithm>

// {841A0AAD-AA11-4B50-84D9-7F8E727D77D7}
static const GUID CLSID_FileContextMenuExt = { 0x841a0aad, 0xaa11, 0x4b50, { 0x84, 0xd9, 0x7f, 0x8e, 0x72, 0x7d, 0x77, 0xd7 } };

HINSTANCE   g_hInst = NULL;
long        g_cDllRef = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        // Hold the instance of this DLL module, we will use it to get the
        // path of the DLL to register the component.
        g_hInst = hModule;
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

	if (IsEqualCLSID(CLSID_FileContextMenuExt, rclsid))	{
		hr = E_OUTOFMEMORY;

		CUContextMenuFactory *pClassFactory = new CUContextMenuFactory();
		if (pClassFactory) {
			hr = pClassFactory->QueryInterface(riid, ppv);
			pClassFactory->Release();
		}
	}

	return hr;
}

STDAPI DllCanUnloadNow(void)
{
	return g_cDllRef > 0 ? S_FALSE : S_OK;
}

STDAPI DllRegisterServer(void)
{
	HRESULT hr;

	wchar_t szModule[MAX_PATH];
	if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	// Register the component.
	hr = CUContextMenuRegHandler::RegisterInprocServer(szModule, CLSID_FileContextMenuExt,
		L"CUContextMenuHandler Class",	L"Apartment");
	if (SUCCEEDED(hr))	{
		// Register the context menu handler. The context menu handler is 
		// associated with the .cpp file class.
		hr = CUContextMenuRegHandler::RegisterShellExtContextMenuHandler(L"AllFileSystemObjects", CLSID_FileContextMenuExt, L"CUContextMenuHandler");
	}

	return hr;
}

STDAPI DllUnregisterServer(void)
{
	HRESULT hr = S_OK;

	wchar_t szModule[MAX_PATH];
	if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	// Unregister the component.
	hr = CUContextMenuRegHandler::UnregisterInprocServer(CLSID_FileContextMenuExt);
	if (SUCCEEDED(hr))	{
		// Unregister the context menu handler.
		hr = CUContextMenuRegHandler::UnregisterShellExtContextMenuHandler(L"AllFileSystemObjects", L"CUContextMenuHandler");
	}

	return hr;
}
