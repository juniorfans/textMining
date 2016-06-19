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
	
	/*待挖掘的文件名 - 程序会正确分析文件的编码并处理好，不需指定编码*/
	a.add<string>("file", 'f', "file to minning", true, "");

	/*频繁序列的支持度。可缺省，默认值为 2*/
	a.add<int>("support", 's', "support for frequent minning", false, 2);

	/*大小写敏感:0->不敏感，其它->敏感。可缺省，默认值为 1(敏感)*/
	a.add<int>("caseSensitive",'c', "up low case sensitive", false,1);

	///*格式化频繁序列，忽略掉以空格，\t,\r,\n开头的频繁序列，:0->不格式化，其它->格式化。可缺省，默认值为 1(格式化)*/
	//a.add<int>("format", 'F', "Format :ignore fps start with [space,\\t,\\r,\\n].", false,1);

	/*频繁序列最短长度，低于此长度的频繁序列被忽略。可缺省，默认值为 1*/
	a.add<int>("minLength", 'm', "fps min length", false,1);

	/*频繁序列结果集最大保存数。可缺省，默认值为 -1，即保存所有的频繁序列。*/
	a.add<int>("limitNumsOfFps", 'l', "fps nums to save", false,-1);

	/*纯净计数。可缺省，默认值为 1(纯净)。如原串为 aaaaa，'aa'的纯净计数为2次，非纯净计数为4次。即纯净计数情况下不允许一个位置被多个匹配同时占用*/
	a.add<int>("pureCount",'p',"fps pure count",false,1);

	/*采用纯净计数时，某些序列的前缀与该序列计次相同，删除该前缀序列。可缺省，默认值为 1(删除)。如原串为 aaabaaa，'aa'的纯净计数为2次，'aaa'的纯净计数也为2次，此时应该删除 aa*/
	a.add<int>("pureCountEx",'P',"fps pure count ex",false,1);

	/*频繁序列写入文件。可缺省，默认值 1(写入文件)*/
	a.add<int>("writeFpstoFile", 'w', "need to write to file", false,1);

	/*显示频繁序列。可缺省，默认值 0(不显示)*/
	a.add<int>("displayFps", 'd', "need to display the fps", false,0);

	/*频繁序列排列的次序。0->字典序，1->频次降序，2->频次升序。可缺省，默认值 1(频繁降序)*/
	a.add<int>("orderOfFps", 'o', "how to order the fps.\r\n\\t\t[0:with the dictionary order.]\r\n\t\t[1:with the frequent order desc.]\r\n\t\t[2:with the frequent order asc]", false,1);

	/*剔除掉自包含频繁序列，如 abcd 和 bcd 都是频繁序列，都出现 4 次，则 bcd 应该被剔除。可缺省，默认值 1(剔除)*/
	a.add<int>("removeContainedFps",'r',"remove the contained fps",false,1);

	/*转义特殊字符，如频繁序列中的 '\t' 若转义，则在文件中和控制台显示的将是 '\t'。可缺省，默认值 0(不转义)*/
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

