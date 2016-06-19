#pragma once

#include <iostream>
using namespace::std;
#include <memory>
#include <limits>
#include <cassert>
#define BLOCK_SIZE 20		//	int BLOCK_SIZE;	//使用黑书里描述的 logn/2 区段大小可以获得线性的 rmq ，然而这可能导致块的数量瞬间爆炸 :)，造成内存使用量增长
#include "SectionM.h"
#include "RMQEffective.h"
#define MEMORY_SAVING

#include <algorithm>
using std::make_heap;
using std::push_heap;
using std::pop_heap;
#include <functional>
#include <limits>

template<typename valueType>
extern valueType minValue(valueType *sorArray,int start,int end,int *index);
template<typename valueType>
extern valueType maxValue(valueType *sorArray,int start,int end,int *index);


template<typename valueType>
class RMQEffective
{
public:
	RMQEffective(valueType *sorArray,int len,int type = 0);
	~RMQEffective(void);
	valueType query(int s,int e,int *index);
private:
	void init();
	void initMin();
	void initMax();

	valueType queryMin(int s,int e,int *index);
	valueType queryMax(int s,int e,int *index);
	
private:
	valueType *_sorArray;
	int _len;
	const int _type;	//最小值或者最大值 RMQ[ 0 : 最小值 ， 1 : 最大值]
	int *blockMIndexes;
	valueType *blockMs;
	int blockCounts;
	SectionM<valueType> *sm;
};

template<typename valueType>
RMQEffective<valueType>::~RMQEffective(void)
{
	delete[] blockMIndexes;
	delete[] blockMs;
	delete sm;
}

template<typename valueType>
RMQEffective<valueType>::RMQEffective(valueType *sorArray,int len,int type)
:_sorArray(sorArray),_len(len),_type(type)
{
	init();
}

template<typename valueType>
void RMQEffective<valueType>::initMin()
{
	blockCounts  = _len / BLOCK_SIZE + ((0 != (_len % BLOCK_SIZE)) ? 1 : 0);
	blockMIndexes = new int[blockCounts];
	memset(blockMIndexes,-1,sizeof(int) * blockCounts);
	blockMs = new valueType[blockCounts];
//	memset(blockMs,valueType(0),sizeof(valueType) * blockCounts);


	int minIndex = -1;
	int i ;
	for(i = 0;i < _len;i += BLOCK_SIZE)
	{
		blockMs[i / BLOCK_SIZE] = minValue<valueType>(_sorArray,i,i+BLOCK_SIZE - 1,&minIndex);
		blockMIndexes[i / BLOCK_SIZE] = minIndex;
	}
	if(_len != (i -= BLOCK_SIZE))
	{
		assert(blockCounts-1 == i / BLOCK_SIZE);
		blockMs[blockCounts - 1] = minValue<valueType>(_sorArray,i,_len - 1,&minIndex);
		blockMIndexes[blockCounts - 1] = minIndex;
	}
	sm = new SectionM<valueType>(blockMs,blockCounts,blockCounts - 1);
}

template<typename valueType>
void RMQEffective<valueType>::initMax()
{
	blockCounts  = _len / BLOCK_SIZE + ((0 != (_len % BLOCK_SIZE)) ? 1 : 0);
	blockMIndexes = new int[blockCounts];
	memset(blockMIndexes,-1,sizeof(int) * blockCounts);
	blockMs = new valueType[blockCounts];
//	memset(blockMs,valueType(0),sizeof(valueType) * blockCounts);


	int maxIndex = -1;
	int i ;
	for(i = 0;i <= _len - BLOCK_SIZE;i += BLOCK_SIZE)	//
	{
		blockMs[i / BLOCK_SIZE] = maxValue<valueType>(_sorArray,i,i+BLOCK_SIZE - 1,&maxIndex);
		blockMIndexes[i / BLOCK_SIZE] = maxIndex;
	}
	if(_len != i)
	{
		assert(blockCounts-1 == i / BLOCK_SIZE);
		blockMs[blockCounts - 1] = maxValue<valueType>(_sorArray,i,_len - 1,&maxIndex);
		blockMIndexes[blockCounts - 1] = maxIndex;
	}
	sm = new SectionM<valueType>(blockMs,blockCounts,blockCounts - 1,1);
}

template<typename valueType>
void RMQEffective<valueType>::init()
{
	if(0 == _type)
		initMin();
	else
		initMax();
}

