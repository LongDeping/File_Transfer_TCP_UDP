#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include "./MD5Lib/md5.h"

#define MAXNUM 4096
#define PORT_TCP 8123
#define PORT_UDP 8124

#define MD5LEN (32+1)

#define SENDFILE '0'
#define RECIVEFILE '1'

#define FILEEXIST '1'
#define NOFILE '0'

char ServerIP[20][20];	//搜索到的服务器ip地址
int ServerNum=0;		//搜索到的服务器个数

void ClientSendFile_TCP(int sockfd);
void ClientReciveFile_TCP(int sockfd);
void ClientSendFile_UDP(int sockfd, struct sockaddr* servaddr);
void ClientReciveFile_UDP(int sockfd, struct sockaddr* servaddr, int* szAddr);

void GetIpPart(char* ip, char* ippart);
void SearchServer(char* ippart);

void TCP_Client(void)
{
	char Cmd[MAXNUM];
    struct sockaddr_in servaddr;
	int sockfd;

	printf("TCP Client is running!\n");
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("creat socket error: %s(errno: %d)\n", strerror(errno), errno);
        goto error; 
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT_TCP);
	if (inet_pton(AF_INET, "10.1.74.53", &servaddr.sin_addr) <= 0)
	{
	    printf("inet_pton error \n");
	    goto error; 
	}
	
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        goto error; 
    }
	
InputCmd:
    printf("Input 0 for send file, 1 for recive file(end with Enter):");

	bzero(Cmd, MAXNUM);
	
	fgets(Cmd, MAXNUM, stdin); //从命令行获取字符串

	if(strlen(Cmd)!=2)
	{
		printf("Wrong Cmd! Please input again:\n");
		goto InputCmd;
	}

	if(SENDFILE== Cmd[0])
	{
		if (send(sockfd, Cmd, strlen(Cmd), 0) < 0)
    	{
        	printf("send Cmd error!\n");
       		goto error;
    	}
		ClientSendFile_TCP(sockfd);
	}
	else if(RECIVEFILE==Cmd[0])
	{
		if (send(sockfd, Cmd, strlen(Cmd), 0) < 0)
    	{
        	printf("send Cmd error!\n");
       		goto error;
    	}
		ClientReciveFile_TCP(sockfd);
	}
	else
	{
		printf("Wrong Cmd! Please input again:\n");
		goto InputCmd;
	}	
		
error:
	close(sockfd);
	return;
}

