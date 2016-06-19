#include <vector>
using std::vector;
#include <string>
using std::string;
#include "RMQEffective.h"
#include "TopK.h"
#include "FPSGettor.h"
#include "RMQEffective.h"
#include <stdio.h>
#include "Utils.h"
#include <algorithm>
using std::make_heap;
using std::push_heap;
#include "CircleString.h"
#include "FPSGettor.h"

/************************************************************************/
/* 
01:
�����Ƿ�һ��Ƶ�����б��������㷨
a a a b
a a b
a b
b

����������˵ڶ��к͵����к�׺����{aab, ab}���õ�Ƶ�����м���{a}�������ж����Ƿ񱻱��Ƶ�����а�����
�����ǣ�
1.����aab ��ab ����������֮��������ֱ����Ƶ�����м��Ĵ�С��ʾ����Ϊ 2
2.���������зֱ�ġ�ǰһ��������{aaab,aab}���׸��ַ��Ƿ���ȣ�������������֮�����Ƿ�ﵽ�ڵ�һ������õļ����
���������������������˵����һ���е�Ƶ�����б�Ȼ����ǰƵ�����а���


�ٸ���ȫ��ķ���(ԭ���У�a c a a b a c)��
��׺�������������
a a b a c
a b a c
a c
a c a a b a c
b a c
c
c a a b a c

1.���Ƶ������ a [��1�е���4��]
2.���е����иպ��ǵ� 0 ����׺����ǰ��û�к�׺�ˣ����ԣ�û���κ�Ƶ�����м���������ĸ����С�

3.���Ƶ������ c [��7�е���8��]
4.�����зֱ��ǰһ����׺������ [ac,acaabac]�������к͵����У�����������Ҫ��[�׸��ַ�����Ҽ���ﵽ��2]
���µ�Ƶ������ ac ��Ȼ��������ߣ����е�Ƶ������ c 
*/

/************************************************************************/
/* 
02:
�Ѿ�ȥ contained ��Ƶ����������Ȼ���ܳ��� contained�����ԭ����ô���ʣ�
���ھ������д��� \r\nabcd ��Ƶ�����У��� abcdedf Ҳ��Ƶ�����У���ʹ����
ȥ�������������ַ���ͷ��Ƶ�����С����򽫵���"\r\nabcd" ����ʶ�𣬶����ĺ�׺ "abcd"
����ʶ��ΪƵ�����У���ѡ���ˡ����������ַ���ͷ��Ƶ�����С����򲻻���Ϊ "abcd"����\r\nabcd����������
���⽫����һ������֮��Ľ������ʶ���Ƶ������"abcd"������abcdedf�� ����������������ʶ����������⡣


����İ취�ǣ����ṩ �����������ַ���ͷ��Ƶ�����С� ���ѡ������еġ��������ַ���ת����ʾ���ɡ�
*/
/************************************************************************/

/************************************************************************/
/* 
03:
���ھ��㷨������һ�����ԣ�һ�����е�ǰ׺�ļ�����Ȼ���ڸ����еĴ�������ǰ׺�ļ������ڸ����еĴ�������ǰ׺��Ӧ�õ������ھ����Ӧ��
�����ڸ����еĴ�����Ϣ�С�
����һ���ܲ����ʵ����⣺
ʹ���˴�����������ԭ��Ϊ aaaaa��'aa'�Ĵ�������Ϊ2�Σ��Ǵ�������Ϊ4�Ρ���������������²�����һ��λ�ñ����ƥ��ͬʱռ�ã���
�ھ������ܻ�����޷�Ԥ�ƵĽ����
�磺����ԭ����aaabaaa��֧�ֶ�2��ʹ�ô�������������£��õ����½����
a : 6
aaa : 2
aa : 2
�ӽ���ı����Ͽ�������һ�������ܳ��ֵľ��棬��Ϊ�㷨��������Ծ����ˣ�һ��ǰ׺�ַ����Ĵ��������ĸ���Ĵ���һ���������ǰ׺�ַ����ǲ���
���ھ���ģ�Ҳ��Ӧ�ñ��ڳ���
֮���Ի�õ������Ľ�����������㷨��������⣬���Ƿ������ɺϣ�ʹ���˴��������Ľϳ����еĸ�������������ǰ׺�Ĵ���������ȡ�
�������Ľ�������ǣ���������ȵ�ǰ׺����Ƶ�����м�����ȥ����
*/
/************************************************************************/

