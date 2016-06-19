#include <cstdio>
#include <cstdlib>

#include <ctime>
#include <fstream>
using std::fstream;
using std::ofstream;
using std::ios_base;
#include "constants.h"
#include "Utils.h"
#include "MiningFile.h"
#include <iostream>
#include <cassert>
#include <windows.h>
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")

#include <sstream>
using std::stringstream;

#include <algorithm>
#include "divsufsortxx.h"
#include "divsufsortxx_utility.h"


#include "RMQEffective.h"
#include "TopK.h"
#include "FPSGettor.h"
#include "FrequentMinning.h"
#include "Utils.h"

//#define DEBUG_OUTPUT

FrequentMinning::FrequentMinning(const FrequentConfig& config)
:_fconfig(config),_fps(NULL),_fpsSize(0)
{

}

FrequentMinning::~FrequentMinning()
{
	destroy();
}

void FrequentMinning::init()
{
	if(_fconfig._binaryFile)
	{	
		_file = new BinaryFile(_fconfig._fileName);
		_file->toTchars();
	}
	else
	{	
		_file = new TextFile(_fconfig._fileName,_fconfig._upLowCaseSensitive);
		_file->toTchars();
	}
	_SA = new int[_file->getRealCounts()];
	_height = new int[_file->getRealCounts()];
}

void FrequentMinning::destroy()
{
	if(NULL != _SA)
	{
		delete[] _SA;
		_SA = NULL;
	}
	if(NULL != _height)
	{
		delete[] _height;
		_height = NULL;
	}
	if(NULL != _file)
	{
		delete _file;
		_file = NULL;
	}
	if(NULL != _fps && 0 != _fpsSize)
	{
		for(int i = 0;i < _fpsSize;++ i)
		{
			delete _fps[i];
		}
		delete[] _fps;
		_fps = NULL;
		_fpsSize = 0;
	}
}

#define EXSORT

void FrequentMinning::minning()
{
	clock_t totalStart,start,finish;
	totalStart = clock();
	start = clock();
	printf("file parsing with %s... ",_fconfig._upLowCaseSensitive ? "letter sensitive" : "letter not sensitive");
	init();
	finish = clock();
	printf("finished.Cost : %d\r\n",finish - start);
	_len = _file->getRealCounts();
	char_content = (byte*) _file->toTchars();
	wchar_content = (wchar_t*) _file->toTchars();
	_alphabetSize = _file->getAlphabetSize();

	for (int x = 0;x < _len;++ x)
	{
		if(!(char_content >= 0 && char_content[x] < 256))
			printf("not resonable\r\n");
	}
	

	if(0 == _len)
	{
		printf("unsupported encoding, you can transfer the encoding of file to UTF-8. Then try it again!\r\n");
		return;
	}
	//itssort
	start = clock();
	printf("suffix sorting...");
#ifndef EXSORT
	initIttSortConstants(_alphabetSize);
	int errorCode = itssort(_content,_SA,_len);
	if(-2 == errorCode)
	{
		printf("memory is not enough in itssort ...\r\n");
		exit(-2);
	}
	else if(-1 == errorCode)
	{
		printf("invalid parameters in itssort ...\r\n");
		exit(-2);
	}
#else
	if(_fconfig._binaryFile)
	{
		divsufsortxx::constructSA(char_content,char_content+_len,_SA,_SA+_len,_alphabetSize);
	}
	else
	{
		divsufsortxx::constructSA(wchar_content,wchar_content+_len,_SA,_SA+_len,_alphabetSize);
	}
#endif

	finish = clock();
	printf("finished.Cost : %d millsecs\r\n",finish - start);

	//cal height

	start = clock();
	printf("height calculating...");
	calHeight();
	//	verifyHeight(height);
	finish = clock();
	printf("finished.Cost : %d millsecs\r\n",finish - start);

	//get fps
	start = clock();
	printf("gathering ");
	printf(-1 == _fconfig._fpsListNums ? "all" : "top %d ",_fconfig._fpsListNums);
	printf("frequent sequences[%s]...",0 == _fconfig._fpsOrder ? "dictionary order":"frequency order");
	FPSFilter fft;
	fft._limits = _fconfig._fpsListNums;
	fft._minLen = _fconfig._minLen;
	fft._order = _fconfig._fpsOrder;
	fft._support = _fconfig._support;
	fft._pureCount = _fconfig._pureCount;
	fft._pureCountEx = _fconfig._pureCountEx;
	
	if(_fconfig._binaryFile)
	{
		FPSGettor<byte> allFs(fft,_height,_SA,char_content,_len);
		_fps = allFs.getFS(_fpsSize);
	}
	else
	{
		FPSGettor<wchar_t> allFs(fft,_height,_SA,wchar_content,_len);
		_fps = allFs.getFS(_fpsSize);
	}
	
	
	finish = clock();
	printf("finished.Cost : %d millsecs\r\n",finish - start);

	start = clock();
	printf("syncing %d fps to std::out or file...\r\n",_fpsSize);
	syncTo();
	finish = clock();
	printf("\r\nsync finished.Cost : %d millsecs\r\n",finish - start);
	//report
	finish = clock();
	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
	std::cout << "total cost : "<<finish - totalStart << " mill seconds, peakMemory used: "<<pmc.PeakWorkingSetSize / 1024 / 1024 << " M." <<std::endl;
	printf("destroying memory...\r\n");
	destroy();
}