void UDP_Client(void)
{
	char Cmd[MAXNUM];
	char FileName[MAXNUM];
    struct sockaddr_in servaddr;
	int sockfd;
	int SendNum;
	int szAddr= sizeof(servaddr);
	
	printf("UDP Client is running!\n");

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("creat socket error: %s(errno: %d)\n", strerror(errno), errno);
        goto error; 
    }

	memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT_UDP);
	if (inet_pton(AF_INET, "10.1.74.53", &servaddr.sin_addr) <= 0)
	{
	    printf("inet_pton error \n");
	    goto error; 
	}

	InputCmd:
    printf("Input 0 for send file, 1 for recive file(end with Enter):");

	bzero(Cmd, MAXNUM);
	
	fgets(Cmd, MAXNUM, stdin); //从命令行获取字符串

	if(strlen(Cmd)!=2)
	{
		printf("Wrong Cmd! Please input again:\n");
		goto InputCmd;
	}

	if(SENDFILE== Cmd[0])
	{
		if (sendto(sockfd,Cmd,strlen(Cmd),0,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
    	{
        	printf("send Cmd error!\n");
       		goto error;
    	}

		ClientSendFile_UDP(sockfd, (struct sockaddr*)&servaddr);
	}
	else if(RECIVEFILE==Cmd[0])
	{
		if (sendto(sockfd,Cmd,strlen(Cmd),0,(struct sockaddr*)&servaddr,sizeof(struct sockaddr_in)) < 0)
    	{
        	printf("send Cmd error!\n");
       		goto error;
    	}
		ClientReciveFile_UDP(sockfd,(struct sockaddr*)&servaddr, &szAddr);
	}
	else
	{
		printf("Wrong Cmd! Please input again:\n");
		goto InputCmd;
	}	

error:
	close(sockfd);
	return;	
}



int main(int argc, char **argv)
{
	char ippart[20];

	if(4 != argc)
	{
		printf("usage : %s <bFindServ> <ClientIP> <0:TCP/1:UDP>\n", argv[0]);
		exit(1);
	}

	if('1' == *argv[1])
	{
		GetIpPart( argv[2], ippart);
		SearchServer( ippart);
	}

	if('0' == *argv[3])
	{
		TCP_Client();
	}
	else if('1' == *argv[3])
	{
		UDP_Client();
	}
	else
	{
		printf("usage : %s <bFindServ> <ClientIP> <0:TCP/1:UDP>\n", argv[0]);
	}

	return 0;
	
}

void ClientSendFile_TCP(int sockfd)
{
	int fp;
	int ret;
	char FileName[MAXNUM];
	char FilePath[MAXNUM];
	struct stat st;
	int Size=0;
	char md5_str[MD5LEN];

	bzero(md5_str, MD5LEN);
	
InputFileName:
	bzero(FileName, MAXNUM);
	bzero(FilePath, MAXNUM);
	
	printf("Client Send File, Please input the file name: ");
	scanf("%s", FileName);

	sprintf(FilePath, "./file/%s", FileName);

	fp = open(FilePath, O_RDONLY);
	if(fp < 0)
	{
		printf("File:%s Not Found! Please input again!\n", FileName);
		goto InputFileName;
	}
	else
	{
		/*发送文件名到服务器*/
		if (send(sockfd, FileName, strlen(FileName), 0) < 0)
   		{
        	printf("send FileName error!\n");
       		goto Exit;
    	}
		close(fp);
	}

	/*计算文件的MD5加密信息，为32位，并发送到服务器以作校验*/
	ret = Compute_file_md5(FilePath, md5_str);
	if (0 == ret)
	{
		printf("Client file %s's MD5 value is : %s\n", FileName,md5_str);
	}
	else
	{
		printf("Calculate MD5 value error!\n");
		goto Exit;
	}

	if (send(sockfd, md5_str, strlen(md5_str), 0) < 0)
    {
        printf("Send MD5 value error!\n");
       	goto Exit;
    }

	sleep(1);	//若两次发生时间太短，对方接收时容易产生错误，变为一次性接收；

	/*获取文件大小，并将文件发送到服务器*/
	fp = open(FilePath, O_RDONLY);
	stat(FilePath, &st);
	if(sendfile(sockfd, fp, 0, st.st_size) < 0)
	{
		printf("Send file error!\n");
		goto Exit;
	}
	close(fp);	
	
	printf("File %s send successful!\n\n\n", FileName);

Exit:
	close(fp);
	return;
	
}

void ClientReciveFile_TCP(int sockfd)
{
	FILE* fp;
	int length = 0;
	int ret;
	char FileName[MAXNUM];
	char Msg[MAXNUM];
	char buffer[MAXNUM];
	char FilePath[MAXNUM];
	char md5_str_client[MD5LEN];
	char md5_str_server[MD5LEN];
	
	bzero(buffer, MAXNUM);
	bzero(md5_str_client, MD5LEN);
	bzero(md5_str_server, MD5LEN);

InputFileName:
	bzero(FileName, MAXNUM);
	bzero(Msg, MAXNUM);
	bzero(FilePath, MAXNUM);
	
	printf("Client Recive File, Please input the file name: ");
	scanf("%s", FileName);
	
	if (send(sockfd, FileName, strlen(FileName), 0) < 0)
    {
        printf("Send FileName error!\n");
       	goto Exit;
    }

	if(recv(sockfd, Msg, MAXNUM, 0)<0)
    {
        printf("Recive Msg error!\n");
		goto Exit;
    }
	
	if(Msg[0]==FILEEXIST)
	{
		if(recv(sockfd, md5_str_server, MD5LEN, 0)<0)
   		{
    		printf("Recive MD5 value error!\n");
			goto Exit;
    	}
		printf("Recived MD5 value is : %s\n",md5_str_server);
		
		sprintf(FilePath, "./file/%s", FileName);
		fp = fopen(FilePath, "w");
		while((length = recv(sockfd, buffer, sizeof(buffer), 0)) > 0)
		{
			fwrite(buffer, 1, length, fp);
		}
		fclose(fp);

		ret = Compute_file_md5(FilePath, md5_str_client);
		if (0 == ret)
		{
			printf("Client file %s's MD5 value is : %s\n", FileName,md5_str_client);
		}
		else
		{
			printf("Calculate MD5 value error!\n");
			goto Exit;
		}
		
		if(0 == strcmp(md5_str_server, md5_str_client))
		{
			printf("MD5 check successed!\nFile %s recived successfully!\n\n\n", FileName);
		}
		else
		{
			printf("MD5 check failed! Recived file wrong!\n");
		}
	}
	else
	{
		printf("File doesn't exist! Please input file name again!\n ");
		goto InputFileName;
	}
		
Exit:
	return;

}


void GetIpPart(char* ip, char* ippart)
{
	int i=0;
	int count=0;
	int num=0;
	
	while((*(ip+i)) != '\0')
	{
		num++;
		i++;
		if((*(ip+i))=='.')
		{
			count++;
			if(3 == count)
			{
				break;
			}
		}
	}
	strncpy(ippart, ip, num+1);
	return;
}

void SearchServer(char* ippart)
{
	char iptest[20];
	struct sockaddr_in servaddr;
	int i;
	int sockfd;

	printf("Start search server!\n");
	
	for(i=1; i<256;i++)
	{
		sprintf(iptest, "%s%d", ippart, i);

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    	{
        	printf("creat socket error: %s(errno: %d)\n", strerror(errno), errno);
			close(sockfd);
			exit(0);
    	}

    	memset(&servaddr, 0, sizeof(servaddr));
    	servaddr.sin_family = AF_INET;
   	 	servaddr.sin_port = htons(PORT_TCP);
		
	    /*将IP地址点分十进制和转换成整数*/
	    if (inet_pton(AF_INET, iptest, &servaddr.sin_addr) <= 0)
	    {
	        printf("inet_pton error \n");
			close(sockfd);
			exit(0);
	    }

		if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0)
		{
			printf("Find Server! Server ip is :%s\n", iptest);
			sprintf(ServerIP[ServerNum++], "%s", iptest);
			close(sockfd);
		}
	}

	printf("Server search over!\n");
		
}

