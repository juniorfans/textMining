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
		if(_k - 1 > (_curIndex + 1))	// _curIndex + 1 ��Ԫ�ظ���
		{
			_values[++ _curIndex] = v;
		}
		else if(_k -1 == (_curIndex + 1))
		{
			_values[++ _curIndex] = v;
			make_heap(_values,_values + _k,_func);	//�� 0 ~ _k-1 ������
		}
		else
		{
			//������С�ѣ��µ�Ԫ�ش��ڶ�Ԫ����������ֱ�Ӻ���
			//�������ѣ��µ�Ԫ��С�ڶ�Ԫ����������ֱ�Ӻ���
			if(1 == _type && greater_<valueType,_valueIsPointer>(v,_values[0]) /*v > _values[0]*/ || 1 != _type && less_<valueType,_valueIsPointer>(v,_values[0]) /*v < _values[0]*/)
			{
				pop_heap(_values,_values+_k,_func);	//���°� _values[0] �� _values[_k-1] ������ͬʱ�� _values[0 ~ _k-2] ����
				//��ʱ _values[_k-1] ���Ǳ� pop ����Ԫ��,�����Ǳ����������� _values[0]
				//ֱ���� v ���Ǽ���
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
		if(_k > (_curIndex + 1)) return true;	//��ǰԪ��û�ﵽ _k ������ֱ�Ӳ��� :)
		return (1 == _type && greater_<valueType,_valueIsPointer>(v,_values[0]) || 1 != _type && less_<valueType,_valueIsPointer>(v,_values[0]));
	}

	valueType* getTopK()
	{
		//�����û�����յ� k ��Ԫ�أ���˵����û�б�����[û�����ü����� ���� make_heap]
		if(_curIndex + 1 < _k)
		{
			make_heap(_values,_values + _curIndex + 1,_func);
		}
		sort_heap(_values,_values + _curIndex + 1,_func);	//�������ǲ��ȶ��ģ�
		return _values;
	}
	inline int getRealSize()
	{
		return _curIndex + 1;
	}
	~TopK(void)
	{
		//_values �Ķ��ڴ潻�ɵ�����ȥ����
	}
private:
	int _k;		//top k
	int _type;	//_type Ϊ 1 ��ʾ������С�ѣ���ʱ��������� top k; �����ʾ�������ѣ�����С�� top k
	valueType *_values;
	int _curIndex;
	typedef bool (*func)(valueType a,valueType b);
	func _func;
};