#include<WinSock2.h>
#include <iostream>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

int main() {

	WSADATA wsd;//����	WSADATA����
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {//��ʼ��WSA
		WSACleanup();
		return -1;
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET) {
		cout << "error:" << WSAGetLastError() << endl;
		WSACleanup();
		return -2;
	}

	SOCKADDR_IN client;
	client.sin_family = AF_INET;
	client.sin_port = htons(2590);
	client.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	int const SERVER_MSG_SIZE = 128;
	char inMSG[SERVER_MSG_SIZE] = { 0 };//�û��������Ϣ
	char outMSG[SERVER_MSG_SIZE];//Ҫ���͸�����������Ϣ

	//���ӷ�����ʧ��
	if (connect(clientSocket, (struct sockaddr*)&client, sizeof(client)) < 0) {
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(clientSocket);
		WSACleanup();
		return -3;
	}
	//���ӷ������ɹ�
	else {
		cout << "���������������������...\n" << endl;
		cout << "���ӷ������ɹ�\n" << endl;
		cout << "��ʾ:" << endl;
		cout << "     1.���롰��ǰʱ�䡱�����Բ�ѯ��ǰʱ��" << endl;
		cout << "     2.���롰�˳����ӡ��������˳��������������" << endl;
		while (true) {
			memset(outMSG, 0, SERVER_MSG_SIZE);
			cout << "����������...��" << endl;

			cin >> outMSG;
			send(clientSocket, outMSG, SERVER_MSG_SIZE, 0);
			if (strcmp(outMSG, "�˳�����") == 0) {
				break;
			}
			int size = recv(clientSocket, inMSG, SERVER_MSG_SIZE, 0);
			cout << "�������˻ش�" << endl;
			cout << "    ��ǰʱ��Ϊ��" << inMSG << endl;
			memset(inMSG, 0, SERVER_MSG_SIZE);
		}
	}

	closesocket(clientSocket);
	WSACleanup();

	system("pause");
	return 0;

}