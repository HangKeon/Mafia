#define _WINSOCK_DEPRECATED_NO_WARNINGS			//inet_addr을 사용하기 위해 선언!
#define _CRT_SECURE_NO_WARNINGS					//sprintf를 사용하기 위해 선언

#include <iostream>
#include <winsock2.h>
#include <thread>								//멀티쓰레드 사용하기 위해!
#include <string>

#pragma comment(lib,"ws2_32")

#define PACKET_SIZE 1024						//패킷 크기

SOCKET skt;										//소켓 선언

void recv_data(SOCKET &s)						//서버로부터 데이터를 받는 함수
{
	char buf[PACKET_SIZE];


	while (1)
	{
		std::string str;

		ZeroMemory(buf, PACKET_SIZE);			//buf 초기화
		recv(s, buf, PACKET_SIZE, 0);			//recv(소켓, 수신 정보를 담을 배열 주소, 그 배열의 크기, flag)
												//recv 함수는 대상 소켓으로부터 보내온 정보를 받아주는 역할
												//보내준 데이터가 없다면 여기에서 받을때까지 계속 대기
												//flag 값으로는 flag를 활성화시키지 않을 것이기에 0을 지정

		str.append(buf);

		if (WSAGetLastError())					//서버종료 감지
			break;

		if (str == "[Server] >> <vote>")		//서버로부터 "<vote>" 메시지를 받아들이면
		{	
			std::string num;					//마피아라고 생각하는 번호

			std::cout << "\n투표를 시작하겠습니다.\n마피아라고 생각되는 사람의 번호를 입력해주세요 : ";
			std::cin >> num;

			send(skt, num.c_str(), strlen(num.c_str()), 0);	//클라이언트가 메세지를 서버측에 전달
		}

		//std::cout << "\n[Server] >> " << buf << "\n보낼 데이터를 입력 >> ";
		std::cout << "\n" << buf << "\n보낼 데이터를 입력 >> ";
	}

	return;
}

void openSocket(char SERVER_IP[], int PORT)	//IP 주소, 포트 번호 전달!
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

	//클라이언트측 코드, 소켓 구성요소 구조체에 접속할 서버의 ip를 적어준다
	SOCKADDR_IN addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	//클라이언트에서는 bind함수 대신 connect함수를 사용
	//connect(소켓, 소켓 구성요소 구조체의 주소, 그 구조체의 크기)
	//지정된 소켓에 연결을 설정
	while (connect(skt, (SOCKADDR*)&addr, sizeof(addr)));

	char buffer[PACKET_SIZE] = { 0 };
	char buffer2[PACKET_SIZE] = { 0 };

	recv(skt, buffer, PACKET_SIZE, 0);			//자신이 접속한 클라이언트 번호 수신
	recv(skt, buffer2, PACKET_SIZE, 0);			//자신이 접속한 직업을 수신

	int mynum = atoi(buffer);					//자신의 번호, char형에서 int형으로 형변환
	std::string myrole = buffer2;				//자신의 역할

	sprintf(buffer, "[%d] %s::%d", mynum, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	std::cout << "\n나는 Client#[" << mynum << "] 입니다.\n";
	std::cout << "당신의 역할은 " << myrole << " 입니다.\n\n";

	SetConsoleTitle(buffer);	// 콘솔제목지정 >> [내가 접속한 클라이언트번호] [서버아이피]::[서버포트]

	std::thread(recv_data, std::ref(skt)).detach();

	while (!WSAGetLastError())					//서버가 나갈시 자동으로 종료, 메시지 전송!
	{
		std::cout << "\n보낼 데이터를 입력 >> ";
		std::cin.getline(buffer, PACKET_SIZE);	//입력

		send(skt, buffer, strlen(buffer), 0);	//클라이언트가 메세지를 서버측에 전달
	}

	closesocket(skt);
	WSACleanup();								//WSAStartup 을 하면서 지정한 내용을 지움
}


int main()
{
	char IP[100];
	int PORT;
	std::cout << "아이피주소 설정 >> ";
	std::cin >> IP;								// 아이피 주소
	std::cout << "포트 설정 >> ";
	std::cin >> PORT;							// 포트 번호

	std::cout << "\n----------------------------------------------------------------\n";
	std::cout << "마피아 게임을 시작합니다.\n\n";
	std::cout << "직업은 시민과 마피아가 있습니다.\n";
	std::cout << "즐거운 게임을 하시기 바랍니다.\n\n";
	std::cout << "<시민의 승리 조건>\n";
	std::cout << "마피아를 모두 처치하라~!\n\n";
	std::cout << "<마피아의 승리 조건>\n";
	std::cout << "살아 있는 시민의 수와 살아 있는 마피아의 수를 같게 만들어라~!\n";
	std::cout << "----------------------------------------------------------------\n\n\n";

	openSocket(IP, PORT);						//소켓 오픈!

	return 0;
}