#ifndef SET_RESULT_IF_NOT_NULL
#define SET_RESULT_IF_NOT_NULL(p,r) if(NULL!=(p)) *(p)=(r)
#endif


template<typename valueType>
valueType RMQEffective<valueType>::query(int s,int e,int *index)
{
	if(0 == _type)
		return queryMin(s,e,index);
	else
		return queryMax(s,e,index);
}

template<typename valueType>
valueType RMQEffective<valueType>::queryMin(int s,int e,int *index)
{
	assert(s <= e);
	int sBlock = s / BLOCK_SIZE;
	int eBlock = e / BLOCK_SIZE;
	if(sBlock == eBlock)
	{
		valueType theMin = _sorArray[s];
		int theIndex = s;
		for(int i = s + 1;i <= e;++ i)
		{
			if(_sorArray[i] < theMin)
			{
				theIndex = i;
				theMin = _sorArray[i];
			}
		}
		SET_RESULT_IF_NOT_NULL(index,theIndex);
		return theMin;
	}
	valueType min1,min2,min3;
	min1 = min2 = min3 = (numeric_limits<valueType>::max)();
	
	int index1,index2,index3;
	//s -> 当前块最末
	int toEnd = (sBlock + 1) * BLOCK_SIZE;
	index1 = s;
	min1 = _sorArray[index1];
	for(int i = s +1;i < toEnd;++ i)
	{
		if(_sorArray[i] < min1)
		{
			min1 = _sorArray[i];
			index1 = i;
		}
	}
	//sBlock + 1 -> eBlock - 1
	if(sBlock + 1 <= eBlock - 1)
	{
		min2 = sm->query(sBlock+1,eBlock-1,&index2);
		index2 = blockMIndexes[index2];
	}
	//eBlock 首 -> e
	int sStartPos = eBlock * BLOCK_SIZE;
	toEnd = e + 1;
	index3 = sStartPos;
	min3 = _sorArray[index3];
	for(int i = sStartPos +1;i < toEnd;++ i)
	{
		if(_sorArray[i] < min3)
		{
			min3 = _sorArray[i];
			index3 = i;
		}
	}

	SET_RESULT_IF_NOT_NULL(index,min1 <= min2 ? (min1 <= min3 ? index1 : index3) : (min2 <= min3 ? index2 : index3));
	return min1 <= min2 ? (min1 <= min3 ? min1 : min3) : (min2 <= min3 ? min2 : min3);
}


template<typename valueType>
valueType RMQEffective<valueType>::queryMax(int s,int e,int *index)
{
	assert(s <= e);
	int sBlock = s / BLOCK_SIZE;
	int eBlock = e / BLOCK_SIZE;
	if(sBlock == eBlock)
	{
		valueType theMax = _sorArray[s];
		int theIndex = s;
		for(int i = s + 1;i <= e;++ i)
		{
			if(_sorArray[i] > theMax)
			{
				theIndex = i;
				theMax = _sorArray[i];
			}
		}
		SET_RESULT_IF_NOT_NULL(index,theIndex);
		return theMax;
	}
	valueType max1,max2,max3;
	max1 = max2 = max3 = (numeric_limits<valueType>::min)();
	int index1,index2,index3;
	//s -> 当前块最末
	int toEnd = (sBlock + 1) * BLOCK_SIZE;
	index1 = s;
	max1 = _sorArray[index1];
	for(int i = s +1;i < toEnd;++ i)
	{
		if(_sorArray[i] > max1)
		{
			max1 = _sorArray[i];
			index1 = i;
		}
	}
	//sBlock + 1 -> eBlock - 1
	if(sBlock + 1 <= eBlock - 1)
	{
		max2 = sm->query(sBlock+1,eBlock-1,&index2);
		index2 = blockMIndexes[index2];
	}
	//eBlock 首 -> e
	int sStartPos = eBlock * BLOCK_SIZE;
	toEnd = e + 1;
	index3 = sStartPos;
	max3 = _sorArray[index3];
	for(int i = sStartPos +1;i < toEnd;++ i)
	{
		if(_sorArray[i] > max3)
		{
			max3 = _sorArray[i];
			index3 = i;
		}
	}

	SET_RESULT_IF_NOT_NULL(index,max1 >= max2 ? (max1 >= max3 ? index1 : index3) : (max2 >= max3 ? index2 : index3));
	return max1 >= max2 ? (max1 >= max3 ? max1 : max3) : (max2 >= max3 ? max2 : max3);
}