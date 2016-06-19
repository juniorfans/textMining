#include <stdio.h>
#include <math.h>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <ctime>
#include <windows.h>
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")
#include "SectionM.h"
#include "Utils.h"
#include "FrequentMinning.h"
#include <iostream>
using std::cout;
using std::cin;
using std::endl;

#include "cmdline.h"
#include <cassert>

#define MINNING
//#define DOUBLEFILE
//#define HALFFILE

extern void testSectionMin();
extern void testLongestFS();
extern void testIttsort();
extern void testEffectiveRMQ();
//#define DEBUG_MODE
void frequentMinning(int argc,char **argv)
{
	cmdline::parser a;
	
	/*���ھ���ļ��� - �������ȷ�����ļ��ı��벢����ã�����ָ������*/
	a.add<string>("file", 'f', "file to minning", true, "");

	/*Ƶ�����е�֧�ֶȡ���ȱʡ��Ĭ��ֵΪ 2*/
	a.add<int>("support", 's', "support for frequent minning", false, 2);

	/*��Сд����:0->�����У�����->���С���ȱʡ��Ĭ��ֵΪ 1(����)*/
	a.add<int>("caseSensitive",'c', "up low case sensitive", false,1);

	///*��ʽ��Ƶ�����У����Ե��Կո�\t,\r,\n��ͷ��Ƶ�����У�:0->����ʽ��������->��ʽ������ȱʡ��Ĭ��ֵΪ 1(��ʽ��)*/
	//a.add<int>("format", 'F', "Format :ignore fps start with [space,\\t,\\r,\\n].", false,1);

	/*Ƶ��������̳��ȣ����ڴ˳��ȵ�Ƶ�����б����ԡ���ȱʡ��Ĭ��ֵΪ 1*/
	a.add<int>("minLength", 'm', "fps min length", false,1);

	/*Ƶ�����н������󱣴�������ȱʡ��Ĭ��ֵΪ -1�����������е�Ƶ�����С�*/
	a.add<int>("limitNumsOfFps", 'l', "fps nums to save", false,-1);

	/*������������ȱʡ��Ĭ��ֵΪ 1(����)����ԭ��Ϊ aaaaa��'aa'�Ĵ�������Ϊ2�Σ��Ǵ�������Ϊ4�Ρ���������������²�����һ��λ�ñ����ƥ��ͬʱռ��*/
	a.add<int>("pureCount",'p',"fps pure count",false,1);

	/*���ô�������ʱ��ĳЩ���е�ǰ׺������мƴ���ͬ��ɾ����ǰ׺���С���ȱʡ��Ĭ��ֵΪ 1(ɾ��)����ԭ��Ϊ aaabaaa��'aa'�Ĵ�������Ϊ2�Σ�'aaa'�Ĵ�������ҲΪ2�Σ���ʱӦ��ɾ�� aa*/
	a.add<int>("pureCountEx",'P',"fps pure count ex",false,1);

	/*Ƶ������д���ļ�����ȱʡ��Ĭ��ֵ 1(д���ļ�)*/
	a.add<int>("writeFpstoFile", 'w', "need to write to file", false,1);

	/*��ʾƵ�����С���ȱʡ��Ĭ��ֵ 0(����ʾ)*/
	a.add<int>("displayFps", 'd', "need to display the fps", false,0);

	/*Ƶ���������еĴ���0->�ֵ���1->Ƶ�ν���2->Ƶ�����򡣿�ȱʡ��Ĭ��ֵ 1(Ƶ������)*/
	a.add<int>("orderOfFps", 'o', "how to order the fps.\r\n\\t\t[0:with the dictionary order.]\r\n\t\t[1:with the frequent order desc.]\r\n\t\t[2:with the frequent order asc]", false,1);

	/*�޳����԰���Ƶ�����У��� abcd �� bcd ����Ƶ�����У������� 4 �Σ��� bcd Ӧ�ñ��޳�����ȱʡ��Ĭ��ֵ 1(�޳�)*/
	a.add<int>("removeContainedFps",'r',"remove the contained fps",false,1);

	/*ת�������ַ�����Ƶ�������е� '\t' ��ת�壬�����ļ��кͿ���̨��ʾ�Ľ��� '\t'����ȱʡ��Ĭ��ֵ 0(��ת��)*/
	a.add<int>("transformSpecialChars",'t',"transform the special chars like \\r,\\n in the fps",false,0);

	a.parse_check(argc, argv);

	FrequentConfig fconfig;
	fconfig._fileName.assign(a.get<string>("file"));
	fconfig._support = a.get<int>("support");
	fconfig._upLowCaseSensitive = (0!=a.get<int>("caseSensitive"));
//	fconfig._formatFps = (0!=a.get<int>("format"));
	fconfig._minLen = a.get<int>("minLength");
	fconfig._pureCount = (0 != a.get<int>("pureCount"));
	fconfig._pureCountEx = (0 != a.get<int>("pureCountEx"));
	fconfig._needDisplayFps = (0 !=a.get<int>("displayFps"));
	fconfig._needWriteFpsToFile = (0 !=a.get<int>("writeFpstoFile"));
	fconfig._fpsListNums = a.get<int>("limitNumsOfFps");
	fconfig._fpsOrder = a.get<int>("orderOfFps");
	fconfig._fpsRemoveContained = (0 !=a.get<int>("removeContainedFps"));
	fconfig._formatFps_removeFpsStartWchars = L" \t\r\n";
	fconfig._formatFps_transform = (0!=a.get<int>("transformSpecialChars"));
	FrequentMinning fpMinning(fconfig);
	fpMinning.minning();
}


