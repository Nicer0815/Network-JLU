
#include<WinSock2.h>//Windows socket的头文件
#include <iostream>
using namespace std;

#pragma comment (lib,"ws2_32.lib")//接下来要使用的一些API函数，需要加载这个库

int main() {

	WSADATA wsd;//定义WSADATA对象，调用WSASTART后返回数据给这个对象
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {//和相应的SOCKET库进行绑定，等于0说明成功绑定
		WSACleanup();//解除与Socket库的绑定并且释放Socket库所占用的系统资源。
		return -1;
	}

	SOCKET serverSocket;//定义服务器套接字
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//初始化套接字（地址族（Address Family），IPV4，套接口所用协议（TCP））
	if (serverSocket == INVALID_SOCKET) {//等于INVALID_SOCKET表示创建失败
		cout << "error:" + WSAGetLastError() << endl;//WSAGetLastError() 返回上次发生的网络错误
		WSACleanup();
		return -2;
	}

	SOCKADDR_IN server;//用于建立serverSocket的本地关联的结构，也就是存储网络地址的数据结构
	server.sin_family = AF_INET;//IPV4
	server.sin_port = htons(2589);//端口
	server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//地址，INADDR_ANY对所有有效IP都适用

	if (bind(serverSocket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return  -3;
	}

	/*
	作用bind函数通过安排一个本地名称到未命名的socket而建立此socket的本地关联。
	将本机IP地址，选定的一个端口与前面的套接字绑定即：将serverSocket和server绑定。
	*/

	if (listen(serverSocket, 2) == SOCKET_ERROR) {//同时最多有两个客户机连接
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return -4;
	}

	/*
	作用：设置套接字进入监听状态。
	用法：套接字，监听队列中允许保持的尚未处理的最大连接数量
	特性：函数执行成功后，套接字s进入了被动模式，到来的连接会被通知要排队等候接受处理。
	*/

	SOCKET clientSocket;//定义接收客户端的套接字
	SOCKADDR_IN client;//
	int addrSize = sizeof(SOCKADDR_IN);
	int const CLIENT_MSG_SIZE = 128;//接收缓冲区长度
	char inMSG[CLIENT_MSG_SIZE];//来自于客户端的消息
	char outMSG[CLIENT_MSG_SIZE];//发送给客户端的消息
	char wx[] = "本服务器仅支持查询当前时间！！！";//由于实验需要，方便一点
	int size;//接收消息是否失败
	while (true) {//循环等待连接
		cout << "\n等待客户端连接。。。" << endl;
		clientSocket = accept(serverSocket, (struct sockaddr*)&client, &addrSize);
		//accept:成功返回一个新的socket文件描述符，用于和客户端通信，失败返回-1
		if (clientSocket == INVALID_SOCKET) {//连接失败
			cout << "客户端accept失败，错误提示：" << WSAGetLastError() << endl;
			closesocket(serverSocket);
			WSACleanup();
			return -5;
		}
		else {//连接成功
			cout << "客户端： "
				<< inet_ntoa(client.sin_addr)//inet_ntoa将一个十进制网络字节序转换为点分十进制IP格式的字符串。
				<< "\n通过端口："
				<< ntohs(client.sin_port)//ntohs将一个16位数由网络字节顺序转换为主机字节顺序
				<< "\n连接成功" << endl;

			//一直不断地接收消息，直到客户端选择退出
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

			closesocket(clientSocket);

		}
	}

	closesocket(serverSocket);
	WSACleanup();

	return 0;

}

