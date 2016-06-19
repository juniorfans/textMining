#pragma once

class TextFile;
class BinaryFile;
class MiningFile
{
private:
	const string _fileName;
	char *_bytes;
	int _sorCounts;
	int _realCounts;	//�п��ܻ���ַ�ѹ�����������н����� 2,123,54�������ַ��������ת��Ϊ 1, 2, 3����ߺ���ĺ�׺���鹹���ٶ�
	int _alphabetSize;	//��ĸ��Ĵ�С
protected:
	MiningFile(const string& file)
		:_fileName(file),_sorCounts(0),_realCounts(0),_bytes(NULL)
	{

	}
	
public:
	~MiningFile()
	   {
		   if(NULL != _bytes)
		   {
			   delete[] _bytes;
			   _bytes = NULL;
		   }
	   }
	
	void setRealCounts(int ncount)
	{
		_realCounts = ncount;
	}
	int getRealCounts()
	{
		return _realCounts;
	}
	int getSorCounts()
	{
		return _sorCounts;
	}
	int getAlphabetSize()
	{
		return _alphabetSize;
	}
	void setAlphabetSize(int nsize)
	{
		_alphabetSize = nsize;
	}
	char *toBytes();
public:
	virtual void *toTchars() = 0;
};


/************************************************************************/
/* 
	���ı��ļ��е�ÿ���ַ�ת��Ϊ wchar_t ����
	�ٽ���ĸ��ѹ����
	����ļ��н����� 3,45,125 �������ַ�������Խ� 1->3, 2->45, 3 -> 125 ��Ӧ����
	����ʹ����ĺ�׺���鴴���㷨���Ч��
*/
/************************************************************************/
class TextFile:public MiningFile
{
private:
	const static int _keyLowBond = 0;//��С�� key �߽磬ͬʱҲ����С�� unicode �߽�| exclusive
	wchar_t *_keyToUnicode;
	int _maxKey;
	bool _upLowCaseSensitive;
	wchar_t *_wChars;
	void compressContent(char *buf,int bufLen,wchar_t* &bytes,int *realNumLen,int *alphabetSize);
public:
	TextFile(const string& file,bool upLowCaseSensitive):
	  MiningFile(file),_keyToUnicode(NULL),_maxKey(_keyLowBond),_upLowCaseSensitive(upLowCaseSensitive),_wChars(NULL)
	{
		
	}
	 ~TextFile()
	 {
		 if(NULL != _keyToUnicode)
		 {
			 delete[] _keyToUnicode;
			 _keyToUnicode = NULL;
			 _maxKey = _keyLowBond;
		 }
		 if(NULL != _wChars)
		 {
			 delete[] _wChars;
			 _wChars = NULL;
		 }
	 }
	 //���ص��� w_chars* ��Ϊ�˽ӿڱ���һ��ʹ�� void *
	void* toTchars();

	inline wchar_t keytoUnicode(int key)
	{
		if(key > _maxKey || key <= _keyLowBond)	return _keyLowBond;
		return *(_keyToUnicode + key);
	}

	wchar_t* getKeyToUnicodeMap()
	{
		return _keyToUnicode;
	}
};


class BinaryFile:public MiningFile
{
public:
		BinaryFile(const string& file):
	  MiningFile(file)
	  {

	  }
	  ~BinaryFile()
	  {
		  
	  }

	  
	  void* toTchars()
	  {
		  setAlphabetSize(256);	//ÿ���ֽڵķ�Χ��0 ~ 255
		  return (void*)toBytes();
	  }
};