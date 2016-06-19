#pragma once
#include "RMQEffective.h"
#include "TopK.h"


/************************************************************************/
/* 
SA 数组是后缀数组 ：SA[i] 表示排名第 i 的后缀序列在原序列中的索引号
height[i] 表示排名第 i 的后缀序列与排名第 i-1 的后缀序列的最长公共前缀的长度.height[0] 没有意义。
support 是频繁序列的支持度
minLen 指定了挖掘的频繁序列的最小长度（即小于此长度的频繁序列被忽略）
len 是原序列长度

通过 height 数组挖掘频繁序列的算法非常直观：通过二分法得到指定某段 height 的最小值 theMin，它就是这一段
height 数组对应的后缀序列集合的最小公共前缀的长度，只要它大于 minLen，同时这些后缀序列集合的个数(counter)
达到支持度 support，则就挖掘到了一个频繁序列：它的长度为 theMin，频繁序列的次数为 counter。
再以最小值 theMin 为分界点挖掘两边的最小值，递归处理。注意，本次的最小值和递归处理这两个紧挨的最小值不能一样，
否则会造成频繁序列重复挖掘。

算法过程详见 getFS

改进：
1.FPSGettor 接受 FpsFilter 参数，获得频繁序列
2.将 sort(fps) 步骤删除，改为在 FPSGettor 中建立最大堆/最小堆去收集频繁序列[若要求频繁序列为降序则使用最大堆，减序则使用最小堆]
3.将 cutFPs 步骤删除，改为在 FPSGettor 往堆中插入频繁序列时 cut
4.往堆中插入频繁序列时，对于相同次数的频繁序列检查是否存在频繁序列包含.设height的[s,e]区域最小值大于支持度，则[s,e]对应的序列集是
	频繁序列。判断这个集是否被包含，只需要看 [s,e] 的首尾两个后缀分别的“前一个后缀”的排名是否也相隔 e-s，如果是，则说明当前频繁序列集
	一定被包含。
5.若是频繁序列按次数递减排列，是否应该提供 RMQEffective.queryMax 的方法。因为如果有这个方法，则可以在二分法查找频繁序列
时优先找到次数大的频繁序列，这样就可以控制递归的深度。
*/
/************************************************************************/


class FsNode
{
public :
	/************************************************************************/
	/* 
		在原序列中，从 index 处开始的长度为 length 的后缀序列在原序列中出现了 counter 次
	*/
	/************************************************************************/
	int index,counter,length;
	FsNode(int tIndex,int tCounter,int tLength)
		:index(tIndex),counter(tCounter),length(tLength)
	{
		
	}

	FsNode()
	{
		
	}

	~FsNode(){}

	typedef FsNode* pFsNode;
	static inline bool greater(pFsNode lf,pFsNode rf)
	{
		return lf->counter > rf->counter;
	}

	static inline bool lesser(pFsNode lf,pFsNode rf)
	{
		return lf->counter < rf->counter;
	}

	FsNode& operator = (const FsNode& rf)
	{
		index = rf.index;
		counter = rf.counter;
		length = rf.length;
		return *this;
	}

	inline bool operator ==(const FsNode& rf)
	{
		return index == rf.index && counter == rf.counter && length == rf.length;
	}

	inline bool operator > (const FsNode& rf)
	{
		return counter > rf.counter;
	}

	inline bool operator < (const FsNode& rf)
	{
		return counter < rf.counter;
	}
};

typedef FsNode* pFsNode;
class CircleString;
class FPSFilter
{
public:
	int _limits;	//频繁序列的限制数目
	int _support;	//支持度
	int _minLen;	//频繁序列的最小长度
//	bool _formatFps;	//忽略掉[即删除掉]以空格,\t,\r,\n 等影响视觉的字符开头的频繁序列,这些字符一个个存储在 _formatFps_noneStartWchars
	wchar_t *_formatFps_removeStartWchars;
	int _order;		//频繁序列集以何种顺序返回[0 : 字典顺序，1 : 频次降序，2 : 频次增序]
	bool _removeContained;	//去掉被包含的频繁序列
	bool _pureCount;	//"频繁序列的纯净计数"，如原串为 aaaaa，'aa'的纯净计数为2次，非纯净计数为4次。即纯净计数情况下不允许一个位置被多个匹配同时占用
	bool _pureCountEx;	//采用纯净计数时，某些序列的前缀与该序列计次相同，删除该前缀序列。如原串为 aaabaaa，'aa'的纯净计数为2次，'aaa'的纯净计数也为2次，此时应该删除 aa

	FPSFilter()
		:_formatFps_removeStartWchars(L"\0")
	{}
};

class File;
class FPSGettor
{
public:
	FPSGettor(FPSFilter &filter,int *height,int *SA,int len,wchar_t *keyToUnicodemap,wchar_t* content);//int minLen,int *height,int len,int support,int *SA);
	
	pFsNode* getFS(int &nsize);
	~FPSGettor(void);
private:
	void doGetFSOrderly(int s,int e,int egnore);
	bool fpsContained(FsNode *curNode);
	pFsNode* innerGetFs(int &nsize);
	wchar_t * getFpFromFsNode(FsNode * curFP,int &theLen);
	void removePrefixWithSameCount(pFsNode* & fps,int& nsize);
	//bool fpsStartWithInvalidWchar(FsNode* curFP);
	//bool isInvalidStartWchar(wchar_t);
	void pureCountExAddFps(pFsNode curNode);
	void realAddFps(pFsNode node);
	void printFP(pFsNode node);
	int *_height;
	int _len,*_SA,*_rank;
	RMQEffective<int> *_rmqMin;
	TopK<pFsNode,1> *_topK;
	FPSFilter _filter;
	wchar_t *_keyToUnicode;
	wchar_t*_content;	//读取文件获得的数据
	pFsNode _lastAddInNode;
	vector<pFsNode> _noneTopKFs;
	CircleString *_cs;
};

#define KEY_TO_UNICODE(key) (*(_keyToUnicode + key))
