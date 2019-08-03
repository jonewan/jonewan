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
#ifdef LISTEN_MODE //如果需要使用监听模式时定义LISTEN_MODE这个宏
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
	HINF * whereto = NULL;//到哪去
	HINF * wherefrom = NULL;//从哪来
	IA * ouraddr = NULL;
	IA * themaddr = NULL;

	USHORT o_lport = 0;

	int arg_host_index = 0;//host信息的下标
	int arg_file_index = 0;//文件及目录的下标
	
	/*打包文件使用的一些局部变量*/
	char *opt_zip_fileName = NULL;
	int opt_overwrite = 1;
	int opt_compress_level = Z_DEFAULT_COMPRESSION;
	char filename_try[MAXFILENAME + 16];
	int err = 0;
	zipFile zf;
	char* password = NULL;
	struct stat s_buf;
	int zipFileFD = 0;
	int opt_send = 0;//0:无参数 1:发送方 2:接收方
	int opt_send_org = 0;//是否给出-r或-m选项

#ifdef HAVE_BIND
	/* can *you* say "cc -yaddayadda netcat.c -lresolv -l44bsd" on SunLOSs? */
	res_init();
#endif

	if (argc == 1)//如果只有一个参数，则显示帮助信息
		helpme();

	lclend = (SAI *)Hmalloc(sizeof(SA));
	remend = (SAI *)Hmalloc(sizeof(SA));
	bigbuf_in = Hmalloc(BIGSIZ);//申请标准输入输出的缓冲区空间
	bigbuf_net = Hmalloc(BIGSIZ);//申请来自网络输入输出的缓冲区空间
	ding1 = (fd_set *)Hmalloc(sizeof(fd_set));//用来存储文件描述符集合的空间
	ding2 = (fd_set *)Hmalloc(sizeof(fd_set));//用来存储文件描述符集合的空间
	portpoop = (PINF *)Hmalloc(sizeof(PINF));//用来存储端口信息的空间
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

	/*分析命令行*/
	while ((x = getopt(argc, argv, "hlnmrp:s:tuvw:zoaf:k:019")) != EOF) {
		switch (x) {
		case 'h':
			errno = 0;
			helpme();			/* exits by itself */
		case 'o':	//以覆盖方式进行写zip文件
			opt_overwrite = 1;
			break;
		case 'a':	//以追加方式进行写zip文件
			opt_overwrite = 2;
			break;
		/*压缩等级*/
		case '0':
			opt_compress_level = 0;
		break;
		case '1':
			opt_compress_level = 1;
			break;
		case '9':
			opt_compress_level = 9;
			break;
		/*指定zip文件名称*/
		case 'f':
			opt_zip_fileName = optarg;
			break;
		/*指定zip文件密码*/
		case 'k':
			password = optarg;
			break;
		case 'm':
			opt_send_org = 1;
			opt_send = 1;//m代表发送方master
			break;
		case 'r':
			opt_send_org = 1;
			opt_send = 2;//r代表接受方recesiver
			break;
#ifdef LISTEN_MODE //如果需要使用监听模式时定义LISTEN_MODE这个宏
		case 'l':				/* listen mode */
			o_listen++; break;
#endif
		case 'n':				/* 不指定DNS，直接使用IP地址 */
			o_nflag++; break;
#ifdef LISTEN_MODE
		case 'p':				/* 指定的端口号*/
			o_lport = getportpoop(optarg, 0);
			if (o_lport == 0)//检查端口号，如果非法，报错退出
				bail("invalid local port %s", optarg);
			break;
#endif
		case 's'://指定主机信息，也就是本机ip地址
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
		if (o_lport == 0)//检查端口号，如果非法，报错退出
			bail("invalid local port %s", DEFAULT_PORT);
	}
#ifdef LISTEN_MODE //如果需要使用监听模式时定义LISTEN_MODE这个宏
	 /*如果在监听模式下，没有给出-p加端口号则报错退出*/
	if (o_listen) {
		arg_file_index = optind;//如果给出端口号则从optind开始为文件或目录
		arg_host_index = 0;//监听模式下不需要给出主机名称
		if(opt_send_org == 0) 
			opt_send = 2;//默认情况下如果监听模式,没有给出-m或-r参数时，默认为接收端
	}
	else 
#endif	
	{//连接模式下，也就是客户端模式下
		arg_host_index = optind;//认为第一个非选项参数是host信息
		arg_file_index = arg_host_index + 1;//之后的非选项参数都是目录或者文件
		if (opt_send_org == 0)
			opt_send = 1;//默认情况下如果监听模式,没有给出-m或-r参数时，默认为接收端
	}

	if (opt_send == 2)//在接收模式下，用户输入的多余参数无效
		arg_file_index = 0;

	if (opt_zip_fileName == NULL) {//如果用户没有给出-f zip的名字，则默认生成“日期时间.cvtelog.zip”
		time_t now;
		struct tm *timenow; //实例化tm结构指针    
		time(&now);
		timenow = localtime(&now);
		sprintf(filename_try, "%d%02d%02d", timenow->tm_year + 1900, timenow->tm_mon + 1, timenow->tm_mday);
	}
	else {//如果用户给出了zip的名字则按照用户给出的名字命名
		strncpy(filename_try, opt_zip_fileName, MAXFILENAME - 1);//命令行参数最大长度为MAXFILENAME个
		filename_try[MAXFILENAME] = '\0';
	}
	strcat(filename_try, ".cvtelog.zip");//统一命名为filename.cvtelog.zip

	if (opt_overwrite == 2 && check_exist_file(filename_try) == 0) {//若用户给-a选项，但是文件不存在，则默认以覆盖创建形式打开zip文件
			opt_overwrite = 1;
	}

	/*打开zip文件*/
	zf = zipOpen64(filename_try, (opt_overwrite == 2) ? 2 : 0);
	if (zf == NULL)//打开zip文件失败		
		bail("error in open %s\n", filename_try);
	else
		printf("creating %s\n", filename_try);

	//开始打包文件
	while (arg_file_index && argv[arg_file_index]) {
		const char* filenameinzip = argv[arg_file_index];

		err = stat(filenameinzip, &s_buf);//获取文件或目录信息，把信息放到s_buf中
		if (err != 0) {//如果文件名输入错误，提醒用户并退出
			bail("Warning: name not matched: [ %s ] \n", filenameinzip);
		}
		if (S_ISDIR(s_buf.st_mode)) {//判断输入的文件路径是否目录，若是目录，分析目录下的文件
			err = writeDirInZip(filenameinzip, zf, opt_compress_level, password);
		}
		else if (S_ISREG(s_buf.st_mode)) {//若输入的文件路径是普通文件
			err = writeOneFilesInzip(filenameinzip, zf, opt_compress_level, password);
		}
		arg_file_index++;
	}
	err = zipClose(zf, NULL);//所有需要处理的文件处理完成后，关闭zip文件
	if (err != ZIP_OK) 
		bail("error in closing %s\n", filename_try);

	FD_SET(0, ding1);//将标准输入加入到ding1文件描述符集合中
	extern char *wrote_txt;
#ifdef LISTEN_MODE //如果需要使用监听模式时定义LISTEN_MODE这个宏
	//监听模式下
	if (o_listen) {//必须给出-p加端口号，端口号不为0
		netfd = dolisten(themaddr, 0, ouraddr, o_lport);//根据host信息、端口信息，本机地址，和端口号返回一个socket套接字
		if (netfd > 0) {//套接字返回正确后开始读写操作
			/*在这里将标准输入重定向为文件*/
			if(opt_send == 1)
				zipFileFD = freopen(filename_try, "r", stdin);
			else if(opt_send == 2)
				zipFileFD = freopen(filename_try, "w", stdout);
			if (zipFileFD == 0) {
				printf("Error in open [ %s ]... \n", filename_try);
			}
			err = readwrite(netfd);		/* 在UDP模式下依然工作 */
			freopen("/dev/tty", "r", stdin);
			if (o_verbose > 1)		/* normally we don't care */
				holler(wrote_txt, wrote_net, wrote_out);
			/*在这里将标准输入复原*/
			bail("");				//读写完成后退出
		}
		else /* if no netfd */
			bail("no connection");
	} /* o_listen */
	else 	//客户机模式下
#endif
	{
		if (arg_host_index)//如果arg_host_index != 0说明有给出host信息
		{
			if (argv[arg_host_index]) {
				char *hoststr = NULL;
				char *portstr = NULL;
				char *substr = NULL;
				int defaultPort = 0;

				substr = strtok(argv[arg_host_index], ":");
				//printf("%d : %s\n", __LINE__, substr);
				if (substr == NULL) {//没有检索到后面有端口号
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
				whereto = gethostpoop(hoststr, o_nflag);//返回host信息
				if (whereto && whereto->iaddrs)//如果host有信息，并且ip不为空
					themaddr = &whereto->iaddrs[0];//themaddr就是真实的ip地址
				if (!themaddr)//如果ip地址为空，则报错退出
					bail("no destination");
				if (!defaultPort) {//如果不使用默认端口
					o_lport = getportpoop(portstr, 0);
					if (o_lport == 0)//检查端口号，如果非法，报错退出
						bail("Invalid local port %s", portstr);
				}
			}
			else {//如果argv[arg_host_index]为空，说明没有host信息
				bail("no destination");
			}
		}
		netfd = doconnect(themaddr, o_lport, ouraddr, 0);
		if (netfd > 0 && o_zero && o_udpmode)	/* 如果用户选择UDP模式 */
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
