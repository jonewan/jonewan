#include "packlog.h"

int writeDirInZip(const char *path, zipFile zf, int opt_compress_level, char *password) {
	struct dirent *filename = NULL;
	DIR *dp = NULL;
	struct stat s_buf;
	char file_path[MAXFILENAME];
	int err = 0;

	if (path == NULL) {
		printf("Error: PATH NOT CORRECT!\n");
		return -1;
	}
	dp = opendir(path);
	if (dp == NULL) {
		printf("Can not open %s\n", path);
		return -1;
	}
	while ((filename = readdir(dp)) != NULL) {//readdir()必须循环调用，要读完整个目录的文件，readdir才会返回NULL,若未读完，继续循环
		bzero(file_path, MAXFILENAME);
		strcat(file_path, path);
		if (path[strlen(path)-1] != '/')
			strcat(file_path, "/");
		strcat(file_path, filename->d_name);

		/*略过每层目录的.与..，因为没有意义且会导致死循环*/
		if (strcmp(filename->d_name, ".") == 0 || strcmp(filename->d_name, "..") == 0) {
			continue;
		}

		/*获取文件信息，把信息放到s_buf中*/
		err = stat(file_path, &s_buf);
		if (err != 0) {//如果文件名输入错误，提醒用户并退出
			printf("Warning: name not matched: [ %s ] \n", file_path);
			return (-1);
		}
		/*判断是否目录*/
		if (S_ISDIR(s_buf.st_mode)) {
			err = writeDirInZip(file_path, zf, opt_compress_level, password);//若是目录，递归调用自身，寻找目录中的文件
		}else if (S_ISREG(s_buf.st_mode)) {/*判断是否为普通文件*/
			//printf("%s\n", file_path);
			err = writeOneFilesInzip(file_path, zf, opt_compress_level, password);//若是文件，将文件添加到zip文件中
		}
		else {
			printf("Error in something worng!\n");
		}
	}

	return err;
}

int writeOneFilesInzip(char *filenameinzip, zipFile zf, int opt_compress_level, char *password) {
	zip_fileinfo zi = {0};
	FILE * fin;//创建一个文件指针
	int err = ZIP_OK;
	int size_read;
	int zip64 = isLargeFile(filenameinzip);
	unsigned long crcFile = 0;
	void* buf = NULL;
	int size_buf = WRITEBUFFERSIZE;

	if (access(filenameinzip, R_OK) != 0) {
		printf("No Access Permission in [ %s ]!!!\n", filenameinzip);
		return -1;
	}
	filetime((char *)filenameinzip, &zi.tmz_date, &zi.dosDate);

	/*申请buf空间*/
	buf = (void*)malloc(size_buf);//申请缓冲区空间
	if (buf == NULL) {//未申请到空间
		printf("Error allocating memory\n");
		return ZIP_INTERNALERROR;//返回错误
	}
	
	/*获得文件CRC校验*/
	if ((password != NULL)) {
		err = getFileCrc(filenameinzip, buf, size_buf, &crcFile);//获得文件的CRC校验，有密码时才用
		if (err != ZIP_OK) {
			printf("Error in get file CRC.\n");
			free(buf);
			return ZIP_INTERNALERROR;//返回错误
		}
	}

	/*在zip文件中创建一个新文件*/
	err = zipOpenNewFileInZip3_64(zf, filenameinzip, &zi,
		NULL, 0, NULL, 0, NULL /* comment*/,
		(opt_compress_level != 0) ? Z_DEFLATED : 0,
		opt_compress_level, 0,
		/* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
		-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
		password, crcFile, zip64);
	if (err != ZIP_OK) {
		printf("error in opening %s in zipfile\n", filenameinzip);
		free(buf);
		return ZIP_ERRNO;
	}
	
	/*以读写方式打开一个二进制文件，并且要求文件存在*/
	fin = FOPEN_FUNC(filenameinzip, "rb");
	if (fin == NULL) {//打开文件失败
		printf("error in opening %s for reading\n", filenameinzip);
		free(buf);
		return ZIP_ERRNO;
	}

	do {
		err = ZIP_OK;
		size_read = (int)fread(buf, 1, size_buf, fin);
		if (size_read < size_buf)
			if (feof(fin) == 0) {
				printf("error in reading %s\n", filenameinzip);
				err = ZIP_ERRNO;
			}

		if (size_read > 0) {
			err = zipWriteInFileInZip(zf, buf, size_read);
			if (err < 0) {
				printf("error in writing %s in the zipfile\n", filenameinzip);
			}
		}
	} while ((err == ZIP_OK) && (size_read > 0));
	if (fin)
		fclose(fin);//关闭文件

	err = zipCloseFileInZip(zf);//关闭zip文件中的文件
	if (err != ZIP_OK)
		printf("error in closing %s in the zipfile\n", filenameinzip);

	free(buf);
	return err;
}
