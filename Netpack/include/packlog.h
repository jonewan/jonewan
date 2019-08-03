#ifndef __PACKLOG_H_
#define __PACKLOG_H_

#include <time.h>
#include <dirent.h>//提供目录操作流的函数

#include "minizip.h"

int writeOneFilesInzip(const char *filenameinzip, zipFile zf, int opt_compress_level, char *password);
int writeDirInZip(const char *path, zipFile zf, int opt_compress_level, char *password);

#endif
