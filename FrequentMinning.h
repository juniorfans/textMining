#pragma once

#include <vector>
using std::vector;
#include <string>
using std::string;
#include "FPSGettor.h"

class FrequentConfig
{
public:
	string _fileName;		//挖掘的文本文件
	int _minLen;			//频繁序列字符串的最小长度，低于此长度的字符串被忽略
	int _support;			//频繁序列频数最小支持度
	int _fpsListNums;		//频繁序列结果集保存条目数
	bool _pureCount;		//纯净计数，如原串为 aaaaa，'aa'的纯净计数为2次，非纯净计数为4次。即纯净计数情况下不允许一个位置被多个匹配同时占用
	bool _pureCountEx;		//采用纯净计数时，某些序列的前缀与该序列计次相同，删除该前缀序列。如原串为 aaabaaa，'aa'的纯净计数为2次，'aaa'的纯净计数也为2次，此时应该删除 aa
	bool _needWriteFpsToFile;	//是否将频繁序列写入文件
	bool _needDisplayFps;	//是否显示频繁序列
	bool _upLowCaseSensitive;	//处理文件时是否大小写敏感
	//bool _formatFps;	//将以{_formatFps_noneStartWchars}等字符开头的频繁序列删除。将包含{_formatFps_noneContainedWchars}等字符在中间或结尾的频繁序列转义
	wchar_t *_formatFps_removeFpsStartWchars;	//以某些字符开头的频繁序列被删除，如空格，\t,\r,\n等，它们一个一个被存放在字符串中
	bool _formatFps_transform;//对频繁序列中的影响正常显示的字符是否予以转义。如 \r,\n
	int _fpsOrder;	//将频繁序列显示或写文件时的顺序	0 : 原序/字典序列 | 1 ：频次降序 | 2 ：频次升序
	bool _fpsRemoveContained;	//去掉自包含的频繁序列
	//设置默认值
	FrequentConfig()
	{
		_minLen = 1;
		_support = 2;
		_fpsListNums = -1;	//当为 - 1 时保留所有的频繁序列
		_pureCount = true;
		_pureCountEx = true;
		_needWriteFpsToFile = true;
		_needDisplayFps = false;
		_upLowCaseSensitive = true;
		//_formatFps = true;
		_formatFps_removeFpsStartWchars = L" \t\r\n";
		_formatFps_transform = false;
		_fpsOrder = 1;
		_fpsRemoveContained = true;
	}
};

class FrequentMinning
{
public:
	FrequentMinning(const FrequentConfig& config);
	~FrequentMinning(void);
	void minning();
	wchar_t *getFpFromFsNode(FsNode *,int&);
private:
	void init();
	void destroy();
	void calHeight();
	wchar_t *transformSepecialChars(wchar_t *,int ,int &);
	void verifyHeight();
	void syncTo();
	void doSync(ostream *file,wchar_t *theChs,int nDocLength,int fpCounter);

	void showSuffixAndHeight(int *);
private:

	FrequentConfig _fconfig;

	File *_file;

	wchar_t*_content;	//读取文件获得的数据
	int _len;	//源数据长度
	int _alphabetSize;	//待处理的字符表的长度
	int *_height;	//_height[x] 表示排名第 x 的后缀和排在它前一名的后缀的最长公共子串（连续）的长度
	int *_SA;	//后缀数组

	pFsNode *_fps;
	int _fpsSize;
};
