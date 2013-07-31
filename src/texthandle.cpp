#include "texthandle.h"

#include <iconv.h>
#include <sstream>
#include <string.h>

std::string sjisToUTF8(char *sjis)
{
	char	inbuf[MAX_BUF+1] = { 0 };
	char	outbuf[MAX_BUF+1] = { 0 };
		
	char	*in = inbuf;
	char	*out = outbuf;
	size_t	in_size = (size_t)MAX_BUF;
	size_t	out_size = (size_t)MAX_BUF;
	iconv_t ic = iconv_open("UTF-8", "SJIS");

	memcpy( in, sjis, strlen(sjis) );

	iconv( ic, &in, &in_size, &out, &out_size );
	iconv_close(ic);

	//int ret = memcmp( outbuf, utf8, sizeof(utf8) );
	//printf(ret == 0 ? "success\n" : "failed\n" );
	//printf("%s\n", outbuf );
	
	//cout<<outbuf<<endl;
		
	std::stringstream ss;
	ss<<outbuf;
	return ss.str();
}
