#pragma once
#include <algorithm>
using std::make_heap;
using std::push_heap;
using std::pop_heap;
using std::sort_heap;
#include <vector>
using std::vector;
#include <iostream>
using std::cout;
using std::endl;
#include <ctime>

template<typename valueType,int _valueIsPointer>
bool greater_(valueType a,valueType b)
{
	return _valueIsPointer ? ((*a) > (*b)) : (a > b);
}

template<typename valueType,int _valueIsPointer>
bool less_(valueType a,valueType b)
{
	return _valueIsPointer ? ((*a) < (*b)) : (a < b);
}


template<typename valueType,int _valueIsPointer = 0>
class TopK
{
private:
	TopK(int k,int type)
	{
		_k = k;
		_type = type;
		_func = (1 == _type) ? greater_<valueType,_valueIsPointer> : less_<valueType,_valueIsPointer>;
		_curIndex = -1;
		_values = new valueType[_k];
		memset(_values,0,sizeof(valueType) * _k);
	}
public:
	static TopK * maxTopK(int k)
	{
		return new TopK(k,1);
	}
	static TopK * minTopK(int k)
	{
		return new TopK(k,0);
	}
	inline bool add(valueType v)
	{
		bool addSuucess = true;
		if(_k - 1 > (_curIndex + 1))	// _curIndex + 1 是元素个数
		{
			_values[++ _curIndex] = v;
		}
		else if(_k -1 == (_curIndex + 1))
		{
			_values[++ _curIndex] = v;
			make_heap(_values,_values + _k,_func);	//对 0 ~ _k-1 建立堆
		}
		else
		{
			//对于最小堆，新到元素大于顶元素则处理，否则直接忽略
			//对于最大堆，新到元素小于顶元素则处理，否则直接忽略
			if(1 == _type && greater_<valueType,_valueIsPointer>(v,_values[0]) /*v > _values[0]*/ || 1 != _type && less_<valueType,_valueIsPointer>(v,_values[0]) /*v < _values[0]*/)
			{
				pop_heap(_values,_values+_k,_func);	//导致把 _values[0] 和 _values[_k-1] 交换，同时对 _values[0 ~ _k-2] 建堆
				//此时 _values[_k-1] 上是被 pop 掉的元素,它即是被交换过来的 _values[0]
				//直接用 v 覆盖即可
				_values[_k - 1] = v;
				push_heap(_values,_values + _k,_func);
			}
			else
			{
				addSuucess = false;
			}
		}
		return addSuucess;
	}
	
	inline bool couldAdd(valueType v)
	{
		if(_k > (_curIndex + 1)) return true;	//当前元素没达到 _k 个可以直接插入 :)
		return (1 == _type && greater_<valueType,_valueIsPointer>(v,_values[0]) || 1 != _type && less_<valueType,_valueIsPointer>(v,_values[0]));
	}

	valueType* getTopK()
	{
		//如果堆没有吸收到 k 个元素，则说明堆没有被构建[没有来得及调用 函数 make_heap]
		if(_curIndex + 1 < _k)
		{
			make_heap(_values,_values + _curIndex + 1,_func);
		}
		sort_heap(_values,_values + _curIndex + 1,_func);	//堆排序是不稳定的，
		return _values;
	}
	inline int getRealSize()
	{
		return _curIndex + 1;
	}
	~TopK(void)
	{
		//_values 的堆内存交由调用者去回收
	}
private:
	int _k;		//top k
	int _type;	//_type 为 1 表示建立最小堆，此时求的是最大的 top k; 否则表示建立最大堆，求最小的 top k
	valueType *_values;
	int _curIndex;
	typedef bool (*func)(valueType a,valueType b);
	func _func;
};