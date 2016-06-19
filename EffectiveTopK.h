#pragma once

#include <algorithm>
using std::make_heap;
using std::push_heap;
using std::pop_heap;
#include <functional>
#include <queue>
using std::queue;

template<typename keyType,typename valueType>
struct KV 
{
	keyType _key;
	valueType _value;
};


template<typename valueType,typename keyType>
static inline bool operator < (const KV<valueType,keyType> &l,const KV<valueType,keyType> &r)
{
	return l._value < r._value;
}
template<typename valueType,typename keyType>
static inline bool operator > (const KV<valueType,keyType> &l,const KV<valueType,keyType> &r)
{
	return l._value > r._value;
}

template<typename valueType>
extern bool less_(valueType a,valueType b);

template<typename valueType>
extern bool greater_(valueType a,valueType b);


//template<typename valueType>
//class RMQEffective<valueType>;

template<typename valueType>
class EffectiveTopK
{
public:
	EffectiveTopK(valueType *sorArray,int len,int type)
		:_sorArray(sorArray),_len(len),_type(type)
	{
		_rmq = new RMQEffective<valueType>(_sorArray,_len,_type);
		_func = (1 == _type) ? greater_<KV<int,valueType>> : less_<KV<int,valueType>>;
	}
	~EffectiveTopK(void)
	{
		if(NULL != _rmq)
		{
			delete _rmq;
			_rmq = NULL;
		}
	}
	void doQueryTopKRecursive(int s,int e,KV<int,valueType> *topKs,int &curIndex,int k);
	void doQueryTopK(int s,int e,KV<int,valueType> *topKs,int &curIndex,int k);
	KV<int,valueType> * EffectiveTopK<valueType>:: queryTopK(int s,int e,int k);
private:
	valueType* _sorArray;
	int _len;
	int _type;//_type Ϊ 1 ��ʾ������С�ѣ���ʱ��������� top k; �����ʾ�������ѣ�����С�� top k
	RMQEffective<valueType> *_rmq;
	typedef bool (* FUNC) (KV<int,valueType> a,KV<int,valueType> b);
	FUNC _func;
};

struct SE
{
	int s,e;
	SE(int ls,int le):
	s(ls),e(le){}
};
template<typename valueType>
void EffectiveTopK<valueType>:: doQueryTopK(int s,int e,KV<int,valueType> *topKs,int &curIndex,int k)
{
	queue<SE> ses;
	ses.push(SE(s,e));
	while (0 != ses.size())
	{
		SE curSe(ses.front());
		s = curSe.s;
		e = curSe.e;
		ses.pop();
		int mIndex;
		valueType mValue = _rmq->query(s,e,&mIndex);
		if (curIndex + 1 > k - 1)
		{
			valueType topValue = topKs[0]._value;
			if(1 == _type && mValue > topValue || 1 != _type && mValue < topValue)
			{
				pop_heap(topKs,topKs + k,_func);
				topKs[k-1]._key = mIndex;
				topKs[k-1]._value = mValue;
				push_heap(topKs,topKs + k,_func);
			}
			else
			{
				continue;
			}
		}
		else if (curIndex + 1 < k - 1)	//curIndex + 1 ΪԪ�ظ���
		{
			++ curIndex;
			topKs[curIndex]._key = mIndex;
			topKs[curIndex]._value = mValue;
		}
		else // if(curIndex + 1 == k - 1)
		{
			++ curIndex;
			topKs[curIndex]._key = mIndex;
			topKs[curIndex]._value = mValue;
			make_heap(topKs,topKs+k,_func);
		}
		if(mIndex -1 >= s)
		{
			ses.push(SE(s,mIndex-1));
		}
		if(e >= mIndex + 1)
		{
			ses.push(SE(mIndex + 1,e));
		}
	}
}

/************************************************************************/
/* 
	ʹ����ȱ����Ǻܵ�Ч��������
	������ A[0......n]��������������Ҫ������ topk
	��ȱ����������ǣ�A[n/2],A[n/4],A[n/8]...�൱������߱�����������ʹ����С��
	�д�ŵ�ֵ����С��
	Ȼ������С���д�ŵ�ֵ���ϴ���㷨��Ч���������ģ����ڣ����ǿ���ʹ�öѶ�Ԫ��
	�����������Сֵ mValue( = _rmq->query(s,e)) �Ƚϣ�����ֵС�ڶѶ�Ԫ�أ�����Ҫ
	�� s,e ��������ٵݹ��ˣ��������ܼ��ٺܶ಻��Ҫ�ĵݹ�
*/
/************************************************************************/
template<typename valueType>
void EffectiveTopK<valueType>::doQueryTopKRecursive(int s,int e,KV<int,valueType> *topKs,int &curIndex,int k)
{
	if(s > e)
	{
		return;
	}
	int mIndex;
	int mValue = _rmq->query(s,e,&mIndex);
	if (curIndex + 1 > k - 1)
	{
		int topValue = topKs[0]._value;
		if(1 == _type && mValue > topValue || 1 != _type && mValue < topValue)
		{
			pop_heap(topKs,topKs + k,_func);
			topKs[k-1]._key = mIndex;
			topKs[k-1]._value = mValue;
			push_heap(topKs,topKs + k,_func);
		}
		else
		{
			return;//
		}
	}
	else if (curIndex + 1 < k - 1)	//curIndex + 1 ΪԪ�ظ���
	{
		++ curIndex;
		topKs[curIndex]._key = mIndex;
		topKs[curIndex]._value = mValue;
	}
	else // if(curIndex + 1 == k - 1)
	{
		++ curIndex;
		topKs[curIndex]._key = mIndex;
		topKs[curIndex]._value = mValue;
		make_heap(topKs,topKs+k,_func);
	}
	//�ݹ鴦��
//	if(e - s < 2) return;
	//left : s -> mIndex - 1
	
	doQueryTopKRecursive(s,mIndex-1,topKs,curIndex,k);
	//right : mIndex + 1 -> e
	doQueryTopKRecursive(mIndex + 1,e,topKs,curIndex,k);
}




template<typename valueType>
KV<int,valueType> * EffectiveTopK<valueType>:: queryTopK(int s,int e,int k)
{
	KV<int,valueType> *topKs = new KV<int,valueType>[k];
	int curIndex = -1;
	//doQueryTopKRecursive(s,e,topKs,curIndex,k);
	doQueryTopK(s,e,topKs,curIndex,k);
	sort_heap(topKs,topKs+curIndex+1,_func);	//�п���û��ȡ�� top k����������Ҫ�� curIndex ָʾ��ʵ�� top ����
	return topKs;
}
