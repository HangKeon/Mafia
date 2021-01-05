#define _WINSOCK_DEPRECATED_NO_WARNINGS		//inet_addr�� ����ϱ� ���� ����!
#define _CRT_SECURE_NO_WARNINGS				//_itoa�� ����ϱ� ���� ����

#include <iostream>
#include <winsock2.h>						//������ ����ϱ� ���� �������
#include <thread>
#include <stdlib.h>							//itoa ����ϱ� ���� �������	
#include <string>
#include <vector>
#include <cstdlib>							//���� ���� -> client�� ������ �������� ����

#pragma comment(lib,"ws2_32") //winsock2.h �� ������ ��ɵ��� ����ϰڴٰ�, ws2_32 ���̺귯���� ��ũ�ɾ��ִ� ��

#define PACKET_SIZE 1024					//��Ŷ ũ��

WSADATA wsa;								//Windows�� ���� �ʱ�ȭ ������ �����ϱ����� ����ü
SOCKET skt, *client_sock;					//client_sock : Ŭ�����̾�Ʈ�� ������ ������� ����
SOCKADDR_IN *client;						//�ּ� ������ ��Ƶδ� ����ü
int *client_size, MAX;						//MAX : Ŭ���̾�Ʈ �ִ� ���� ��
enum Role { CITIZEN, MAFIA };				//���� : �ù�,  ���Ǿ�
enum Time { DAY, NIGHT };					//�ð� : ��, ��
int sw;										//���� ���� ������ ����
std::vector<int> v;							//������ client, 1 : ��� ����, -1 : ����
std::vector<int> vote_cnt;					//��ǥ�� ���� client�� ��ȣ�� ���� ����
std::vector<int> role;						//���� : �ù�, ���Ǿ�,
int mafia_num = (MAX / 7) + 1;				//���Ǿ� �� = (�ο� �� / 7) + 1
int cnt_alive = MAX;						//��� �ִ� ��� ��

void recv_data(SOCKET &s, int client_num)  //Ŭ���̾�Ʈ recv�Լ����� ��Ƽ������
{
	char buf[PACKET_SIZE];

	while (1)
	{
		ZeroMemory(buf, PACKET_SIZE);			//buf �ʱ�ȭ

		//recv(����, ���� ������ ���� �迭 �ּ�, �� �迭�� ũ��, flag)
		//recv �Լ��� ��� �������κ��� ������ ������ �޾��ִ� ����
		//������ �����Ͱ� ���ٸ� ���⿡�� ���������� ��� ���
		//flag �����δ� flag�� Ȱ��ȭ��Ű�� ���� ���̱⿡ 0�� ����

		if (recv(s, buf, PACKET_SIZE, 0) == -1) //Ŭ���̾�Ʈ ���� ����
			break;

		if (sw == Time::DAY)					//���� ���
		{
			std::cout << "\nClient #" << client_num << " << " << buf << "\n���� �����͸� �Է� >> ";

			for (int i = 0; i < MAX; i++)						//��ȭ�� ��ο��� �鸮���� �Ѵ�.
			{
				if (i != client_num)							//���� client�� ����
				{
					std::string str;							//� client�� ���� �������� �߰�!

					str = "[Client #" + std::to_string(client_num) + "] >> " + buf;

					send(client_sock[i], str.c_str(), strlen(str.c_str()), 0);	//������ �޼����� Ŭ���̾�Ʈ���� ����
				}
			}
		}
		else if (sw == Time::NIGHT)				//���� ���
		{
			std::cout << "\Mafia #" << client_num << " << " << buf << "\n���� �����͸� �Է� >> ";

			for (int i = 0; i < MAX; i++)						//��ȭ�� ��ο��� �鸮���� �Ѵ�.
			{
				if (v[i] == 1 && role[i] == Role::MAFIA)		//��� �����鼭 ���Ǿ��� ���
				{
					std::string str;							//� client�� ���� �������� �߰�!

					str.append("[Mafia #");
					str.append(std::to_string(i));
					str.append("] >> ");
					str.append(buf);

					send(client_sock[i], str.c_str(), strlen(str.c_str()), 0);	//������ �޼����� Ŭ���̾�Ʈ���� ����
				}
			}
		}
	}

	return;
}

