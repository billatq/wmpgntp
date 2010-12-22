// Copyright (c) 2010 William Reading
// Available under the terms of the Microsoft Public License (Ms-PL) 

#include "Util.h"

#pragma pack(8)
struct AtomHeader
{
	DWORD length;
	char type[4];
};

CStringA Util::GetUTF8String(const CString &WideString)
{
	CStringA strUTF8;
	int iChars = AtlUnicodeToUTF8(WideString, WideString.GetLength(), NULL, 0);
	if (iChars > 0)
	{
		LPSTR pszUTF8 = strUTF8.GetBuffer(iChars);
		AtlUnicodeToUTF8(WideString, WideString.GetLength(), pszUTF8, iChars);
		strUTF8.ReleaseBuffer(iChars);
	}
	return strUTF8;
}


// Note: This function will convert UTF-16 strings to UTF-8.
// That may be undesireable depending on application.
// Adapted from MSDN: http://msdn.microsoft.com/en-us/library/aa382380(VS.85).aspx
// TODO: Refactor error handling to be cleaner
HRESULT Util::MD5HashString(const CString &InputString, CString &HashOutput)
{
	HashOutput.Empty();

	static const int MD5LEN = 16;
	HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
	BYTE rgbHash[MD5LEN];
	DWORD cbHash = MD5LEN;
	CHAR rgbDigits[] = "0123456789abcdef";
	DWORD dwStatus;

	// Get handle to the crypto provider
    if (!CryptAcquireContext(&hProv,
        NULL,
        NULL,
        PROV_RSA_FULL,
        CRYPT_VERIFYCONTEXT))
    {
		return HRESULT_FROM_WIN32(GetLastError());
    }

	// Create new MD5 Hash
	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    {
        dwStatus = GetLastError();
        CryptReleaseContext(hProv, 0);
        return HRESULT_FROM_WIN32(dwStatus);
    }

	// Convert input to UTF8 and use buffer for hash
	CStringA inputString = GetUTF8String(InputString);

    if (!CryptHashData(
		hHash,
		(const BYTE *)inputString.GetBuffer(inputString.GetLength()),
		inputString.GetLength() * sizeof(const char),
		0))
    {
        dwStatus = GetLastError();
        CryptReleaseContext(hProv, 0);
        CryptDestroyHash(hHash);
		return HRESULT_FROM_WIN32(dwStatus);
    }

	// Convert hash to a string value
	if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
    {
		CString outputString;
		for (DWORD i = 0; i < cbHash; i++)
		{
			outputString += rgbDigits[rgbHash[i] >> 4];
			outputString += rgbDigits[rgbHash[i] & 0xf];
		}
		HashOutput = outputString;
	}
	else
	{
		dwStatus = GetLastError(); 
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return HRESULT_FROM_WIN32(dwStatus);
	}

	CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

	return S_OK;
}

// Adapted from MSDN: http://msdn.microsoft.com/en-us/library/aa382380(VS.85).aspx
// TODO: Refactor error handling to be cleaner
HRESULT Util::MD5HashFile(const CString &InputFilePath, CString &HashOutput)
{
	HashOutput.Empty();

	static const int MD5LEN = 16;
	static const int BUFSIZE = 1024;
	BOOL bResult = FALSE;
	BYTE rgbFile[BUFSIZE];
	HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
	BYTE rgbHash[MD5LEN];
	DWORD cbHash = MD5LEN;
	DWORD cbRead = 0;
	HANDLE hFile = NULL;
	CHAR rgbDigits[] = "0123456789abcdef";
	DWORD dwStatus;

	// Get handle to file
	hFile = CreateFile(InputFilePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);

	if (INVALID_HANDLE_VALUE == hFile)
    {
        dwStatus = GetLastError();
        return HRESULT_FROM_WIN32(dwStatus);
    }

	// Get handle to the crypto provider
    if (!CryptAcquireContext(&hProv,
        NULL,
        NULL,
        PROV_RSA_FULL,
        CRYPT_VERIFYCONTEXT))
    {
		CloseHandle(hFile);
		return HRESULT_FROM_WIN32(GetLastError());
    }

	// Create new MD5 Hash
	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    {
        dwStatus = GetLastError();
        CryptReleaseContext(hProv, 0);
		CloseHandle(hFile);
        return HRESULT_FROM_WIN32(dwStatus);
    }

	// Hash through file in 1024-byte chunks
    while (bResult = ReadFile(hFile, rgbFile, BUFSIZE, &cbRead, NULL))
    {
        if (0 == cbRead)
        {
            break;
        }

        if (!CryptHashData(hHash, rgbFile, cbRead, 0))
        {
            dwStatus = GetLastError(); 
            CryptReleaseContext(hProv, 0);
            CryptDestroyHash(hHash);
            CloseHandle(hFile);
            return HRESULT_FROM_WIN32(dwStatus);
        }
    }

	if (!bResult)
    {
        dwStatus = GetLastError();
        CryptReleaseContext(hProv, 0);
        CryptDestroyHash(hHash);
        CloseHandle(hFile);
        return HRESULT_FROM_WIN32(dwStatus);
    }

	// Convert hash to a string value
	if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
    {
		CString outputString;
		for (DWORD i = 0; i < cbHash; i++)
		{
			outputString += rgbDigits[rgbHash[i] >> 4];
			outputString += rgbDigits[rgbHash[i] & 0xf];
		}
		HashOutput = outputString;
	}
	else
	{
		dwStatus = GetLastError(); 
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		CloseHandle(hFile);
		return HRESULT_FROM_WIN32(dwStatus);
	}

	CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
	CloseHandle(hFile);

	return S_OK;
}

