#include <hash_map>
using stdext::hash_map;
#include <string>
using std::string;
#include <fstream>
using std::ifstream;
using std::ios_base;
#include <string>
using std::string;
#include <windows.h>

#include <iostream>
using namespace::std;
#include <memory>
#include <limits>
#include <cassert>
#include "constants.h"
#include <hash_map>
using stdext::hash_map;
#include "MiningFile.h"
#include "Utils.h"
#include <assert.h>
#include <algorithm>

char * MiningFile::toBytes()
{
	if(NULL != _bytes)
		return _bytes;
	HANDLE hFile = 
		CreateFileA(_fileName.c_str()
		, GENERIC_READ
		, FILE_SHARE_READ
		, NULL
		, OPEN_EXISTING
		, FILE_ATTRIBUTE_NORMAL
		, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{   
		printf("Could not open file \"%s\" (error %d)\n",_fileName.c_str(), GetLastError()); 
		exit(-2);
	}

	_sorCounts = GetFileSize(hFile, NULL);

	HANDLE hMapFile =
		CreateFileMapping(hFile
		, NULL
		, PAGE_READONLY 
		, 0
		, _sorCounts
		, NULL);



	char* pBuff
		= (char*)MapViewOfFile(hMapFile
		, FILE_MAP_READ 
		, 0
		, 0
		, 0);

	char *_bytes = new char[_sorCounts];
	memcpy_s(_bytes,_sorCounts,pBuff,_sorCounts);
	UnmapViewOfFile(pBuff);
	CloseHandle(hMapFile);
	CloseHandle(hFile);
	setRealCounts(_sorCounts);
	return _bytes;
}


/************************************************************************/
/* 
��UTF-8������ļ� fileName �ж������ݣ���ÿ���ַ�ӳ��Ϊһ�� KEY���γ�
һ�� wchar_t ���飻���ز�ͬ�� KEY �ĸ�����alphabetSize

ints�Ĵ�С
*/
/************************************************************************/
void* TextFile:: toTchars()
{
	if(NULL != _wChars)
		return (void*)_wChars;
	char *dupBuff = toBytes();
	int realIntsCounts = 0;
	int alphabetSize = 0;
	compressContent(dupBuff,getSorCounts(),_wChars,&realIntsCounts,&alphabetSize);
	setRealCounts(realIntsCounts);
	setAlphabetSize(alphabetSize);
	return (void*)_wChars;
}


void TextFile::compressContent(char *buf,int bufLen,wchar_t* &bytes,int *realNumLen,int *alphabetSize)
{
	setlocale(LC_ALL,"chs");
	if(!_upLowCaseSensitive)
	{
		upToLowCase(buf,bufLen);
	}
	//wchar_t �ܱ�������ֵ
	const int keySize = (1 << (sizeof(wchar_t) * 8)) - 1;
	wchar_t keyToUnicode [keySize];		//key -> unicode | key > 0
	wchar_t unicodeToKey[keySize];			//unicode -> key | unicode > 0

	for (int i = 0;i < keySize;++ i)
	{
		*(unicodeToKey+i) = _keyLowBond;
		*(keyToUnicode+i) = _keyLowBond;
	}

	wchar_t *szWcsBuffer;
	szWcsBuffer = multCharSetToWide(buf,bufLen,realNumLen);
	bytes = new wchar_t[*realNumLen];
	memset(bytes,0,sizeof(wchar_t) * *realNumLen);

	wchar_t curKey = _keyLowBond + 1;
	//���ַ�ԭ���Ĵ�С��������Ĵ�С����ԭ��Դ�С����һ��

	vector<wchar_t> sw;
	wchar_t wMap[keySize] = {0};
	for (int i = 0;i < *realNumLen;++ i)
	{
		if(0 == wMap[szWcsBuffer[i]])
		{
			wMap[szWcsBuffer[i]] = 1;
			sw.push_back(szWcsBuffer[i]);
		}
	}

	sort(sw.begin(),sw.end());
	for (size_t i = 0;i < sw.size();++ i)
	{
		curKey = i + _keyLowBond + 1;
		unicodeToKey[sw[i]] = curKey;
		keyToUnicode[curKey] = sw[i];
	}
	++ curKey;
	for (int i = 0;i < *realNumLen;++ i)
	{
		bytes[i] = unicodeToKey[szWcsBuffer[i]];
	}
	//debug ģʽ�»����ڴ�ǳ���
#ifndef _DEBUG	
	free(szWcsBuffer);
	free(buf);
#else
#endif
	*alphabetSize = curKey -_keyLowBond;	//ÿ�� bytes ���� (_keyLowBond) ~ (curKey - 1)

	if(NULL != _keyToUnicode)
	{
		delete[] _keyToUnicode;
	}	
	_keyToUnicode = new wchar_t[curKey];
	memcpy_s(_keyToUnicode,curKey * sizeof(wchar_t),keyToUnicode,sizeof(wchar_t) * curKey);
	_maxKey = curKey-1;

}