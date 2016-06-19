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
	int _rank_S,_rank_E;	//_circleBase 的排名开始，结束
	int _contentLen,_circleLen;
	int *_rank;
	int *_SA;
	int *_height;
	bool *_exsitMap;
private:
	
	/************************************************************************/
	/*
		将串 realCircleBase 中的可能的最小循环串对照原串向前后周期性扩展，得到最前面的开始位置 s，和最后面的开始位置 e
		如原串：abababababababab
		realCircleBase: abab,	len=2, 表示 readCircleBase 中开始位置的字符串是可能循环串，即：ab ;
								strPos=6, 表示 readCircleBase 在原串中的起始位置
		则 realCircleBase 将被扩展为：
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
			while (_content[i] == _content[i + len] && i != (len+curPos))	//最多匹配 len 个
			{
				++ i;
			}
			if(i != (len+curPos))//循环中断
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
			if(i != (len+curPos))//循环中断
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

