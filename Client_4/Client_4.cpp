#include <WinSock2.h>
#include <windows.h>
#include "size.h"
#include <tchar.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

//��ȡ�ظ�����
void do_read_rspns(SOCKET fd, RspnsPacket* ptr)
{
	int count = 0;
	int size = sizeof(RspnsPacket);
	while (count < size)
	{
		int nRead = recv(fd, (char*)ptr + count, size - count, 0);
		if (nRead <= 0)
		{
			printf("��ȡ�������Ļظ�ʧ�ܣ�\n");
			closesocket(fd);
			exit(1);
		}
		count += nRead;
	}
}

//���������
void do_write_cmd(SOCKET fd, CmdPacket* ptr)
{
	int size = sizeof(CmdPacket);
	int flag = send(fd, (char*)ptr, size, 0);
	if (flag == SOCKET_ERROR)
	{
		printf("����������������ʧ�ܣ�\n");
		closesocket(fd);
		WSACleanup();
		exit(1);
	}
}

//�������������׽��ֲ���������״̬
SOCKET create_data_socket()
{
	SOCKET sockfd;
	struct sockaddr_in my_addr;
	//���������������ӵ��׽���
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("���������������ӵ��׽���ʧ�ܣ�\n");
		WSACleanup();
		exit(1);
	}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(DATA_PORT);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(&(my_addr.sin_zero), 0, sizeof(my_addr.sin_zero));

	//��
	if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		printf("�󶨵�ַʧ�ܣ�������룺%d\n", err);
		closesocket(sockfd);
		WSACleanup();
		exit(1);
	}

	//����������������
	if (listen(sockfd, 1) == SOCKET_ERROR)
	{
		printf("������������ʧ�ܣ�\n");
		closesocket(sockfd);
		WSACleanup();
		exit(1);
	}
	return sockfd;
}

//����list����
void list(SOCKET sockfd)
{
	int sin_size;
	int nRead;
	CmdPacket cmd_packet;
	SOCKET newsockfd, data_sockfd;
	struct sockaddr_in their_add;
	char data_buf[DATA_BUFSIZE];

	//������������
	newsockfd = create_data_socket();
	//��������Ĳ�������������
	cmd_packet.cmdid = LS;//û�в���
	do_write_cmd(sockfd, &cmd_packet);
	sin_size = sizeof(struct sockaddr_in);
	//���ܷ�������������������
	if ((data_sockfd = accept(newsockfd, (struct sockaddr*)&their_add, &sin_size)) == INVALID_SOCKET)
	{
		printf("��ȡ�ļ��б�ʧ�ܣ�\n");
		closesocket(newsockfd);
		closesocket(sockfd);
		WSACleanup();
		exit(1);
	}

	//ÿ�ζ����������ݾ���ʾ���٣�ֱ���������ӶϿ�
	while (true)
	{
		nRead = recv(data_sockfd, data_buf, DATA_BUFSIZE - 1, 0);
		if (nRead == SOCKET_ERROR)
		{
			printf("��ȡ�������ظ�ʧ�ܣ�\n");
			closesocket(data_sockfd);
			closesocket(newsockfd);
			closesocket(sockfd);
			WSACleanup();
			exit(1);
		}

		if (nRead == 0)//���ݶ�ȡ����
			break;

		//��ʾ����
		data_buf[nRead] = '\0';
		printf("%s", data_buf);

	}
	closesocket(data_sockfd);
	closesocket(newsockfd);
}
//����pwd���
void pwd(int sockfd)
{
	CmdPacket cmd_packet;
	RspnsPacket rspns_packet;

	cmd_packet.cmdid = PWD;
	//��������Ĳ���ȡ�ظ���
	do_write_cmd(sockfd, &cmd_packet);
	do_read_rspns(sockfd, &rspns_packet);
	printf("%s\n", rspns_packet.text);
}

//����cd����:
void cd(int sockfd)
{
	CmdPacket cmd_packet;
	RspnsPacket rspns_packet;


	cmd_packet.cmdid = CD;
	scanf("%s", cmd_packet.param);

	//��������Ĳ���ȡ�ظ���
	do_write_cmd(sockfd, &cmd_packet);
	do_read_rspns(sockfd, &rspns_packet);
	if (rspns_packet.rspnsid == ERR)
		printf("%s", rspns_packet.text);
}


