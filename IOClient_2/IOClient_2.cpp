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
		cout << "�׽��ִ�������!";
		return 0;
	}
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (connect(sclient, (sockaddr*)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		cout << "���Ӵ���!";
		closesocket(sclient);
		return 0;
	}


	cout << "����������ӳɹ�\n";
	cout << "��ʾ:\n";
	cout << "     ���롰�Ͽ����ӡ������ԶϿ��������������:\n";
	//���ʹ��к������� 
	char* sendData = "���,������.....";

	send(sclient, sendData, strlen(sendData), 0);

	char close[11] = "�Ͽ�����";
	while (judge == 1) {
		
		char recData[255];
		int ret = recv(sclient, recData, 255, 0);
		if (ret > 0) {
			recData[ret] = 0x00;  //ת��Ϊ16����
			cout << "��������Ϣ��\n";
			cout <<"           "<< recData;
		}
		
		char input[265];
		cout << "\n";
		cout << "\n";
		cout << "����������:\n";
		
		gets_s(input);
		if (strcmp(input, close) == 0) {
			judge = 0;
		}
		else {

			char sendData[1000] = "�ͻ�����Ϣ:";
			strcat(sendData, input);
			send(sclient, sendData, strlen(sendData), 0);
		}
	}
	closesocket(sclient);
	WSACleanup();
	return 0;
}