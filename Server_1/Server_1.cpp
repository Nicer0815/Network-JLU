
#include<WinSock2.h>//Windows socket��ͷ�ļ�
#include <iostream>
using namespace std;

#pragma comment (lib,"ws2_32.lib")//������Ҫʹ�õ�һЩAPI��������Ҫ���������

int main() {

	WSADATA wsd;//����WSADATA���󣬵���WSASTART�󷵻����ݸ��������
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {//����Ӧ��SOCKET����а󶨣�����0˵���ɹ���
		WSACleanup();//�����Socket��İ󶨲����ͷ�Socket����ռ�õ�ϵͳ��Դ��
		return -1;
	}

	SOCKET serverSocket;//����������׽���
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//��ʼ���׽��֣���ַ�壨Address Family����IPV4���׽ӿ�����Э�飨TCP����
	if (serverSocket == INVALID_SOCKET) {//����INVALID_SOCKET��ʾ����ʧ��
		cout << "error:" + WSAGetLastError() << endl;//WSAGetLastError() �����ϴη������������
		WSACleanup();
		return -2;
	}

	SOCKADDR_IN server;//���ڽ���serverSocket�ı��ع����Ľṹ��Ҳ���Ǵ洢�����ַ�����ݽṹ
	server.sin_family = AF_INET;//IPV4
	server.sin_port = htons(2589);//�˿�
	server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//��ַ��INADDR_ANY��������ЧIP������

	if (bind(serverSocket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return  -3;
	}

	/*
	����bind����ͨ������һ���������Ƶ�δ������socket��������socket�ı��ع�����
	������IP��ַ��ѡ����һ���˿���ǰ����׽��ְ󶨼�����serverSocket��server�󶨡�
	*/

	if (listen(serverSocket, 2) == SOCKET_ERROR) {//ͬʱ����������ͻ�������
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return -4;
	}

	/*
	���ã������׽��ֽ������״̬��
	�÷����׽��֣����������������ֵ���δ����������������
	���ԣ�����ִ�гɹ����׽���s�����˱���ģʽ�����������ӻᱻ֪ͨҪ�ŶӵȺ���ܴ���
	*/

	SOCKET clientSocket;//������տͻ��˵��׽���
	SOCKADDR_IN client;//
	int addrSize = sizeof(SOCKADDR_IN);
	int const CLIENT_MSG_SIZE = 128;//���ջ���������
	char inMSG[CLIENT_MSG_SIZE];//�����ڿͻ��˵���Ϣ
	char outMSG[CLIENT_MSG_SIZE];//���͸��ͻ��˵���Ϣ
	char wx[] = "����������֧�ֲ�ѯ��ǰʱ�䣡����";//����ʵ����Ҫ������һ��
	int size;//������Ϣ�Ƿ�ʧ��
	while (true) {//ѭ���ȴ�����
		cout << "\n�ȴ��ͻ������ӡ�����" << endl;
		clientSocket = accept(serverSocket, (struct sockaddr*)&client, &addrSize);
		//accept:�ɹ�����һ���µ�socket�ļ������������ںͿͻ���ͨ�ţ�ʧ�ܷ���-1
		if (clientSocket == INVALID_SOCKET) {//����ʧ��
			cout << "�ͻ���acceptʧ�ܣ�������ʾ��" << WSAGetLastError() << endl;
			closesocket(serverSocket);
			WSACleanup();
			return -5;
		}
		else {//���ӳɹ�
			cout << "�ͻ��ˣ� "
				<< inet_ntoa(client.sin_addr)//inet_ntoa��һ��ʮ���������ֽ���ת��Ϊ���ʮ����IP��ʽ���ַ�����
				<< "\nͨ���˿ڣ�"
				<< ntohs(client.sin_port)//ntohs��һ��16λ���������ֽ�˳��ת��Ϊ�����ֽ�˳��
				<< "\n���ӳɹ�" << endl;

			//һֱ���ϵؽ�����Ϣ��ֱ���ͻ���ѡ���˳�
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

			closesocket(clientSocket);

		}
	}

	closesocket(serverSocket);
	WSACleanup();

	return 0;

}

