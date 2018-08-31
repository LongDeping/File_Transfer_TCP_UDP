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
#include <sys/sendfile.h>
#include <sys/stat.h>
#include "./MD5Lib/md5.h"
#include <pthread.h>


#define PORT_TCP 8123
#define PORT_UDP 8124

#define MAXNUM 4096
#define MD5LEN (32+1)

#define SENDFILE '1'
#define RECIVEFILE '0'

#define FILEEXIST "1"
#define NOFILE "0"

void ServerReciveFile_TCP(int sockfd);
void ServerSendFile_TCP(int sockfd);
void ServerReciveFile_UDP(int sockfd, struct sockaddr * servaddr, int* szAddr);
void ServerSendFile_UDP(int sockfd, struct sockaddr * servaddr, int* szAddr);



void Thread_TCP_Server(void)
{
	char Cmd[MAXNUM];
	struct sockaddr_in servaddr;
	int socket_fd, connect_fd;
	
	/*初始化Socket IPV4 TCP*/
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Creat socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}
	/*初始化*/
	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET;				 //IPV4 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//自动获取本机IP
	servaddr.sin_port = htons(PORT_TCP);	 

	/*本地地址绑定到创建的嵌套字*/
	if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		printf("Bind socket error: %s(errno: %d)\n", strerror(errno), errno);
		goto exit;
	}
	/*监听客服端连接*/
	if (listen(socket_fd, 10) < 0)
	{
		printf("Listen socket error: %s(errno: %d)\n", strerror(errno), errno);
		goto exit;
	}
	
	printf("Waiting for client request\n");
	
	while(1)
	{
		/*阻塞直到有客户端连接*/
		if ((connect_fd = accept(socket_fd, (struct sockaddr *)NULL, NULL)) < 0)
		{
			printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
			goto exit;
		}
		else
		{
			printf("Client connected!\n");
		}
		
		if(recv(connect_fd, Cmd, MAXNUM, 0)<0)
		{
			printf("Recive Cmd error!\n");
			goto exit;
		}

		if(Cmd[0]==RECIVEFILE)
		{
			ServerReciveFile_TCP(connect_fd);
		}
		else if (Cmd[0]==SENDFILE)
		{	
			ServerSendFile_TCP(connect_fd);
		}
		else
		{
			printf("Wrong Cmd!");
			goto exit;
		}
		
		close(connect_fd);
	}

exit:
	close(connect_fd);
	close(socket_fd);

}

void Thread_UDP_Server(void)
{
	char Cmd[MAXNUM];
	struct sockaddr_in servaddr;
	int socket_fd;
	int szAddr= sizeof(servaddr);
		
	/*初始化Socket IPV4 UCP*/
	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("Creat socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}
	
	/*初始化*/
	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET;				 //IPV4 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//自动获取本机IP
	servaddr.sin_port = htons(PORT_UDP);	 

	/*本地地址绑定到创建的嵌套字*/
	if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		printf("Bind socket error: %s(errno: %d)\n", strerror(errno), errno);
		goto exit;
	}

	printf(" UDP Client is running!\n");

	while(1)
	{
		if(recvfrom(socket_fd, Cmd, MAXNUM, 0, (struct sockaddr *)&servaddr, &szAddr)<0)
		{
			printf("UDP server recive Cmd error!\n");
			goto exit;
		}

		printf("UDP recive cmd is: %s\n", Cmd);

		if(Cmd[0] == RECIVEFILE)
		{
			ServerReciveFile_UDP(socket_fd, (struct sockaddr *)&servaddr, &szAddr);
		}
		else if (Cmd[0] == SENDFILE)
		{	
			ServerSendFile_UDP(socket_fd, (struct sockaddr *)&servaddr, &szAddr);
		}
		else
		{
			printf("Wrong Cmd!");
			goto exit;
		}

	}

exit:
	close(socket_fd);
}

