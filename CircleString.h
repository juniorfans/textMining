#pragma once
#include <vector>
using std::vector;

/************************************************************************/
/* 
��������������ѭ��������ʵ�������� abababab �е� abab �Ĵ���ӦΪ 2 �Ρ�

��ԭ��Ϊ absabedabsabedabsabed��absabedabs�����еļƴ��� 2�Ρ����ڻ��� absabedabs �ļ������Ǵ����ģ�Ҫ���������ʵ������
�� _circleBase = "absabedabs"��_circleLen = 10��mayCircelLen = 7�������ܵ���Сѭ������ absabed.(��ʵҲ���)
�� absabedabs ����ʵ�����ǡ�absabedabsabedabsabed�������� T �еĸ�����

���������ھ�����У�
��ԭ��Ϊ absabed absabed absabed absabed absabed absabed absabed absabed x absabed absabed x absabed absabed
����Ĵ��пո���û�еģ�Ϊ�˿ɹ��Ա������ˡ�ע�� x.�������ѭ���������ڵ�ĸ�������ˡ�
Ҫͳ�� absabedabs �Ĵ�����������Ҫ�ҳ�����ĸ���������ڸ���ĸ���еĴ�����ӣ���������
�㷨˼·�ǻ����������죬�ҳ�����ĸ����˼·��
���� absabedabs ��λ�� absabed absabed absabed absabed absabed absabed absabed absabed �еĴ����� absabedabs ��������
����õ���absabedabs absabedabs absabedabs absabedabs absabedabs�������� absabedabs ����һ��������ƥ��������
��������һ�� absabedabs Ҳλ�� absabed absabed absabed absabed absabed absabed absabed absabed �У����������ǿ��Ա��
��һ�����Ѿ������( _exsitMap ���)���ʿ��Բ��ô���
����˼�룬������λ�õĸ�������ͬ�����������ҳ��������Ĵ�����


*/
/************************************************************************/
/************************************************************************/
/*
���� realCircleBase �еĿ��ܵ���Сѭ��������ԭ����ǰ����������չ���õ���ǰ��Ŀ�ʼλ�� s���������Ŀ�ʼλ�� e
��ԭ����abababababababab
realCircleBase: abab,	len=2, ��ʾ readCircleBase �п�ʼλ�õ��ַ����ǿ���ѭ����������ab ;
strPos=6, ��ʾ readCircleBase ��ԭ���е���ʼλ��
�� realCircleBase ������չΪ��
abababababababab
s=0, e= 10.
*/
/************************************************************************/

struct SE 
{
	SE(int ls,int le)
		:s(ls),e(le){}
	int s,e;
};

template<typename CHAR_TYPE>
class CircleString
{
private:
	const CHAR_TYPE *_content;
	const CHAR_TYPE *_circleBase;
	int _rank_S,_rank_E;	//_circleBase ��������ʼ������
	int _contentLen,_circleLen;
	int *_rank;
	int *_SA;
	int *_height;
	bool *_exsitMap;
private:
	void doExpand(const CHAR_TYPE *realCircleBase,int len,int strPos,int *s,int *e)
	{
		int startV = strPos;
		int curPos = strPos;
		//-> start 
		while (0 <= (curPos -= len))
		{
			//[curPos,strPos - 1]
			int i = curPos;
			while (_content[i] == _content[i + len] && i != (len+curPos))	//���ƥ�� len ��
			{
				++ i;
			}
			if(i != (len+curPos))//ѭ���ж�
			{
				break;
			}
			else
			{
				startV = curPos;
			}
		}
		*s = startV;
		//-> end
		int endV = strPos;
		curPos = strPos;
		while ((curPos += len)< _contentLen)
		{
			//[curPos,strPos - 1]
			int i = curPos;
			while (_content[i] == _content[i - len] && i != (len+curPos))
			{
				++ i;
			}
			if(i != (len+curPos))//ѭ���ж�
			{
				break;
			}
			else
			{
				endV = curPos;
			}
		}
		*e = endV;
	}

public:
	int circleCount(const CHAR_TYPE *_circleBase,int _circleLen,int _rank_S,int _rank_E)
	{
		//�ȼ��㵱ǰƵ�������Ƿ���� ǰ׺�ͺ�׺ ��ȣ���Ϊֻ�������Ϊ�Ⱦ�����
		//�ſ��ܴ��ڡ��Ǵ���������
		int tlen = 0;
		{
			int *next = getWcharsNext(_circleBase,_circleLen);//next [0,totalLength]
			tlen = next[_circleLen];
			delete[] next;
		}
		if(0 != tlen)	//kmp �㷨�У����һ���ַ���ǰ��ĳһ���ַ��ж�Ӧ��˵��ǰ׺���׺��ȣ������������  tlen
		{
			//���ܵ�ѭ�����ڳ���Ϊ
			int mayCircelLen = _circleLen - tlen;
			vector<SE> record;

			int i = _rank_S;
			int s,e;
			doExpand(_circleBase,mayCircelLen,_SA[i ++],&s,&e);	//�������� i �� circleBase Ϊ��λ��ǰ���������ַ��� T	
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
			//���� circleBase �ĸ���

			//�׸���С�� _circleLen �� mayCircelLen����������
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
				memset(_exsitMap + se.s,false,sizeof(bool) * (se.e-se.s+1));	//��ԭ
			}
			return rc;
		}
		return _rank_E - _rank_S + 1;
	}

	CircleString(const CHAR_TYPE *content,int contentLen,
		int *rank,int *SA)
	{
		_content = content;
		_contentLen = contentLen;
		_rank = rank;
		_SA = SA;
		_exsitMap = (bool*)malloc(sizeof(bool) * _contentLen);
		memset(_exsitMap,false,sizeof(bool) * _contentLen);
	}
	~CircleString(void)
	{
		memoryDelete();
	}
	
	void memoryDelete()
	{
		if(NULL != _exsitMap)
		{
			free (_exsitMap);
		}
		_exsitMap = NULL;
	}
};