HRESULT Util::MD5HashData(const CAtlArray<BYTE> &InputData, CString &HashOutput)
{
	HashOutput.Empty();

	static const int MD5LEN = 16;
	HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
	BYTE rgbHash[MD5LEN];
	DWORD cbHash = MD5LEN;
	CHAR rgbDigits[] = "0123456789abcdef";
	DWORD dwStatus;

	// Get handle to the crypto provider
    if (!CryptAcquireContext(&hProv,
        NULL,
        NULL,
        PROV_RSA_FULL,
        CRYPT_VERIFYCONTEXT))
    {
		return HRESULT_FROM_WIN32(GetLastError());
    }

	// Create new MD5 Hash
	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    {
        dwStatus = GetLastError();
        CryptReleaseContext(hProv, 0);
        return HRESULT_FROM_WIN32(dwStatus);
    }

	// Add in data byte-wise
	for (unsigned int i = 0; i < InputData.GetCount(); ++i)
	{
		const BYTE b = InputData[i];
	    if (!CryptHashData(hHash, &b, 1, 0))
        {
            dwStatus = GetLastError(); 
            CryptReleaseContext(hProv, 0);
            CryptDestroyHash(hHash);
            return HRESULT_FROM_WIN32(dwStatus);
        }
	}

	// Convert hash to a string value
	if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
    {
		CString outputString;
		for (DWORD i = 0; i < cbHash; i++)
		{
			outputString += rgbDigits[rgbHash[i] >> 4];
			outputString += rgbDigits[rgbHash[i] & 0xf];
		}
		HashOutput = outputString;
	}
	else
	{
		dwStatus = GetLastError(); 
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return HRESULT_FROM_WIN32(dwStatus);
	}

	CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

	return S_OK;
}

HRESULT Util::ReadFileData(const CString &FilePath, CAtlArray<BYTE> &Data)
{
	BYTE buf;
	DWORD bytesRead;
	BOOL bResult;
	DWORD dwStatus = 0;

	Data.RemoveAll();

	HANDLE hFile = CreateFile(
		FilePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);

	if (INVALID_HANDLE_VALUE == hFile)
    {
        return HRESULT_FROM_WIN32(dwStatus);
    }

	while (bResult = ReadFile(hFile, &buf, 1, &bytesRead, NULL))
    {
        if (0 == bytesRead)
        {
            break;
        }

		Data.Add(buf);
    }

	if (!bResult)
    {
        dwStatus = GetLastError();
        return HRESULT_FROM_WIN32(dwStatus);
    }

	return S_OK;
}

