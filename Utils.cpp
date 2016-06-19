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
#include "Utils.h"
#include <assert.h>
#include "FileEncoding.h"
#include <algorithm>

void upToLowCase(char *buf,int bufLen)
{
	char ch;
	const int diff = 'a' - 'A';
	for (int i = 0;i < bufLen;++ i)
	{
		ch = *(buf + i);
		if(ch >= 'A' && ch <= 'Z')
		{
			*(buf + i) = ch + diff;
		}
	}
}


File::File(const string& file)
:_fileName(file),_keyToUnicode(NULL),_maxKey(_keyLowBond)
{
	
}

File::~File()
{
	if(NULL != _keyToUnicode)
	{
		delete[] _keyToUnicode;
		_keyToUnicode = NULL;
		_maxKey = _keyLowBond;
	}
}

wchar_t* multCharSetToWide(const char * buf,int bufLen,int *realNumLen)
{
	FileEncoding fed;	//CP_UTF8
	int testLen = bufLen < 4096 ? bufLen : 4096;
	int CP_PAGE = fed.getCharSetPageCode(buf,testLen);
	//TODO ���ﻹ�� BUG����ʱ��һ�ε��� MultiByteToWideChar ���� 0 �ֽ�
	int theUTF8 = CP_UTF8;
	//���ֽ��ַ���ת��Ϊ���ַ�������һ��������ʾԴ�ַ����Ĵ���ҳ
	int nMBLen = MultiByteToWideChar(CP_PAGE,0,buf,bufLen,NULL,NULL);
	if(0 == nMBLen)
	{
		int ne = GetLastError();
		printf("MultiByteToWideChar failed. source file page code is : %d\r\n",CP_PAGE);
	}
	wchar_t* szWcsBuffer = (wchar_t*)malloc(sizeof(wchar_t) * (nMBLen + 1));
	
	ZeroMemory(szWcsBuffer, sizeof(wchar_t)*(1+nMBLen));
	MultiByteToWideChar(CP_PAGE,0,buf,bufLen, szWcsBuffer, nMBLen);
	*realNumLen = nMBLen;
	return szWcsBuffer;
}

//#define OCC_ORDER

