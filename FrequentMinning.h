#pragma once

#include <vector>
using std::vector;
#include <string>
using std::string;
#include "FPSGettor.h"

class FrequentConfig
{
public:
	string _fileName;		//�ھ���ı��ļ�
	int _minLen;			//Ƶ�������ַ�������С���ȣ����ڴ˳��ȵ��ַ���������
	int _support;			//Ƶ������Ƶ����С֧�ֶ�
	int _fpsListNums;		//Ƶ�����н����������Ŀ��
	bool _pureCount;		//������������ԭ��Ϊ aaaaa��'aa'�Ĵ�������Ϊ2�Σ��Ǵ�������Ϊ4�Ρ���������������²�����һ��λ�ñ����ƥ��ͬʱռ��
	bool _pureCountEx;		//���ô�������ʱ��ĳЩ���е�ǰ׺������мƴ���ͬ��ɾ����ǰ׺���С���ԭ��Ϊ aaabaaa��'aa'�Ĵ�������Ϊ2�Σ�'aaa'�Ĵ�������ҲΪ2�Σ���ʱӦ��ɾ�� aa
	bool _needWriteFpsToFile;	//�Ƿ�Ƶ������д���ļ�
	bool _needDisplayFps;	//�Ƿ���ʾƵ������
	bool _upLowCaseSensitive;	//�����ļ�ʱ�Ƿ��Сд����
	//bool _formatFps;	//����{_formatFps_noneStartWchars}���ַ���ͷ��Ƶ������ɾ����������{_formatFps_noneContainedWchars}���ַ����м���β��Ƶ������ת��
	wchar_t *_formatFps_removeFpsStartWchars;	//��ĳЩ�ַ���ͷ��Ƶ�����б�ɾ������ո�\t,\r,\n�ȣ�����һ��һ����������ַ�����
	bool _formatFps_transform;//��Ƶ�������е�Ӱ��������ʾ���ַ��Ƿ�����ת�塣�� \r,\n
	int _fpsOrder;	//��Ƶ��������ʾ��д�ļ�ʱ��˳��	0 : ԭ��/�ֵ����� | 1 ��Ƶ�ν��� | 2 ��Ƶ������
	bool _fpsRemoveContained;	//ȥ���԰�����Ƶ������
	//����Ĭ��ֵ
	FrequentConfig()
	{
		_minLen = 1;
		_support = 2;
		_fpsListNums = -1;	//��Ϊ - 1 ʱ�������е�Ƶ������
		_pureCount = true;
		_pureCountEx = true;
		_needWriteFpsToFile = true;
		_needDisplayFps = false;
		_upLowCaseSensitive = true;
		//_formatFps = true;
		_formatFps_removeFpsStartWchars = L" \t\r\n";
		_formatFps_transform = false;
		_fpsOrder = 1;
		_fpsRemoveContained = true;
	}
};

class FrequentMinning
{
public:
	FrequentMinning(const FrequentConfig& config);
	~FrequentMinning(void);
	void minning();
	wchar_t *getFpFromFsNode(FsNode *,int&);
private:
	void init();
	void destroy();
	void calHeight();
	wchar_t *transformSepecialChars(wchar_t *,int ,int &);
	void verifyHeight();
	void syncTo();
	void doSync(ostream *file,wchar_t *theChs,int nDocLength,int fpCounter);

	void showSuffixAndHeight(int *);
private:

	FrequentConfig _fconfig;

	File *_file;

	wchar_t*_content;	//��ȡ�ļ���õ�����
	int _len;	//Դ���ݳ���
	int _alphabetSize;	//��������ַ���ĳ���
	int *_height;	//_height[x] ��ʾ������ x �ĺ�׺��������ǰһ���ĺ�׺��������Ӵ����������ĳ���
	int *_SA;	//��׺����

	pFsNode *_fps;
	int _fpsSize;
};