int main(int argc, char **argv)
{
	int ret;
	pthread_t Id1;
	pthread_t Id2;

	ret=pthread_create(&Id1,NULL,(void *) Thread_TCP_Server,NULL);
	if(ret!=0)
	{
		printf ("Create pthread error!n");
	}

	ret=pthread_create(&Id2,NULL,(void *) Thread_UDP_Server,NULL);
	if(ret!=0)
	{
		printf ("Create pthread error!n");
	}

	while(1)
	{
		sleep(5);
	}

#if 0
    char Cmd[MAXNUM];
    struct sockaddr_in servaddr;
    /*初始化Socket IPV4 TCP*/
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Creat socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    /*初始化*/
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;               //IPV4 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//自动获取本机IP
    servaddr.sin_port = htons(DEFAULT_PORT);     

    /*本地地址绑定到创建的嵌套字*/
    if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("Bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        goto exit;
    }
    /*监听客服端连接*/
    if (listen(socket_fd, 10) < 0)
    {
        printf("Listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        goto exit;
    }
	
    printf("Waiting for client request\n");
	
    while(1)
    {
        /*阻塞直到有客户端连接*/
        if ((connect_fd = accept(socket_fd, (struct sockaddr *)NULL, NULL)) < 0)
        {
            printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
            goto exit;
        }
		else
		{
			printf("Client connected!\n");
		}
        
        if(recv(connect_fd, Cmd, MAXNUM, 0)<0)
        {
        	printf("Recive Cmd error!\n");
			goto exit;
        }

		if(Cmd[0]==RECIVEFILE)
		{
			ServerReciveFile(connect_fd);
		}
		else if (Cmd[0]==SENDFILE)
		{	
			ServerSendFile(connect_fd);
		}
		else
		{
			printf("Wrong Cmd!");
			goto exit;
		}
		
        close(connect_fd);
    }

exit:
	close(connect_fd);
    close(socket_fd);
#endif
    return 0;
}


void ServerReciveFile_TCP(int sockfd)
{
	char FileName[MAXNUM];
	FILE *fp ;
	int length = 0;
	char buffer[MAXNUM];
	char FilePath[MAXNUM];
	char md5_str_client[MD5LEN];
	char md5_str_server[MD5LEN];
	int ret;

	bzero(FileName, MAXNUM);
	bzero(buffer, MAXNUM);
	bzero(FilePath, MAXNUM);
	bzero(md5_str_client, MD5LEN);
	bzero(md5_str_server, MD5LEN);
	
	if(recv(sockfd, FileName, MAXNUM, 0)<0)
    {
    	printf("TCP server : Recive FileName error!\n");
		goto Exit;
    }
	else
	{
		printf("TCP server : Recived file name is: %s\n",FileName);
	}

	if(recv(sockfd, md5_str_client, MD5LEN, 0)<0)
    {
    	printf("TCP server : Recive MD5 value error!\n");
		goto Exit;
    }
	else
	{
		printf("TCP server : Recived MD5 value is : %s\n",md5_str_client);
	
}
	
	sprintf(FilePath, "./file/%s", FileName);
	fp = fopen(FilePath, "w");
	if(fp == NULL)
	{
		printf("TCP server : Open file error!\n");
		goto Exit;
	}
	while((length = recv(sockfd, buffer, sizeof(buffer), 0)) > 0)
	{
		fwrite(buffer, 1, length, fp);
	}
	fclose(fp);

	ret = Compute_file_md5(FilePath, md5_str_server);
	if (0 == ret)
	{
		printf("TCP server : Server file %s's MD5 value is : %s\n", FileName,md5_str_server);
	}
	else
	{
		printf("TCP server : Calculate MD5 value error!\n");
		goto Exit;
	}

	if(0 == strcmp(md5_str_server, md5_str_client))
	{
		printf("TCP server : MD5 check successed!\nFile %s recived successfully!\n\n\n", FileName);
	}
	else
	{
		printf("TCP server : MD5 check failed! Recived file wrong!\n");
	}

	
Exit:
	return;
}

void ServerSendFile_TCP(int sockfd)
{
	char FileName[MAXNUM];
	char Msg[MAXNUM];
	char FilePath[MAXNUM];
	char md5_str[MD5LEN];
	int fp;
	int ret;
	struct stat st;

	bzero(md5_str, MD5LEN);
	
ReciveFileName:
	
	bzero(FileName, MAXNUM);
	bzero(Msg, MAXNUM);
	bzero(FilePath, MAXNUM);
	
	if(recv(sockfd, FileName, MAXNUM, 0)<0)
    {
    	printf("Recive FileName error!\n");
		goto Exit;
    }

	sprintf(FilePath, "./file/%s", FileName);
	fp = open(FilePath, O_RDONLY);
	if(fp < 0)
	{	
		
		sprintf(Msg, NOFILE);
		if (send(sockfd, Msg, strlen(Msg), 0) < 0)
    	{
        	printf("Send Msg error!\n");
       		goto Exit;
   		}
		goto ReciveFileName;
	}
	else
	{
		sprintf(Msg, FILEEXIST);
		if (send(sockfd, Msg, strlen(Msg), 0) < 0)
    	{
        	printf("Send Msg error!\n");
       		goto Exit;
   		}
		close(fp);
	}

	/*计算文件的MD5加密信息，为32位，并发送到服务器以作校验*/
	ret = Compute_file_md5(FilePath, md5_str);
	if (0 == ret)
	{
		printf("Server file %s's MD5 value is : %s\n", FileName,md5_str);
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

	sleep(1);	//若两次send之间的间隔太短，对方接收容易发生错误！
		
	/*获取文件大小，并将文件发送到客户端*/
	fp = open(FilePath, O_RDONLY);
	stat(FilePath, &st);
	if(sendfile(sockfd, fp, 0, st.st_size) < 0)
	{
		printf("Send file error!\n");
		goto Exit;
	}
	close(fp);
	
	printf("File %s send successfully!\n\n\n", FileName);
	
Exit:
	close(fp);
	return;
		
}


void ServerReciveFile_UDP(int sockfd, struct sockaddr * servaddr, int* szAddr)
{
	char FileName[MAXNUM];
	FILE *fp ;
	int length = 0;
	char buffer[MAXNUM];
	char FilePath[MAXNUM];
	char md5_str_client[MD5LEN];
	char md5_str_server[MD5LEN];
	int ret;
	long long int FileSize=0;

	bzero(FileName, MAXNUM);
	bzero(buffer, MAXNUM);
	bzero(FilePath, MAXNUM);
	bzero(md5_str_client, MD5LEN);
	bzero(md5_str_server, MD5LEN);

	if(recvfrom(sockfd, FileName, MAXNUM, 0, servaddr, szAddr)<0)
	{
		printf("UDP server recive FileName error!\n");
		goto exit;
	}

	if(recvfrom(sockfd, md5_str_client, MD5LEN, 0, servaddr, szAddr)<0)
	{
		printf("UDP server recive MD5 value error!\n");
		goto exit;
	}
	printf("Recived MD5 value is : %s\n",md5_str_client);

	if(recvfrom(sockfd, &FileSize, sizeof(FileSize), 0, servaddr, szAddr)<0)
	{
		printf("UDP server recive FileSize error!\n");
		goto exit;
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
					goto exit;
				}
			}
		else
			{
				length = 0;
			}
	}
	fclose(fp);

	printf("Server runs here!\n");
	ret = Compute_file_md5(FilePath, md5_str_server);
	if (0 == ret)
	{
		printf("Server file %s's MD5 value is : %s\n", FileName,md5_str_server);
	}
	else
	{
		printf("Calculate MD5 value error!\n");
		goto exit;
	}

	if(0 == strcmp(md5_str_server, md5_str_client))
	{
		printf("MD5 check successed!\nFile %s recived successfully!\n\n\n", FileName);
	}
	else
	{
		printf("MD5 check failed! Recived file wrong!\n");
	}

exit:
	return;

}

