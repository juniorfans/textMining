#include "Utils.h"

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



wchar_t* multCharSetToWide(const char * buf,int bufLen,int *realNumLen)
{
	FileEncoding fed;	//CP_UTF8
	int testLen = bufLen < 4096 ? bufLen : 4096;
	int CP_PAGE = fed.getCharSetPageCode(buf,testLen);
	//TODO 这里还有 BUG，有时第一次调用 MultiByteToWideChar 返回 0 字节
	int theUTF8 = CP_UTF8;
	//多字节字符串转化为宽字符串，第一个参数表示源字符串的代码页
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
	两个字符串是否呈前缀关系。
	0 ： 不存在
	1 ： 第一个是第二个的前缀
	2 ： 第二个是第一个的前缀
	若两个字符串一样，则认为前一个是后一个的前缀
*/
/************************************************************************/
template<typename CHAR_TYPE>
char innerPrefixRelation(const CHAR_TYPE* firstStr,const CHAR_TYPE* secondStr)
{
	if(NULL == firstStr || NULL == secondStr)
		return 0;
	for(;;++ firstStr,++secondStr)
	{
		if(0 == *firstStr)	//firstStr 结束了表示前一个是后一个的前缀
			return 1;
		if(0 == *secondStr)	//prefix 还没结束但 secondStr 结束了，表示后一个是前一个的前缀
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
	long long ret = 1;//这里必须要定义多于 32 位的整数，如对于 INT_MAX 来说，ret 不断左移直到小于0，再到小于0.
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
	const wchar_t *pStr = L"呀";
	const wchar_t *sStr = L"李sdf志浩和薇薇，李志sdf浩是谁？李志浩sdf就是我呀！";
	int *t1Length = initT1Length(pStr,charsLen(pStr));
	wprintf(L"%s : first index of %s is %d\r\n",sStr,pStr,firstIndexOf<wchar_t>(sStr,charsLen(sStr),pStr,charsLen(pStr),0,t1Length));
	wprintf(L"count %s in %s is : %d\r\n",pStr,sStr,countIn<wchar_t>(sStr,charsLen(sStr),pStr,charsLen(pStr),0,t1Length));
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


void doWSync(ostream *file,bool display,wchar_t *theChs,int nDocLength,int fpCounter)
{
	//第一个参数表示目标字符串的代码页
	//这个参数只影响输出文件的编码，如果设置为 0 则使用系统默认的代码页，即 asscii 编码。如果使用 UTF-8，则输出的文件编码为 UTF-8
	int nMBLen = WideCharToMultiByte(CP_UTF8, 0,theChs, nDocLength, NULL, 0, NULL, NULL);
	char* pBuffer = new char[nMBLen+1];
	pBuffer[nMBLen] = 0;
	WideCharToMultiByte(CP_UTF8, 0,theChs, nDocLength, pBuffer, nMBLen+1, NULL, NULL);
	if(NULL != file)	*file << pBuffer << " : " << fpCounter << "\n";
	if(display)	wprintf(L"%s : %d\r\n",theChs,fpCounter);
	delete[] pBuffer;
}

void doByteSync(ostream *file,bool display,byte *theChs,int nDocLength,int fpCounter)
{
	//第一个参数表示目标字符串的代码页
	//这个参数只影响输出文件的编码，如果设置为 0 则使用系统默认的代码页，即 asscii 编码。如果使用 UTF-8，则输出的文件编码为 UTF-8

	if(NULL != file)	//*file << theChs << " : " << fpCounter << "\n";
		*file << nDocLength << " : " << fpCounter << "\n";
	if(display)
	{
		for(int i = 0;i < nDocLength;++ i)
			printf("%c",theChs[i]);
		printf(" : %d\r\n",fpCounter);
	}
}