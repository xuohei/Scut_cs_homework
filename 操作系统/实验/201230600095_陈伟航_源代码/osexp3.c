#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<dirent.h>
#include<regex.h>
#include<string.h>
#include<string.h>
#include<time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>

#define TRUE 1
#define FALSE 0
#define MAX_COUNT 500//查询结果存放上界

int nameFlag=FALSE;//是否按名搜索
int pruneFlag=FALSE;//是否排除目录
int mtimeFlag=FALSE;//是否按时间查找修改的文件
int ctimeFlag=FALSE;//是否按时间查找创建的文件
int printFlag=FALSE;//是否输出搜索结果
int execFlag=FALSE;//是否开启-exec选项

int nameIndex=0;//-name是第几个参数
int pruneIndex=0;//-prune是第几个参数
int mtimeIndex=0;//-mtime是第几个参数
int ctimeIndex=0;//-ctime是第几个参数
int execIndex=0;//-exec是第几个参数

char* result[MAX_COUNT];//存储查询结果
int resultIndex=0;//result数组下标

void printHelp()
{//输出帮助信息
	printf("用法：./myfind 待查找路径 选项 查找条件正则表达式\n");
	printf("选项说明：\n");
	printf("-name \"文件\"\n\t\"指定要查找的文件名\"\n");
	printf("-prune 目录\n\t指出搜索时不搜索该目录\n");
	printf("-mtime +n或-n\n\t按时间查找\n");
	printf("\t+n表示n天之前修改过的文件\n");
	printf("\t-n表示n今天到n天之前修改过的文件\n");
	printf("-ctime +n或-n\n\t按时间查找\n");
	printf("\t+n表示n天之前创建的文件\n");
	printf("\t-n表示今天到n天前之间创建的文件\n");
	printf("-print\n\t将搜索结果输出到标准输出\n");
	printf("-exec 程序名\n\t对查找到的结果执行指定的程序\n");
}
void init(int argc,char*argv[])
{
	int i;
	if(argv[2][0]!='-')
	{//第三个参数要求以'-'开头
		printHelp();
		exit(EXIT_FAILURE);
	}
	for(i=2;i<argc;i++){
		if(argv[i][0]!='-')
			continue;
		if(strcmp(argv[i],"-name")==0)
		{
			nameFlag=TRUE;
			nameIndex=i;		
		}
		else if(strcmp(argv[i],"-prune")==0)
		{
			pruneFlag=TRUE;
			pruneIndex=i;
		}
		else if(strcmp(argv[i],"-mtime")==0)
		{
			mtimeFlag=TRUE;
			mtimeIndex=i;
		}
		else if(strcmp(argv[i],"-ctime")==0)
		{
			ctimeFlag=TRUE;
			ctimeIndex=i;
		}
		else if(strcmp(argv[i],"-print")==0)
		{
			printFlag=TRUE;
		}
		else if(strcmp(argv[i],"-exec")==0)
		{
			execFlag=TRUE;
			execIndex=i;
		}
	}
}
int matchName(char* string,char* pattern)
{
	regex_t reg;//正则表达式
	if(regcomp(&reg,pattern,REG_NOSUB|REG_ICASE)!=0){
		perror("compile regex error");
		printf("regex:%s\n",pattern);
		exit(EXIT_FAILURE);
	}
	int status=regexec(&reg,string,(size_t)0,NULL,0);
	regfree(&reg);
	if(status==0)
		return TRUE;
	return FALSE;
}
int matchMtime(struct stat stat_buf,int n)
{
	time_t now=time(NULL);
	time_t mtime=stat_buf.st_mtime;
	int days=(int)difftime(now,mtime)/(60*60*24);
	if(n<0)
	{//今天到n天前
		if(days+n<0)	return TRUE;
	}
	else
	{//n天前
		if(days>=n)	return TRUE;
	}
	return FALSE;
}
int matchCtime(struct stat stat_buf,int n)
{
	time_t now=time(NULL);
	time_t ctime_=stat_buf.st_ctime;
	int day=(int)difftime(now,ctime_)/(60*60*24);
	if(n<0)
	{//今天到n天前
		if(day+n<0) return TRUE;
	}
	else{//n天前
		if(day>=n)	return TRUE;
	}
	return FALSE;
}
void myexec(int argc,char* argv[])
{
	pid_t pid;	
	pid=fork();
	if(pid==0)
	{
		char* cmd[resultIndex+2];
		int i,j=0;
		for(i=execIndex+1;i<argc&&argv[i][0]!='-';i++)
		{
			cmd[j++]=argv[i];
		}
		for(i=0;i<resultIndex-1;i++)
		{
			cmd[j++]=result[i];
		}
		cmd[resultIndex]=(char*)0;
		execvp(cmd[0],cmd);
	}
	else if(pid>0){
		wait(NULL);
	}
}
char* getPath(char* path,char* filename)
{//获取文件路径
	int len1=strlen(path);
	int len2=strlen(filename);
	char* str=(char*)malloc(len1+len2+2);
	str[0]='\0';
	str=strcat(str,path);
	if(path[len1-1]!='/')	str=strcat(str,"/");
	str=strcat(str,filename);
	return str;
}

