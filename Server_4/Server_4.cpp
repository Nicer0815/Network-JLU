#include <WinSock2.h>
#include "size.h"
#include <iostream>
#include<time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
//�����߳�ʱ���ݵ����ݽṹ,�ں����������׽��ֺͿͻ��˵�ַ��Ϣ:
struct threadData {
	SOCKET tcps;
	sockaddr_in clientaddr;
};

//ȫ�ֺ�������:
//FTP��ʼ��,����һ�������׽���:
int InitFTP(SOCKET* pListenSock);
int InitDataSocket(SOCKET* pDatatcps, SOCKADDR_IN* pClientAddr);
int ProcessCmd(SOCKET tcps, CmdPacket* pCmd, SOCKADDR_IN* pClientAddr);
int SendRspns(SOCKET tcps, RspnsPacket* prspns);
int RecvCmd(SOCKET tcps, char* pCmd);
int SendFileList(SOCKET datatcps);
int SendFileRecord(SOCKET datatcps, WIN32_FIND_DATA* pfd);
int SendFile(SOCKET datatcps, FILE* file);
int RecvFile(SOCKET datatcps, char* filename);
int FileExists(const char* filename);

//�̺߳���,����������Ӧ�������ӵ��׽���:
DWORD WINAPI ThreadFunc(LPVOID lpParam) {
	SOCKET tcps;
	sockaddr_in clientaddr;
	tcps = ((struct threadData*)lpParam)->tcps;
	clientaddr = ((struct threadData*)lpParam)->clientaddr;
	printf("socket�ı���ǣ�%u.\n", tcps);

	//���ͻظ����ĸ��ͻ���,�ں�����ʹ��˵��:
	printf("Serve client %s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	char path[_MAX_PATH];
	_getcwd(path, _MAX_PATH); //��ǰ��������·�����Ƶ�buf
	printf("��������ǰĿ¼:\n	%s\n", path);
	
	RspnsPacket rspns = { OK,
			"��ӭ����FTP�ۺ�Ӧ��ϵͳ!\n"
			"�����ʹ�õ�����:\n"
			"ls\t<չʾ��ǰĿ¼�µ��ļ�(��)���������>\n"
			"pwd\t<չʾ��ǰĿ¼�ľ���·�����������>\n"
			"cd\t<�л���ָ��Ŀ¼������Ϊ·��>\n"
			"down\t<�����ļ�������Ϊ�ļ���>\n"
			"up\t<�ϴ��ļ�������Ϊ�ļ���>\n"		  
			"quit\t<�˳�ϵͳ���������>\n"
	};

	SendRspns(tcps, &rspns);

	//ѭ����ȡ�ͻ�������Ĳ����д���
	for (;;) {
		CmdPacket cmd;
		if (!RecvCmd(tcps, (char*)&cmd))
			break;
		if (!ProcessCmd(tcps, &cmd, &clientaddr))
			break;
	}

	//�߳̽���ǰ�رտ��������׽���:
	closesocket(tcps);
	delete lpParam;
	return 0;
}

int main(int argc, char* argv[]) {
	SOCKET tcps_listen;  //FTP�������������������׽���
	struct threadData* pThInfo;

	if (!InitFTP(&tcps_listen))  //FTP��ʼ��
		return 0;
	printf("FTP��������ʼ�������˿ں�Ϊ��%d������������\n", CMD_PORT);

	//ѭ�����ܿͻ�����������,�������߳�ȥ����:
	for (;;) {
		pThInfo = NULL;
		pThInfo = new threadData;
		if (pThInfo == NULL) {
			printf("Ϊ���߳�����ռ�ʧ�ܡ�\n");
			continue;
		}

		int len = sizeof(struct threadData);
		//�ȴ����ܿͻ��˿�����������
		pThInfo->tcps = accept(tcps_listen, (SOCKADDR*)&pThInfo->clientaddr, &len);

		//����һ���߳���������Ӧ�ͻ��˵�����:
		DWORD dwThreadId, dwThrdParam = 1;
		HANDLE hThread;

		hThread = CreateThread(
			NULL,               //���谲ȫ�Եļ̳�
			0,					//Ĭ���߳�ջ��С
			ThreadFunc,			//�߳���ں���
			pThInfo,			//�߳���ں����Ĳ���
			0,					//���������߳�
			&dwThreadId);		//�����̵߳�idֵ

		//��鷵��ֵ�Ƿ񴴽��̳߳ɹ�
		if (hThread == NULL) {
			printf("�����߳�ʧ�ܡ�\n");
			closesocket(pThInfo->tcps);
			delete pThInfo;
		}
	}

	return 0;
}

//FTP��ʼ��,����һ�������׽���:
int InitFTP(SOCKET* pListenSock) {
	//���մ˲��贴���µķ��������׽��֣��ţ�û��ǰ���������������
	//startup->socket->bind->listen
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	SOCKET tcps_listen;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		printf("Winsock��ʼ��ʱ��������!\n");
		return 0;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		printf("��ЧWinsock�汾!\n");
		return 0;
	}

	tcps_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//TCP��ʽ�׽�������
	if (tcps_listen == INVALID_SOCKET) {
		WSACleanup();
		printf("����Socketʧ��!\n");
		return 0;
	}

	SOCKADDR_IN tcpaddr;
	tcpaddr.sin_family = AF_INET;
	tcpaddr.sin_port = htons(CMD_PORT);
	tcpaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	err = bind(tcps_listen, (SOCKADDR*)&tcpaddr, sizeof(tcpaddr));

	if (err != 0) {
		err = WSAGetLastError();
		WSACleanup();
		printf("Scoket��ʱ��������!\n");
		return 0;
	}
	err = listen(tcps_listen, 3);
	if (err != 0) {
		WSACleanup();
		printf("Scoket����ʱ��������!\n");
		return 0;
	}

	*pListenSock = tcps_listen;
	return 1;
}

