#include <vector>
using std::vector;
#include <string>
using std::string;
#include "RMQEffective.h"
#include "TopK.h"
#include "FPSGettor.h"
#include "RMQEffective.h"
#include <stdio.h>
#include "Utils.h"
#include <algorithm>
using std::make_heap;
using std::push_heap;
#include "CircleString.h"
#include "FPSGettor.h"

/************************************************************************/
/* 
01:
计算是否一个频繁序列被包含的算法
a a a b
a a b
a b
b

假设遍历到了第二行和第三行后缀序列{aab, ab}，得到频繁序列集：{a}，下面判断它是否被别的频繁序列包含。
方法是：
1.计算aab ，ab 这两个序列之间的相隔｛直接用频繁序列集的大小表示｝，为 2
2.这两个序列分别的“前一个”序列{aaab,aab}的首个字符是否相等，且这两个序列之间间隔是否达到在第一步中求得的间隔，
如果这两个条件被满足则说明第一步中的频繁序列必然被当前频繁序列包含


举个更全面的反例(原序列：a c a a b a c)：
后缀序列排序情况：
a a b a c
a b a c
a c
a c a a b a c
b a c
c
c a a b a c

1.求得频繁序列 a [第1行到第4行]
2.其中第四行刚好是第 0 个后缀，它前面没有后缀了，所以，没有任何频繁序列集会包含这四个序列。

3.求得频繁序列 c [第7行到第8行]
4.这两行分别的前一个后缀序列是 [ac,acaabac]即第三行和第四行，满足了两个要求：[首个字符相等且间隔达到了2]
故新的频繁序列 ac 必然会包含第七，八行的频繁序列 c 
*/

/************************************************************************/
/* 
02:
已经去 contained 的频繁序列中依然可能出现 contained，这个原因不那么明朗：
若挖掘序列中存在 \r\nabcd 是频繁序列，且 abcdedf 也是频繁序列，若使用了
去掉“以无意义字符开头的频繁序列”，则将导致"\r\nabcd" 不被识别，而它的后缀 "abcd"
将被识别为频繁序列（若选择了“以无意义字符开头的频繁序列“，则不会认为 "abcd"被“\r\nabcd”包含），
而这将导致一个意料之外的结果：新识别的频繁序列"abcd"将被“abcdedf” 所包含，而程序意识不到这个问题。


解决的办法是，不提供 “以无意义字符开头的频繁序列“ 这个选项，将所有的“无意义字符”转义显示即可。
*/
/************************************************************************/

/************************************************************************/
/* 
03:
本挖掘算法自身有一个属性：一个序列的前缀的计数必然大于该序列的次数。若前缀的计数等于该序列的次数，则前缀不应该单独被挖掘出，应被
包含在该序列的次数信息中。
来看一个很不明朗的问题：
使用了纯净计数（如原串为 aaaaa，'aa'的纯净计数为2次，非纯净计数为4次。即纯净计数情况下不允许一个位置被多个匹配同时占用）的
挖掘结果可能会出现无法预计的结果：
如：对于原串：aaabaaa，支持度2，使用纯净计数的情况下，得到以下结果：
a : 6
aaa : 2
aa : 2
从结果的表面上看，这是一个不可能出现的局面，因为算法本身的属性决定了，一切前缀字符串的次数如果与母串的次数一样，则这个前缀字符串是不会
被挖掘出的，也不应该被挖出。
之所以会得到这样的结果，并不是算法本身的问题，而是发生了巧合：使用了纯净计数的较长序列的个数碰巧与它的前缀的纯净计数相等。
这个问题的解决方法是：将次数相等的前缀串从频繁序列集合中去掉。
*/
/************************************************************************/

//#define MY_DEBUG

FPSGettor::FPSGettor(FPSFilter &filter,int *height,int *SA,int len,wchar_t *keyToUnicodeMap,wchar_t* content)//int minLen,int *height,int len,int support,int *SA)
:_filter(filter),_height(height),_SA(SA),_len(len),_keyToUnicode(keyToUnicodeMap),_content(content),
_rank(NULL),_topK(NULL),_lastAddInNode(NULL)
{
#ifdef MY_DEBUG
	printf("\r\n----------\r\n");
	for (int i = 0;i < _len;++ i)
	{
		printf("%d ",_height[i]);
	}printf("\r\n");
#endif
	_rmqMin = new RMQEffective<int>(height,len,0);//最小值 RMQ 
	if((-1 != _filter._limits && 1 <= _filter._order && 2 >= _filter._order))
	{
		_topK  = (1 == _filter._order) ? TopK<pFsNode,1>::maxTopK(_filter._limits) : TopK<pFsNode,1>::minTopK(_filter._limits);
	}
	else
	{
		if(-1 != _filter._limits)
		{
			_noneTopKFs.reserve(_filter._limits);
		}
	}

	_rank = new int[_len];
	for (int i = 0;i < _len;++ i)
	{
		_rank[_SA[i]] = i;//_SA[i] 表示排名第 i 的后缀的下标. _rank[i] 表示第 i 个后缀的排名
	}

	_cs = new CircleString(_content,_len,_rank,_SA);
}

