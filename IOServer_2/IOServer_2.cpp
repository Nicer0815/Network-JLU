#include<stdio.h>
#include<time.h>
#include <stdlib.h>
#include<winsock2.h>
#include<iostream>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
int main()
{
	// 加载win socket   
	WSADATA ws;
	int ret;
	ret = WSAStartup(MAKEWORD(1, 1), &ws);//加载socket库
	if (ret != 0)
	{
		cout<<"加载套接字库失败!\n";
		return -1;
	}
	
	SOCKET sListen;// 创建侦听SOCKET   
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == INVALID_SOCKET)   //无效嵌套字
	{
		cout << "套接字创建失败!\n";
		return -1;
	}
	
	sockaddr_in servAddr;// 填充服务器地址结构   
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(8888);

	 
	ret = bind(sListen, (sockaddr*)&servAddr, sizeof(servAddr));// 绑定服务器套接字  
	if (ret == SOCKET_ERROR)
	{
		cout << "绑定失败!\n";
		return -1;
	}

	ret = listen(sListen, 5); //开始侦听，缓冲区长度为5  
	if (ret == SOCKET_ERROR)
	{
		cout << "创建监听失败!\n";
		return -1;
	}
	cout << "服务器启动成功，在端口"<< ntohs(servAddr.sin_port)<<" 监听…\n";//ntohs显示出当前服务器端口号
	//使用 模型   
	// 创建套接字集合   
	fd_set allSockSet; // 总的套接字集合   
	fd_set readSet; // 可读套接字集合   
	fd_set writeSet; // 可写套接字集合  

	FD_ZERO(&allSockSet); // 清空套接字集合   
	FD_SET(sListen, &allSockSet); // 将sListen套接字加入套接字集合中   
	char bufRecv[100]; // 接收缓冲区   
	// 进入服务器主循环   
	while (1)
	{
		FD_ZERO(&readSet); // 清空可读套接字   
		FD_ZERO(&writeSet); // 清空可写套接字   
		readSet = allSockSet; // 赋值   
		writeSet = allSockSet; // 赋值   
		// 调用select函数，timeout设置为NULL   
		ret = select(0, &readSet, 0, NULL, NULL);
		//   
		if (ret == SOCKET_ERROR)
		{
			cout << "select方法失败!\n";
			return -1;
		}
		// 存在套接字的I/O已经准备好   
		if (ret > 0)
		{
			// 遍历所有套接字

			for (int i = 0; i < allSockSet.fd_count; i++)
			{
				SOCKET s = allSockSet.fd_array[i];
				// 存在可读的套接字   
				if (FD_ISSET(s, &readSet))
				{
					// 可读套接字为sListen   
					if (s == sListen)
					{
						// 接收新的连接   
						sockaddr_in clientAddr;
						int len = sizeof(clientAddr);
						SOCKET sClient = accept(s, (sockaddr*)&clientAddr, &len);
						// 将新创建的套接字加入到集合中   
						FD_SET(sClient, &allSockSet);
						cout << "创建新的链接\n";
						cout << "客户端： "
							<< inet_ntoa(clientAddr.sin_addr)//inet_ntoa将一个十进制网络字节序转换为点分十进制IP格式的字符串。
							<< "\n通过端口："
							<< ntohs(clientAddr.sin_port)//ntohs将一个16位数由网络字节顺序转换为主机字节顺序
							<< "\n连接成功" << endl;
					
						cout <<"目前客户端数目为:"<< allSockSet.fd_count - 1<<endl;

					}
					else // 接收客户端信息   
					{
						ret = recv(s, bufRecv, 100, 0);
						// 接收错误   
						if (ret == SOCKET_ERROR)
						{
							DWORD err = WSAGetLastError();
							if (err == WSAECONNRESET)   //对方关闭链接错误码
								cout << "客户端被强行关闭\n";
							else
								cout << "接收消息失败!";
							// 删除套接字   
							FD_CLR(s, &allSockSet);
							cout << "目前客户端数目为:" << allSockSet.fd_count - 1 << endl;
							break;
						}
						if (ret == 0)
						{
							cout << "客户端已经退出!\n";
							// 删除套接字   
							FD_CLR(s, &allSockSet);
							cout << "目前客户端数目为:" << allSockSet.fd_count - 1 << endl;
							break;
						}
					
						bufRecv[ret] = 0x00;
						cout << "已接收："<<endl;
						cout << "        " << bufRecv << endl;
						
						char  sendData[2000] = "当前时间为:";
						time_t t = time(0);
						char ch[64];
						strftime(ch, sizeof(ch), "%Y-%m-%d %H:%M:%S", localtime(&t)); //年-月-日 时-分-秒
						strcat(sendData, ch);
						cout << "已发送："<<endl;
						cout << "        " << sendData << endl;
						cout << "\n";
						send(s, sendData, strlen(sendData), 0);

						
					}

				}

			}
		}
	}
	return 0;
}