#pragma once
#include "RMQEffective.h"
#include "TopK.h"


/************************************************************************/
/* 
SA �����Ǻ�׺���� ��SA[i] ��ʾ������ i �ĺ�׺������ԭ�����е�������
height[i] ��ʾ������ i �ĺ�׺������������ i-1 �ĺ�׺���е������ǰ׺�ĳ���.height[0] û�����塣
support ��Ƶ�����е�֧�ֶ�
minLen ָ�����ھ��Ƶ�����е���С���ȣ���С�ڴ˳��ȵ�Ƶ�����б����ԣ�
len ��ԭ���г���

ͨ�� height �����ھ�Ƶ�����е��㷨�ǳ�ֱ�ۣ�ͨ�����ַ��õ�ָ��ĳ�� height ����Сֵ theMin����������һ��
height �����Ӧ�ĺ�׺���м��ϵ���С����ǰ׺�ĳ��ȣ�ֻҪ������ minLen��ͬʱ��Щ��׺���м��ϵĸ���(counter)
�ﵽ֧�ֶ� support������ھ���һ��Ƶ�����У����ĳ���Ϊ theMin��Ƶ�����еĴ���Ϊ counter��
������Сֵ theMin Ϊ�ֽ���ھ����ߵ���Сֵ���ݹ鴦��ע�⣬���ε���Сֵ�͵ݹ鴦����������������Сֵ����һ����
��������Ƶ�������ظ��ھ�

�㷨������� getFS

�Ľ���
1.FPSGettor ���� FpsFilter ���������Ƶ������
2.�� sort(fps) ����ɾ������Ϊ�� FPSGettor �н�������/��С��ȥ�ռ�Ƶ������[��Ҫ��Ƶ������Ϊ������ʹ�����ѣ�������ʹ����С��]
3.�� cutFPs ����ɾ������Ϊ�� FPSGettor �����в���Ƶ������ʱ cut
4.�����в���Ƶ������ʱ��������ͬ������Ƶ�����м���Ƿ����Ƶ�����а���.��height��[s,e]������Сֵ����֧�ֶȣ���[s,e]��Ӧ�����м���
	Ƶ�����С��ж�������Ƿ񱻰�����ֻ��Ҫ�� [s,e] ����β������׺�ֱ�ġ�ǰһ����׺���������Ƿ�Ҳ��� e-s������ǣ���˵����ǰƵ�����м�
	һ����������
5.����Ƶ�����а������ݼ����У��Ƿ�Ӧ���ṩ RMQEffective.queryMax �ķ�������Ϊ��������������������ڶ��ַ�����Ƶ������
ʱ�����ҵ��������Ƶ�����У������Ϳ��Կ��Ƶݹ����ȡ�
*/
/************************************************************************/


class FsNode
{
public :
	/************************************************************************/
	/* 
		��ԭ�����У��� index ����ʼ�ĳ���Ϊ length �ĺ�׺������ԭ�����г����� counter ��
	*/
	/************************************************************************/
	int index,counter,length;
	FsNode(int tIndex,int tCounter,int tLength)
		:index(tIndex),counter(tCounter),length(tLength)
	{
		
	}

	FsNode()
	{
		
	}

	~FsNode(){}

	typedef FsNode* pFsNode;
	static inline bool greater(pFsNode lf,pFsNode rf)
	{
		return lf->counter > rf->counter;
	}

	static inline bool lesser(pFsNode lf,pFsNode rf)
	{
		return lf->counter < rf->counter;
	}

	FsNode& operator = (const FsNode& rf)
	{
		index = rf.index;
		counter = rf.counter;
		length = rf.length;
		return *this;
	}

	inline bool operator ==(const FsNode& rf)
	{
		return index == rf.index && counter == rf.counter && length == rf.length;
	}

	inline bool operator > (const FsNode& rf)
	{
		return counter > rf.counter;
	}

	inline bool operator < (const FsNode& rf)
	{
		return counter < rf.counter;
	}
};

typedef FsNode* pFsNode;
class CircleString;
class FPSFilter
{
public:
	int _limits;	//Ƶ�����е�������Ŀ
	int _support;	//֧�ֶ�
	int _minLen;	//Ƶ�����е���С����
//	bool _formatFps;	//���Ե�[��ɾ����]�Կո�,\t,\r,\n ��Ӱ���Ӿ����ַ���ͷ��Ƶ������,��Щ�ַ�һ�����洢�� _formatFps_noneStartWchars
	wchar_t *_formatFps_removeStartWchars;
	int _order;		//Ƶ�����м��Ժ���˳�򷵻�[0 : �ֵ�˳��1 : Ƶ�ν���2 : Ƶ������]
	bool _removeContained;	//ȥ����������Ƶ������
	bool _pureCount;	//"Ƶ�����еĴ�������"����ԭ��Ϊ aaaaa��'aa'�Ĵ�������Ϊ2�Σ��Ǵ�������Ϊ4�Ρ���������������²�����һ��λ�ñ����ƥ��ͬʱռ��
	bool _pureCountEx;	//���ô�������ʱ��ĳЩ���е�ǰ׺������мƴ���ͬ��ɾ����ǰ׺���С���ԭ��Ϊ aaabaaa��'aa'�Ĵ�������Ϊ2�Σ�'aaa'�Ĵ�������ҲΪ2�Σ���ʱӦ��ɾ�� aa

	FPSFilter()
		:_formatFps_removeStartWchars(L"\0")
	{}
};

class File;
class FPSGettor
{
public:
	FPSGettor(FPSFilter &filter,int *height,int *SA,int len,wchar_t *keyToUnicodemap,wchar_t* content);//int minLen,int *height,int len,int support,int *SA);
	
	pFsNode* getFS(int &nsize);
	~FPSGettor(void);
private:
	void doGetFSOrderly(int s,int e,int egnore);
	bool fpsContained(FsNode *curNode);
	pFsNode* innerGetFs(int &nsize);
	wchar_t * getFpFromFsNode(FsNode * curFP,int &theLen);
	void removePrefixWithSameCount(pFsNode* & fps,int& nsize);
	//bool fpsStartWithInvalidWchar(FsNode* curFP);
	//bool isInvalidStartWchar(wchar_t);
	void pureCountExAddFps(pFsNode curNode);
	void realAddFps(pFsNode node);
	void printFP(pFsNode node);
	int *_height;
	int _len,*_SA,*_rank;
	RMQEffective<int> *_rmqMin;
	TopK<pFsNode,1> *_topK;
	FPSFilter _filter;
	wchar_t *_keyToUnicode;
	wchar_t*_content;	//��ȡ�ļ���õ�����
	pFsNode _lastAddInNode;
	vector<pFsNode> _noneTopKFs;
	CircleString *_cs;
};

#define KEY_TO_UNICODE(key) (*(_keyToUnicode + key))