//������������
//pDatatcps:���ڴ洢���������׽���
//pClientAddr:ָ��ͻ��˵Ŀ��������׽��ֵ�ַ,��Ҫʹ�����е�IP��ַ
//����ֵ:0��ʾʧ��,1����
int InitDataSocket(SOCKET* pDatatcps, SOCKADDR_IN* pClientAddr) {
	SOCKET datatcps;

	//����socket
	datatcps = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (datatcps == INVALID_SOCKET) {
		printf("Creating data socket failed!\n");
		return 0;
	}

	SOCKADDR_IN tcpaddr;
	memcpy(&tcpaddr, pClientAddr, sizeof(SOCKADDR_IN));
	tcpaddr.sin_port = htons(DATA_PORT);    //������ʲô����ֻ��Ҫ��ͷ�ļ��޸Ķ˿�ֵ

	//�������ӿͻ���
	if (connect(datatcps, (SOCKADDR*)&tcpaddr, sizeof(tcpaddr)) == SOCKET_ERROR) {
		printf("Connecting to client failed!\n");
		closesocket(datatcps);
		return 0;
	}

	*pDatatcps = datatcps;
	return 1;
}

//���������
//tcps:���������׽���
//pcmd:ָ�������������
//pClientAddr:ָ��ͻ��˿��������׽��ֵ�ַ
//����ֵ:0��ʾ�д������Ҫ��������,1����
int ProcessCmd(SOCKET tcps, CmdPacket* pCmd, SOCKADDR_IN* pClientAddr) {
	SOCKET datatcps;   //���������׽���
	RspnsPacket rspns;  //�ظ�����
	FILE* file;

	//�����������ͷ���ִ��:
	switch (pCmd->cmdid) {
	case LS://չʾ��ǰĿ¼�µ��ļ��б�
		//���Ƚ�����������:
		if (!InitDataSocket(&datatcps, pClientAddr))
			return 0;
		//�����ļ��б���Ϣ:
		if (!SendFileList(datatcps))
			return 0;
		break;
	case PWD://չʾ��ǰĿ¼�ľ���·��
		rspns.rspnsid = OK;
		//��ȡ��ǰĿ¼,�������ظ�������
		if (!GetCurrentDirectory(RSPNS_TEXT_SIZE, rspns.text))
			strcpy(rspns.text, "�޷���ȡ��ǰĿ¼!\n");
		if (!SendRspns(tcps, &rspns))
			return 0;
		break;
	case CD://���õ�ǰĿ¼,ʹ��win32 API �ӿں���
		if (SetCurrentDirectory(pCmd->param)) {
			rspns.rspnsid = OK;
			if (!GetCurrentDirectory(RSPNS_TEXT_SIZE, rspns.text))
				strcpy(rspns.text, "�л���ǰĿ¼�ɹ������ǲ��ܻ�ȡ����ǰ���ļ��б�\n");
		}
		else {
			strcpy(rspns.text, "���ܸ�������ѡĿ¼��\n");
		}
		if (!SendRspns(tcps, &rspns))   //���ͻظ�����
			return 0;
		break;
	case DOWN://���������ļ�����:
		file = fopen(pCmd->param, "rb");   //��Ҫ���ص��ļ�
		if (file) {
			rspns.rspnsid = OK;
			sprintf(rspns.text, "�����ļ�%s\n", pCmd->param);
			if (!SendRspns(tcps, &rspns)) {
				fclose(file);

				return 0;
			}
			else {
				//���������������������������:
				if (!InitDataSocket(&datatcps, pClientAddr)) {
					fclose(file);
					return 0;
				}
				if (!SendFile(datatcps, file))
					return 0;
				fclose(file);
			}
		}
		else  //���ļ�ʧ��
		{
			rspns.rspnsid = ERR;
			strcpy(rspns.text, "���ܴ��ļ���\n");
			if (!SendRspns(tcps, &rspns))
				return 0;
		}
		break;
	case UP://�����ϴ��ļ�����
		//���ȷ��ͻظ�����
		char filename[64];
		strcpy(filename, pCmd->param);
		//���ȿ�һ�·��������Ƿ��Ѿ�������ļ������о͸��߿ͻ��˲��ô�����
		if (FileExists(filename)) {
			rspns.rspnsid = ERR;
			sprintf(rspns.text, "�������Ѿ���������Ϊ%s���ļ���\n", filename);
			if (!SendRspns(tcps, &rspns))
				return 0;
		}
		else {
			rspns.rspnsid = OK;
			if (!SendRspns(tcps, &rspns))
				return 0;
			//����һ��������������������:
			if (!InitDataSocket(&datatcps, pClientAddr))
				return 0;
			if (!RecvFile(datatcps, filename))
				return 0;
		}
		break;
	case QUIT:
		printf("�ͻ��˶Ͽ����ӡ�\n");
		rspns.rspnsid = OK;
		strcpy(rspns.text, "��ӭ�ٴι��٣�\n");
		SendRspns(tcps, &rspns);
		return 0;


	}

	return 1;

}

