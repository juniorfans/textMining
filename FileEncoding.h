#pragma once

class FileEncoding
{
private:
	inline bool strEqual(const char *l,const char *r)
	{
		if(NULL == l && NULL == r)	return false;
		for (;0 != *l && 0 != *r && *l == * r;++ l,++ r)
		{

		}
		return *l == *r && 0 == *l;
	}
	static const int MAX_CHARSET_NAME = 16;
	char charSet[MAX_CHARSET_NAME];
public:
	FileEncoding(void);
	~FileEncoding(void);
	const char * getCharSet(const char *buf,int bufLen);

	int getCharSetPageCode(const char *buf,int bufLen);
};
