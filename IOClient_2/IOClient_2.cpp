#include <WINSOCK2.h>
#include <iostream>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
int main(int argc, char* argv[])
{
	int judge = 1;
	WORD sockVersion = MAKEWORD(1, 1);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}

	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		cout << "套接字创建错误!";
		return 0;
	}
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (connect(sclient, (sockaddr*)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		cout << "连接错误!";
		closesocket(sclient);
		return 0;
	}


	cout << "与服务器连接成功\n";
	cout << "提示:\n";
	cout << "     输入“断开连接”，可以断开与服务器的连接:\n";
	//发送打招呼的数据 
	char* sendData = "你好,服务器.....";

	send(sclient, sendData, strlen(sendData), 0);

	char close[11] = "断开连接";
	while (judge == 1) {
		
		char recData[255];
		int ret = recv(sclient, recData, 255, 0);
		if (ret > 0) {
			recData[ret] = 0x00;  //转化为16进制
			cout << "服务器消息：\n";
			cout <<"           "<< recData;
		}
		
		char input[265];
		cout << "\n";
		cout << "\n";
		cout << "请输入数据:\n";
		
		gets_s(input);
		if (strcmp(input, close) == 0) {
			judge = 0;
		}
		else {

			char sendData[1000] = "客户端消息:";
			strcat(sendData, input);
			send(sclient, sendData, strlen(sendData), 0);
		}
	}
	closesocket(sclient);
	WSACleanup();
	return 0;
}