void File::compressContent(char *buf,int bufLen,wchar_t* &bytes,int *realNumLen,int *alphabetSize,bool upLowCaseSensitive)
{
	setlocale(LC_ALL,"chs");
	if(!upLowCaseSensitive)
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
#ifndef OCC_ORDER
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
		for (int i = 0;i < sw.size();++ i)
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
#else
	//���ַ����ֵ��Ⱥ�˳������ַ�����Դ�С���ȳ��ֵ��ַ��ı�����Խ�С
	for (int i = 0;i < *realNumLen && curKey != keySize;++ i)
	{
		if(_keyLowBond == *(unicodeToKey + *(szWcsBuffer + i)))
		{
			*(unicodeToKey + *(szWcsBuffer + i)) = curKey;
			*(keyToUnicode + curKey) = *(szWcsBuffer + i);
			*(bytes + i) = curKey;
			++ curKey;
		}
		else
		{
			*(bytes + i) = *(unicodeToKey + *(szWcsBuffer + i));
		}
	}
	if(curKey >= keySize)
	{
		printf("====check the encoding of file====\r\n");
		exit(3);
	}
#endif
	//debug ģʽ�»����ڴ�ǳ���
#ifndef _DEBUG	
	free(szWcsBuffer);	
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

/************************************************************************/
/* 
��UTF-8������ļ� fileName �ж������ݣ���ÿ���ַ�ӳ��Ϊһ�� KEY���γ�
һ�� wchar_t ���飻���ز�ͬ�� KEY �ĸ�����alphabetSize

ints�Ĵ�С
*/
/************************************************************************/
void File:: toWchars(wchar_t* &ints,int *realIntsCounts,int *alphabetSize,bool upLowCaseSensitive)
{
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

	int size = GetFileSize(hFile, NULL);

	HANDLE hMapFile =
		CreateFileMapping(hFile
		, NULL
		, PAGE_READONLY 
		, 0
		, size
		, NULL);



	char* pBuff
		= (char*)MapViewOfFile(hMapFile
		, FILE_MAP_READ 
		, 0
		, 0
		, 0);
	
	char *dupBuff = new char[size];
	memcpy_s(dupBuff,size,pBuff,size);
	UnmapViewOfFile(pBuff);
	CloseHandle(hMapFile);
	CloseHandle(hFile);

	compressContent(dupBuff,size,ints,realIntsCounts,alphabetSize,upLowCaseSensitive);
	
}

void intsToFile(int *ints,int len,const string &fileName)
{
	ofstream file(fileName.c_str());
	if(file.is_open())
	{
		for(int i = 0;i < len;++ i)
		{
			file << ints[i] << '\n';
		}
		file.close();
	}
}

/************************************************************************/
/* 
	�����ַ����Ƿ��ǰ׺��ϵ��
	0 �� ������
	1 �� ��һ���ǵڶ�����ǰ׺
	2 �� �ڶ����ǵ�һ����ǰ׺
	�������ַ���һ��������Ϊǰһ���Ǻ�һ����ǰ׺
*/
/************************************************************************/
template<typename CHAR_TYPE>
char innerPrefixRelation(const CHAR_TYPE* firstStr,const CHAR_TYPE* secondStr)
{
	if(NULL == firstStr || NULL == secondStr)
		return 0;
	for(;;++ firstStr,++secondStr)
	{
		if(0 == *firstStr)	//firstStr �����˱�ʾǰһ���Ǻ�һ����ǰ׺
			return 1;
		if(0 == *secondStr)	//prefix ��û������ secondStr �����ˣ���ʾ��һ����ǰһ����ǰ׺
			return 2;

		if(*firstStr != *secondStr)
			break;
	}
	return 0;
}

char prefixRelation(const wchar_t* firstStr,const wchar_t* secondStr)
{
	return innerPrefixRelation(firstStr,secondStr);
}

void testPrefixRelation()
{
	wchar_t* dispaly[3] = {L"no relation.",L"first is prefix",L"second is prefix"};
	wchar_t *first = L"i love wangwei";
	wchar_t *second = L"i love wangwei, my name is lizhihao";
	wprintf(L"[%s]\r\n[%s]\r\n---->(%s)\r\n\r\n",first,second,dispaly[prefixRelation(first,second)]);

	first = L"i love wangwei";
	second = L"i love wangwei";
	wprintf(L"[%s]\r\n[%s]\r\n---->(%s)\r\n\r\n",first,second,dispaly[prefixRelation(first,second)]);

	first = L"i love wangwei";
	second = L"";
	wprintf(L"[%s]\r\n[%s]\r\n---->(%s)\r\n\r\n",first,second,dispaly[prefixRelation(first,second)]);

	first = L"";
	second = L"i love wangwei";
	wprintf(L"[%s]\r\n[%s]\r\n---->(%s)\r\n\r\n",first,second,dispaly[prefixRelation(first,second)]);

	first = L"ilove";
	second = L"i love wangwei";
	wprintf(L"[%s]\r\n[%s]\r\n---->(%s)\r\n\r\n",first,second,dispaly[prefixRelation(first,second)]);

	first = 0;
	second = 0;
	wprintf(L"[%s]\r\n[%s]\r\n---->(%s)\r\n\r\n",first,second,dispaly[prefixRelation(first,second)]);

	first = L"i love wangwei,but i dont't want to";
	second = L"i love wangwei,b";
	wprintf(L"[%s]\r\n[%s]\r\n---->(%s)\r\n\r\n",first,second,dispaly[prefixRelation(first,second)]);
}



int radix(int num)
{
	if(num < 0)
	{
		num = -num;
	}
	int retDix = 1;
	long long ret = 1;//�������Ҫ������� 32 λ������������� INT_MAX ��˵��ret ��������ֱ��С��0���ٵ�С��0.
	while (ret <= num)
	{
		ret <<= 1;
		++ retDix;
	}
	return --retDix;//ret >> 1;
}

#define SET_RESULT_IF_NOT_NULL(theIndexPointer,theIndex) if(NULL!=(theIndexPointer)) *(theIndexPointer)=(theIndex)
int minValue(int *sorArray,int start,int end,int *index = NULL)
{
	int theMin = sorArray[start];
	SET_RESULT_IF_NOT_NULL(index,start);
	for(int i = start + 1;i <= end;++ i)
	{
		if(theMin > sorArray[i])
		{
			theMin = sorArray[i];
			SET_RESULT_IF_NOT_NULL(index,i);
		}
	}
	return theMin;
}

int maxValue(int *sorArray,int start,int end,int *index = NULL)
{
	int theMax = sorArray[start];
	SET_RESULT_IF_NOT_NULL(index,start);
	for(int i = start + 1;i <= end;++ i)
	{
		if(theMax < sorArray[i])
		{
			theMax = sorArray[i];
			SET_RESULT_IF_NOT_NULL(index,i);
		}
	}
	return theMax;
}
#undef SET_RESULT_IF_NOT_NULL




//��ʼ������� t1LengthArray
template<typename CHAR_TYPE>
inline int* initT1Length(const CHAR_TYPE *pStr,size_t totalLength)
{
	const CHAR_TYPE firstChar = *pStr;
	CHAR_TYPE lastChar;	//���һ���ַ�
	CHAR_TYPE beforeLastChar;	//�����ڶ����ַ�
	int lastT1Length;	//��һ����� t1 �ĳ���

	int *t1Length = new int[totalLength + 1];// t1Length[i] ��ʾ�ַ��� pStr[0 ~ i-1] ��õ� t1 �ĳ���
	memset(t1Length,0,sizeof(int) * (1 + totalLength));
	t1Length[0] = t1Length[1] = 0;
	for (int i = 2;i <= totalLength;++ i)
	{
		lastT1Length = t1Length[i-1];	//ǰһ���������� i - 1
		if(0 == lastT1Length)	//ǰһ����ĸû�ж�Ӧ
		{
			t1Length[i] = (firstChar == *(pStr + i - 1));
		}
		else
		{
			lastChar = *(pStr + i - 1);
			beforeLastChar = *(pStr + i - 2);
			while(0 != lastT1Length && *(pStr + lastT1Length) != lastChar)	//��Ӧ�ַ��ĺ���һ���ַ���������һ���ַ�������˳�
			{
				lastT1Length = t1Length[lastT1Length];
			}
			if(0 != lastT1Length)	//��Ϊ *(pStr + lastT1Length) == lastChar ������ѭ��
			{
				t1Length[i] = lastT1Length + 1;
			}
			else
			{
				t1Length[i] = (firstChar == *(pStr + i - 1));
			}
		}
	}
	return t1Length;
}


//��������
template<typename CHAR_TYPE>
inline int firstIndexOf(const CHAR_TYPE *sourceStr,int sLength,const CHAR_TYPE *pStr,int pLength,int beginPos,int* t1Length)
{
	int firstIndex = -1;
	for (int i = beginPos,j=0;i < sLength;)
	{
		if(*(sourceStr + i) != *(pStr + j))
		{
			if(0 == j)
			{
				++ i;
				//j = 0;//��ʡ��
			}
			else if(1 == j)
			{
				j = 0;
			}
			else 
			{
				//i ���䣬j ��ǰ�ƶ�
				j = *(t1Length + j);
			}
		}
		else
		{
			++ j;
			++ i;
			if(pLength == j)
			{
				firstIndex = i - pLength;
				break;
			}
		}
	}
	return firstIndex;
}

#include <vector>
using std::vector;
#include <algorithm>

template<typename CHAR_TYPE>
inline int countIn(const CHAR_TYPE *sourceStr,int sLength,const CHAR_TYPE *pStr,int pLength,int beginPos,int* t1Length)
{
	int counter = 0;
	int occIndex = firstIndexOf(sourceStr,sLength,pStr,pLength,beginPos,t1Length);
	while (-1 != occIndex)
	{
		occIndex = firstIndexOf(sourceStr,sLength,pStr,pLength,occIndex + pLength,t1Length);
		++ counter;
	}
	return counter;
}

template<typename CHAR_TYPE>
int charsLen(const CHAR_TYPE *sor)
{
	assert(NULL != sor);
	int i = 0;
	while(0 != *sor)
	{
		++ i;
		sor += 1;
	}
	return i;
}

template<typename CHAR_TYPE>
int normalCounts(CHAR_TYPE* sStr,CHAR_TYPE * pStr)
{
	int ncounts = 0;
	int sLen = charsLen(sStr);
	int pLen = charsLen(pStr);
	for (int i = 0;i < sLen;)
	{
		int recordI = i;
		if(sLen - i < pLen)	//i ~ sLen - 1 
		{
			break;
		}
		else
		{
			int j = 0;
			for (j = 0;j < pLen;++ j,++ i)
			{
				if (sStr[i] != pStr[j])
				{
					break;
				}
			}
			if(j == pLen)	//ƥ��
			{
				++ ncounts;
				i = recordI + pLen;
			}
			else
			{
				i = recordI + 1;
			}
		}
	}
	return ncounts;
}

//���һ��������ַ�����������
template<typename CHAR_TYPE>
void randomStr(CHAR_TYPE *str,int maxLen,int seed = 0)
{
	if(0 == seed) srand((unsigned)time(NULL));  /*�������*/
	//97 ~ 122
	for (int i = 0;i < maxLen;++ i)
	{
		*(str + i) = rand() % 26 + 97;
	}
}
#include <ctime>
#include <cmath>
void testKmpPerfomance()
{
	int testCase = 10000000;
	wchar_t *sStr = new wchar_t[testCase];
	randomStr<wchar_t>(sStr,testCase);
	ofstream f;
	f.open("E:\\randomStr.txt",ios_base::out|ios_base::trunc);

	int nMBLen = WideCharToMultiByte(CP_UTF8, 0,sStr, testCase, NULL, 0, NULL, NULL);
	char* pBuffer = new char[nMBLen+1];
	pBuffer[nMBLen] = 0;
	WideCharToMultiByte(CP_UTF8, 0,sStr, testCase, pBuffer, nMBLen+1, NULL, NULL);
	printf("length of pbuffer is : %d\r\n",charsLen(pBuffer));
	if(f.is_open())
	{
		f << pBuffer;// << endl<< "i am lzh" << endl;
	}
	delete [] pBuffer;
	f.close();

	long st,et;
	st = clock();
	const static int pStrLen = 4;
	for(int i = 0;i < 100;++ i)
	{
		wchar_t pStr [pStrLen + 1] = {0};
		randomStr(pStr,pStrLen,clock() + i);
		int *t1 = initT1Length(pStr,pStrLen);
		int counter1 = countIn(sStr,testCase,pStr,pStrLen,0,t1);
	//	int counter2 = normalCounts(sStr,pStr);
	//	if(counter1 != counter1)
		{
	//		printf("error\r\n");
		}
	}
	et = clock();
	printf("cost %ld millsecs\r\n",(et - st));
	delete[] sStr;
}

void testCmp()
{
	int testCase = 100000000;
	char *sStr = new char[testCase];
	randomStr<char>(sStr,testCase);
	long st,et;
	st = clock();
	const static int pStrLen = 100;
	for(int i = 0;i < 100;++ i)
	{
		char pStr [pStrLen + 1] = {0};
		randomStr(pStr,pStrLen,clock() + i);
		int *t1 = initT1Length(pStr,pStrLen);
		int index1 = firstIndexOf(sStr,testCase,pStr,pStrLen,0,t1);
	}
	et = clock();
	printf("My.cost %ld millsecs\r\n",(et - st));

	string theStr(sStr);
	st = clock();
	for(int i = 0;i < 100;++ i)
	{
		char pStr [pStrLen + 1] = {0};
		randomStr(pStr,pStrLen,clock() + i);
		int index1 = theStr.find(pStr);
	}
	et = clock();
	printf("It.cost %ld millsecs\r\n",(et - st));
	
	delete[] sStr;
}

void testKMP()
{
	setlocale(LC_ALL,"chs");
	const wchar_t *pStr = L"ѽ";
	const wchar_t *sStr = L"��sdf־�ƺ�ޱޱ����־sdf����˭����־��sdf������ѽ��";
	int *t1Length = initT1Length(pStr,charsLen(pStr));
	wprintf(L"%s : first index of %s is %d\r\n",sStr,pStr,firstIndexOf<wchar_t>(sStr,charsLen(sStr),pStr,charsLen(pStr),0,t1Length));
	wprintf(L"count %s in %s is : %d\r\n",pStr,sStr,countIn<wchar_t>(sStr,charsLen(sStr),pStr,charsLen(pStr),0,t1Length));
}

template<typename char_type>
void getnext(const char_type* str,int len,int *next)
{      
	int i=0,j=-1;
	next[0]=-1;
	while(i < len)
	{
		if(j==-1 || str[i]==str[j])
		{
			next[++ i] = ++j;
		}
		else
			j=next[j];
	}
}


int * getnext(const char* str,int len)
{      
	int i=0,j=-1;
	int *next = new int[len + 1];
	next[0] = -1;
	while(i < len)
	{
		if(j==-1 || str[i]==str[j])
		{
			next[++ i] = ++j;
		}
		else
			j=next[j];
	}
	return next;
}

void testNextPerformance()
{
	const int len = 10;
	char *str = "abcdff0abc";//new char[len];
//	memset(str,0,len);
	while (true)
	{
	//	randomStr(str,len-1);
		clock_t st,et;
		st = clock();
		int *next1 = initT1Length(str,len);
		et = clock();
		clock_t spend1 = et - st;
		st = et;
		int *next2 = getnext(str,len);
		et = clock();
		clock_t spend2 = et - st;
		bool equals = true;
		for (int i = 1;i <= len;++ i)
		{
			if (next1[i] != next2[i])
			{
				equals = false;
			}
		}
		if (equals)
		{
			printf("ok.next1 : %d,next2 : %d\r\n",spend1,spend2);
			for (int i = 1;i <= len;++ i)
			{
				printf("%d\t",next1[i]);
			}
			
			printf("\r\nlength is : %d\r\n",next1[len]);
		}
		else
		{
			printf("no\r\n");
		}
		delete[] next1;
		delete[] next2;
	}
	
}

/************************************************************************/
/* 
	kmp �㷨�е� next ����
*/
/************************************************************************/
int *getWcharsNext(const wchar_t * pStr,size_t totalLength)
{
	return initT1Length(pStr,totalLength);
}