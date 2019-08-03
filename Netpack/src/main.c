#include <string.h>
#include "packlog.h"
#include "netcat.h"

#define	DEFAULT_PORT	"9090"

extern int netfd;
extern unsigned int wrote_out;	/* total stdout bytes */
extern unsigned int wrote_net;	/* total net bytes */

extern struct timeval * timer1;
extern struct timeval * timer2;

extern SAI * lclend;		/* sockaddr_in structs */
extern SAI * remend;
extern char * bigbuf_in;		/* data buffers */
extern char * bigbuf_net;
extern fd_set * ding1;			/* for select loop */
extern fd_set * ding2;
extern PINF * portpoop;		/* for getportpoop / getservby* */

/* global cmd flags: */
#ifdef LISTEN_MODE //�����Ҫʹ�ü���ģʽʱ����LISTEN_MODE�����
extern USHORT o_listen;
#endif

extern USHORT o_nflag;
extern USHORT o_udpmode;
extern USHORT o_verbose;
extern unsigned int o_wait;
extern USHORT o_zero;

/* main */
int main(int argc, char ** argv)
{
#ifndef HAVE_GETOPT
	extern char * optarg;
	extern int optind, optopt;
#endif

	register int x;
	HINF * whereto = NULL;//����ȥ
	HINF * wherefrom = NULL;//������
	IA * ouraddr = NULL;
	IA * themaddr = NULL;

	USHORT o_lport = 0;

	int arg_host_index = 0;//host��Ϣ���±�
	int arg_file_index = 0;//�ļ���Ŀ¼���±�
	
	/*����ļ�ʹ�õ�һЩ�ֲ�����*/
	char *opt_zip_fileName = NULL;
	int opt_overwrite = 1;
	int opt_compress_level = Z_DEFAULT_COMPRESSION;
	char filename_try[MAXFILENAME + 16];
	int err = 0;
	zipFile zf;
	char* password = NULL;
	struct stat s_buf;
	int zipFileFD = 0;
	int opt_send = 0;//0:�޲��� 1:���ͷ� 2:���շ�
	int opt_send_org = 0;//�Ƿ����-r��-mѡ��

#ifdef HAVE_BIND
	/* can *you* say "cc -yaddayadda netcat.c -lresolv -l44bsd" on SunLOSs? */
	res_init();
#endif

	if (argc == 1)//���ֻ��һ������������ʾ������Ϣ
		helpme();

	lclend = (SAI *)Hmalloc(sizeof(SA));
	remend = (SAI *)Hmalloc(sizeof(SA));
	bigbuf_in = Hmalloc(BIGSIZ);//�����׼��������Ļ������ռ�
	bigbuf_net = Hmalloc(BIGSIZ);//��������������������Ļ������ռ�
	ding1 = (fd_set *)Hmalloc(sizeof(fd_set));//�����洢�ļ����������ϵĿռ�
	ding2 = (fd_set *)Hmalloc(sizeof(fd_set));//�����洢�ļ����������ϵĿռ�
	portpoop = (PINF *)Hmalloc(sizeof(PINF));//�����洢�˿���Ϣ�Ŀռ�
	errno = 0;
	/* catch a signal or two for cleanup */
	signal(SIGINT, catch);
	signal(SIGQUIT, catch);
	signal(SIGTERM, catch);
	/* and suppress others... */
#ifdef SIGURG
	signal(SIGURG, SIG_IGN);
#endif
#ifdef SIGPIPE
	signal(SIGPIPE, SIG_IGN);		/* important! */
#endif

	/*����������*/
	while ((x = getopt(argc, argv, "hlnmrp:s:tuvw:zoaf:k:019")) != EOF) {
		switch (x) {
		case 'h':
			errno = 0;
			helpme();			/* exits by itself */
		case 'o':	//�Ը��Ƿ�ʽ����дzip�ļ�
			opt_overwrite = 1;
			break;
		case 'a':	//��׷�ӷ�ʽ����дzip�ļ�
			opt_overwrite = 2;
			break;
		/*ѹ���ȼ�*/
		case '0':
			opt_compress_level = 0;
		break;
		case '1':
			opt_compress_level = 1;
			break;
		case '9':
			opt_compress_level = 9;
			break;
		/*ָ��zip�ļ�����*/
		case 'f':
			opt_zip_fileName = optarg;
			break;
		/*ָ��zip�ļ�����*/
		case 'k':
			password = optarg;
			break;
		case 'm':
			opt_send_org = 1;
			opt_send = 1;//m�����ͷ�master
			break;
		case 'r':
			opt_send_org = 1;
			opt_send = 2;//r������ܷ�recesiver
			break;
#ifdef LISTEN_MODE //�����Ҫʹ�ü���ģʽʱ����LISTEN_MODE�����
		case 'l':				/* listen mode */
			o_listen++; break;
#endif
		case 'n':				/* ��ָ��DNS��ֱ��ʹ��IP��ַ */
			o_nflag++; break;
#ifdef LISTEN_MODE
		case 'p':				/* ָ���Ķ˿ں�*/
			o_lport = getportpoop(optarg, 0);
			if (o_lport == 0)//���˿ںţ�����Ƿ��������˳�
				bail("invalid local port %s", optarg);
			break;
#endif
		case 's'://ָ��������Ϣ��Ҳ���Ǳ���ip��ַ
			wherefrom = gethostpoop(optarg, o_nflag);
			ouraddr = &wherefrom->iaddrs[0];
			break;
#ifdef TELNET
		case 't':				/* do telnet fakeout */
			o_tn++; break;
#endif /* TELNET */
		case 'u':				/* use UDP */
			o_udpmode++; break;
		case 'v':				/* verbose */
			o_verbose++; break;
		case 'w':				/* wait time */
			o_wait = atoi(optarg);
			if (o_wait <= 0)
				bail("invalid wait-time %s", optarg);
			timer1 = (struct timeval *) Hmalloc(sizeof(struct timeval));
			timer2 = (struct timeval *) Hmalloc(sizeof(struct timeval));
			timer1->tv_sec = o_wait;	/* we need two.  see readwrite()... */
			break;
		case 'z':				/* little or no data xfer */
			o_zero++;
			break;
		default:
			errno = 0;
			bail("np -h for help");
		} /* switch x */
	}/* while getopt */

	if (o_lport <= 0) {
		o_lport = getportpoop(DEFAULT_PORT, 0);
		if (o_lport == 0)//���˿ںţ�����Ƿ��������˳�
			bail("invalid local port %s", DEFAULT_PORT);
	}
#ifdef LISTEN_MODE //�����Ҫʹ�ü���ģʽʱ����LISTEN_MODE�����
	 /*����ڼ���ģʽ�£�û�и���-p�Ӷ˿ں��򱨴��˳�*/
	if (o_listen) {
		arg_file_index = optind;//��������˿ں����optind��ʼΪ�ļ���Ŀ¼
		arg_host_index = 0;//����ģʽ�²���Ҫ������������
		if(opt_send_org == 0) 
			opt_send = 2;//Ĭ��������������ģʽ,û�и���-m��-r����ʱ��Ĭ��Ϊ���ն�
	}
	else 
#endif	
	{//����ģʽ�£�Ҳ���ǿͻ���ģʽ��
		arg_host_index = optind;//��Ϊ��һ����ѡ�������host��Ϣ
		arg_file_index = arg_host_index + 1;//֮��ķ�ѡ���������Ŀ¼�����ļ�
		if (opt_send_org == 0)
			opt_send = 1;//Ĭ��������������ģʽ,û�и���-m��-r����ʱ��Ĭ��Ϊ���ն�
	}

	if (opt_send == 2)//�ڽ���ģʽ�£��û�����Ķ��������Ч
		arg_file_index = 0;

	if (opt_zip_fileName == NULL) {//����û�û�и���-f zip�����֣���Ĭ�����ɡ�����ʱ��.cvtelog.zip��
		time_t now;
		struct tm *timenow; //ʵ����tm�ṹָ��    
		time(&now);
		timenow = localtime(&now);
		sprintf(filename_try, "%d%02d%02d", timenow->tm_year + 1900, timenow->tm_mon + 1, timenow->tm_mday);
	}
	else {//����û�������zip�����������û���������������
		strncpy(filename_try, opt_zip_fileName, MAXFILENAME - 1);//�����в�����󳤶�ΪMAXFILENAME��
		filename_try[MAXFILENAME] = '\0';
	}
	strcat(filename_try, ".cvtelog.zip");//ͳһ����Ϊfilename.cvtelog.zip

	if (opt_overwrite == 2 && check_exist_file(filename_try) == 0) {//���û���-aѡ������ļ������ڣ���Ĭ���Ը��Ǵ�����ʽ��zip�ļ�
			opt_overwrite = 1;
	}

	/*��zip�ļ�*/
	zf = zipOpen64(filename_try, (opt_overwrite == 2) ? 2 : 0);
	if (zf == NULL)//��zip�ļ�ʧ��		
		bail("error in open %s\n", filename_try);
	else
		printf("creating %s\n", filename_try);

	//��ʼ����ļ�
	while (arg_file_index && argv[arg_file_index]) {
		const char* filenameinzip = argv[arg_file_index];

		err = stat(filenameinzip, &s_buf);//��ȡ�ļ���Ŀ¼��Ϣ������Ϣ�ŵ�s_buf��
		if (err != 0) {//����ļ���������������û����˳�
			bail("Warning: name not matched: [ %s ] \n", filenameinzip);
		}
		if (S_ISDIR(s_buf.st_mode)) {//�ж�������ļ�·���Ƿ�Ŀ¼������Ŀ¼������Ŀ¼�µ��ļ�
			err = writeDirInZip(filenameinzip, zf, opt_compress_level, password);
		}
		else if (S_ISREG(s_buf.st_mode)) {//��������ļ�·������ͨ�ļ�
			err = writeOneFilesInzip(filenameinzip, zf, opt_compress_level, password);
		}
		arg_file_index++;
	}
	err = zipClose(zf, NULL);//������Ҫ������ļ�������ɺ󣬹ر�zip�ļ�
	if (err != ZIP_OK) 
		bail("error in closing %s\n", filename_try);

	FD_SET(0, ding1);//����׼������뵽ding1�ļ�������������
	extern char *wrote_txt;
#ifdef LISTEN_MODE //�����Ҫʹ�ü���ģʽʱ����LISTEN_MODE�����
	//����ģʽ��
	if (o_listen) {//�������-p�Ӷ˿ںţ��˿ںŲ�Ϊ0
		netfd = dolisten(themaddr, 0, ouraddr, o_lport);//����host��Ϣ���˿���Ϣ��������ַ���Ͷ˿ںŷ���һ��socket�׽���
		if (netfd > 0) {//�׽��ַ�����ȷ��ʼ��д����
			/*�����ｫ��׼�����ض���Ϊ�ļ�*/
			if(opt_send == 1)
				zipFileFD = freopen(filename_try, "r", stdin);
			else if(opt_send == 2)
				zipFileFD = freopen(filename_try, "w", stdout);
			if (zipFileFD == 0) {
				printf("Error in open [ %s ]... \n", filename_try);
			}
			err = readwrite(netfd);		/* ��UDPģʽ����Ȼ���� */
			freopen("/dev/tty", "r", stdin);
			if (o_verbose > 1)		/* normally we don't care */
				holler(wrote_txt, wrote_net, wrote_out);
			/*�����ｫ��׼���븴ԭ*/
			bail("");				//��д��ɺ��˳�
		}
		else /* if no netfd */
			bail("no connection");
	} /* o_listen */
	else 	//�ͻ���ģʽ��
#endif
	{
		if (arg_host_index)//���arg_host_index != 0˵���и���host��Ϣ
		{
			if (argv[arg_host_index]) {
				char *hoststr = NULL;
				char *portstr = NULL;
				char *substr = NULL;
				int defaultPort = 0;

				substr = strtok(argv[arg_host_index], ":");
				//printf("%d : %s\n", __LINE__, substr);
				if (substr == NULL) {//û�м����������ж˿ں�
					defaultPort = 1;
					hoststr = argv[arg_host_index];
				}
				else {
					hoststr = substr;
					//printf("%d : %s\n", __LINE__, hoststr);
					substr = strtok(NULL, ":");
					if(!substr)
						defaultPort = 1;
					//printf("%d : %s\n", __LINE__, substr);
					portstr = substr;
				}
				whereto = gethostpoop(hoststr, o_nflag);//����host��Ϣ
				if (whereto && whereto->iaddrs)//���host����Ϣ������ip��Ϊ��
					themaddr = &whereto->iaddrs[0];//themaddr������ʵ��ip��ַ
				if (!themaddr)//���ip��ַΪ�գ��򱨴��˳�
					bail("no destination");
				if (!defaultPort) {//�����ʹ��Ĭ�϶˿�
					o_lport = getportpoop(portstr, 0);
					if (o_lport == 0)//���˿ںţ�����Ƿ��������˳�
						bail("Invalid local port %s", portstr);
				}
			}
			else {//���argv[arg_host_index]Ϊ�գ�˵��û��host��Ϣ
				bail("no destination");
			}
		}
		netfd = doconnect(themaddr, o_lport, ouraddr, 0);
		if (netfd > 0 && o_zero && o_udpmode)	/* ����û�ѡ��UDPģʽ */
			netfd = udptest(netfd, themaddr);
		if (netfd > 0) {			
			holler("%s [%s] %d (%s) open", whereto->name, whereto->addrs[0], o_lport, portpoop->name);
			if (!o_zero) {
				if (opt_send == 1)
					zipFileFD = freopen(filename_try, "r", stdin);
				else if (opt_send == 2)
					zipFileFD = freopen(filename_try, "w", stdout);
				if (zipFileFD == 0)
					printf("Error in open [ %s ]... \n", filename_try);
				err = readwrite(netfd);
				//freopen("/dev/tty", "r", stdin);
				//freopen("/dev/tty", "w", stdout);
			}
			bail("");
		}
		else {
			bail("no connection");
		}
	}
} 
