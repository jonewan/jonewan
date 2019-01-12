#include "packlog.h"
#include "netcat.h"


#ifdef POSIX_SETJMP
sigjmp_buf jbuf;		/* timer crud */
#else
extern jmp_buf jbuf;			/* timer crud */
#endif
extern int jval;			/* timer crud */
extern int netfd;
extern int ofd;			/* hexdump output fd */

extern USHORT Single;		/* zero if scanning */
extern unsigned int wrote_out;	/* total stdout bytes */
extern unsigned int wrote_net;	/* total net bytes */

extern struct timeval * timer1;
extern struct timeval * timer2;

extern SAI * lclend;		/* sockaddr_in structs */
extern SAI * remend;
extern HINF ** gates;		/* LSRR hop hostpoop */
extern char * optbuf;		/* LSRR or sockopts */
extern char * bigbuf_in;		/* data buffers */
extern char * bigbuf_net;
extern fd_set * ding1;			/* for select loop */
extern fd_set * ding2;
extern PINF * portpoop;		/* for getportpoop / getservby* */

/* global cmd flags: */
extern USHORT o_listen;
extern USHORT o_nflag;
extern USHORT o_udpmode;
extern USHORT o_verbose;
extern USHORT o_holler_stderr;
extern unsigned int o_wait;
extern USHORT o_zero;
extern int o_quit; /* 0 == quit-now; >0 == quit after o_quit seconds */

/* main :
   now we pull it all together... */
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
	USHORT curport = 0;

	int arg_host_index = 0;//host��Ϣ���±�
	int arg_file_index = 0;//�ļ���Ŀ¼���±�
	int arg_port_index = 0;//�˿ں��±�
	
	/*����ļ�ʹ�õ�һЩ�ֲ�����*/
	char *opt_zip_fileName = NULL;
	int opt_overwrite = 0;
	int opt_compress_level = Z_DEFAULT_COMPRESSION;
	char filename_try[MAXFILENAME + 16];
	int zipok = 1;
	int err = 0;
	zipFile zf;
	int errclose;
	char* password = NULL;
	struct stat s_buf;
	int zipFileFD = 0;

#ifdef HAVE_BIND
	/* can *you* say "cc -yaddayadda netcat.c -lresolv -l44bsd" on SunLOSs? */
	res_init();