FPSGettor::~FPSGettor(void)
{
	if(NULL != _rmqMin)
	{
		delete _rmqMin;
	}
	if(NULL != _topK)
	{
		delete _topK;
	}
	if(NULL != _rank)
	{
		delete _rank;
	}
	if(NULL != _cs)
	{
		delete _cs;
	}
}
#define SET_RESULT_IF_NOT_NULL(p,r) if(NULL!=(p)) *(p)=(r)

pFsNode* FPSGettor::innerGetFs(int &nsize)
{
	doGetFSOrderly(1,_len - 1,-1);//_height[0] 是没有意义的，所以 1 ~ len-1

	if(NULL != _topK)
	{
		nsize = _topK->getRealSize();
		pFsNode *fps = _topK->getTopK();
		nsize = _topK->getRealSize();
		return fps;
	}
	else
	{
		nsize = _noneTopKFs.size();
		pFsNode * fps = new pFsNode[nsize];

		for(int i = 0;i < nsize;++ i)
		{
			fps[i] = _noneTopKFs[i];
		}
		if(1 == _filter._order)
			sort(fps,fps+nsize,FsNode::greater);
		else if(2 == _filter._order)
			sort(fps,fps+nsize,FsNode::lesser);
		return fps;
	}
}

pFsNode* FPSGettor::getFS(int &nsize)
{
	pFsNode *retFpNode = innerGetFs(nsize);
	
	return retFpNode;
}

/************************************************************************/
/* 
	获得未转换字符前的频繁序列
*/
/************************************************************************/
wchar_t * FPSGettor::getFpFromFsNode(FsNode * curFP,int &theLen)
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
		theChs[i-1] = curWCh;
	}
	return theChs;
}


/************************************************************************/
/* 
	删除与原串次数相同的前缀串
	此处的参数 fps 是已按次数排序的频繁序列(不管是正序还是逆序)

	@Deprecation
	这种解决思路有可能是错误的：不一定这两个序列挨在一起。。。
*/
/************************************************************************/
void FPSGettor::removePrefixWithSameCount(pFsNode* & fps,int& nsize)
{
	if(0 == nsize)
		return;
	vector<pFsNode> retFps;
	pFsNode lastFps = fps[0];
	int lastFpsLen = 0;
	for (int i = 1;i < nsize;++ i)
	{
		pFsNode curFps = fps[i];
		if(curFps->counter == lastFps->counter)
		{
			int curFpsLen = 0;
			wchar_t *lastfpsStr = getFpFromFsNode(lastFps,lastFpsLen);
			wchar_t *curFpStr = getFpFromFsNode(curFps,curFpsLen);
			char fr = prefixRelation(lastfpsStr,curFpStr);
			if(1 == fr)	//lastfpsStr 是 curFpStr 的前缀，保留更长的序列 curFpStr(去掉 lastFps)
			{
				retFps.push_back(curFps);
			}
			else if(2 == fr)
			{
				retFps.push_back(lastFps);
			}
			else
			{
				assert(0 != fr);	//不可能出现这两个序列一样的情况
			}
		}
	}

	delete[] fps;
	nsize = retFps.size();
	fps = new pFsNode[nsize];
	for (int i = 0;i < nsize;++ i)
	{
		fps[i] = retFps[i];
	}
}

void FPSGettor:: printFP(pFsNode node)
{
	int si = node->index;
	int sl = node->length;
	wprintf(L"\r\nFsNode:");
	for (int i = 0;i < sl;++ i)
	{
		wprintf(L"%c",KEY_TO_UNICODE(_content[si + i]));
	}
	wprintf(L"\r\n");
}