//#define MY_DEBUG

FPSGettor::FPSGettor(FPSFilter &filter,int *height,int *SA,int len,wchar_t *keyToUnicodeMap,wchar_t* content)//int minLen,int *height,int len,int support,int *SA)
:_filter(filter),_height(height),_SA(SA),_len(len),_keyToUnicode(keyToUnicodeMap),_content(content),
_rank(NULL),_topK(NULL),_lastAddInNode(NULL)
{
#ifdef MY_DEBUG
	printf("\r\n----------\r\n");
	for (int i = 0;i < _len;++ i)
	{
		printf("%d ",_height[i]);
	}printf("\r\n");
#endif
	_rmqMin = new RMQEffective<int>(height,len,0);//��Сֵ RMQ 
	if((-1 != _filter._limits && 1 <= _filter._order && 2 >= _filter._order))
	{
		_topK  = (1 == _filter._order) ? TopK<pFsNode,1>::maxTopK(_filter._limits) : TopK<pFsNode,1>::minTopK(_filter._limits);
	}
	else
	{
		if(-1 != _filter._limits)
		{
			_noneTopKFs.reserve(_filter._limits);
		}
	}

	_rank = new int[_len];
	for (int i = 0;i < _len;++ i)
	{
		_rank[_SA[i]] = i;//_SA[i] ��ʾ������ i �ĺ�׺���±�. _rank[i] ��ʾ�� i ����׺������
	}

	_cs = new CircleString(_content,_len,_rank,_SA);
}

FPSGettor::~FPSGettor(void)
{
	if(NULL != _rmqMin)
	{
		delete _rmqMin;
	}
	if(NULL != _topK)
	{
		delete _topK;
	}
	if(NULL != _rank)
	{
		delete _rank;
	}
	if(NULL != _cs)
	{
		delete _cs;
	}
}
#define SET_RESULT_IF_NOT_NULL(p,r) if(NULL!=(p)) *(p)=(r)

pFsNode* FPSGettor::innerGetFs(int &nsize)
{
	doGetFSOrderly(1,_len - 1,-1);//_height[0] ��û������ģ����� 1 ~ len-1

	if(NULL != _topK)
	{
		nsize = _topK->getRealSize();
		pFsNode *fps = _topK->getTopK();
		nsize = _topK->getRealSize();
		return fps;
	}
	else
	{
		nsize = _noneTopKFs.size();
		pFsNode * fps = new pFsNode[nsize];

		for(int i = 0;i < nsize;++ i)
		{
			fps[i] = _noneTopKFs[i];
		}
		if(1 == _filter._order)
			sort(fps,fps+nsize,FsNode::greater);
		else if(2 == _filter._order)
			sort(fps,fps+nsize,FsNode::lesser);
		return fps;
	}
}

pFsNode* FPSGettor::getFS(int &nsize)
{
	pFsNode *retFpNode = innerGetFs(nsize);
	
	return retFpNode;
}

/************************************************************************/
/* 
	���δת���ַ�ǰ��Ƶ������
*/
/************************************************************************/
wchar_t * FPSGettor::getFpFromFsNode(FsNode * curFP,int &theLen)
{
	int suffixIndex;
	int suffixLen;
	int fpCounter;

	suffixLen = curFP->length;
	suffixIndex = curFP->index;
	fpCounter = curFP->counter;
	theLen = suffixLen + 1;
	wchar_t *theChs = new wchar_t[theLen];	//��һ�� wchar_t�� '\0'
	theChs[suffixLen] = 0;

	wchar_t uCode;
	int i = 0;
	wchar_t curWCh;
	while (suffixLen != i)
	{
		curWCh = _content[suffixIndex + i ++];
		theChs[i-1] = curWCh;
	}
	return theChs;
}