void ClientSendFile_UDP(int sockfd, struct sockaddr* servaddr)
{
	int fp;
	int ret;
	char FileName[MAXNUM];
	char FilePath[MAXNUM];
	char Buff[MAXNUM];
	struct stat st;
	int Size=0;
	char md5_str[MD5LEN];
	int Num;
	long long int FileSize;
	
	bzero(md5_str, MD5LEN);
	bzero(Buff, MAXNUM);

InputFileName:
	bzero(FileName, MAXNUM);
	bzero(FilePath, MAXNUM);
	
	printf("Client Send File, Please input the file name: ");
	scanf("%s", FileName);

	sprintf(FilePath, "./file/%s", FileName);

	fp = open(FilePath, O_RDONLY);
	if(fp < 0)
	{
		printf("File:%s Not Found! Please input again!\n", FileName);
		goto InputFileName;
	}
	else
	{
		/*发送文件名到服务器*/
		if (sendto(sockfd,FileName,strlen(FileName),0, servaddr,sizeof(struct sockaddr_in)) < 0)
    	{
        	printf("send FileName error!\n");
       		goto Exit;
    	}
		close(fp);
	}
	

	/*计算文件的MD5加密信息，为32位，并发送到服务器以作校验*/
	ret = Compute_file_md5(FilePath, md5_str);
	if (0 == ret)
	{
		printf("Client file %s's MD5 value is : %s\n", FileName,md5_str);
	}
	else
	{
		printf("Calculate MD5 value error!\n");
		goto Exit;
	}

	if (sendto(sockfd,md5_str,strlen(md5_str),0, servaddr,sizeof(struct sockaddr_in)) < 0)
    {
        printf("Send MD5 value error!\n");
       	goto Exit;
    }

	sleep(1);	//若两次发生时间太短，对方接收时容易产生错误，变为一次性接收；

	/*获取文件大小，并将文件发送到服务器*/
	fp = open(FilePath, O_RDONLY);
	
	stat(FilePath, &st);
	FileSize = st.st_size;

	printf("The FileSize is : %lld\n", FileSize);
	if(sendto(sockfd, &FileSize, sizeof(FileSize), 0, servaddr,sizeof(struct sockaddr_in)) < 0)
	{
		printf("Send file size error!\n");
		goto Exit;
	}
	
	bzero(Buff, MAXNUM);
	Num = read(fp, Buff, MAXNUM);
	while(Num)
		{
			if(-1 == Num)
			{
				printf("Read file error!\n");
				goto Exit;
			}
			
			if(sendto(sockfd, Buff, Num, 0, servaddr,sizeof(struct sockaddr_in)) < 0)
			{
				printf("Send file error!\n");
				goto Exit;
			}
			
			bzero(Buff, MAXNUM);
			Num = read(fp, Buff, MAXNUM);
		}
	
	close(fp);	
	printf("File %s send successful!\n\n\n", FileName);

	
Exit:
	close(fp);
	return;
}