void doubleFile(int argc,char** argv)
{
	cmdline::parser a;

	a.add<string>("file", 'f', "file to double", true, "");
	a.parse_check(argc, argv);
	string fileName = a.get<string>("file");
	FILE *f;
	f = fopen(fileName.c_str(),"r+b");
	if (NULL == f)
	{
		printf("open file failed.error : %d\r\n",GetLastError());
		return;
	}
	fseek(f,0,SEEK_END);
	long fsize = ftell(f);
	fseek(f,0,SEEK_SET);
	printf("before doubling : %ld\r\n",fsize);
	byte *content = new byte[fsize];
	long rbytes = fread(content,sizeof(byte),fsize,f);
	assert(rbytes == fsize);
	fseek(f,0,SEEK_END);
	long wbytes = fwrite(content,sizeof(byte),fsize,f);
	fflush(f);
	fseek(f,0,SEEK_END);
	fsize = ftell(f);
	printf("after doubling : %ld\r\n",fsize);
	assert(wbytes == (fsize >> 1));
	fclose(f);
}

#ifdef HALFFILE
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <share.h>

void halfFile(int argc,char** argv)
{
	cmdline::parser a;

	a.add<string>("file", 'f', "file to half", true, "");
	a.parse_check(argc, argv);
	string fileName = a.get<string>("file");
	int fhandle;
	int result;
	long fsize;
	if( _sopen_s( &fhandle,fileName.c_str(), _O_RDWR, _SH_DENYWR,
		_S_IREAD | _S_IWRITE ) == 0 )
	{
		printf( "File length before: %ld\n", fsize = _filelength( fhandle ) );
		if( ( result = _chsize( fhandle, fsize >> 1 ) ) != 0 )
			printf( "Problem in changing the size\n" );
		printf( "File length after:  %ld\n", _filelength( fhandle ) );
		_close( fhandle );
	}
	else
	{
		printf("File open failed.\r\n");
	}
}
#endif

extern void testKMP();
extern void testKmpPerfomance();
extern void testCmp();
extern void testSectionMin();

#include "RepeatString.h"

extern void testNextPerformance();
extern void testPrefixRelation();

int main(int argc,char** argv)
{
#ifdef DOUBLEFILE
	doubleFile(argc,argv);
#endif

#ifdef HALFFILE
	halfFile(argc,argv);
#endif

#ifdef MINNING
	//testPrefixRelation();
	frequentMinning(argc,argv);
	//testKMP();
	//testKmpPerfomance();
	//testCmp();
	//testEffectiveRMQ();
	//testPrime();
	//testNextPerformance();
#endif

	return 0;
}

