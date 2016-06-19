#pragma once

#include <vector>
using std::vector;
#include <string>
using std::string;
#include "FPSGettor.h"

/************************************************************************/
/* 
	FrequentConfig 是整个挖掘算法的配置，它包含所有的配置。
	FPSFilter 是专门用于过滤频繁序列的配置
*/
/************************************************************************/
class FrequentConfig
{
public:
	string _fileName;		//挖掘的文本文件
	bool _binaryFile;		//将文件当作二进制文件还是文本文件挖掘。默认是二进制文件
							//按文本文件挖掘的结果是字符序列，按二进制方式挖掘则是字节序列。
							//若将文本文件以二进制方式挖掘将得不到正确的字符序列，将二进制文件按文本文件方式挖掘可能导致内存耗费极大
							//当以文本文件方式挖掘时程序会正确分析文件的编码并处理好，不需指定编码

	int _minLen;			//频繁序列字符串的最小长度，低于此长度的字符串被忽略
	int _support;			//频繁序列频数最小支持度
	int _fpsListNums;		//频繁序列结果集保存条目数
	bool _pureCount;		//纯净计数，如原串为 aaaaa，'aa'的纯净计数为2次，非纯净计数为4次。即纯净计数情况下不允许一个位置被多个匹配同时占用
	bool _pureCountEx;		//采用纯净计数时，某些序列的前缀与该序列计次相同，删除该前缀序列。如原串为 aaabaaa，'aa'的纯净计数为2次，'aaa'的纯净计数也为2次，此时应该删除 aa
	bool _needWriteFpsToFile;	//是否将频繁序列写入文件
	bool _needDisplayFps;	//是否显示频繁序列
	bool _upLowCaseSensitive;	//处理文件时是否大小写敏感
	bool _formatFps_transform;//对频繁序列中的影响正常显示的字符是否予以转义。如 \r,\n
	int _fpsOrder;	//将频繁序列显示或写文件时的顺序	0 : 原序/字典序列 | 1 ：频次降序 | 2 ：频次升序
	bool _fpsRemoveContained;	//去掉自包含的频繁序列
	//设置默认值
	FrequentConfig()
	{
		_binaryFile = 1;
		_minLen = 1;
		_support = 2;
		_fpsListNums = -1;	//当为 - 1 时保留所有的频繁序列
		_pureCount = true;
		_pureCountEx = true;
		_needWriteFpsToFile = true;
		_needDisplayFps = false;
		_upLowCaseSensitive = true;
		//_formatFps = true;
		//_formatFps_removeFpsStartWchars = L" \t\r\n";
		_formatFps_transform = false;
		_fpsOrder = 1;
		_fpsRemoveContained = true;
	}
};

class MiningFile;
class FrequentMinning
{
public:
	FrequentMinning(const FrequentConfig& config);
	~FrequentMinning(void);
	void minning();
	wchar_t *getWCharFpFromFsNode(FsNode *,int&);
	byte * getByteFpFromFsNode(FsNode * curFP,int &);
private:
	void init();
	void destroy();
	void calHeight();
	void verifyHeight();
	void syncTo();
	void showSuffixAndHeight(int *);
private:

	FrequentConfig _fconfig;

	MiningFile *_file;

	//void *_content;	//读取文件获得的数据
	wchar_t *wchar_content;
	byte *char_content;
	int _len;			//源数据长度
	int _alphabetSize;	//待处理的字符表的长度
	int *_height;	//_height[x] 表示排名第 x 的后缀和排在它前一名的后缀的最长公共子串（连续）的长度
	int *_SA;	//后缀数组

	pFsNode *_fps;
	int _fpsSize;
};