/************************************************************************/
/* 
	参考上面 03 注释
	c : 频繁序列次数
	f : 频繁序列
	m : 频繁序列长度

	
	上一轮迭代：
	c1, f1, m1

	本轮迭代：
	c2, f2, m2
	
	下一轮迭代：
	c3, f3, m3

	-----------------------------------------------------------
	在不考虑纯净计数的情况下有：
	c1 > c2 > c3
	m1 < m2 < m3
	f1 -> f2 -> f3	(f1 是 f2 的前缀，f2 是 f3 的前缀)
	-----------------------------------------------------------

	当考虑纯净计数的情况时：
	c1 >= c2 >= c3
	m1 < m2 < m3
	f1 -> f2 -> f3	(f1 是 f2 的前缀，f2 是 f3 的前缀)

	故不可能存在 c1 与 c3 相等，而与 c2 不相等。
	导出的结论是：
	当考虑纯净计数时，为了去掉次数相同的两个序列：一个序列是另一个序列的前缀，
	只需要对相邻的迭代调用控制：
		上一次加入的频繁序列若与这次加入的序列的次数相同即抛弃前面加入的一个(它是本次频繁序列的前缀)。
	
	-----------------------------------------------------------

	注意 curNode 一定是已经使用纯净计数后的频繁序列。

	使用一个缓存机制：接受到一个频繁序列时，不真正地加入，而是等下一个频繁序列到来，比较次数是否
	相同，若相同则丢弃前一个，更新“前一个”，返回。否则加入前一个，更新“前一个”，返回。

	注意 pureCountExAddFps 连接的两次调用不仅仅会出现在 doGetFSOrderly 前后两次纵深迭代中，
	还会出现在 doGetFSOrderly 中的两个平行分支 doGetFSOrderly 中：一条分支的迭代完毕，切换到另一
	条分支的迭代上。但是上面的逻辑只适用于前后两次纵深迭代，不适应于后者。
	处理方法：
		pureCountExAddFps 增加逻辑：当 curNode 为 NULL 时视作一种“迭代完毕通知”，此时可以做两件事
			a)如果缓存中还有没有加入的频繁序列(即 _lastAddInNode 不为 NULL)则直接加入
			b)设置 _lastAddInNode 为 NULL
		然后在一次 doGetFSOrderly 迭代完毕之时调用 pureCountExAddFps(NULL) 即可。
*/
/************************************************************************/
void FPSGettor::pureCountExAddFps(pFsNode curNode)
{
	if(NULL == curNode)	//纵深迭代终止通知
	{
		if(NULL != _lastAddInNode)
		{
			realAddFps(_lastAddInNode);
			_lastAddInNode = NULL;
		}
		return;
	}

	if(NULL == _lastAddInNode)
	{
		_lastAddInNode = curNode;
		return;
	}
	else
	{
		if(_lastAddInNode->counter == curNode->counter)
		{
			_lastAddInNode = curNode;
		}
		else
		{
			realAddFps(_lastAddInNode);
			_lastAddInNode = curNode;
		}
	}
}

void FPSGettor::realAddFps(pFsNode node)
{
	if(NULL != _topK)
	{
		if(!_topK->add(node))
		{
			delete node;
		}
	}
	else
	{
		if(_noneTopKFs.size() != _filter._limits)
		{
			_noneTopKFs.push_back(node);
		}
		else
		{
			delete node;
			return;
		}
	}
}