void ServerSendFile_UDP(int sockfd, struct sockaddr * servaddr, int* szAddr)
{
	char FileName[MAXNUM];
	char Msg[MAXNUM];
	char FilePath[MAXNUM];
	char Buff[MAXNUM];
	char md5_str[MD5LEN];
	int fp;
	int ret;
	int Num;
	struct stat st;
	long long int FileSize;
	
ReciveFileName:
		
	bzero(FileName, MAXNUM);
	bzero(Msg, MAXNUM);
	bzero(FilePath, MAXNUM);

	if(recvfrom(sockfd, FileName, sizeof(FileName), 0, servaddr, szAddr)<0)
    {
    	printf("Recive FileName error!\n");
		goto Exit;
    }

	printf("UDP Server recived file name is: %s\n", FileName);

	sprintf(FilePath, "./file/%s", FileName);
	fp = open(FilePath, O_RDONLY);
	if(fp < 0)
	{	
		sprintf(Msg, NOFILE);
		if (sendto(sockfd, Msg, strlen(Msg), 0, servaddr,sizeof(struct sockaddr_in)) < 0)
    	{
        	printf("Send Msg error!\n");
			close(fp);
       		goto Exit;
   		}
		goto ReciveFileName;
	}
	else
	{
		sprintf(Msg, FILEEXIST);
		if (sendto(sockfd, Msg, strlen(Msg), 0, servaddr,sizeof(struct sockaddr_in)) < 0)
    	{
        	printf("Send Msg error!\n");
			close(fp);
       		goto Exit;
   		}
		close(fp);
	}

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

	sleep(1);

	fp = open(FilePath, O_RDONLY);
	
	stat(FilePath, &st);
	FileSize = st.st_size;

	printf("The FileSize is : %lld", FileSize);
	if(sendto(sockfd, &FileSize, sizeof(FileSize), 0, servaddr,sizeof(struct sockaddr_in)) < 0)
	{
		printf("Send file size error!\n");
		goto Exit;
	}

	bzero(Buff, MAXNUM);
	Num = read(fp, Buff, MAXNUM);
	printf("Num = %d\n", Num);
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
	return;
}





