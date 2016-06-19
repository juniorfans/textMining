
#include "Utils.h"
extern int *getWcharsNext(const wchar_t * pStr,size_t totalLength);
#include "CircleString.h"

/************************************************************************/
/* 
	此类的作用是求出循环串的真实个数。如 abababab 中的 abab 的次数应为 2 次。
 
	若原串为 absabedabsabedabsabed，absabedabs在其中的计次是 2次。现在怀疑 absabedabs 的计数不是纯净的，要求出它的真实次数。
	令 _circleBase = "absabedabs"，_circleLen = 10，mayCircelLen = 7，即可能的最小循环串是 absabed.(事实也如此)
	则 absabedabs 的真实次数是“absabedabsabedabsabed”出现在 T 中的个数。

	在真正的挖掘过程中：
	设原串为 absabed absabed absabed absabed absabed absabed absabed absabed x absabed absabed x absabed absabed
	上面的串中空格是没有的，为了可观性被加上了。注意 x.它将多个循环串的所在的母串隔断了。
	要统计 absabedabs 的纯净次数，需要找出所有母串，将它在各个母串中的次数相加，即是所求。
	算法思路是基于两端延伸，找出所有母串的思路。
	假设 absabedabs 是位于 absabed absabed absabed absabed absabed absabed absabed absabed 中的串，对 absabedabs 两边周期
	延伸得到：absabedabs absabedabs absabedabs absabedabs absabedabs，它就是 absabedabs 在这一区域中有匹配的最长串。
	假设另外一个 absabedabs 也位于 absabed absabed absabed absabed absabed absabed absabed absabed 中，但由于我们可以标记
	这一区域已经处理过( _exsitMap 标记)，故可以不用处理。
	依此思想，对所有位置的该序列做同样操作，能找出它真正的次数。


*/
/************************************************************************/




CircleString::CircleString(const wchar_t *content,int contentLen,
						   int *rank,int *SA)
{
	_content = content;
	_contentLen = contentLen;
	_rank = rank;
	_SA = SA;
	_exsitMap = (bool*)malloc(sizeof(bool) * _contentLen);
	memset(_exsitMap,false,sizeof(bool) * _contentLen);
}

void CircleString::memoryDelete()
{
	if(NULL != _exsitMap)
	{
		free (_exsitMap);
	}
	_exsitMap = NULL;
}

CircleString::~CircleString(void)
{
	memoryDelete();
}

int CircleString:: circleCount(const wchar_t *_circleBase,int _circleLen,int _rank_S,int _rank_E)
{
	//先计算当前频繁序列是否存在 前缀和后缀 相等，因为只有以这个为先决条件
	//才可能存在“非纯净计数”
	int tlen = 0;
	{
		int *next = getWcharsNext(_circleBase,_circleLen);//next [0,totalLength]
		tlen = next[_circleLen];
		delete[] next;
	}
	if(0 != tlen)	//kmp 算法中，最后一个字符与前面某一个字符有对应，说明前缀与后缀相等，且这个长度是  tlen
	{
		//可能的循环周期长度为
		int mayCircelLen = _circleLen - tlen;
		vector<SE> record;

		int i = _rank_S;
		int s,e;
		doExpand(_circleBase,mayCircelLen,_SA[i ++],&s,&e);	//以排名第 i 的 circleBase 为单位向前，后延伸字符串 T	
		memset(_exsitMap + s,true,(e-s+1)*sizeof(bool));
		record.push_back(SE(s,e));
		
		while (i <= _rank_E)
		{
			if(!_exsitMap[_SA[i]])
			{
				doExpand(_circleBase,mayCircelLen,_SA[i],&s,&e);
				memset(_exsitMap + s,true,sizeof(bool) * (e-s+1));
				record.push_back(SE(s,e));
			}
			++ i;
		}
		//计算 circleBase 的个数
		
		//首个不小于 _circleLen 的 mayCircelLen周期整倍数
		int firstDiv = mayCircelLen;
		while (_circleLen > firstDiv)
		{
			firstDiv += mayCircelLen;
		}
		int rc = 0;
		vector<SE>::iterator it = record.begin();
		for (;it != record.end();++ it)
		{
			const SE &se = *it;
			rc += (se.e - se.s + mayCircelLen) / firstDiv;
			memset(_exsitMap + se.s,false,sizeof(bool) * (se.e-se.s+1));	//复原
		}
		return rc;
	}
	return _rank_E - _rank_S + 1;
}