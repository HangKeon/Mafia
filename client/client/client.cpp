#define _WINSOCK_DEPRECATED_NO_WARNINGS			//inet_addr�� ����ϱ� ���� ����!
#define _CRT_SECURE_NO_WARNINGS					//sprintf�� ����ϱ� ���� ����

#include <iostream>
#include <winsock2.h>
#include <thread>								//��Ƽ������ ����ϱ� ����!
#include <string>

#pragma comment(lib,"ws2_32")

#define PACKET_SIZE 1024						//��Ŷ ũ��

SOCKET skt;										//���� ����

void recv_data(SOCKET &s)						//�����κ��� �����͸� �޴� �Լ�
{
	char buf[PACKET_SIZE];


	while (1)
	{
		std::string str;

		ZeroMemory(buf, PACKET_SIZE);			//buf �ʱ�ȭ
		recv(s, buf, PACKET_SIZE, 0);			//recv(����, ���� ������ ���� �迭 �ּ�, �� �迭�� ũ��, flag)
												//recv �Լ��� ��� �������κ��� ������ ������ �޾��ִ� ����
												//������ �����Ͱ� ���ٸ� ���⿡�� ���������� ��� ���
												//flag �����δ� flag�� Ȱ��ȭ��Ű�� ���� ���̱⿡ 0�� ����

		str.append(buf);

		if (WSAGetLastError())					//�������� ����
			break;

		if (str == "[Server] >> <vote>")		//�����κ��� "<vote>" �޽����� �޾Ƶ��̸�
		{	
			std::string num;					//���Ǿƶ�� �����ϴ� ��ȣ

			std::cout << "\n��ǥ�� �����ϰڽ��ϴ�.\n���Ǿƶ�� �����Ǵ� ����� ��ȣ�� �Է����ּ��� : ";
			std::cin >> num;

			send(skt, num.c_str(), strlen(num.c_str()), 0);	//Ŭ���̾�Ʈ�� �޼����� �������� ����
		}

		//std::cout << "\n[Server] >> " << buf << "\n���� �����͸� �Է� >> ";
		std::cout << "\n" << buf << "\n���� �����͸� �Է� >> ";
	}

	return;
}

void openSocket(char SERVER_IP[], int PORT)	//IP �ּ�, ��Ʈ ��ȣ ����!
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "WSA error";
		WSACleanup();
		return;
	}

	skt = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (skt == INVALID_SOCKET)
	{
		std::cout << "socket error";
		closesocket(skt);
		WSACleanup();
		return;
	}

	//Ŭ���̾�Ʈ�� �ڵ�, ���� ������� ����ü�� ������ ������ ip�� �����ش�
	SOCKADDR_IN addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	//Ŭ���̾�Ʈ������ bind�Լ� ��� connect�Լ��� ���
	//connect(����, ���� ������� ����ü�� �ּ�, �� ����ü�� ũ��)
	//������ ���Ͽ� ������ ����
	while (connect(skt, (SOCKADDR*)&addr, sizeof(addr)));

	char buffer[PACKET_SIZE] = { 0 };
	char buffer2[PACKET_SIZE] = { 0 };

	recv(skt, buffer, PACKET_SIZE, 0);			//�ڽ��� ������ Ŭ���̾�Ʈ ��ȣ ����
	recv(skt, buffer2, PACKET_SIZE, 0);			//�ڽ��� ������ ������ ����

	int mynum = atoi(buffer);					//�ڽ��� ��ȣ, char������ int������ ����ȯ
	std::string myrole = buffer2;				//�ڽ��� ����

	sprintf(buffer, "[%d] %s::%d", mynum, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	std::cout << "\n���� Client#[" << mynum << "] �Դϴ�.\n";
	std::cout << "����� ������ " << myrole << " �Դϴ�.\n\n";

	SetConsoleTitle(buffer);	// �ܼ��������� >> [���� ������ Ŭ���̾�Ʈ��ȣ] [����������]::[������Ʈ]

	std::thread(recv_data, std::ref(skt)).detach();

	while (!WSAGetLastError())					//������ ������ �ڵ����� ����, �޽��� ����!
	{
		std::cout << "\n���� �����͸� �Է� >> ";
		std::cin.getline(buffer, PACKET_SIZE);	//�Է�

		send(skt, buffer, strlen(buffer), 0);	//Ŭ���̾�Ʈ�� �޼����� �������� ����
	}

	closesocket(skt);
	WSACleanup();								//WSAStartup �� �ϸ鼭 ������ ������ ����
}


int main()
{
	char IP[100];
	int PORT;
	std::cout << "�������ּ� ���� >> ";
	std::cin >> IP;								// ������ �ּ�
	std::cout << "��Ʈ ���� >> ";
	std::cin >> PORT;							// ��Ʈ ��ȣ

	std::cout << "\n----------------------------------------------------------------\n";
	std::cout << "���Ǿ� ������ �����մϴ�.\n\n";
	std::cout << "������ �ùΰ� ���Ǿư� �ֽ��ϴ�.\n";
	std::cout << "��ſ� ������ �Ͻñ� �ٶ��ϴ�.\n\n";
	std::cout << "<�ù��� �¸� ����>\n";
	std::cout << "���ǾƸ� ��� óġ�϶�~!\n\n";
	std::cout << "<���Ǿ��� �¸� ����>\n";
	std::cout << "��� �ִ� �ù��� ���� ��� �ִ� ���Ǿ��� ���� ���� ������~!\n";
	std::cout << "----------------------------------------------------------------\n\n\n";

	openSocket(IP, PORT);						//���� ����!

	return 0;
}