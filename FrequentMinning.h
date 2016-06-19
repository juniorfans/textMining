#pragma once

#include <vector>
using std::vector;
#include <string>
using std::string;
#include "FPSGettor.h"

/************************************************************************/
/* 
	FrequentConfig �������ھ��㷨�����ã����������е����á�
	FPSFilter ��ר�����ڹ���Ƶ�����е�����
*/
/************************************************************************/
class FrequentConfig
{
public:
	string _fileName;		//�ھ���ı��ļ�
	bool _binaryFile;		//���ļ������������ļ������ı��ļ��ھ�Ĭ���Ƕ������ļ�
							//���ı��ļ��ھ�Ľ�����ַ����У��������Ʒ�ʽ�ھ������ֽ����С�
							//�����ı��ļ��Զ����Ʒ�ʽ�ھ򽫵ò�����ȷ���ַ����У����������ļ����ı��ļ���ʽ�ھ���ܵ����ڴ�ķѼ���
							//�����ı��ļ���ʽ�ھ�ʱ�������ȷ�����ļ��ı��벢����ã�����ָ������

	int _minLen;			//Ƶ�������ַ�������С���ȣ����ڴ˳��ȵ��ַ���������
	int _support;			//Ƶ������Ƶ����С֧�ֶ�
	int _fpsListNums;		//Ƶ�����н����������Ŀ��
	bool _pureCount;		//������������ԭ��Ϊ aaaaa��'aa'�Ĵ�������Ϊ2�Σ��Ǵ�������Ϊ4�Ρ���������������²�����һ��λ�ñ����ƥ��ͬʱռ��
	bool _pureCountEx;		//���ô�������ʱ��ĳЩ���е�ǰ׺������мƴ���ͬ��ɾ����ǰ׺���С���ԭ��Ϊ aaabaaa��'aa'�Ĵ�������Ϊ2�Σ�'aaa'�Ĵ�������ҲΪ2�Σ���ʱӦ��ɾ�� aa
	bool _needWriteFpsToFile;	//�Ƿ�Ƶ������д���ļ�
	bool _needDisplayFps;	//�Ƿ���ʾƵ������
	bool _upLowCaseSensitive;	//�����ļ�ʱ�Ƿ��Сд����
	bool _formatFps_transform;//��Ƶ�������е�Ӱ��������ʾ���ַ��Ƿ�����ת�塣�� \r,\n
	int _fpsOrder;	//��Ƶ��������ʾ��д�ļ�ʱ��˳��	0 : ԭ��/�ֵ����� | 1 ��Ƶ�ν��� | 2 ��Ƶ������
	bool _fpsRemoveContained;	//ȥ���԰�����Ƶ������
	//����Ĭ��ֵ
	FrequentConfig()
	{
		_binaryFile = 1;
		_minLen = 1;
		_support = 2;
		_fpsListNums = -1;	//��Ϊ - 1 ʱ�������е�Ƶ������
		_pureCount = true;
		_pureCountEx = true;
		_needWriteFpsToFile = true;
		_needDisplayFps = false;
		_upLowCaseSensitive = true;
		//_formatFps = true;
		//_formatFps_removeFpsStartWchars = L" \t\r\n";
		_formatFps_transform = false;
		_fpsOrder = 1;
		_fpsRemoveContained = true;
	}
};

class MiningFile;
class FrequentMinning
{
public:
	FrequentMinning(const FrequentConfig& config);
	~FrequentMinning(void);
	void minning();
	wchar_t *getWCharFpFromFsNode(FsNode *,int&);
	byte * getByteFpFromFsNode(FsNode * curFP,int &);
private:
	void init();
	void destroy();
	void calHeight();
	void verifyHeight();
	void syncTo();
	void showSuffixAndHeight(int *);
private:

	FrequentConfig _fconfig;

	MiningFile *_file;

	//void *_content;	//��ȡ�ļ���õ�����
	wchar_t *wchar_content;
	byte *char_content;
	int _len;			//Դ���ݳ���
	int _alphabetSize;	//��������ַ���ĳ���
	int *_height;	//_height[x] ��ʾ������ x �ĺ�׺��������ǰһ���ĺ�׺��������Ӵ����������ĳ���
	int *_SA;	//��׺����

	pFsNode *_fps;
	int _fpsSize;
};