/************************************************************************/
/* 
	ɾ����ԭ��������ͬ��ǰ׺��
	�˴��Ĳ��� fps ���Ѱ����������Ƶ������(����������������)

	@Deprecation
	���ֽ��˼·�п����Ǵ���ģ���һ�����������а���һ�𡣡���
*/
/************************************************************************/
void FPSGettor::removePrefixWithSameCount(pFsNode* & fps,int& nsize)
{
	if(0 == nsize)
		return;
	vector<pFsNode> retFps;
	pFsNode lastFps = fps[0];
	int lastFpsLen = 0;
	for (int i = 1;i < nsize;++ i)
	{
		pFsNode curFps = fps[i];
		if(curFps->counter == lastFps->counter)
		{
			int curFpsLen = 0;
			wchar_t *lastfpsStr = getFpFromFsNode(lastFps,lastFpsLen);
			wchar_t *curFpStr = getFpFromFsNode(curFps,curFpsLen);
			char fr = prefixRelation(lastfpsStr,curFpStr);
			if(1 == fr)	//lastfpsStr �� curFpStr ��ǰ׺���������������� curFpStr(ȥ�� lastFps)
			{
				retFps.push_back(curFps);
			}
			else if(2 == fr)
			{
				retFps.push_back(lastFps);
			}
			else
			{
				assert(0 != fr);	//�����ܳ�������������һ�������
			}
		}
	}

	delete[] fps;
	nsize = retFps.size();
	fps = new pFsNode[nsize];
	for (int i = 0;i < nsize;++ i)
	{
		fps[i] = retFps[i];
	}
}

void FPSGettor:: printFP(pFsNode node)
{
	int si = node->index;
	int sl = node->length;
	wprintf(L"\r\nFsNode:");
	for (int i = 0;i < sl;++ i)
	{
		wprintf(L"%c",KEY_TO_UNICODE(_content[si + i]));
	}
	wprintf(L"\r\n");
}

/************************************************************************/
/* 
	�ο����� 03 ע��
	c : Ƶ�����д���
	f : Ƶ������
	m : Ƶ�����г���

	
	��һ�ֵ�����
	c1, f1, m1

	���ֵ�����
	c2, f2, m2
	
	��һ�ֵ�����
	c3, f3, m3

	-----------------------------------------------------------
	�ڲ����Ǵ���������������У�
	c1 > c2 > c3
	m1 < m2 < m3
	f1 -> f2 -> f3	(f1 �� f2 ��ǰ׺��f2 �� f3 ��ǰ׺)
	-----------------------------------------------------------

	�����Ǵ������������ʱ��
	c1 >= c2 >= c3
	m1 < m2 < m3
	f1 -> f2 -> f3	(f1 �� f2 ��ǰ׺��f2 �� f3 ��ǰ׺)

	�ʲ����ܴ��� c1 �� c3 ��ȣ����� c2 ����ȡ�
	�����Ľ����ǣ�
	�����Ǵ�������ʱ��Ϊ��ȥ��������ͬ���������У�һ����������һ�����е�ǰ׺��
	ֻ��Ҫ�����ڵĵ������ÿ��ƣ�
		��һ�μ����Ƶ������������μ�������еĴ�����ͬ������ǰ������һ��(���Ǳ���Ƶ�����е�ǰ׺)��
	
	-----------------------------------------------------------

	ע�� curNode һ�����Ѿ�ʹ�ô����������Ƶ�����С�

	ʹ��һ��������ƣ����ܵ�һ��Ƶ������ʱ���������ؼ��룬���ǵ���һ��Ƶ�����е������Ƚϴ����Ƿ�
	��ͬ������ͬ����ǰһ�������¡�ǰһ���������ء��������ǰһ�������¡�ǰһ���������ء�

	ע�� pureCountExAddFps ���ӵ����ε��ò������������ doGetFSOrderly ǰ��������������У�
	��������� doGetFSOrderly �е�����ƽ�з�֧ doGetFSOrderly �У�һ����֧�ĵ�����ϣ��л�����һ
	����֧�ĵ����ϡ�����������߼�ֻ������ǰ�������������������Ӧ�ں��ߡ�
	��������
		pureCountExAddFps �����߼����� curNode Ϊ NULL ʱ����һ�֡��������֪ͨ������ʱ������������
			a)��������л���û�м����Ƶ������(�� _lastAddInNode ��Ϊ NULL)��ֱ�Ӽ���
			b)���� _lastAddInNode Ϊ NULL
		Ȼ����һ�� doGetFSOrderly �������֮ʱ���� pureCountExAddFps(NULL) ���ɡ�
*/
/************************************************************************/
void FPSGettor::pureCountExAddFps(pFsNode curNode)
{
	if(NULL == curNode)	//���������ֹ֪ͨ
	{
		if(NULL != _lastAddInNode)
		{
			realAddFps(_lastAddInNode);
			_lastAddInNode = NULL;
		}
		return;
	}

	if(NULL == _lastAddInNode)
	{
		_lastAddInNode = curNode;
		return;
	}
	else
	{
		if(_lastAddInNode->counter == curNode->counter)
		{
			_lastAddInNode = curNode;
		}
		else
		{
			realAddFps(_lastAddInNode);
			_lastAddInNode = curNode;
		}
	}
}