//���ͻظ�����
int SendRspns(SOCKET tcps, RspnsPacket* prspns) {
	if (send(tcps, (char*)prspns, sizeof(RspnsPacket), 0) == SOCKET_ERROR) {
		printf("��ͻ���ʧȥ���ӡ�\n");
		return 0;
	}
	return 1;
}

//���������
//tcps:���������׽���
//pCmd:���ڴ洢���ص������
//����ֵ:0��ʾ�д���������Ѿ��Ͽ�,1��ʾ����
int RecvCmd(SOCKET tcps, char* pCmd) {					//used to receive command from client
	int nRet;
	int left = sizeof(CmdPacket);

	//�ӿ��������ж�ȡ����,��СΪ sizeof(CmdPacket):
	while (left) {
		nRet = recv(tcps, pCmd, left, 0);
		if (nRet == SOCKET_ERROR) {
			printf("�ӿͻ��˽�������ʱ����δ֪����\n");
			return 0;
		}
		if (!nRet) {
			printf("�ͻ��˹ر������ӣ�\n");
			return 0;
		}

		left -= nRet;
		pCmd += nRet;
	}
	return 1;   //�ɹ���ȡ�����
}


//����һ���ļ���Ϣ:
int SendFileRecord(SOCKET datatcps, WIN32_FIND_DATA* pfd) {                    //used to send response to client
	char filerecord[MAX_PATH + 32];
	FILETIME ft;
	FileTimeToLocalFileTime(&pfd->ftLastWriteTime, &ft);
	SYSTEMTIME lastwtime;
	FileTimeToSystemTime(&ft, &lastwtime);
	char* dir = (char*)(pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? "<DIR>" : "");
	sprintf(filerecord, "%04d-%02d-%02d%02d:%02d   %5s   %10d   %-20s\n",
		lastwtime.wYear,
		lastwtime.wMonth,
		lastwtime.wDay,
		lastwtime.wHour,
		lastwtime.wMinute,
		dir,
		pfd->nFileSizeLow,
		pfd->cFileName);
	if (send(datatcps, filerecord, strlen(filerecord), 0) == SOCKET_ERROR) {
		printf("�����ļ��б�ʱ����δ֪����\n");
		return 0;

	}
	return 1;
}