void ClientReciveFile_UDP(int sockfd, struct sockaddr* servaddr, int* szAddr)
{
	FILE* fp;
	int length = 0;
	int ret;
	char FileName[MAXNUM];
	char Msg[MAXNUM];
	char buffer[MAXNUM];
	char FilePath[MAXNUM];
	char md5_str_client[MD5LEN];
	char md5_str_server[MD5LEN];
	long long int FileSize=0;
	
	bzero(buffer, MAXNUM);
	bzero(md5_str_client, MD5LEN);
	bzero(md5_str_server, MD5LEN);

InputFileName:
	bzero(FileName, MAXNUM);
	bzero(Msg, MAXNUM);
	bzero(FilePath, MAXNUM);
	
	printf("UDP Client Recive File, Please input the file name: ");
	scanf("%s", FileName);

	if (sendto(sockfd, FileName, strlen(FileName), 0, servaddr,sizeof(struct sockaddr_in)) < 0)
    {
        printf("Send FileName error!\n");
       	goto Exit;
    }

	if(recvfrom(sockfd, Msg, sizeof(Msg), 0, servaddr, szAddr)<0)
    {
        printf("Recive Msg error!\n");
		goto Exit;
    }

	if(Msg[0]==FILEEXIST)
	{
		if(recvfrom(sockfd, md5_str_server, sizeof(md5_str_server), 0, servaddr, szAddr)<0)
   		{
    		printf("Recive MD5 value error!\n");
			goto Exit;
    	}
		printf("Recived MD5 value is : %s\n",md5_str_server);

		if(recvfrom(sockfd, &FileSize, sizeof(FileSize), 0, servaddr, szAddr)<0)
		{
			printf("UDP server recive FileSize error!\n");
			goto Exit;
		}
		printf("Recived FileSize is : %lld\n",FileSize);
		
		sprintf(FilePath, "./file/%s", FileName);
		fp = fopen(FilePath, "w");
		
		length = recvfrom(sockfd, buffer, sizeof(buffer), 0, servaddr, szAddr);
		while(length)
		{
			fwrite(buffer, 1, length, fp);
			FileSize = FileSize - length;
			if(FileSize>0)
				{
					length = recvfrom(sockfd, buffer, sizeof(buffer), 0, servaddr, szAddr);
					if(length < 0)
					{
						printf("UDP server recive file error!\n");
						fclose(fp);
						goto Exit;
					}
				}
			else
				{
					length = 0;
				}
		}
		fclose(fp);


		ret = Compute_file_md5(FilePath, md5_str_client);
		if (0 == ret)
		{
			printf("Client file %s's MD5 value is : %s\n", FileName,md5_str_client);
		}
		else
		{
			printf("Calculate MD5 value error!\n");
			goto Exit;
		}
		
		if(0 == strcmp(md5_str_server, md5_str_client))
		{
			printf("MD5 check successed!\nFile %s recived successfully!\n\n\n", FileName);
		}
		else
		{
			printf("MD5 check failed! Recived file wrong!\n");
		}
	}
	else
	{
		printf("File doesn't exist! Please input file name again!\n ");
		goto InputFileName;
	}

Exit:
	return;
}



