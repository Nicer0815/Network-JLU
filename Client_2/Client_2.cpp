#include<WinSock2.h>
#include <iostream>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

int main() {

	WSADATA wsd;//定义	WSADATA对象
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {//初始化WSA
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
	char inMSG[SERVER_MSG_SIZE] = { 0 };//用户输入的消息
	char outMSG[SERVER_MSG_SIZE];//要发送给服务器的消息

	//连接服务器失败
	if (connect(clientSocket, (struct sockaddr*)&client, sizeof(client)) < 0) {
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(clientSocket);
		WSACleanup();
		return -3;
	}
	//连接服务器成功
	else {
		cout << "正在请求与服务器的连接...\n" << endl;
		cout << "连接服务器成功\n" << endl;
		cout << "提示:" << endl;
		cout << "     1.输入“当前时间”，可以查询当前时间" << endl;
		cout << "     2.输入“退出连接”，可以退出与服务器的连接" << endl;
		while (true) {
			memset(outMSG, 0, SERVER_MSG_SIZE);
			cout << "请输入请求...：" << endl;

			cin >> outMSG;
			send(clientSocket, outMSG, SERVER_MSG_SIZE, 0);
			if (strcmp(outMSG, "退出连接") == 0) {
				break;
			}
			int size = recv(clientSocket, inMSG, SERVER_MSG_SIZE, 0);
			cout << "服务器端回答：" << endl;
			cout << "    当前时间为：" << inMSG << endl;
			memset(inMSG, 0, SERVER_MSG_SIZE);
		}
	}

	closesocket(clientSocket);
	WSACleanup();

	system("pause");
	return 0;

}