void myfind(int argc,char* argv[],char * path)
{//主函数
	DIR* dir;
	struct dirent *doc;
	struct stat stat_buf;
	if(path==NULL)	path=argv[1];
	char* filepath;

	//在目录中搜索
	if((dir=opendir(path))==NULL)
	{
		perror("function opendir:");
		printf("dir:%s\n",path);
		return; 
	}
	do{
		errno=0;
		if((doc=readdir(dir))!=NULL)
		{
			int matchFlag=TRUE;
			filepath=getPath(path,doc->d_name);
			if(lstat(filepath,&stat_buf)!=0)
			{
				perror("function stat error");
				printf("stat path:%s\n",filepath);
				continue;
			}
			if(nameFlag==TRUE)
			{//-name
				int len=strlen(argv[nameIndex+1]);
				if(argv[nameIndex+1][0]=='"'&&argv[nameIndex+1][len-1]=='"')
				{
					int index=0;
					for(;index<len-2;index++)
						argv[nameIndex+1][index]=argv[nameIndex+1][index+1];
					argv[nameIndex+1][len-2]='\0';
				}
				if(!matchName(doc->d_name,argv[nameIndex+1]))
					matchFlag=FALSE;
			}
			if(mtimeFlag==TRUE)
			{//-mtime
				if(!matchMtime(stat_buf,atoi(argv[mtimeIndex+1])))
					matchFlag=FALSE;
			}
			if(ctimeFlag==TRUE)
			{//-ctime
				if(!matchCtime(stat_buf,atoi(argv[ctimeIndex+1])))
					matchFlag=FALSE;
			}
			if(matchFlag==TRUE)
			{
				if(execFlag==TRUE)
				{
					if(resultIndex<500)
						result[resultIndex++]=filepath;
					myexec(argc,argv);
				}
				if(printFlag==TRUE)
					printf("%s\n",filepath);
			}
			if(S_ISDIR(stat_buf.st_mode)
					&&strcmp(doc->d_name,".")!=0
					&&strcmp(doc->d_name,"..")!=0)
			{
				if(pruneFlag==TRUE)
				{
					struct stat stat_buf1,stat_buf2;
					if(lstat(argv[pruneIndex+1],&stat_buf1)!=0)
					{
						perror("function lstat error");
						printf("stat path:%s\n",argv[pruneIndex+1]);
						exit(EXIT_FAILURE);
					}
					if(lstat(filepath,&stat_buf2)!=0)
					{
						perror("function lstat error");
						printf("stat path:%s\n",filepath);
						exit(EXIT_FAILURE);
					}
					if(stat_buf1.st_ino==stat_buf2.st_ino
							&&stat_buf1.st_dev==stat_buf2.st_dev)
						continue;//判断是否为排除目录
				}
				myfind(argc,argv,filepath);//递归搜索
			}
		}
	}while(doc!=NULL);
	if(errno!=0)
	{
		perror("readdir error");
		exit(EXIT_FAILURE);
	}
	closedir(dir);
}
int main ( int argc, char *argv[] )
{	
	if(argc<=2)
	{
		printf("格式错误!\n");
		printHelp();
		exit(EXIT_FAILURE);
	}
	init(argc,argv);
	myfind(argc,argv,argv[1]);
	return EXIT_SUCCESS;
}