void acceptclients()						//Ŭ���̾�Ʈ ���� ���� �Լ�(��Ƽ ������)
{
	char buffer[PACKET_SIZE];
	char client_num[10];					//Ŭ���̾�Ʈ �������� ���ڿ��� �����ϱ� ���� ����� ����

	std::srand(1000);						//�õ尪�� 1000���� ���� -> ���� ���� �� ���

	for (int i = 0; i < MAX; i++)
	{
		ZeroMemory(buffer, PACKET_SIZE);	//buffer �ʱ�ȭ
		client_size[i] = sizeof(client[i]);

		v.push_back(1);						//i�� client�� v�� �����ϰ� 1�̶�� ���� ����!(��� ����)
		vote_cnt.push_back(0);				//���߿� ��ǥ�� ���ؼ� client�� ����ŭ ����

		if (mafia_num != 0)					//���Ǿư� ���� �� �� ������ ���
		{
			if (MAX - i == mafia_num)		 //���� ���Ǿ� ���� ä������ ���� ��� 
			{
				role.push_back(Role::MAFIA); //���� �ο� ���� ���Ǿ�!
			}
			else
			{
				role.push_back(std::rand() % 2);
			}

			if(role[i])						//���Ǿ��� ��� -> ���Ǿ� �� 1 ����
				mafia_num--;
		}
		else								//���Ǿư� �� ���������� �������� ������ '�ù�'
		{
			role.push_back(Role::CITIZEN);
		}

		if (role[i])						//���Ǿ�(1)�� ���
		{
			strcpy(buffer, "���Ǿ�");
			
		}
		else if (!role[i])					//�ù�(0)�� ���
		{
			strcpy(buffer, "�ù�");
		}

		


		//accept(����, ���� ������� �ּ�ü�� �ּ�, �� ����ü�� ũ�⸦ ����ִ� ������ �ּ�)
		//���� ��û�� ����. ����ȭ�� ������� ����
		//����ȭ�� ����̶� ��û�� ������ �ϱ� ������ ��� ��� ���¿� ���̰� �Ǵ� ��
		//�� ��û�� ������ ������ �� �Լ��� �Ⱥ�������
		//���� ��û�� �����ϸ� ����� ������ ��������� ����
		//ù ��° ���ڷδ� ������ �־���
		//�� ��° ���ڷδ� accept �� Ŭ���̾�Ʈ �� �ּ� ���� ����ü�� �ּ�
		//�� ��° ���ڷδ� �ι�° ���ڷ� ���� ����ü�� ũ�⸦ ������ �� ������ �ּ�
		client_sock[i] = accept(skt, (SOCKADDR*)&client[i], &client_size[i]);

		if (client_sock[i] == INVALID_SOCKET)
		{
			std::cout << "accept error\n";
			closesocket(client_sock[i]);
			closesocket(skt);
			WSACleanup();
			return;
		}

		std::cout << "Client #" << i << " Joined!\n";					// Ŭ���̾�Ʈ ���� ����
		ZeroMemory(client_num, sizeof(client_num));						// ����� ���� ���� �ʱ�ȭ

		std::cout << "Client #" << i << "�� " << buffer << " �Դϴ�.\n";

		//������ itoa�ε� visual studio�� ���� �������� _itoa�� ����϶�� �ؼ� �Ʒ�ó�� ���!
		_itoa(i, client_num, 10);										// i�� �������� client_num���ٰ� 10������ ����
		send(client_sock[i], client_num, strlen(client_num), 0);		// Ŭ���̾�Ʈ ��ȣ ����
		send(client_sock[i], buffer, strlen(buffer), 0);				//���� -> client���� ������ �˷���
		std::thread(recv_data, std::ref(client_sock[i]), i).detach();	// �ش� Ŭ���̾�Ʈ ������ ����
		//��Ƽ�����带 ����Ͽ� 'accecptclients�Լ��� ȣ��'
		//.detach(); �����Լ��� ���̻� �ش� �����尡 ����ɶ�����
		//��ٷ����� ����(������ ������ ������� �и�)

	}

	return;
}

