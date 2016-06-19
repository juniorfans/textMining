#pragma once
#include <hash_map>
using stdext::hash_map;
#include <string>
using std::string;


class File
{
private:
	const string _fileName;
	const static int _keyLowBond = 0;//最小的 key 边界，同时也是最小的 unicode 边界| exclusive
	wchar_t *_keyToUnicode;
	int _maxKey;
	void compressContent(char *buf,int bufLen,wchar_t* &bytes,int *realNumLen,int *alphabetSize,bool upLowCaseSensitive);
public:
	File(const string& file);
	~File();
	void toWchars(wchar_t*& ints,int *realIntsCounts,int *alphabetSize,bool upLowCaseSensitive);
	
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


void intsToFile(int *ints,int len,const string &fileName);
char prefixRelation(const wchar_t* firstStr,const wchar_t* secondStr);

int radix(int num);

int minValue(int *sorArray,int start,int end,int *index);

int maxValue(int *sorArray,int start,int end,int *index);