void FPSGettor::realAddFps(pFsNode node)
{
	if(NULL != _topK)
	{
		if(!_topK->add(node))
		{
			delete node;
		}
	}
	else
	{
		if(_noneTopKFs.size() != _filter._limits)
		{
			_noneTopKFs.push_back(node);
		}
		else
		{
			delete node;
			return;
		}
	}
}


void FPSGettor::doGetFSOrderly(int s,int e,int egnore)
{

	const int MINSEC = _filter._support - 1;
	int minIndex;
	int theMin = _rmqMin->query(s,e,&minIndex);

	//height[s] ��ʾ������ s �������� s-1 �������ǰ׺
	//���Ե�ǰƵ������Ӧ���Ǵӣ������� s-1 �ĺ�׺�������� e �ĺ�׺������ e-s+2��
	if(e - s + 1 >= MINSEC)						//��С֧�ֶ�
		if(egnore != theMin)					//���ӵݹ�����.���������������������
		{
			if (theMin >= _filter._minLen)		//��С����
			{
				pFsNode curNode = new FsNode(_SA[s-1],e - s + 2,theMin);
				
				if(NULL == _topK || _topK->couldAdd(curNode))	//��ǰ���Ƿ��ܲ��뵽 _topK ������ _noneTopKFs
				{
					//v7.0
					//������ formatFps ʱ��curNode �� invalid char ��ͷ������Ե���Ƶ������
					//������ formatFps ʱ��curNode ���벻�� invalid char ��ͷ���Ž������´���
					//�������� formatFps ʱ���������´���

					//v7.1
					//���� _formatFps ���ܵ���һ�������ʵĴ��󣬲μ����ļ�ע�� 02
					//�˴�ȥ�� _formatFps ������
					/*
					if(!_filter._formatFps || !fpsStartWithInvalidWchar(curNode))
					{*/
					
					if(!_filter._removeContained || !fpsContained(curNode))
					{
						//���� curNode ����ʵ������
						if(_filter._pureCount)
						{
							const wchar_t *circleBase = _content + _SA[s-1];
							int circleLen = theMin;

							int realCount = _cs->circleCount(circleBase,circleLen,s-1,e);
							curNode->counter = realCount;
						}
						
						if(curNode->counter - 1 >= MINSEC)	//�ο�������ж�
						{
							if(_filter._pureCount && _filter._pureCountEx)
								pureCountExAddFps(curNode);
							else
								realAddFps(curNode);
						}
					}
					else
					{
						delete curNode;
						return;
						//�����ǰ��Ƶ�����ж�����������ݹ���������Ƶ������Ҳ��Ȼ�����������Կ�����ֹ�ݹ�						
					}//end for fpsContained check
					/*
					}
					else
					{
						delete curNode;
						//���ܵ�ǰƵ�������� invalid �ַ���ͷ��Ϊ���ھ��������Ƶ�����У���Ӧ����ֹ�ݹ�
					}
					*/
					
				}
				else
				{
					delete curNode;
				}
			}
		}
	//����
	int i;
	i = minIndex - 1;
	while (-1 != i && theMin == _height[i])
	{
		-- i;
	}
	if(i >= s)
	{
		doGetFSOrderly(s,i,theMin);
		pureCountExAddFps(NULL);	//һ�����������ɣ���Ҫ����һ�»��棬���� _lastAddInNode
	}
	i = minIndex + 1;
	while (_len != i && theMin == _height[i])
	{
		++ i;
	}
	if(e >= i)
	{
		doGetFSOrderly(i,e,theMin);
		pureCountExAddFps(NULL);	//һ�����������ɣ���Ҫ����һ�»��棬���� _lastAddInNode
	}

	pureCountExAddFps(NULL);	//�������������ɣ���Ҫ����һ�»��棬���� _lastAddInNode
}