void FrequentMinning:: calHeight()
{
	int len = _len;
	int *rank = new int[len];
	int i,j,k = 0;
	for(i = 0;i < len;i++)
	{
		*(rank + _SA[i]) = i;
	}
	//_height[x] 表示排名第 x 的后缀和排在它前一名的后缀的最长公共子串（连续）的长度
	//_height[0] 无用
	_height[0] = -1;
	for(i=0;i < len; *(_height + *(rank + i++)) = k )
	{
		if(0 == *(rank + i))
		{
			if(k > 0)k --;
			continue;	//排在第 0 名前面的不存在
		}

		if(_fconfig._binaryFile)
		{
			for(k?k--:0,j= *(_SA + *(rank+i)-1);*(char_content + (i+k))==*(char_content + (j+k));k++);
		}
		else
		{
			for(k?k--:0,j= *(_SA + *(rank+i)-1);*(wchar_content + (i+k))==*(wchar_content + (j+k));k++);
		}
		
	}
	delete[] rank;
	return;
}

wchar_t * FrequentMinning::getWCharFpFromFsNode(FsNode * curFP,int &theLen)
{
	TextFile *_tfile = (TextFile*)_file;

	int suffixIndex;
	int suffixLen;
	int fpCounter;

	suffixLen = curFP->length;
	suffixIndex = curFP->index;
	fpCounter = curFP->counter;
	theLen = suffixLen + 1;
	wchar_t *theChs = new wchar_t[theLen];	//多一个 wchar_t放 '\0'
	theChs[suffixLen] = 0;

	wchar_t uCode;
	int i = 0;
	wchar_t curWCh;
	while (suffixLen != i)
	{
		curWCh = wchar_content[suffixIndex + i ++];
		uCode = _tfile->keytoUnicode(curWCh);
		theChs[i-1] = uCode;
	}
	return theChs;
}

byte * FrequentMinning::getByteFpFromFsNode(FsNode * curFP,int &theLen)
{
	int suffixIndex;
	int suffixLen;
	int fpCounter;

	suffixLen = curFP->length;
	suffixIndex = curFP->index;
	fpCounter = curFP->counter;
	theLen = suffixLen + 1;
	return char_content + suffixIndex;
	/*
	char *theChs = new char[theLen];	//多一个 wchar_t放 '\0'
	theChs[suffixLen] = 0;

	memcpy_s(theChs,suffixLen,char_content + suffixIndex,suffixLen);

	return theChs;
	*/
}

void FrequentMinning::syncTo()
{
	if(0 == _fpsSize) return;
	setlocale(LC_ALL,"chs");
	ofstream *outFile = NULL;
	if(_fconfig._needWriteFpsToFile)
	{
		outFile = new ofstream();
		outFile->open((_fconfig._fileName+".fps.txt").c_str(),ios_base::out|ios_base::trunc);
	}

	FsNode *curFP;
	int fpCounter;
	for(int j = 0;j < _fpsSize;++ j)
	{
		curFP = *(_fps + j);
		fpCounter = curFP->counter;
		int fpsLen;

		if(_fconfig._binaryFile)
		{
			byte *curFpStr = getByteFpFromFsNode(curFP,fpsLen);	//curFP 是 char_content 中的内容，不能 delete，交由 _file 去回收
			if(_fconfig._formatFps_transform)
			{
				int outLen;
				byte *realFpStr = transformSepecialChars(curFpStr,fpsLen,outLen);
				doByteSync(outFile,_fconfig._needDisplayFps,realFpStr,outLen,fpCounter);
				delete realFpStr;
			}
			else
			{
				doByteSync(outFile,_fconfig._needDisplayFps,curFpStr,fpsLen,fpCounter);
			}
		}
		else
		{
			wchar_t *curFpStr = getWCharFpFromFsNode(curFP,fpsLen);
			if(_fconfig._formatFps_transform)
			{
				int outLen;
				wchar_t *realFpStr = transformSepecialChars(curFpStr,fpsLen,outLen);
				doWSync(outFile,_fconfig._needDisplayFps,realFpStr,outLen,fpCounter);
				delete realFpStr;
			}
			else
			{
				doWSync(outFile,_fconfig._needDisplayFps,curFpStr,fpsLen,fpCounter);
			}
			delete curFpStr;
		}
		
		
		
	}
	if(NULL != outFile)
	{
		outFile->close();
		delete outFile;
	}
}
//检验 height 计算的是否有误
//heigth[x] 表示排名第 x 的后缀与排名第 x-1 的后缀的最长公共前缀.height[0] 无用
void FrequentMinning::verifyHeight()
{
	for(int i = 1;i < _len;++ i)
	{
		int lcp = _height[i];
		//SA[i] 和 SA[i-1]是否公共前缀长度为 lcp
		if(!_fconfig._binaryFile)
		{
			const wchar_t *l = wchar_content + _SA[i],*r = wchar_content + _SA[i - 1];
			while(-1 != --lcp)
			{
				if(l[lcp] != r[lcp])
				{
					printf("verify failed.\r\n");
					exit(-3);
				}
			}
		}
		else
		{
			const byte *l = char_content + _SA[i],*r = char_content + _SA[i - 1];
			while(-1 != --lcp)
			{
				if(l[lcp] != r[lcp])
				{
					printf("verify failed.\r\n");
					exit(-3);
				}
			}
		}
		
	}
	printf("verify height success.\r\n");
}



void FrequentMinning:: showSuffixAndHeight(int *height)
{
	if(!_fconfig._binaryFile)
	{
		for(int i = 1;i < _len;++ i)
		{
			printf("[%d] %d    ",i,height[i]);
			for(int j = _SA[i];j < _len;++ j)
			{
				printf("%d ",wchar_content[j]);
			}printf("\r\n");
		}
	}
	else
	{
		for(int i = 1;i < _len;++ i)
		{
			printf("[%d] %d    ",i,height[i]);
			for(int j = _SA[i];j < _len;++ j)
			{
				printf("%d ",char_content[j]);
			}printf("\r\n");
		}
	}
}


