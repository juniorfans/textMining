#pragma once

class TextFile;
class BinaryFile;
class MiningFile
{
private:
	const string _fileName;
	char *_bytes;
	int _sorCounts;
	int _realCounts;	//有可能会对字符压缩，如内容中仅包括 2,123,54这三个字符，则可以转化为 1, 2, 3，提高后面的后缀数组构建速度
	int _alphabetSize;	//字母表的大小
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
	将文本文件中的每个字符转化为 wchar_t 类型
	再将字母表压缩：
	如该文件中仅出现 3,45,125 这三个字符，则可以将 1->3, 2->45, 3 -> 125 对应起来
	可以使后面的后缀数组创建算法提高效率
*/
/************************************************************************/
class TextFile:public MiningFile
{
private:
	const static int _keyLowBond = 0;//最小的 key 边界，同时也是最小的 unicode 边界| exclusive
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
	 //返回的是 w_chars* ，为了接口保持一致使用 void *
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
		  setAlphabetSize(256);	//每个字节的范围是0 ~ 255
		  return (void*)toBytes();
	  }
};