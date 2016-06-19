
#include "itssort.h"

#include <cstdio>
#include <cstdlib>

#include <ctime>
#include <fstream>
using std::fstream;
using std::ofstream;
using std::ios_base;
#include "constants.h"
#include "Utils.h"
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
	_file = new File(_fconfig._fileName);
	_file->toWchars(_content,&_len,&_alphabetSize,_fconfig._upLowCaseSensitive);
	_SA = new int[_len];
	_height = new int[_len];
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
	if(NULL != _content)
	{
		delete[] _content;
		_content = NULL;
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
	divsufsortxx::constructSA(_content,_content+_len,_SA,_SA+_len,_alphabetSize);
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
	//fft._formatFps = _fconfig._formatFps;
	fft._formatFps_removeStartWchars = _fconfig._formatFps_removeFpsStartWchars;
	fft._order = _fconfig._fpsOrder;
	fft._support = _fconfig._support;
	fft._pureCount = _fconfig._pureCount;
	fft._pureCountEx = _fconfig._pureCountEx;
	
	FPSGettor allFs(fft,_height,_SA,_len,_file->getKeyToUnicodeMap(),_content);

	_fps = allFs.getFS(_fpsSize);
	
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
		for(k?k--:0,j= *(_SA + *(rank+i)-1);*(_content + (i+k))==*(_content + (j+k));k++);
	}
	delete[] rank;
	return;
}


void FrequentMinning:: doSync(ostream *file,wchar_t *theChs,int nDocLength,int fpCounter)
{
	//第一个参数表示目标字符串的代码页
	//这个参数只影响输出文件的编码，如果设置为 0 则使用系统默认的代码页，即 asscii 编码。如果使用 UTF-8，则输出的文件编码为 UTF-8
	int nMBLen = WideCharToMultiByte(CP_UTF8, 0,theChs, nDocLength, NULL, 0, NULL, NULL);
	char* pBuffer = new char[nMBLen+1];
	pBuffer[nMBLen] = 0;
	WideCharToMultiByte(CP_UTF8, 0,theChs, nDocLength, pBuffer, nMBLen+1, NULL, NULL);
	if(NULL != file)	*file << pBuffer << " : " << fpCounter << "\n";
	if(_fconfig._needDisplayFps)	wprintf(L"%s : %d\r\n",theChs,fpCounter);
	delete[] pBuffer;
}


wchar_t *FrequentMinning:: transformSepecialChars(wchar_t *inChs,int len,int &outLen)
{
	vector<wchar_t> theChs;
	theChs.reserve(len);
	int i = 0;
	wchar_t curWch;
	while (len != i)
	{
		curWch = inChs[i ++];
		if ('\r' == curWch)
		{
			theChs.push_back('\\');
			theChs.push_back('r');
		}
		else if ('\n' == curWch)
		{
			theChs.push_back('\\');
			theChs.push_back('n');
		}
		else
		{
			theChs.push_back(curWch);
		}
	}
	int nsize = theChs.size();
	outLen = nsize + 1;
	wchar_t *retChs = new wchar_t[outLen];
	retChs[nsize] = 0;
	for (int i = 0;i < nsize;++ i)
	{
		retChs[i] = theChs[i];
	}
	return retChs;
}

wchar_t * FrequentMinning::getFpFromFsNode(FsNode * curFP,int &theLen)
{
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
		curWCh = _content[suffixIndex + i ++];
		uCode = _file->keytoUnicode(curWCh);
		theChs[i-1] = uCode;
	}
	return theChs;
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
		wchar_t *curFpStr = getFpFromFsNode(curFP,fpsLen);
		if(_fconfig._formatFps_transform)
		{
			int outLen;
			wchar_t *realFpStr = transformSepecialChars(curFpStr,fpsLen,outLen);
			doSync(outFile,realFpStr,outLen,fpCounter);
			delete realFpStr;
		}
		else
		{
			doSync(outFile,curFpStr,fpsLen,fpCounter);
		}
		
		delete curFpStr;
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
		const wchar_t *l = _content + _SA[i],*r = _content + _SA[i - 1];
		while(-1 != --lcp)
		{
			if(l[lcp] != r[lcp])
			{
				printf("verify failed.\r\n");
				exit(-3);
			}
		}
	}
	printf("verify height success.\r\n");
}



void FrequentMinning:: showSuffixAndHeight(int *height)
{
	for(int i = 1;i < _len;++ i)
	{
		printf("[%d] %d    ",i,height[i]);
		for(int j = _SA[i];j < _len;++ j)
		{
			printf("%d ",_content[j]);
		}printf("\r\n");
	}
}


