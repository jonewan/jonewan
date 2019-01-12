#include "packlog.h"

int writeDirInZip(const char *path, zipFile zf, int opt_compress_level, char *password) {
	struct dirent *filename = NULL;
	struct stat s_buf;
	DIR *dp = NULL;
	int err = 0;
	char file_path[MAXFILENAME];

	if (path == NULL) {
		printf("Error: PATH NOT CORRECT!\n");
		return -1;
	}
	printf("%d\n", __LINE__);
	dp = opendir(path);
	if (dp == NULL) {
		printf("Can not open %s\n", path);
		return 0;
	}
	printf("****dp = %p*****\n", dp);
	while ((filename = readdir(dp)) != NULL) {//readdir()����ѭ�����ã�Ҫ��������Ŀ¼���ļ���readdir�Ż᷵��NULL,��δ���꣬����ѭ��
		bzero(file_path, MAXFILENAME);
		strcat(file_path, path);
		if (path[strlen(path)-1] != '/')
			strcat(file_path, "/");
		strcat(file_path, filename->d_name);
		printf("%d\n", __LINE__);

		/*�Թ�ÿ��Ŀ¼��.��..����Ϊû�������һᵼ����ѭ��*/
		if (strcmp(filename->d_name, ".") == 0 || strcmp(filename->d_name, "..") == 0) {
			continue;
		}

		/*��ȡ�ļ���Ϣ������Ϣ�ŵ�s_buf��*/

		err = stat(file_path, &s_buf);
		if (err != 0) {//����ļ���������������û����˳�
			printf("Warning: name not matched: [ %s ] \n", file_path);
			exit(EXIT_FAILURE);
		}
		/*�ж��Ƿ�Ŀ¼*/
		if (S_ISDIR(s_buf.st_mode)) {
			err = writeDirInZip(file_path, zf, opt_compress_level, password);//����Ŀ¼���ݹ��������Ѱ��Ŀ¼�е��ļ�
		}else if (S_ISREG(s_buf.st_mode)) {/*�ж��Ƿ�Ϊ��ͨ�ļ�*/
			//printf("%s\n", file_path);
			err = writeOneFilesInzip(file_path, zf, opt_compress_level, password);//�����ļ������ļ���ӵ�zip�ļ���
		}
		else {
			printf("Error in something worng!\n");
		}
	}

	return err;
}

int writeOneFilesInzip(char *filenameinzip, zipFile zf, int opt_compress_level, char *password) {
	zip_fileinfo zi = {0};
	FILE * fin;//����һ���ļ�ָ��
	int err = ZIP_OK;
	int size_read;
	int zip64 = isLargeFile(filenameinzip);
	unsigned long crcFile = 0;
	void* buf = NULL;
	int size_buf = WRITEBUFFERSIZE;

	//	while (filenameinzip[0] == '\\' || filenameinzip[0] == '/') {
	//		filenameinzip++;//�Թ��ļ������е�·������
	//	}
	/*����zip���ļ���Ϣ*/
	//zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour = 0;
	//zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
	//zi.dosDate = zi.internal_fa = zi.external_fa = 0;

	if (access(filenameinzip, R_OK) != 0) {
		printf("No Access Permission in [ %s ]!!!\n", filenameinzip);
		return -1;
	}
	filetime((char *)filenameinzip, &zi.tmz_date, &zi.dosDate);

	/*����buf�ռ�*/
	buf = (void*)malloc(size_buf);//���뻺�����ռ�
	if (buf == NULL) {//δ���뵽�ռ�
		printf("Error allocating memory\n");
		return ZIP_INTERNALERROR;//���ش���
	}
	
	/*����ļ�CRCУ��*/
	if ((password != NULL)) {
		err = getFileCrc(filenameinzip, buf, size_buf, &crcFile);//����ļ���CRCУ�飬������ʱ����
		if (err != ZIP_OK) {
			printf("Error in get file CRC.\n");
			free(buf);
			return ZIP_INTERNALERROR;//���ش���
		}
	}

	/*��zip�ļ��д���һ�����ļ�*/
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
	
	/*�Զ�д��ʽ��һ���������ļ�������Ҫ���ļ�����*/
	fin = FOPEN_FUNC(filenameinzip, "rb");
	if (fin == NULL) {//���ļ�ʧ��
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
		fclose(fin);//�ر��ļ�

	err = zipCloseFileInZip(zf);//�ر�zip�ļ��е��ļ�
	if (err != ZIP_OK)
		printf("error in closing %s in the zipfile\n", filenameinzip);

	free(buf);
	return err;
}