#endif

	if (argc == 1)//���ֻ��һ������������ʾ������Ϣ
		helpme();

	/* I was in this barbershop quartet in Skokie IL ... */
	/* round up the usual suspects, i.e. malloc up all the stuff we need */
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

	/* If your shitbox doesn't have getopt, step into the nineties already. */
	/* optarg, optind = next-argv-component [i.e. flag arg]; optopt = last-char */

	while ((x = getopt(argc, argv, "hlnp:s:tuvw:zoaf:k:019")) != EOF) {
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
		case 'l':				/* listen mode */
			o_listen++; break;
		case 'n':				/* ��ָ��DNS��ֱ��ʹ��IP��ַ */
			o_nflag++; break;
		case 'p':				/* ָ���Ķ˿ں�*/
			o_lport = getportpoop(optarg, 0);
			if (o_lport == 0)//���˿ںţ�����Ƿ��������˳�
				bail("invalid local port %s", optarg);
			break;
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

	 /*����ڼ���ģʽ�£�û�и���-p�Ӷ˿ں��򱨴��˳�*/
	if (o_listen) {
		if (o_lport <= 0)
			bail("Warning : np -l -p portnumber");
		else {
			arg_file_index = optind;//��������˿ں����optind��ʼΪ�ļ���Ŀ¼
			arg_host_index = 0;//����ģʽ�²���Ҫ������������
			arg_port_index = 0;//û����
		}
	}
	else {//����ģʽ�£�Ҳ���ǿͻ���ģʽ��
		arg_host_index = optind;//��Ϊ��һ����ѡ�������host��Ϣ
		arg_port_index = arg_host_index + 1;//�ڶ�����ѡ������Ƕ˿ں�
		arg_file_index = arg_port_index + 1;//֮��ķ�ѡ���������Ŀ¼�����ļ�
	}

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

	if (opt_overwrite == 2) {//����û�ѡ��׷�ӵ�zip��
	/* ����û����ļ��Ƿ���� */
		if (check_exist_file(filename_try) == 0)
			opt_overwrite = 1;
	}
	else if (opt_overwrite == 0) {//�û�û�и���-a����-oѡ��
		if (check_exist_file(filename_try) != 0) {//�ļ�����
			char rep = 0;
			do {
				char answer[128];
				int ret;
				printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ", filename_try);
				ret = scanf("%1s", answer);
				if (ret != 1) {
					exit(EXIT_FAILURE);
				}
				rep = answer[0];
				if ((rep >= 'a') && (rep <= 'z'))//�û�д��Сд��ĸ
					rep -= 0x20;
			} while ((rep != 'Y') && (rep != 'N') && (rep != 'A'));
			if (rep == 'N')//�����ǣ�ֱ�ӷ���
				zipok = 0;
			if (rep == 'A')//�ļ�׷��
				opt_overwrite = 2;
		}
	}

	if (zipok != 1) {//��������д�����ṩ������Ϣ
		helpme();
		return 0;
	}
	//��������д��ȷ����ʼ����������
	zf = zipOpen64(filename_try, (opt_overwrite == 2) ? 2 : 0);

	if (zf == NULL) {//��zip�ļ�ʧ��		
		printf("error opening %s\n", filename_try);
		err = ZIP_ERRNO;
	}
	else
		printf("creating %s\n", filename_try);

	//��ʼ����ļ�
	while (argv[arg_file_index]) {
		const char* filenameinzip = argv[arg_file_index];

		err = stat(filenameinzip, &s_buf);//��ȡ�ļ���Ŀ¼��Ϣ������Ϣ�ŵ�s_buf��
		if (err != 0) {//����ļ���������������û����˳�
			printf("Warning: name not matched: [ %s ] \n", filenameinzip);
			exit(EXIT_FAILURE);
		}
		if (S_ISDIR(s_buf.st_mode)) {//�ж�������ļ�·���Ƿ�Ŀ¼������Ŀ¼������Ŀ¼�µ��ļ�
			err = writeDirInZip(filenameinzip, zf, opt_compress_level, password);
		}
		else if (S_ISREG(s_buf.st_mode)) {//��������ļ�·������ͨ�ļ�
			err = writeOneFilesInzip(filenameinzip, zf, opt_compress_level, password);
		}
		arg_file_index++;
	}
	errclose = zipClose(zf, NULL);//������Ҫ������ļ�������ɺ󣬹ر�zip�ļ�
	if (errclose != ZIP_OK) {
		printf("error in closing %s\n", filename_try);
		exit(-1);
	}

	/* other misc initialization */
	FD_SET(0, ding1);//����׼������뵽ding1�ļ�������������

	//����ģʽ��
	if (o_listen) {//�������-p�Ӷ˿ںţ��˿ںŲ�Ϊ0
		netfd = dolisten(themaddr, 0, ouraddr, o_lport);//����host��Ϣ���˿���Ϣ��������ַ���Ͷ˿ںŷ���һ��socket�׽���
		/* dolisten does its own connect reporting, so we don't holler anything here */
		if (netfd > 0) {//�׽��ַ�����ȷ��ʼ��д����
		 /* GAPING_SECURITY_HOLE */
			/*�����ｫ��׼�����ض���Ϊ�ļ�*/
			zipFileFD = freopen(filename_try, "r", stdin);
			if (zipFileFD == NULL) {
				printf("Error in open [ %s ]... \n", filename_try);
			}
			x = readwrite(netfd);		/* it even works with UDP! */
			if (o_verbose > 1)		/* normally we don't care */
				holler(wrote_txt, wrote_net, wrote_out);
			/*�����ｫ��׼���븴ԭ*/
			freopen("/dev/tty", "r", stdin);
			exit(x);				//��д��ɺ�ֱ���˳�
		}
		else /* if no netfd */
			bail("no connection");
	} /* o_listen */
	else {	//�ͻ���ģʽ��
		printf("%d : arg_host_index = %d\n", __LINE__, arg_host_index);
		if (arg_host_index)//���arg_host_index != 0˵���и���host��Ϣ
		{
			if (argv[arg_host_index]) {
				whereto = gethostpoop(argv[arg_host_index], o_nflag);//����host��Ϣ
				if (whereto && whereto->iaddrs)//���host����Ϣ������ip��Ϊ��
					themaddr = &whereto->iaddrs[0];//themaddr������ʵ��ip��ַ
				if (!themaddr)//���ip��ַΪ�գ��򱨴��˳�
					bail("no destination");
			}
			else {//���argv[arg_host_index]Ϊ�գ�˵��û��host��Ϣ
				bail("no destination");
			}
		}
		if (argv[arg_port_index] == NULL)//optind�����ǵڶ�����ѡ�����������˿ں�Ϊ��
			bail("no port[s] to connect to");//�����˳�
		else {//��������
			curport = getportpoop(argv[arg_port_index], 0);
			if (curport == 0)
				bail("invalid port %s", argv[arg_port_index]);
			netfd = doconnect(themaddr, curport, ouraddr, 0);
			if (netfd > 0 && o_zero && o_udpmode)	/* if UDP scanning... */
				netfd = udptest(netfd, themaddr);
			if (netfd > 0) {			/* Yow, are we OPEN YET?! */
				x = 0;				/* pre-exit status */
				holler("%s [%s] %d (%s) open", whereto->name, whereto->addrs[0], curport, portpoop->name);
				if (!o_zero) {
					zipFileFD = freopen(filename_try, "r", stdin);
					if (zipFileFD == NULL) {
						printf("Error in open [ %s ]... \n", filename_try);
					}
					x = readwrite(netfd);	/* go shovel shit */
					freopen("/dev/tty", "r", stdin);
				}
				exit(x);
			}
			else {
				bail("no connection");
			}
		}
	}

	exit(0);			/* otherwise, we're just done */
} 
