#ifndef _MINIZIP_H_
#define _MINIZIP_H_

#if (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#ifndef _FILE_OFFSET_BIT
#define _FILE_OFFSET_BIT 64
#endif
#endif

#ifdef __APPLE__
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#else
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef _WIN32
	# include <direct.h>
	# include <io.h>
#else
	# include <unistd.h>
	# include <utime.h>
	# include <sys/types.h>
	# include <sys/stat.h>
#endif

#include "zip.h"

#ifdef _WIN32
	#define USEWIN32IOAPI
	#include "iowin32.h"
#endif

#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef _WIN32

uLong filetime(char *f, tm_zip *tmzip, uLong *dt);
/* name of file to get info on */
/* return value: access, modific. and creation times */
/* dostime */

#else
	#if defined(unix) || defined(__APPLE__)
uLong filetime(char *f, tm_zip *tmzip, uLong *dt);
	/* name of file to get info on */
	/* return value: access, modific. and creation times */
	/* dostime */
	#else
uLong filetime(char *f, tm_zip *tmzip, uLong *dt);
	/* name of file to get info on */
	/* return value: access, modific. and creation times */
	/* dostime */

	#endif
#endif

int check_exist_file(const char* filename);


/* calculate the CRC32 of a file,
   because to encrypt a file, we need known the CRC32 of the file before */
int getFileCrc(const char* filenameinzip, void*buf, unsigned long size_buf, unsigned long* result_crc);
int isLargeFile(const char* filename);

#endif