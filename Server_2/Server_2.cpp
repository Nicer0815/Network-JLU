#include<WinSock2.h>
#include <iostream>
using namespace std;
#pragma comment(lib,"ws2_32.lib")

DWORD WINAPI clientChildThread(LPVOID ipParameter) {
	SOCKET clientSocket = (SOCKET)ipParameter;

	int const CLIENT_MSG_SIZE = 128; //���ջ���������
	char inMSG[CLIENT_MSG_SIZE]; //������Ϣ��char����
	char outMSG[CLIENT_MSG_SIZE];//�洢ʱ���char����
	char wx[] = "���������루��֧�ֲ�ѯ��ǰʱ�䣩";

	int size;
	while (true) {
		memset(inMSG, 0, CLIENT_MSG_SIZE);//������Ϣ֮ǰ��ս�����Ϣ����
		size = recv(clientSocket, inMSG, CLIENT_MSG_SIZE, 0);//������Ϣ
		if (size == SOCKET_ERROR) {//���������Ϣ����
			cout << "�Ի��жϣ�������ʾ��" << WSAGetLastError() << endl;
			closesocket(clientSocket);
			break;
		}
		//���������Ϣ
		
		cout << "�ͻ�����Ϣ��" << inMSG << endl;
		//����ͻ�������ǰʱ��
		if (strcmp(inMSG, "��ǰʱ��") == 0) {
			SYSTEMTIME systime = { 0 };
			GetLocalTime(&systime);//��ȡϵͳʱ��
			sprintf(outMSG, "%d-%02d-%02d %02d:%02d:%02d",
				systime.wYear, systime.wMonth, systime.wDay,
				systime.wHour, systime.wMinute, systime.wSecond);
			send(clientSocket, outMSG, CLIENT_MSG_SIZE, 0);
			memset(outMSG, 0, CLIENT_MSG_SIZE);//ÿ�λظ�֮����շ�����Ϣ����
		}
		//����ͻ���Ҫ�˳�����
		else if (strcmp(inMSG, "�˳�����") == 0) {
			closesocket(clientSocket);
			cout << "�ͻ����˳����ӳɹ�" << endl;
			break;
		}
		else {
			send(clientSocket, wx, sizeof(wx), 0);
		}
	}

	return 0;
}



int main() {

	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {//����SOCKET�⣻
		WSACleanup();
		return  -1;
	}

	SOCKET serverSocket;
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		cout << "error:" << WSAGetLastError() << endl;
		WSACleanup();
		return -2;
	}

	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_port = htons(2590);
	server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(serverSocket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {//��server��serverSOCKET��
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return -3;
	}

	if (listen(serverSocket, 2) == SOCKET_ERROR) {//�����ͻ���
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return -4;
	}

	cout << "����������..." << endl;
	cout << "�ȴ��ͻ�������...." << endl;

	SOCKET clientSocket;//���ÿͻ������׽���
	SOCKADDR_IN client;
	int addrsize = sizeof(SOCKADDR_IN);
	HANDLE pThread;
	while (true) {

		clientSocket = accept(serverSocket, (struct sockaddr*)&client, &addrsize);//���տͻ�������

		if (clientSocket == INVALID_SOCKET) {
			cout << "�ͻ���acceptʧ��,������ʾ:" << WSAGetLastError() << endl;
			closesocket(serverSocket);
			WSACleanup();
			return -5;
		}
		else {
			cout << "�ͻ��ˣ� "<< inet_ntoa(client.sin_addr)//inet_ntoa��һ��ʮ���������ֽ���ת��Ϊ���ʮ����IP��ʽ���ַ�����
				<< "\nͨ���˿ڣ�"<< ntohs(client.sin_port)//ntohs��һ��16λ���������ֽ�˳��ת��Ϊ�����ֽ�˳��
				<< "\n���ӳɹ�\n" << endl;

			pThread = CreateThread(NULL, 0, clientChildThread, (LPVOID)clientSocket, 0, NULL);//�����߳�
			/*
			lpsa���߳̾���İ�ȫ�ԣ������ӽ����Ƿ���Լ̳�����߳̾����һ������ΪNULL
			cbStack���߳�ջ��С��һ��ȡ0��ʾĬ�ϴ�С
			lpStartAddr���߳���ں���
			lpvThreadParam���߳���ں����Ĳ���
			fdwCreate�������̴߳����ı�־��һ��Ϊ0����ʾ�߳�����������Ҳ����ѡ����Թ���ʹ��CREATE_SUSPENDED��֮���ڴ�����ʹ��ResumeThread������
			lpIDThread���̵߳�IDֵ�������̷߳��ص�ID
			*/
			if (pThread == NULL) {
				cout << "�����ӽ���ʧ�ܡ�" << endl;
				break;
			}

			CloseHandle(pThread);

		}
	}
	closesocket(serverSocket);
	closesocket(clientSocket);
	WSACleanup();
	return 0;

}