// WMP-GNTP Specific Code
HRESULT Util::ReadResourceData(const int ResId, CAtlArray<BYTE> &Data)
{
	Data.RemoveAll();

	// Use this module, not one that might be hosting it
	HINSTANCE hInst = _Module.get_m_hInst();
	HRSRC hRsrc = FindResource(hInst, MAKEINTRESOURCE(ResId), RT_RCDATA);
	if (hRsrc == NULL)
	{
		return E_FAIL;
	}

	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
    if (hGlobal == NULL)
    {
        return E_FAIL;
    }

	LPVOID resPtr = LockResource(hGlobal);
    if (resPtr == NULL)
    {
        return E_FAIL;
    }

	DWORD resSize = SizeofResource(hInst, hRsrc);
	for (DWORD i = 0; i < resSize; ++i)
	{
		Data.Add(static_cast<BYTE*>(resPtr)[i]);
	}

	return S_OK;
}

HRESULT Util::ReadWMPResourceData(const TCHAR *resName, const TCHAR *type, CAtlArray<BYTE> &Data)
{
	Data.RemoveAll();

	HMODULE hModule = GetModuleHandle(L"wmploc.dll");
	HRSRC hRsrc = FindResource(hModule, resName, type);
	if (hRsrc == NULL)
	{
		return E_FAIL;
	}

	HGLOBAL hGlobal = LoadResource(hModule, hRsrc);
    if (hGlobal == NULL)
    {
        return E_FAIL;
    }

	LPVOID resPtr = LockResource(hGlobal);
    if (resPtr == NULL)
    {
        return E_FAIL;
    }

	DWORD resSize = SizeofResource(hModule, hRsrc);
	for (DWORD i = 0; i < resSize; ++i)
	{
		Data.Add(static_cast<BYTE*>(resPtr)[i]);
	}

	return S_OK;
}

// This method finds the m4a covr atom, and then puts it into the data array
// It seeks through the file using the atoms to quickly find the location of
// the cover and returns when it finds the appropriate one. On failure, E_FAIL
// is returned.
HRESULT Util::ReadMP4CoverData(const CString &FilePath, CAtlArray<BYTE> &Data)
{
	BYTE buf;
	DWORD bytesRead;
	BOOL bResult;
	DWORD dwStatus = 0;
	BOOL bFoundCover = FALSE;

	HANDLE hFile = CreateFile(
		FilePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);

	if (INVALID_HANDLE_VALUE == hFile)
    {
        return HRESULT_FROM_WIN32(dwStatus);
    }

	AtomHeader header;
	int count = 0;
	while (bResult = ReadFile(hFile, &header, sizeof(AtomHeader), &bytesRead, NULL))
	{
		if (0 == bytesRead) { break; }

		if (!bResult)
		{
			dwStatus = GetLastError();
			return HRESULT_FROM_WIN32(dwStatus);
		}

		// Swap around the byte order for our use
		header.length = ntohl(header.length);

#ifdef DEBUG
		printf("Length: %x, Type: %c%c%c%c\n",
			header.length,
			header.type[0],
			header.type[1],
			header.type[2],
			header.type[3]);
#endif

		// At the cover data
		if (strncmp("data", header.type, 4) == 0)
		{
			// Seek an extra eight to get to the next header (we don't care about the format)
			// XXX: It might be more correct to verify that it's a format supported by GFW
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 8, NULL, FILE_CURRENT))
			{
				break;
			}

			// Offset from the header seek above as well as the data header
			for (DWORD i = 0; i < (header.length - 16); ++i)
			{
				bResult = ReadFile(hFile, &buf, 1, &bytesRead, NULL);
				if (0 == bytesRead)
				{
					return E_FAIL;
				}
				Data.Add(buf);
			}
			return S_OK;
		}

		// The covr atom is at moov/udta/meta/ilst/covr/data
		// data doesn't pop up as a leaf under other atoms we
		// look inside.
		if (!(strncmp("moov", header.type, 4) == 0)
			&& !(strncmp("udta", header.type, 4) == 0)
			&& !(strncmp("ilst", header.type, 4) == 0)
			&& !(strncmp("covr", header.type, 4) == 0)
			)
		{
			// Seek an extra four to get to the next header
			if (strncmp("meta", header.type, 4) == 0)
			{
				if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 4, NULL, FILE_CURRENT))
				{
					break;
				}
				continue;
			}

			// Seek to the next atom, remembering that we already moved 8 into the atom
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, header.length - 8, NULL, FILE_CURRENT))
			{
				break;
			}
		}
	}

	return E_FAIL;
}
