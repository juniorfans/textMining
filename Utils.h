#pragma once
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
#include <assert.h>
#include "FileEncoding.h"
#include <algorithm>
#include <hash_map>
using stdext::hash_map;
#include <string>
using std::string;

void intsToFile(int *ints,int len,const string &fileName);
char prefixRelation(const wchar_t* firstStr,const wchar_t* secondStr);

int radix(int num);

int minValue(int *sorArray,int start,int end,int *index);

int maxValue(int *sorArray,int start,int end,int *index);


void upToLowCase(char *buf,int bufLen);

wchar_t* multCharSetToWide(const char * buf,int bufLen,int *realNumLen);


/************************************************************************/
/* 
	kmp �㷨�е� next ����
*/
/************************************************************************/
template<typename CHAR_TYPE>
int *getWcharsNext(const CHAR_TYPE * pStr,size_t totalLength)
{
	return initT1Length(pStr,totalLength);
}

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
	for (size_t i = 2;i <= totalLength;++ i)
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

template<typename CHAR_TYPE>
CHAR_TYPE *transformSepecialChars(CHAR_TYPE *inChs,int len,int &outLen)
{
	vector<CHAR_TYPE> theChs;
	theChs.reserve(len);
	int i = 0;
	wchar_t curWch;
	while (len != i)
	{
		curWch = inChs[i ++];
		if (CHAR_TYPE('\r') == curWch)
		{
			theChs.push_back(CHAR_TYPE('\\'));
			theChs.push_back(CHAR_TYPE('r'));
		}
		else if (CHAR_TYPE('\n') == curWch)
		{
			theChs.push_back(CHAR_TYPE('\\'));
			theChs.push_back(CHAR_TYPE('n'));
		}
		else
		{
			theChs.push_back(curWch);
		}
	}
	int nsize = theChs.size();
	outLen = nsize + 1;
	CHAR_TYPE *retChs = new CHAR_TYPE[outLen];
	retChs[nsize] = 0;
	for (int i = 0;i < nsize;++ i)
	{
		retChs[i] = theChs[i];
	}
	return retChs;
}

void doWSync(ostream *file,bool display,wchar_t *theChs,int nDocLength,int fpCounter);
void doByteSync(ostream *file,bool display,byte *theChs,int nDocLength,int fpCounter);