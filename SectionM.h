#pragma once
#include <stdio.h>
#include <math.h>
#include <string>
using std::string;
#include <ctime>
#include <cassert>

#define oneLevelBucket(element) (_rmq + ((element) * (_maxKLevel + 1)))
#define twoLevelBucket(element,k) (oneLevelBucket(element) + k)

#define MIN_INDEX(lindex,rindex) (INDEX_TO_RESULT(lindex) <= INDEX_TO_RESULT(rindex) ? (lindex) : (rindex))
#define MIN_RESULT(lindex,rindex) INDEX_TO_RESULT(MIN_INDEX(lindex,rindex))

#define MAX_INDEX(lindex,rindex) (INDEX_TO_RESULT(lindex) >= INDEX_TO_RESULT(rindex) ? (lindex) : (rindex))
#define MAX_RESULT(lindex,rindex) INDEX_TO_RESULT(MAX_INDEX(lindex,rindex))

#define INDEX_TO_RESULT(index) *(_sorArray+(index))
#define SET_RESULT_IF_NOT_NULL(p,r) if(NULL!=(p)) *(p)=(r)


template<typename valueType>
class SectionM
{
public:
	SectionM(valueType *height,int len,int maxQuerySection,int type = 0);
	~SectionM();

	valueType query(int start,int end,int *index = NULL);
	valueType queryNoLimit(int start,int end,int *index = NULL);
	int getMaxQuerySection(){return _maxQuerySection;}
private:
	void init();
	void initMin();
	void initMax();
	valueType queryNoLimitMin(int start,int end,int *index = NULL);
	valueType queryNoLimitMax(int start,int end,int *index = NULL);
	void memoryConstruct();
	void memoryDestroy();
	int const _type;
	valueType *_sorArray;
	int _len,_maxKLevel,_maxQuerySection;
	int *_rmq;	//索引
	//_maxQuerySection : 最多允许查询第 i 个及后面 _maxQuerySection 个元素（总共 _maxQuerySection + 1个元素）中的最小值
	//也即可以处理的查询区域最大值为  _maxQuerySection + 1
};





inline static int lowSqrt(int num)
{
	assert(num >= 0);
	int i = 0;
	while(0 != (num >>= 1))
	{
		++ i;
	}
	return i;
}


template<typename valueType>
SectionM<valueType>::SectionM(valueType *height,int len,int maxQuerySection,int type)
:_sorArray(height),_len(len),_type(type)
{
	_maxQuerySection = (0 == maxQuerySection) ? _len : maxQuerySection;
	_maxKLevel = lowSqrt(_maxQuerySection);
	init();
}

template<typename valueType>
SectionM<valueType>::~SectionM()
{
	memoryDestroy();
}

template<typename valueType>
valueType SectionM<valueType>::queryNoLimitMax(int start,int end,int *index)
{
	if(end - start <= _maxQuerySection)
		return query(start,end,index);
	if(end - start > 3 * _maxQuerySection)
	{
		printf("end - start is too big : %d\r\n",end - start);
	}
	if(end - start > _maxQuerySection)
	{
		int i = start;
		int curEnd = 0;
		int retIndex = -1;
		valueType maxmax = query(i,curEnd > end ? end : curEnd,&retIndex);
		SET_RESULT_IF_NOT_NULL(index,retIndex);
		do
		{
			curEnd = i + _maxQuerySection;
			int curMax = query(i,curEnd > end ? end : curEnd,&retIndex);
			if(curMax > maxmax)
			{
				maxmax = curMax;
				SET_RESULT_IF_NOT_NULL(index,retIndex);
			}
			i += _maxQuerySection;
		}while (curEnd < end);
		return maxmax;
	}
}

template<typename valueType>
valueType SectionM<valueType>::queryNoLimitMin(int start,int end,int *index)
{
	if(end - start <= _maxQuerySection)
		return query(start,end,index);
	if(end - start > 3 * _maxQuerySection)
	{
		printf("end - start is too big : %d\r\n",end - start);
	}
	if(end - start > _maxQuerySection)
	{
		int i = start;
		int curEnd = 0;
		int retIndex = -1;
		valueType minmin = query(i,curEnd > end ? end : curEnd,&retIndex);
		SET_RESULT_IF_NOT_NULL(index,retIndex);
		do
		{
			curEnd = i + _maxQuerySection;
			valueType curMin = query(i,curEnd > end ? end : curEnd,&retIndex);
			if(curMin < minmin)
			{
				minmin = curMin;
				SET_RESULT_IF_NOT_NULL(index,retIndex);
			}
			i += _maxQuerySection;
		}while (curEnd < end);
		return minmin;
	}
}