void openSocket(int PORT)
{
	if (WSAStartup(MAKEWORD(2, 2), &wsa))//WSAStartup(���� ����, WSADATA ����ü �ּ�)
	{									 //ù ��° ���� : 2.2������ Ȱ���ϰ� WORD Ÿ������ ����.
										 //�׷��� WORD�� unsigned short�̹Ƿ� 2.2�� �Ǽ��� ��ȯ �ʿ�->MAKEWORD ���!
										 //�� ��° ���� : WSADATA ����ü�� ������ Ÿ��
		std::cout << "WSA error\n";
		return;
	}

	skt = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	//PF_INET : IPV4�� ��� -> 32��Ʈ �ּ� ü��
														//SOCK_STREAM : ���������� ����
														//IPPROTO_TCP : TCP ����ϰڴٰ� ����

	if (skt == INVALID_SOCKET)
	{
		std::cout << "socket error\n";
		closesocket(skt);
		WSACleanup();
		return;
	}

	//������ ���� ��Ҹ� ���� ����ü ���� �� �� �Ҵ�
	SOCKADDR_IN addr = {};								//�ּ� ������ ��Ƶδ� ����ü
	addr.sin_family = AF_INET;							//sin_family�� �ݵ�� AF_INET�̾�� �Ѵ�. -> UDP,TCP
	addr.sin_port = htons(PORT);						//��Ʈ ��ȣ�� ����(2����Ʈ ����)
														//htons : host to network short �� ����
														//������ �򿣵�� ������� �����͸� ��ȯ�� ����

	addr.sin_addr.s_addr = htonl(INADDR_ANY);			//������ ���� ���۵Ǵ� ��ǻ���� IP �ּҷ� ���� �ʿ�
														//INADDR_ANY�� �־��ָ� ���� ���۵Ǵ� ��ǻ���� IP �ּҷ� ����
														//s_addr�� IPv4 Internet address�� �ǹ�

	if (bind(skt, (SOCKADDR*)&addr, sizeof(addr))) //bind(����, ���� ������� ����ü�� �ּ�, �� ����ü�� ũ��)
	{											   //bind �Լ��� ���Ͽ� �ּ������� ����
		//ù��° ���ڷδ� ���� ������ ������ �־��ش�
		//�ι�° ���ڷδ� bind �� ���Ͽ� �Ҵ��� �ּ������� ����ִ� ����ü�� �ּҰ� ����
		//����° ���ڷδ� �ι�° ���ڷ� ���� ����ü�� ũ�Ⱑ ����.

		std::cout << "bind error\n";
		closesocket(skt);
		WSACleanup();
		return;
	}

	if (listen(skt, SOMAXCONN))			//������ �����ϴ� ���·� ������ ���¸� ����
	{									//��, ������ ���� ��� ���·� ����
										//SOMAXCONN�� �Ѳ����� ��û ������ �ִ� ���� ���� ��
		std::cout << "listen error\n";
		closesocket(skt);
		WSACleanup();
		return;
	}

	std::thread(acceptclients).join();	//client�� thread�� ����
										//join�� ��������ν� �����Լ��� ��ٸ�!


	char msg[PACKET_SIZE], sendnum[PACKET_SIZE];	//msg : ���� ������, sendnum : client�� ��ȣ

	while (1)										//client���� ����
	{
		std::string s;								//msg�� [Server] >> �� ���� ���ڿ�

		std::cout << "\n���� �����͸� �Է� >> ";
		std::cin >> msg;							//�����ͳ���

		if (!strcmp(msg, "exit")) 					// �������� ������ "exit"�̸� ���� ����
			break;

		std::cout << "\n��� Ŭ���̾�Ʈ�� �Է�(all:���) >> ";
		std::cin >> sendnum;				//��� Ŭ���̾�Ʈ ��ȣ ����! "all"�Ͻ� ��� Ŭ���̾�Ʈ���� ����

		s.append("[Server] >> ");			//msg�� [Server] >> �� �߰�!
		s.append(msg);

		//strcpy(msg, s.c_str());

		if (!strcmp(sendnum, "all"))		// ���� sendnum�� ������ "all" �̶�� ��ο��� �޽��� ����
		{
			for (int i = 0; i < MAX; i++)
			{
				send(client_sock[i], s.c_str(), strlen(s.c_str()), 0);	//������ �޼����� Ŭ���̾�Ʈ���� ����
			}
		}
		else send(client_sock[atoi(sendnum)], s.c_str(), strlen(s.c_str()), 0); //�ƴ϶�� �� ���Ը� ����



		if (s == "[Server] >> <vote>")			//��ǥ�� �ϴ� ���
		{
			vote_cnt.assign(MAX, 0);			//vote_cnt�� 0���� �ʱ�ȭ(���Ҵ�)->�ٽ� ��ǥ ���� ���� ��� ����

			char buf[PACKET_SIZE];

			ZeroMemory(buf, PACKET_SIZE);		//buf �ʱ�ȭ

			if (sw == Time::DAY)				//���� ���
			{
				for (int i = 0; i < MAX; i++)
				{
					if (v[i] == 1)				//��� �ִ� ����� ���
					{
						//recv(����, ���� ������ ���� �迭�ּ�, �� �迭�� ũ��, flag)
						//recv �Լ��� ��� �������κ��� ������ ������ �޾��ִ� ����
						//������ �����Ͱ� ���ٸ� ���⿡�� ���������� ��� ���
						//flag �����δ� flag�� Ȱ��ȭ��Ű�� �������̱⿡ 0�� ����

						 if(recv(client_sock[i], buf, PACKET_SIZE, 0) == -1) //Ŭ���̾�Ʈ ���� ����
							break;
						//recv(client_sock[i], buf, PACKET_SIZE, 0);

						std::cout << "\nClient #" << i << "�� ��ǥ ��� << " << buf << "\n";

						vote_cnt[std::stoi(buf)]++;						//��ǥ�� client�� ��ȣ�� 1�� ����!
												
						//if (i != client_num)
						//{
						//	std::string str;							//� client�� ���� �������� �߰�!
						//	str = "[Client #" + std::to_string(i) + "] >> " + buf;

						//	send(client_sock[i], str.c_str(), strlen(str.c_str()), 0);	//������ �޼����� Ŭ���̾�Ʈ���� ����
						//}
					}
				}

				int r = 0;									//��ǥ���� ���� ���� �ִ� ��� 1�� �ٲ�
				char res[PACKET_SIZE];						//client���� ���� ���ڿ� ���
				int vote_max = -1;							//�ִ�� ���� ��ǥ��
				int vote_client;							//vote_max�� ���� client�� ��ȣ

				for (int i = 0; i < MAX; i++)				//��ǥ���� ��
				{
					for (int j = i + 1; j < MAX; j++)
					{
						if (vote_cnt[i] == vote_cnt[j])		//��ǥ���� ���� ���� �ִٸ�
						{
							sprintf(res, "�ƹ��� ������� �ʾҽ��ϴ�.\n���� ���Դϴ�.\n\n");
							//send(client_sock[i], res, strlen(res), 0);
							r = 1;
							break;
						}

						if (vote_max < v[i])				//���� ���� ��ǥ���� �� client ��ȣ ���ϱ�
						{
							vote_max = v[i];
							vote_client = i;
						}
					}

					if (r)									//��ǥ���� ���� ���� �ִٸ�
						break;
				}

				if (!r)										//��ǥ���� ���� ���� ���ٸ�
				{
					sprintf(res, "Client[%d]�� ����Ǿ����ϴ�.\n���� ���Դϴ�.\n\n", vote_client);
					v[vote_client] = -1;					//����Ǹ� -1�� ��ȯ!
				}

				for (int i = 0; i < MAX; i++)
				{
					send(client_sock[i], res, strlen(res), 0);	//��ǥ ��� �˸�
				}

				sw = Time::NIGHT;							//���� ��ǥ�� �������Ƿ� ������ ��ȯ!
			}
			else if (sw == Time::NIGHT)						//���� ���
			{
				for (int i = 0; i < MAX; i++)
				{
					if (v[i] == 1 && role[i] == Role::MAFIA)	//����ִ� ���Ǿ��� ���
					{
						//recv(����, ���� ������ ���� �迭�ּ�, �� �迭�� ũ��, flag)
						//recv �Լ��� ��� �������κ��� ������ ������ �޾��ִ� ����
						//������ �����Ͱ� ���ٸ� ���⿡�� ���������� ��� ���
						//flag �����δ� flag�� Ȱ��ȭ��Ű�� �������̱⿡ 0�� ����

						if (recv(client_sock[i], buf, PACKET_SIZE, 0) == -1) //Ŭ���̾�Ʈ ���� ����
							break;


						std::cout << "\nClient #" << i << "�� ��ǥ ��� << " << buf << "\n";

						v[std::stoi(buf)]++;								//��ǥ�� client�� ��ȣ�� 1�� ����!

						//if (i != client_num)
						//{
						//	std::string str;							//� client�� ���� �������� �߰�!
						//	str = "[Client #" + std::to_string(i) + "] >> " + buf;

						//	send(client_sock[i], str.c_str(), strlen(str.c_str()), 0);	//������ �޼����� Ŭ���̾�Ʈ���� ����
						//}
					}
				}

				int r = 0;									//��ǥ���� ���� ���� �ִ� ��� 1�� �ٲ�
				char res[PACKET_SIZE];						//client���� ���� ���ڿ� ���
				int vote_max = -1;							//�ִ�� ���� ��ǥ��
				int vote_client;							//vote_max�� ���� client�� ��ȣ

				for (int i = 0; i < MAX; i++)				//��ǥ���� ��
				{
					for (int j = i + 1; j < MAX; j++)
					{
						if (vote_cnt[i] == vote_cnt[j])					//��ǥ���� ���� ���� �ִٸ�
						{
							sprintf(res, "�ƹ��� ������� �ʾҽ��ϴ�.\n���� ���Դϴ�.\n\n");
							//send(client_sock[i], res, strlen(res), 0);
							r = 1;
							break;
						}

						if (vote_max < v[i])				//���� ���� ��ǥ���� �� client ��ȣ ���ϱ�
						{
							vote_max = v[i];
							vote_client = i;
						}
					}

					if (r)									//��ǥ���� ���� ���� �ִٸ�
						break;
				}

				if (!r)										//��ǥ���� ���� ���� ���ٸ�
				{
					sprintf(res, "Client[%d]�� ����Ǿ����ϴ�.\n���� ���Դϴ�.\n\n", vote_client);
					v[vote_client] = -1;					//����Ǹ� -1�� ��ȯ!
				}

				for (int i = 0; i < MAX; i++)
				{
					send(client_sock[i], res, strlen(res), 0);	//��ǥ ��� �˸�
				}

				sw = Time::DAY;							//���Ǿ��� ��ǥ�� �������Ƿ� ������ ��ȯ!
			}
		}




	}

	for (int i = 0; i < MAX; i++)				//for������ ���� ���� �ø���
		closesocket(client_sock[i]);			

	closesocket(skt);

	WSACleanup();								//WSAStartup �� �ϸ鼭 ������ ������ ����

	return;
}

int main()
{
	int PORT;												//��Ʈ
	std::cout << "��Ʈ���� >> ";
	std::cin >> PORT;										//��Ʈ ����
	std::cout << "Ŭ���̾�Ʈ �ִ���� �� ���� >> ";
	std::cin >> MAX;										//�ִ� ���� �� ����

	//�����Ҵ�
	client_sock = new SOCKET[MAX];
	client = new SOCKADDR_IN[MAX];
	client_size = new int[MAX];


	//�޸� ����
	ZeroMemory(client_sock, sizeof(client_sock));
	ZeroMemory(client, sizeof(client));
	ZeroMemory(client_size, sizeof(client_size));

	openSocket(PORT);										//���Ͽ���

	delete[] client_sock, client, client_size;				//�����Ҵ� ����
	return 0;
}