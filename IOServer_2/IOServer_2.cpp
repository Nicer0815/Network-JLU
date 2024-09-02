#include<stdio.h>
#include<time.h>
#include <stdlib.h>
#include<winsock2.h>
#include<iostream>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
int main()
{
	// ����win socket   
	WSADATA ws;
	int ret;
	ret = WSAStartup(MAKEWORD(1, 1), &ws);//����socket��
	if (ret != 0)
	{
		cout<<"�����׽��ֿ�ʧ��!\n";
		return -1;
	}
	
	SOCKET sListen;// ��������SOCKET   
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == INVALID_SOCKET)   //��ЧǶ����
	{
		cout << "�׽��ִ���ʧ��!\n";
		return -1;
	}
	
	sockaddr_in servAddr;// ����������ַ�ṹ   
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(8888);

	 
	ret = bind(sListen, (sockaddr*)&servAddr, sizeof(servAddr));// �󶨷������׽���  
	if (ret == SOCKET_ERROR)
	{
		cout << "��ʧ��!\n";
		return -1;
	}

	ret = listen(sListen, 5); //��ʼ����������������Ϊ5  
	if (ret == SOCKET_ERROR)
	{
		cout << "��������ʧ��!\n";
		return -1;
	}
	cout << "�����������ɹ����ڶ˿�"<< ntohs(servAddr.sin_port)<<" ������\n";//ntohs��ʾ����ǰ�������˿ں�
	//ʹ�� ģ��   
	// �����׽��ּ���   
	fd_set allSockSet; // �ܵ��׽��ּ���   
	fd_set readSet; // �ɶ��׽��ּ���   
	fd_set writeSet; // ��д�׽��ּ���  

	FD_ZERO(&allSockSet); // ����׽��ּ���   
	FD_SET(sListen, &allSockSet); // ��sListen�׽��ּ����׽��ּ�����   
	char bufRecv[100]; // ���ջ�����   
	// �����������ѭ��   
	while (1)
	{
		FD_ZERO(&readSet); // ��տɶ��׽���   
		FD_ZERO(&writeSet); // ��տ�д�׽���   
		readSet = allSockSet; // ��ֵ   
		writeSet = allSockSet; // ��ֵ   
		// ����select������timeout����ΪNULL   
		ret = select(0, &readSet, 0, NULL, NULL);
		//   
		if (ret == SOCKET_ERROR)
		{
			cout << "select����ʧ��!\n";
			return -1;
		}
		// �����׽��ֵ�I/O�Ѿ�׼����   
		if (ret > 0)
		{
			// ���������׽���

			for (int i = 0; i < allSockSet.fd_count; i++)
			{
				SOCKET s = allSockSet.fd_array[i];
				// ���ڿɶ����׽���   
				if (FD_ISSET(s, &readSet))
				{
					// �ɶ��׽���ΪsListen   
					if (s == sListen)
					{
						// �����µ�����   
						sockaddr_in clientAddr;
						int len = sizeof(clientAddr);
						SOCKET sClient = accept(s, (sockaddr*)&clientAddr, &len);
						// ���´������׽��ּ��뵽������   
						FD_SET(sClient, &allSockSet);
						cout << "�����µ�����\n";
						cout << "�ͻ��ˣ� "
							<< inet_ntoa(clientAddr.sin_addr)//inet_ntoa��һ��ʮ���������ֽ���ת��Ϊ���ʮ����IP��ʽ���ַ�����
							<< "\nͨ���˿ڣ�"
							<< ntohs(clientAddr.sin_port)//ntohs��һ��16λ���������ֽ�˳��ת��Ϊ�����ֽ�˳��
							<< "\n���ӳɹ�" << endl;
					
						cout <<"Ŀǰ�ͻ�����ĿΪ:"<< allSockSet.fd_count - 1<<endl;

					}
					else // ���տͻ�����Ϣ   
					{
						ret = recv(s, bufRecv, 100, 0);
						// ���մ���   
						if (ret == SOCKET_ERROR)
						{
							DWORD err = WSAGetLastError();
							if (err == WSAECONNRESET)   //�Է��ر����Ӵ�����
								cout << "�ͻ��˱�ǿ�йر�\n";
							else
								cout << "������Ϣʧ��!";
							// ɾ���׽���   
							FD_CLR(s, &allSockSet);
							cout << "Ŀǰ�ͻ�����ĿΪ:" << allSockSet.fd_count - 1 << endl;
							break;
						}
						if (ret == 0)
						{
							cout << "�ͻ����Ѿ��˳�!\n";
							// ɾ���׽���   
							FD_CLR(s, &allSockSet);
							cout << "Ŀǰ�ͻ�����ĿΪ:" << allSockSet.fd_count - 1 << endl;
							break;
						}
					
						bufRecv[ret] = 0x00;
						cout << "�ѽ��գ�"<<endl;
						cout << "        " << bufRecv << endl;
						
						char  sendData[2000] = "��ǰʱ��Ϊ:";
						time_t t = time(0);
						char ch[64];
						strftime(ch, sizeof(ch), "%Y-%m-%d %H:%M:%S", localtime(&t)); //��-��-�� ʱ-��-��
						strcat(sendData, ch);
						cout << "�ѷ��ͣ�"<<endl;
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