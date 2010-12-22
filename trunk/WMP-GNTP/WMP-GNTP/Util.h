// Copyright (c) 2010 William Reading
// Available under the terms of the Microsoft Public License (Ms-PL)

#pragma once

#include "common.h"

class Util
{
public:
	static CStringA GetUTF8String(const CString &WideString);
	static HRESULT MD5HashString(const CString &InputString, CString &HashOutput);
	static HRESULT MD5HashFile(const CString &InputFilePath, CString &HashOutput);
	static HRESULT MD5HashData(const CAtlArray<BYTE> &InputData, CString &HashOutput);
	static HRESULT ReadFileData(const CString &FilePath, CAtlArray<BYTE> &Data);
	static HRESULT ReadResourceData(const int ResId, CAtlArray<BYTE> &Data);
	static HRESULT ReadMP4CoverData(const CString &FilePath, CAtlArray<BYTE> &Data);
	static HRESULT ReadWMPResourceData(const TCHAR *resName, const TCHAR *type, CAtlArray<BYTE> &Data);
};
