#include <WinSock2.h>
#include <iostream>
//#include "headers.h"
#pragma once

//IP���ĸ�ʽ
typedef struct IP {
	unsigned char headLen;//4λ�ײ�����
	unsigned char serviceType;//8λ��������
	unsigned short totalLen;//16λ�ܳ���
	unsigned short identifier;//16λ��ʶ��
	unsigned short flags;//3λ��־λ
	unsigned char timeToLive;//8λ����ʱ��
	unsigned char protocal;//8λЭ��
	unsigned short headCheckSum;//16λ�ײ�У���
	unsigned int sourceAddr;//32λԴ��ַ
	unsigned int destinAddr;//32λĿ�ĵ�ַ
}IPHeader;

//TCP���ĸ�ʽ
typedef struct TCP {
	unsigned short sourcePort;//16λԴ�˿ں�
	unsigned short destinPort;//16λĿ�Ķ˿ں�
	unsigned int seqNum;//32λ���к�
	unsigned int ackNum;//32λȷ�Ϻ�
	unsigned char headLen;//4λ�ײ�����
	unsigned char flags;//8λ��־λ
	unsigned short winSize;//16λ���ڴ�С
	unsigned short checkNum;//16λУ���
	unsigned short urgPointer;//16λ����ָ��
}TCPHeader;

using namespace std;
#pragma comment(lib,"ws2_32.lib")
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)//��SIO_RCVALL����Ϊ_WSAIOW(IOC_VENDOR,1)

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
	������ԭʼ�׽��ֺ󣬾�Ҫ�����׽���ѡ���Ҫͨ��setsocketopt������ʵ�֣�setsocketopt�������������£�
	int setsocketopt (SOCKET s,int level,int optname,const char FAR *optval,int optlen )��
	����s�Ǳ�ʶ�׽ӿڵ������֣�Ҫע�����ѡ�������׽��ֱ�������Ч�ġ�
	����Level����ѡ���Ĳ�Σ���TCP/IPЭ������ԣ�֧��SOL_SOCKET��IPPROTO_IP��IPPROTO_TCP��Ρ�
	����Optname����Ҫ���õ�ѡ��������Щѡ��������Winsockͷ�ļ��ڶ���ĳ���ֵ��
	����optval��һ��ָ�룬��ָ����ѡ��ֵ�Ļ�������
	����optlenָʾoptval�������ĳ���
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
	//����������Ϊ����ģʽ�����ǽ�����������
	if (ioctlsocket(sock, SIO_RCVALL, &dwBufferInLen) == SOCKET_ERROR) {
		cout << "error:" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return -5;
	}
	/*
	ioctsocket�����ǿ����׽ӿڵ�ģʽ����������һ״̬����һ�׽ӿڡ������ڻ�ȡ���׽ӿ���صĲ����������������Э���ͨѶ��ϵͳ�޹ء�
	int ioctlsocket( int s, long cmd, u_long * argp);
	s��һ����ʶ�׽ӿڵ������֡�
	cmd�����׽ӿ�s�Ĳ������
	argp��ָ��cmd��������������ָ�롣
	*/

	int bytesRecv;
	char buffer[65535];//���ջ�����������
	//SOCKADDR_IN from;
	struct sockaddr_in from;
	int fromSize = sizeof(from);
	//ѭ������;
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
		��recv�Ĺ��ܲ�࣬���ǽ������ݣ�����from����������UDP����Ϊ����һ��from���㶮�ġ�
		*/

		ip = (struct IP*)buffer;

		if (ip->protocal == 6) {//��������Э�飬ֻ����TCPЭ��
			//tcp = (struct TCP*)(buffer + (4 * ip->headLen & 0xf0 ));//�õ�TCPͷ
			//tcp = (struct TCP*)(buffer + (4 * ip->headLen & 0xf0 >> 4));//�õ�TCPͷ
			tcp = (struct TCP*)(buffer + (4 * (ip->headLen & 0x0f)));
			//tcp = (struct TCP*)(buffer + (4 * ip->headLen));
		
			cout << "Network+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";//���������
			cout << "IP�����ֽ�����" << bytesRecv << "\n";
			cout << "ԴIP��" << inet_ntoa(*(in_addr*)&ip->sourceAddr) << "\n";
			cout << "Ŀ��IP��" << inet_ntoa(*(in_addr*)&ip->destinAddr) << "\n";
			cout << "Transportation++++++++++++++++++++++++++++++++++++++++++++++++++++\n";//���������
			cout << "Դ�˿ڣ�" << ntohs(tcp->sourcePort) << "\n";
			cout << "Ŀ�Ķ˿ڣ�" << ntohs(tcp->destinPort) << "\n";
			cout << "Applications++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";//Ӧ�ò�����

			//char* start = buffer + 5 + 4 * ((tcp->headLen & 0xf0) >> 4 | 0);//��������ͷָ�룬�Ӻδ���ʼ����

			char* start = buffer + (4 * (ip->headLen & 0x0f)) + 4 * ((tcp->headLen & 0xf0) >> 4);

			int dataSize = bytesRecv - 4 * (ip->headLen & 0x0f) - 4 * ((tcp->headLen & 0xf0) >> 4 | 0);
			//int dataSize = bytesRecv - 5 - 4 * ((tcp->headLen & 0xf0) >> 4 | 0);//�������ݳ���
			
			cout << "�������ݣ�";
			memcpy(buffer, start, dataSize);
			for (int i = 0; i < dataSize; i++) {
				
					cout<<(unsigned char)buffer[i];
				
			
			}
			cout << "\n";
		}
	}

}