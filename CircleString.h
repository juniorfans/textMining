#pragma once
#include <vector>
using std::vector;

struct SE 
{
	SE(int ls,int le)
		:s(ls),e(le){}
	int s,e;
};
class CircleString
{
private:
	const wchar_t *_content;
	const wchar_t *_circleBase;
	int _rank_S,_rank_E;	//_circleBase ��������ʼ������
	int _contentLen,_circleLen;
	int *_rank;
	int *_SA;
	int *_height;
	bool *_exsitMap;
private:
	
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
	void doExpand(const wchar_t *realCircleBase,int len,int strPos,int *s,int *e)
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
	int circleCount(const wchar_t *_circleBase,int _circleLen,int _rank_S,int _rank_E);
	CircleString(const wchar_t *content,int contentLen,
		int *rank,int *SA);
	~CircleString(void);
	void memoryDelete();
};