void FPSGettor::doGetFSOrderly(int s,int e,int egnore)
{

	const int MINSEC = _filter._support - 1;
	int minIndex;
	int theMin = _rmqMin->query(s,e,&minIndex);

	//height[s] 表示排名第 s 与排名第 s-1 的最长公共前缀
	//所以当前频繁序列应该是从：排名第 s-1 的后缀到排名第 e 的后缀，故有 e-s+2个
	if(e - s + 1 >= MINSEC)						//最小支持度
		if(egnore != theMin)					//父子递归条件.如果父区块和子区块里面的
		{
			if (theMin >= _filter._minLen)		//最小长度
			{
				pFsNode curNode = new FsNode(_SA[s-1],e - s + 2,theMin);
				
				if(NULL == _topK || _topK->couldAdd(curNode))	//提前看是否能插入到 _topK 或者是 _noneTopKFs
				{
					//v7.0
					//当考虑 formatFps 时，curNode 以 invalid char 开头，则忽略掉该频繁序列
					//当考虑 formatFps 时，curNode 必须不以 invalid char 开头，才进行以下处理
					//当不考虑 formatFps 时，进行以下处理

					//v7.1
					//由于 _formatFps 可能导致一个不明朗的错误，参见本文件注释 02
					//此处去掉 _formatFps 的作用
					/*
					if(!_filter._formatFps || !fpsStartWithInvalidWchar(curNode))
					{*/
					
					if(!_filter._removeContained || !fpsContained(curNode))
					{
						//计算 curNode 的真实次数：
						if(_filter._pureCount)
						{
							const wchar_t *circleBase = _content + _SA[s-1];
							int circleLen = theMin;

							int realCount = _cs->circleCount(circleBase,circleLen,s-1,e);
							curNode->counter = realCount;
						}
						
						if(curNode->counter - 1 >= MINSEC)	//参考上面的判断
						{
							if(_filter._pureCount && _filter._pureCountEx)
								pureCountExAddFps(curNode);
							else
								realAddFps(curNode);
						}
					}
					else
					{
						delete curNode;
						return;
						//如果当前的频繁序列都被包含，则递归调用里面的频繁序列也必然被包含，所以可以终止递归						
					}//end for fpsContained check
					/*
					}
					else
					{
						delete curNode;
						//尽管当前频繁序列以 invalid 字符开头，为了挖掘它后面的频繁序列，不应该阻止递归
					}
					*/
					
				}
				else
				{
					delete curNode;
				}
			}
		}
	//迭代
	int i;
	i = minIndex - 1;
	while (-1 != i && theMin == _height[i])
	{
		-- i;
	}
	if(i >= s)
	{
		doGetFSOrderly(s,i,theMin);
		pureCountExAddFps(NULL);	//一条纵深迭代完成，需要清理一下缓存，更新 _lastAddInNode
	}
	i = minIndex + 1;
	while (_len != i && theMin == _height[i])
	{
		++ i;
	}
	if(e >= i)
	{
		doGetFSOrderly(i,e,theMin);
		pureCountExAddFps(NULL);	//一条纵深迭代完成，需要清理一下缓存，更新 _lastAddInNode
	}

	pureCountExAddFps(NULL);	//本条纵深迭代完成，需要清理一下缓存，更新 _lastAddInNode
}


/************************************************************************/
/* 
//如果考虑了 formatFps，且当前频繁序列集合不以invalidchar 开头，
//且{2 当前频繁序列集合的首尾两个序列的各自前一个序列的第一个字符相等且它们的跨度刚好等于 fpsCounts，则当前的频繁序列集合是被包含的。}
//如果不考虑 formatFps，则只需要满足{2}即可
//如果考虑了 formatFps，且当前频繁序列集合刚好就以invalidchar 开头，则当前频繁序列是会被删除的，则考虑它是否被包含是没有意义的
*/
/************************************************************************/
inline bool FPSGettor:: fpsContained(FsNode *curFP)
{
	int fpsCounts = curFP->counter; 
	int startOfFpIndex = curFP->index;	//频繁序列集中，第一个序列是哪一个后缀序列
	if(0 == startOfFpIndex) return false; //第一个后缀序列不可能是被包含的频繁序列
	int endofFpIndex = _SA[_rank[startOfFpIndex] + fpsCounts - 1];//频繁序列集中，最后一个序列是哪一个后缀序列
	if(0 == endofFpIndex) return false;
	int beforeStartOfFpIndex = startOfFpIndex - 1;
	int beforeEndOfFpIndex = endofFpIndex - 1;
	//如果考虑了 formatFps，且当前频繁序列集合不以invalidchar 开头，
	//且{2 当前频繁序列集合的首尾两个序列的各自前一个序列的第一个字符相等且它们的跨度刚好等于 fpsCounts，则当前的频繁序列集合是被包含的。}
	//如果不考虑 formatFps，则只需要满足{2}即可
	//如果考虑了 formatFps，且当前频繁序列集合刚好就以invalidchar 开头，则当前频繁序列是会被删除的，则考虑它是否被包含是没有意义的
	return /*(!_filter._formatFps || !isInvalidStartWchar(KEY_TO_UNICODE(_content[beforeEndOfFpIndex]))) && */
		 _content[beforeEndOfFpIndex] == _content[beforeStartOfFpIndex] && fpsCounts == (_rank[beforeEndOfFpIndex] - _rank[beforeStartOfFpIndex] + 1);
}

/*
inline bool FPSGettor:: fpsStartWithInvalidWchar(FsNode* curFP)
{
	int suffixIndex = curFP->index;
	wchar_t curWCh = KEY_TO_UNICODE(*(_content + suffixIndex));
	return isInvalidStartWchar(curWCh);
}
*/

/*
inline bool FPSGettor:: isInvalidStartWchar(wchar_t curWCh)
{
	wchar_t *invalidWchars = _filter._formatFps_removeStartWchars;
	for (;*invalidWchars;++invalidWchars)
	{
		if(*invalidWchars == curWCh)
		{
			return true;
		}
	}
	return false;
}
*/


#undef SET_RESULT_IF_NOT_NULL
#undef KEY_TO_UNICODE