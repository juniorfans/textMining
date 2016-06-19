#include <memory>
#include "FileEncoding.h"
#include "../FileEncoding/uchardet.h"

FileEncoding::FileEncoding(void)
{
	memset(charSet,0,sizeof(char) * MAX_CHARSET_NAME);
}

FileEncoding::~FileEncoding(void)
{
}


const char * FileEncoding:: getCharSet(const char *buf,int bufLen)
{
	const char *retCharSet = NULL;
	uchardet_t ud = uchardet_new();
	if(0 == uchardet_handle_data(ud,buf,bufLen))
	{
		uchardet_data_end(ud);
		retCharSet = uchardet_get_charset(ud);
		memcpy_s(charSet,16,retCharSet,sizeof(char) * 16);
	}
	uchardet_delete(ud);
	return charSet;
}

int FileEncoding:: getCharSetPageCode(const char *buf,int bufLen)
{
	const char *charSet = getCharSet(buf,bufLen);
	printf("...[%s]...",charSet);
	if(strEqual(charSet,"UTF-8"))
	{
		return 65001;
	}	
	if(strEqual(charSet,"UTF-16"))
	{
		return 1200;
	}
	else if (strEqual(charSet,"Big5"))
	{
		return 950;
	}
	else if(strEqual(charSet,"GBK"))
	{
		return 936;
	}
	else if(strEqual(charSet,"EUC-JP"))
	{
		return 932;
	}
	else if(strEqual(charSet,"EUC-KR"))
	{
		return 949;
	}
	else if(strEqual(charSet,"gb18030"))
	{
		return 54936;
	}
	else if(strEqual(charSet,"windows-1252"))
	{
		return 1252;
	}
	else if(strEqual(charSet,"Shift_JIS"))
	{
		return 932;
	}
	return 0;//ansi CP_ACP
}
