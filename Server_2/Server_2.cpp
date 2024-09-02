#include<WinSock2.h>
#include <iostream>
using namespace std;
#pragma comment(lib,"ws2_32.lib")

DWORD WINAPI clientChildThread(LPVOID ipParameter) {
	SOCKET clientSocket = (SOCKET)ipParameter;

	int const CLIENT_MSG_SIZE = 128; //接收缓冲区长度
	char inMSG[CLIENT_MSG_SIZE]; //接收信息的char数组
	char outMSG[CLIENT_MSG_SIZE];//存储时间的char数组
	char wx[] = "请重新输入（仅支持查询当前时间）";

	int size;
	while (true) {
		memset(inMSG, 0, CLIENT_MSG_SIZE);//接收消息之前清空接收消息数组
		size = recv(clientSocket, inMSG, CLIENT_MSG_SIZE, 0);//接收消息
		if (size == SOCKET_ERROR) {//如果接收消息出错
			cout << "对话中断，错误提示：" << WSAGetLastError() << endl;
			closesocket(clientSocket);
			break;
		}
		//否则，输出消息
		
		cout << "客户端消息：" << inMSG << endl;
		//如果客户端请求当前时间
		if (strcmp(inMSG, "当前时间") == 0) {
			SYSTEMTIME systime = { 0 };
			GetLocalTime(&systime);//获取系统时间
			sprintf(outMSG, "%d-%02d-%02d %02d:%02d:%02d",
				systime.wYear, systime.wMonth, systime.wDay,
				systime.wHour, systime.wMinute, systime.wSecond);
			send(clientSocket, outMSG, CLIENT_MSG_SIZE, 0);
			memset(outMSG, 0, CLIENT_MSG_SIZE);//每次回复之后，清空发送消息数组
		}
		//如果客户端要退出连接
		else if (strcmp(inMSG, "退出连接") == 0) {
			closesocket(clientSocket);
			cout << "客户端退出连接成功" << endl;
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
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {//加载SOCKET库；
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

	if (bind(serverSocket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {//将server与serverSOCKET绑定
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return -3;
	}

	if (listen(serverSocket, 2) == SOCKET_ERROR) {//监听客户机
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return -4;
	}

	cout << "服务器启动..." << endl;
	cout << "等待客户端连接...." << endl;

	SOCKET clientSocket;//设置客户机的套接字
	SOCKADDR_IN client;
	int addrsize = sizeof(SOCKADDR_IN);
	HANDLE pThread;
	while (true) {

		clientSocket = accept(serverSocket, (struct sockaddr*)&client, &addrsize);//接收客户机请求

		if (clientSocket == INVALID_SOCKET) {
			cout << "客户端accept失败,错误提示:" << WSAGetLastError() << endl;
			closesocket(serverSocket);
			WSACleanup();
			return -5;
		}
		else {
			cout << "客户端： "<< inet_ntoa(client.sin_addr)//inet_ntoa将一个十进制网络字节序转换为点分十进制IP格式的字符串。
				<< "\n通过端口："<< ntohs(client.sin_port)//ntohs将一个16位数由网络字节顺序转换为主机字节顺序
				<< "\n连接成功\n" << endl;

			pThread = CreateThread(NULL, 0, clientChildThread, (LPVOID)clientSocket, 0, NULL);//进入线程
			/*
			lpsa：线程句柄的安全性，比如子进程是否可以继承这个线程句柄，一般设置为NULL
			cbStack：线程栈大小，一般取0表示默认大小
			lpStartAddr：线程入口函数
			lpvThreadParam：线程入口函数的参数
			fdwCreate：控制线程创建的标志，一般为0，表示线程立即启动。也可以选择可以挂起，使用CREATE_SUSPENDED，之后在代码中使用ResumeThread启动。
			lpIDThread：线程的ID值，接收线程返回的ID
			*/
			if (pThread == NULL) {
				cout << "创建子进程失败。" << endl;
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