/************************************************************************/
/* 
//��������� formatFps���ҵ�ǰƵ�����м��ϲ���invalidchar ��ͷ��
//��{2 ��ǰƵ�����м��ϵ���β�������еĸ���ǰһ�����еĵ�һ���ַ���������ǵĿ�ȸպõ��� fpsCounts����ǰ��Ƶ�����м����Ǳ������ġ�}
//��������� formatFps����ֻ��Ҫ����{2}����
//��������� formatFps���ҵ�ǰƵ�����м��ϸպþ���invalidchar ��ͷ����ǰƵ�������ǻᱻɾ���ģ��������Ƿ񱻰�����û�������
*/
/************************************************************************/
inline bool FPSGettor:: fpsContained(FsNode *curFP)
{
	int fpsCounts = curFP->counter; 
	int startOfFpIndex = curFP->index;	//Ƶ�����м��У���һ����������һ����׺����
	if(0 == startOfFpIndex) return false; //��һ����׺���в������Ǳ�������Ƶ������
	int endofFpIndex = _SA[_rank[startOfFpIndex] + fpsCounts - 1];//Ƶ�����м��У����һ����������һ����׺����
	if(0 == endofFpIndex) return false;
	int beforeStartOfFpIndex = startOfFpIndex - 1;
	int beforeEndOfFpIndex = endofFpIndex - 1;
	//��������� formatFps���ҵ�ǰƵ�����м��ϲ���invalidchar ��ͷ��
	//��{2 ��ǰƵ�����м��ϵ���β�������еĸ���ǰһ�����еĵ�һ���ַ���������ǵĿ�ȸպõ��� fpsCounts����ǰ��Ƶ�����м����Ǳ������ġ�}
	//��������� formatFps����ֻ��Ҫ����{2}����
	//��������� formatFps���ҵ�ǰƵ�����м��ϸպþ���invalidchar ��ͷ����ǰƵ�������ǻᱻɾ���ģ��������Ƿ񱻰�����û�������
	return /*(!_filter._formatFps || !isInvalidStartWchar(KEY_TO_UNICODE(_content[beforeEndOfFpIndex]))) && */
		 _content[beforeEndOfFpIndex] == _content[beforeStartOfFpIndex] && fpsCounts == (_rank[beforeEndOfFpIndex] - _rank[beforeStartOfFpIndex] + 1);
}

/*
inline bool FPSGettor:: fpsStartWithInvalidWchar(FsNode* curFP)
{
	int suffixIndex = curFP->index;
	wchar_t curWCh = KEY_TO_UNICODE(*(_content + suffixIndex));
	return isInvalidStartWchar(curWCh);
}
*/

/*
inline bool FPSGettor:: isInvalidStartWchar(wchar_t curWCh)
{
	wchar_t *invalidWchars = _filter._formatFps_removeStartWchars;
	for (;*invalidWchars;++invalidWchars)
	{
		if(*invalidWchars == curWCh)
		{
			return true;
		}
	}
	return false;
}
*/


#undef SET_RESULT_IF_NOT_NULL
#undef KEY_TO_UNICODE