//查询 start(inclusive) ~ end (inclusive)之间的最小值。start -> end 的区间大小不受限制
template<typename valueType>
valueType SectionM<valueType>::queryNoLimit(int start,int end,int *index)
{
	if(0 == _type)
	{
		return queryNoLimitMin(start,end,index);
	}
	else return queryNoLimitMax(start,end,index);
}


//查询 start(inclusive) ~ end (inclusive)之间的最小/大值。start -> end 的区间大小受 _maxQuerySection 的限制
template<typename valueType>
valueType SectionM<valueType>::query(int start,int end,int *index)
{
	if(start == end)
	{
		SET_RESULT_IF_NOT_NULL(index,start);
		return INDEX_TO_RESULT(start);
	}
	assert(end - start<= _maxQuerySection);//start ~ end 共(end - start + 1项，而实际上，可以处理 _maxQuerySection + 1 区域，故有上式)
	int k = lowSqrt(end - start);
	int retIndex;
	if(0 == _type)
	{
		retIndex= MIN_INDEX(*twoLevelBucket(start,k),*twoLevelBucket(end - (1<<k),k));
	}
	else
	{
		retIndex = MAX_INDEX(*twoLevelBucket(start,k),*twoLevelBucket(end - (1<<k),k));
	}
	SET_RESULT_IF_NOT_NULL(index,retIndex);
	return INDEX_TO_RESULT(retIndex);
	//return min(*twoLevelBucket(start,k),*twoLevelBucket(end - (1<<k),k));// rmq[start][k],rmq[end - (1<<k)][k]);
}
/************************************************************************/
/* 
0 : 0 1 2 ... _sqrt
1 : 0 1 2 ... _sqrt
...
...
_len-2 : 0 1 2 ... _sqrt
*/
/************************************************************************/
template<typename valueType>
void SectionM<valueType>::memoryConstruct()
{
	_rmq = new int[(_len - 1) * (_maxKLevel + 1)];
	memset(_rmq,0,sizeof(int) * (_len - 1) * (_maxKLevel + 1));
}

template<typename valueType>
void SectionM<valueType>::memoryDestroy()
{
	delete[] _rmq;
	_rmq = NULL;
}


template<typename valueType>
void SectionM<valueType>::initMax()
{
	memoryConstruct();
	int k = 0;
	for(int i = 0;i < _len - 1;++ i)
	{
		*twoLevelBucket(i,k) = MAX_INDEX(i,i+1);
	}

	for(k = 1;k <= _maxKLevel;++k)
	{
		int sec = 1 << k;
		for(int i = 0;i < _len - 1;++ i)
		{
			if(i + sec >= _len)
			{
				break;
			}
			else
			{
				int index1 = *twoLevelBucket(i,k-1);
				int index2 = *twoLevelBucket(i + (sec>>1),k - 1);
				*twoLevelBucket(i,k) = MAX_INDEX(index1,index2);
			}
		}
	}
}

template<typename valueType>
void SectionM<valueType>::initMin()
{
	memoryConstruct();
	int k = 0;
	for(int i = 0;i < _len - 1;++ i)
	{
		*twoLevelBucket(i,k) = MIN_INDEX(i,i+1);
	}

	for(k = 1;k <= _maxKLevel;++k)
	{
		int sec = 1 << k;
		for(int i = 0;i < _len - 1;++ i)
		{
			if(i + sec >= _len)
			{
				break;
			}
			else
			{
				int index1 = *twoLevelBucket(i,k-1);
				int index2 = *twoLevelBucket(i + (sec>>1),k - 1);
				*twoLevelBucket(i,k) = MIN_INDEX(index1,index2);
			}
		}
	}
}

//第i个元素和后面 2^k 个元素的最小值
template<typename valueType>
void SectionM<valueType>::init()
{
	if(0 == _type)
	{
		initMin();
	}
	else
	{
		initMax();
	}
}


#undef oneLevelBucket
#undef twoLevelBucket

#undef MIN_INDEX
#undef MIN_RESULT

#undef MAX_INDEX
#undef MAX_RESULT

#undef INDEX_TO_RESULT
#undef SET_RESULT_IF_NOT_NULL