//����down����������ļ���
void get_file(SOCKET sockfd)
{
	FILE* fd;
	char data_buf[DATA_BUFSIZE];

	CmdPacket cmd_packet;
	RspnsPacket rspns_packet;

	SOCKET newsockfd, data_sockfd;
	struct sockaddr_in their_addr;
	int sin_size;
	int count;

	//��������ģ�
	cmd_packet.cmdid = DOWN;
	scanf("%s", cmd_packet.param);

	//�򿪻��ߴ��������ļ��Թ�д���ݣ�
	fd = fopen(cmd_packet.param, "wb");//ʹ�ö����Ʒ���
	if (fd == NULL)
	{
		printf("���ļ�%s��д��ʧ�ܣ�\n", cmd_packet.param);
		return;
	}

	//�����������Ӳ���������������������
	newsockfd = create_data_socket();

	//���ͱ�������
	do_write_cmd(sockfd, &cmd_packet);

	//��ȡ�ظ����ģ�
	do_read_rspns(sockfd, &rspns_packet);
	if (rspns_packet.rspnsid == ERR)
	{
		printf("%s", rspns_packet.text);
		closesocket(newsockfd);

		fclose(fd);
		//ɾ���ļ�:
		DeleteFile(cmd_packet.param);
		return;
	}

	sin_size = sizeof(struct sockaddr_in);
	//�ȴ����ܷ���������������
	if ((data_sockfd = accept(newsockfd, (struct sockaddr*)&their_addr, &sin_size)) == INVALID_SOCKET)
	{
		printf("��ȡ�ļ�ʧ�ܣ�\n");
		closesocket(newsockfd);

		fclose(fd);
		//ɾ���ļ���
		DeleteFile(cmd_packet.param);
		return;
	}

	//ѭ����ȡ�������ݲ�д���ļ���
	while ((count = recv(data_sockfd, data_buf, DATA_BUFSIZE, 0)) > 0)
		fwrite(data_buf, sizeof(char), count, fd);

	closesocket(data_sockfd);
	closesocket(newsockfd);
	fclose(fd);
}

//����put������ϴ��ļ�
void put_file(SOCKET sockfd)
{
	FILE* fd;
	CmdPacket cmd_packet;
	RspnsPacket rspns_packet;
	char data_buf[DATA_BUFSIZE];

	SOCKET newsockfd, data_sockfd;
	struct sockaddr_in their_addr;
	int sin_size;
	int count;
	cmd_packet.cmdid = UP;
	scanf("%s", cmd_packet.param);

	//�򿪱����ļ����ڶ�ȡ����
	fd = fopen(cmd_packet.param, "rb");
	if (fd == NULL)
	{
		printf("���ļ�%s����ȡ����ʧ�ܣ�\n", cmd_packet.param);
		return;
	}

	//�������������׽��ֲ���������״̬��
	newsockfd = create_data_socket();

	//���������
	do_write_cmd(sockfd, &cmd_packet);

	//��ȡ�ظ�����
	do_read_rspns(sockfd, &rspns_packet);
	if (rspns_packet.rspnsid == ERR)
	{
		printf("%s", rspns_packet.text);
		closesocket(newsockfd);
		fclose(fd);
		return;
	}

	sin_size = sizeof(struct sockaddr_in);
	//׼��������������
	if ((data_sockfd = accept(newsockfd, (struct sockaddr*)&their_addr, &sin_size)) == INVALID_SOCKET)
	{
		printf("�ϴ��ļ��������\n");
		closesocket(newsockfd);
		fclose(fd);
		return;
	}
	//ѭ�����ļ��ж�ȡ���ݲ�����������
	while (true) {
		count = fread(data_buf, sizeof(char), DATA_BUFSIZE, fd);
		send(data_sockfd, data_buf, count, 0);
		if (count < DATA_BUFSIZE)//�����Ѿ�������߷���cuowu
			break;
	}

	closesocket(data_sockfd);
	closesocket(newsockfd);
	fclose(fd);
}

//�����˳�����
void quit(int sockfd)
{
	CmdPacket cmd_packet;
	RspnsPacket rspns_packet;

	cmd_packet.cmdid = QUIT;
	do_write_cmd(sockfd, &cmd_packet);
	do_read_rspns(sockfd, &rspns_packet);
	printf("%s", rspns_packet.text);
	getchar();
}

void main()
{
	SOCKET sockfd;
	struct sockaddr_in their_addr;
	char cmd[10];
	RspnsPacket rspns_packet;

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	//Winsock��ʼ��
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		printf("WinSock��ʼ��ʧ�ܣ�\n");
		return;
	}

	//ȷ��WindSock DLL�İ汾��2.2
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("WindSock�汾����2.2��\n");
		WSACleanup();
		return;
	}

	//�������ڿ����½��socket
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == INVALID_SOCKET)
	{
		printf("�����׽���ʧ�ܣ�\n");
		WSACleanup();
		exit(1);
	}
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(CMD_PORT);
	their_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	memset(&(their_addr.sin_zero), 0, sizeof(their_addr.sin_zero));

	//���ӷ�����
	if (connect(sockfd, (struct sockaddr*)&their_addr, sizeof(struct sockaddr)) == SOCKET_ERROR)
	{
		printf("���ӷ�����ʧ�ܣ�\n");
		closesocket(sockfd);
		WSACleanup();
		exit(1);
	}

	//���ӳɹ������Ƚ��ܷ��������ص���Ϣ
	do_read_rspns(sockfd, &rspns_packet);
	printf("%s", rspns_packet.text);

	//��ѭ������ȡ�û����벢����ִ��
	while (true)
	{
		scanf("%s", cmd);
		switch (cmd[0])
		{
		case 'l'://����List����
			list(sockfd);
			break;
		case 'p'://����pwd����
			pwd(sockfd);
			break;
		case 'c'://����cd����
			cd(sockfd);
			break;
		case 'd'://����down����
			get_file(sockfd);
			break;
		case 'u'://����up����
			put_file(sockfd);
			break;
		case 'q'://����quit����
			quit(sockfd);
			break;
		default:
			printf("�����ڵ����\n");
			break;
		}
		if (cmd[0] == 'q')
			break;
	}
	WSACleanup();
}