//�����ļ��б���Ϣ
//datatcps:���������׽���
//����ֵ:0��ʾ����,1��ʾ����
int SendFileList(SOCKET datatcps) {
	HANDLE hff;
	WIN32_FIND_DATA fd;

	//�����ļ�
	hff = FindFirstFile("*", &fd);
	if (hff == INVALID_HANDLE_VALUE)  //��������
	{
		const char* errstr = "�����г��ļ���\n";
		printf("�ļ��б����ʧ�ܣ�\n");
		if (send(datatcps, errstr, strlen(errstr), 0) == SOCKET_ERROR) {
			printf("���͸��ļ��б�ʱ����δ֪����\n");
		}
		closesocket(datatcps);			return 0;
	}

	BOOL fMoreFiles = TRUE;
	while (fMoreFiles) {
		//���ʹ����ļ���Ϣ:
		if (!SendFileRecord(datatcps, &fd)) {
			closesocket(datatcps);
			return 0;
		}
		//������һ���ļ�
		fMoreFiles = FindNextFile(hff, &fd);
	}
	closesocket(datatcps);
	return 1;
}

//ͨ���������ӷ����ļ�
int SendFile(SOCKET datatcps, FILE* file) {
	char buf[1024];
	printf("�����ļ�������.../n");
	for (;;) {				//���ļ���ѭ����ȡ���ݲ����Ϳͻ���
		int r = fread(buf, 1, 1024, file);
		if (send(datatcps, buf, r, 0) == SOCKET_ERROR) {
			printf("��ͻ���ʧȥ���ӣ�\n");
			closesocket(datatcps);
			return 0;
		}
		if (r < 1024)   //�ļ��������
		{
			break;
		}
	}
	closesocket(datatcps);
	printf("��ɴ���!\n");
	return 1;
}

//�����ļ�
//datatcps:���������׽���,ͨ��������������
//filename:���ڴ�����ݵ��ļ���
int RecvFile(SOCKET datatcps, char* filename) {
	char buf[1024];
	FILE* file = fopen(filename, "wb");
	if (!file) {
		printf("д���ļ�ʱ����δ֪����\n");
		fclose(file);
		closesocket(datatcps);
		return 0;
	}
	printf("�����ļ������С�����������");
	while (1) {
		int r = recv(datatcps, buf, 1024, 0);
		if (r == SOCKET_ERROR) {
			printf("�ӿͻ��˽����ļ�ʱ����δ֪����\n");
			fclose(file);
			closesocket(datatcps);
			return 0;
		}
		if (!r) {
			break;
		}
		fwrite(buf, 1, r, file);
	}
	fclose(file);
	closesocket(datatcps);
	printf("��ɴ���!\n");
	return 1;
}

//����ļ��Ƿ����:
int FileExists(const char* filename)
{
	WIN32_FIND_DATA fd;
	if (FindFirstFile(filename, &fd) == INVALID_HANDLE_VALUE)
		return 0;
	return 1;
}

