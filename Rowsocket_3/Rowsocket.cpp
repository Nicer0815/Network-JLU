#include <WinSock2.h>
#include <iostream>
//#include "headers.h"
#pragma once

//IP报文格式
typedef struct IP {
	unsigned char headLen;//4位首部长度
	unsigned char serviceType;//8位服务类型
	unsigned short totalLen;//16位总长度
	unsigned short identifier;//16位标识符
	unsigned short flags;//3位标志位
	unsigned char timeToLive;//8位生存时间
	unsigned char protocal;//8位协议
	unsigned short headCheckSum;//16位首部校验和
	unsigned int sourceAddr;//32位源地址
	unsigned int destinAddr;//32位目的地址
}IPHeader;

//TCP报文格式
typedef struct TCP {
	unsigned short sourcePort;//16位源端口号
	unsigned short destinPort;//16位目的端口号
	unsigned int seqNum;//32位序列号
	unsigned int ackNum;//32位确认号
	unsigned char headLen;//4位首部长度
	unsigned char flags;//8位标志位
	unsigned short winSize;//16位窗口大小
	unsigned short checkNum;//16位校验和
	unsigned short urgPointer;//16位紧急指针
}TCPHeader;

using namespace std;
#pragma comment(lib,"ws2_32.lib")
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)//将SIO_RCVALL定义为_WSAIOW(IOC_VENDOR,1)

int main() {

	IP* ip;
	TCP* tcp;

	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
		cout << "error:" << WSAGetLastError() << endl;
		return -1;
	}

	SOCKET sock;
	sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (sock == INVALID_SOCKET) {
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return -2;
	}
	BOOL flag = true;
	if (setsockopt(sock, IPPROTO_IP, 2, (char*)&flag, sizeof(flag)) == SOCKET_ERROR) {
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return -3;
	}

	/*
	创建了原始套接字后，就要设置套接字选项，这要通过setsocketopt函数来实现，setsocketopt函数的声明如下：
	int setsocketopt (SOCKET s,int level,int optname,const char FAR *optval,int optlen )；
	参数s是标识套接口的描述字，要注意的是选项对这个套接字必须是有效的。
	参数Level表明选项定义的层次，对TCP/IP协议族而言，支持SOL_SOCKET、IPPROTO_IP和IPPROTO_TCP层次。
	参数Optname是需要设置的选项名，这些选项名是在Winsock头文件内定义的常数值。
	参数optval是一个指针，它指向存放选项值的缓冲区。
	参数optlen指示optval缓冲区的长度
	*/
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(0);
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return -4;
	}

	DWORD dwBytesReturned;
	DWORD dwBufferInLen = 1;
	//将网卡设置为混听模式，就是接收所有数据
	if (ioctlsocket(sock, SIO_RCVALL, &dwBufferInLen) == SOCKET_ERROR) {
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return -5;
	}
	/*
	ioctsocket功能是控制套接口的模式。可用于任一状态的任一套接口。它用于获取与套接口相关的操作参数，而与具体协议或通讯子系统无关。
	int ioctlsocket( int s, long cmd, u_long * argp);
	s：一个标识套接口的描述字。
	cmd：对套接口s的操作命令。
	argp：指向cmd命令所带参数的指针。
	*/

	int bytesRecv;
	char buffer[65535];//接收缓冲区的内容
	//SOCKADDR_IN from;
	struct sockaddr_in from;
	int fromSize = sizeof(from);
	//循环监听;
	while (true) {
		memset(buffer, 0, 65535);
		bytesRecv = recvfrom(sock, buffer, 65535, 0, (struct sockaddr*)&from, &fromSize);
		if (bytesRecv == SOCKET_ERROR) {
			cout << "error:" << WSAGetLastError() << endl;
			closesocket(sock);
			WSACleanup();
			return -6;
		}
		/*
		与recv的功能差不多，都是接收数据，但是from可以适用于UDP，因为多了一个from，你懂的。
		*/

		ip = (struct IP*)buffer;

		if (ip->protocal == 6) {//过滤其他协议，只留下TCP协议
			//tcp = (struct TCP*)(buffer + (4 * ip->headLen & 0xf0 ));//得到TCP头
			//tcp = (struct TCP*)(buffer + (4 * ip->headLen & 0xf0 >> 4));//得到TCP头
			tcp = (struct TCP*)(buffer + (4 * (ip->headLen & 0x0f)));
			//tcp = (struct TCP*)(buffer + (4 * ip->headLen));
		
			cout << "Network+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";//网络层数据
			cout << "IP报文字节数：" << bytesRecv << "\n";
			cout << "源IP：" << inet_ntoa(*(in_addr*)&ip->sourceAddr) << "\n";
			cout << "目的IP：" << inet_ntoa(*(in_addr*)&ip->destinAddr) << "\n";
			cout << "Transportation++++++++++++++++++++++++++++++++++++++++++++++++++++\n";//运输层数据
			cout << "源端口：" << ntohs(tcp->sourcePort) << "\n";
			cout << "目的端口：" << ntohs(tcp->destinPort) << "\n";
			cout << "Applications++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";//应用层数据

			//char* start = buffer + 5 + 4 * ((tcp->headLen & 0xf0) >> 4 | 0);//计算数据头指针，从何处开始数据

			char* start = buffer + (4 * (ip->headLen & 0x0f)) + 4 * ((tcp->headLen & 0xf0) >> 4);

			int dataSize = bytesRecv - 4 * (ip->headLen & 0x0f) - 4 * ((tcp->headLen & 0xf0) >> 4 | 0);
			//int dataSize = bytesRecv - 5 - 4 * ((tcp->headLen & 0xf0) >> 4 | 0);//计算数据长度
			
			cout << "数据内容：";
			memcpy(buffer, start, dataSize);
			for (int i = 0; i < dataSize; i++) {
				
					cout<<(unsigned char)buffer[i];
				
			
			}
			cout << "\n";
		}
	}

}