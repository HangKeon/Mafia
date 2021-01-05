#define _WINSOCK_DEPRECATED_NO_WARNINGS		//inet_addr을 사용하기 위해 선언!
#define _CRT_SECURE_NO_WARNINGS				//_itoa를 사용하기 위해 선언

#include <iostream>
#include <winsock2.h>						//소켓을 사용하기 위한 헤더파일
#include <thread>
#include <stdlib.h>							//itoa 사용하기 위한 헤더파일	
#include <string>
#include <vector>
#include <cstdlib>							//난수 생성 -> client의 직업을 무작위로 결정

#pragma comment(lib,"ws2_32") //winsock2.h 에 선언한 기능들을 사용하겠다고, ws2_32 라이브러리를 링크걸어주는 것

#define PACKET_SIZE 1024					//패킷 크기

WSADATA wsa;								//Windows의 소켓 초기화 정보를 저장하기위한 구조체
SOCKET skt, *client_sock;					//client_sock : 클라이이언트를 들어오는 순서대로 저장
SOCKADDR_IN *client;						//주소 정보를 담아두는 구조체
int *client_size, MAX;						//MAX : 클라이언트 최대 수용 수
enum Role { CITIZEN, MAFIA };				//역할 : 시민,  마피아
enum Time { DAY, NIGHT };					//시간 : 낮, 밤
int sw;										//낮과 밤을 구분할 변수
std::vector<int> v;							//접속한 client, 1 : 살아 있음, -1 : 죽음
std::vector<int> vote_cnt;					//투표시 적힌 client의 번호를 세는 벡터
std::vector<int> role;						//역할 : 시민, 마피아,
int mafia_num = (MAX / 7) + 1;				//마피아 수 = (인원 수 / 7) + 1
int cnt_alive = MAX;						//살아 있는 사람 수

void recv_data(SOCKET &s, int client_num)  //클라이언트 recv함수전용 멀티쓰레드
{
	char buf[PACKET_SIZE];

	while (1)
	{
		ZeroMemory(buf, PACKET_SIZE);			//buf 초기화

		//recv(소켓, 수신 정보를 담을 배열 주소, 그 배열의 크기, flag)
		//recv 함수는 대상 소켓으로부터 보내온 정보를 받아주는 역할
		//보내준 데이터가 없다면 여기에서 받을때까지 계속 대기
		//flag 값으로는 flag를 활성화시키지 않을 것이기에 0을 지정

		if (recv(s, buf, PACKET_SIZE, 0) == -1) //클라이언트 종료 감지
			break;

		if (sw == Time::DAY)					//낮인 경우
		{
			std::cout << "\nClient #" << client_num << " << " << buf << "\n보낼 데이터를 입력 >> ";

			for (int i = 0; i < MAX; i++)						//대화는 모두에게 들리도록 한다.
			{
				if (i != client_num)							//말한 client는 제외
				{
					std::string str;							//어떤 client가 말한 것인지를 추가!

					str = "[Client #" + std::to_string(client_num) + "] >> " + buf;

					send(client_sock[i], str.c_str(), strlen(str.c_str()), 0);	//서버가 메세지를 클라이언트측에 전달
				}
			}
		}
		else if (sw == Time::NIGHT)				//밤인 경우
		{
			std::cout << "\Mafia #" << client_num << " << " << buf << "\n보낼 데이터를 입력 >> ";

			for (int i = 0; i < MAX; i++)						//대화는 모두에게 들리도록 한다.
			{
				if (v[i] == 1 && role[i] == Role::MAFIA)		//살아 있으면서 마피아인 경우
				{
					std::string str;							//어떤 client가 말한 것인지를 추가!

					str.append("[Mafia #");
					str.append(std::to_string(i));
					str.append("] >> ");
					str.append(buf);

					send(client_sock[i], str.c_str(), strlen(str.c_str()), 0);	//서버가 메세지를 클라이언트측에 전달
				}
			}
		}
	}

	return;
}

void acceptclients()						//클라이언트 연결 관리 함수(멀티 쓰레드)
{
	char buffer[PACKET_SIZE];
	char client_num[10];					//클라이언트 정수값을 문자열로 저장하기 위한 저장용 변수

	std::srand(1000);						//시드값을 1000으로 선언 -> 직업 결정 시 사용

	for (int i = 0; i < MAX; i++)
	{
		ZeroMemory(buffer, PACKET_SIZE);	//buffer 초기화
		client_size[i] = sizeof(client[i]);

		v.push_back(1);						//i번 client를 v에 저장하고 1이라는 값을 저장!(살아 있음)
		vote_cnt.push_back(0);				//나중에 투표를 위해서 client의 수만큼 생성

		if (mafia_num != 0)					//마피아가 아직 다 안 정해진 경우
		{
			if (MAX - i == mafia_num)		 //만약 마피아 수가 채워지지 않은 경우 
			{
				role.push_back(Role::MAFIA); //남은 인원 전부 마피아!
			}
			else
			{
				role.push_back(std::rand() % 2);
			}

			if(role[i])						//마피아인 경우 -> 마피아 수 1 감소
				mafia_num--;
		}
		else								//마피아가 다 정해졌으면 나머지는 무조건 '시민'
		{
			role.push_back(Role::CITIZEN);
		}

		if (role[i])						//마피아(1)인 경우
		{
			strcpy(buffer, "마피아");
			
		}
		else if (!role[i])					//시민(0)인 경우
		{
			strcpy(buffer, "시민");
		}

		


		//accept(소켓, 소켓 구성요소 주소체의 주소, 그 구조체의 크기를 담고있는 변수의 주소)
		//접속 요청을 수락. 동기화된 방식으로 동작
		//동기화된 방식이란 요청을 마무리 하기 전까지 계속 대기 상태에 놓이게 되는 것
		//즉 요청이 들어오기 전까지 이 함수는 안빠져나옴
		//접속 요청을 승인하면 연결된 소켓이 만들어져서 리턴
		//첫 번째 인자로는 소켓을 넣어줌
		//두 번째 인자로는 accept 할 클라이언트 측 주소 정보 구조체의 주소
		//세 번째 인자로는 두번째 인자로 넣은 구조체의 크기를 저장해 둔 변수의 주소
		client_sock[i] = accept(skt, (SOCKADDR*)&client[i], &client_size[i]);

		if (client_sock[i] == INVALID_SOCKET)
		{
			std::cout << "accept error\n";
			closesocket(client_sock[i]);
			closesocket(skt);
			WSACleanup();
			return;
		}

		std::cout << "Client #" << i << " Joined!\n";					// 클라이언트 연결 감지
		ZeroMemory(client_num, sizeof(client_num));						// 저장용 변수 내용 초기화

		std::cout << "Client #" << i << "는 " << buffer << " 입니다.\n";

		//원래는 itoa인데 visual studio의 문제 때문인지 _itoa로 사용하라고 해서 아래처럼 사용!
		_itoa(i, client_num, 10);										// i의 정수값을 client_num에다가 10진수로 저장
		send(client_sock[i], client_num, strlen(client_num), 0);		// 클라이언트 번호 전송
		send(client_sock[i], buffer, strlen(buffer), 0);				//서버 -> client에게 직업을 알려줌
		std::thread(recv_data, std::ref(client_sock[i]), i).detach();	// 해당 클라이언트 쓰레드 생성
		//멀티쓰레드를 사용하여 'accecptclients함수를 호출'
		//.detach(); 메인함수는 더이상 해당 쓰레드가 종료될때까지
		//기다려주지 않음(완전히 별개의 쓰레드로 분리)

	}

	return;
}

void openSocket(int PORT)
{
	if (WSAStartup(MAKEWORD(2, 2), &wsa))//WSAStartup(소켓 버전, WSADATA 구조체 주소)
	{									 //첫 번째 인자 : 2.2버전을 활용하고 WORD 타입으로 들어간다.
										 //그런데 WORD는 unsigned short이므로 2.2를 실수로 변환 필요->MAKEWORD 사용!
										 //두 번째 인자 : WSADATA 구조체의 포인터 타입
		std::cout << "WSA error\n";
		return;
	}

	skt = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	//PF_INET : IPV4를 사용 -> 32비트 주소 체계
														//SOCK_STREAM : 연결지향형 소켓
														//IPPROTO_TCP : TCP 사용하겠다고 지정

	if (skt == INVALID_SOCKET)
	{
		std::cout << "socket error\n";
		closesocket(skt);
		WSACleanup();
		return;
	}

	//소켓의 구성 요소를 담을 구조체 생성 및 값 할당
	SOCKADDR_IN addr = {};								//주소 정보를 담아두는 구조체
	addr.sin_family = AF_INET;							//sin_family는 반드시 AF_INET이어야 한다. -> UDP,TCP
	addr.sin_port = htons(PORT);						//포트 번호를 설정(2바이트 숫자)
														//htons : host to network short 의 약자
														//무조건 빅엔디안 방식으로 데이터를 변환해 설정

	addr.sin_addr.s_addr = htonl(INADDR_ANY);			//서버는 현재 동작되는 컴퓨터의 IP 주소로 설정 필요
														//INADDR_ANY를 넣어주면 현재 동작되는 컴퓨터의 IP 주소로 설정
														//s_addr은 IPv4 Internet address를 의미

	if (bind(skt, (SOCKADDR*)&addr, sizeof(addr))) //bind(소켓, 소켓 구성요소 구조체의 주소, 그 구조체의 크기)
	{											   //bind 함수는 소켓에 주소정보를 연결
		//첫번째 인자로는 위에 선언한 소켓을 넣어준다
		//두번째 인자로는 bind 될 소켓에 할당할 주소정보를 담고있는 구조체의 주소가 들어간다
		//세번째 인자로는 두번째 인자로 넣은 구조체의 크기가 들어간다.

		std::cout << "bind error\n";
		closesocket(skt);
		WSACleanup();
		return;
	}

	if (listen(skt, SOMAXCONN))			//연결을 수신하는 상태로 소켓의 상태를 변경
	{									//즉, 소켓을 접속 대기 상태로 만듬
										//SOMAXCONN은 한꺼번에 요청 가능한 최대 접속 승인 수
		std::cout << "listen error\n";
		closesocket(skt);
		WSACleanup();
		return;
	}

	std::thread(acceptclients).join();	//client를 thread로 선언
										//join을 사용함으로써 메인함수가 기다림!


	char msg[PACKET_SIZE], sendnum[PACKET_SIZE];	//msg : 보낼 데이터, sendnum : client의 번호

	while (1)										//client에게 전송
	{
		std::string s;								//msg에 [Server] >> 를 담을 문자열

		std::cout << "\n보낼 데이터를 입력 >> ";
		std::cin >> msg;							//데이터내용

		if (!strcmp(msg, "exit")) 					// 데이터의 내용이 "exit"이면 소켓 종료
			break;

		std::cout << "\n대상 클라이언트를 입력(all:모두) >> ";
		std::cin >> sendnum;				//대상 클라이언트 번호 지정! "all"일시 모든 클라이언트에게 전송

		s.append("[Server] >> ");			//msg에 [Server] >> 를 추가!
		s.append(msg);

		//strcpy(msg, s.c_str());

		if (!strcmp(sendnum, "all"))		// 변수 sendnum의 내용이 "all" 이라면 모두에게 메시지 전송
		{
			for (int i = 0; i < MAX; i++)
			{
				send(client_sock[i], s.c_str(), strlen(s.c_str()), 0);	//서버가 메세지를 클라이언트측에 전달
			}
		}
		else send(client_sock[atoi(sendnum)], s.c_str(), strlen(s.c_str()), 0); //아니라면 한 명에게만 전송



		if (s == "[Server] >> <vote>")			//투표를 하는 경우
		{
			vote_cnt.assign(MAX, 0);			//vote_cnt를 0으로 초기화(재할당)->다시 투표 위해 이전 기록 지움

			char buf[PACKET_SIZE];

			ZeroMemory(buf, PACKET_SIZE);		//buf 초기화

			if (sw == Time::DAY)				//낮인 경우
			{
				for (int i = 0; i < MAX; i++)
				{
					if (v[i] == 1)				//살아 있는 사람의 경우
					{
						//recv(소켓, 수신 정보를 담을 배열주소, 그 배열의 크기, flag)
						//recv 함수는 대상 소켓으로부터 보내온 정보를 받아주는 역할
						//보내준 데이터가 없다면 여기에서 받을때까지 계속 대기
						//flag 값으로는 flag를 활성화시키지 않을것이기에 0을 지정

						 if(recv(client_sock[i], buf, PACKET_SIZE, 0) == -1) //클라이언트 종료 감지
							break;
						//recv(client_sock[i], buf, PACKET_SIZE, 0);

						std::cout << "\nClient #" << i << "의 투표 결과 << " << buf << "\n";

						vote_cnt[std::stoi(buf)]++;						//투표된 client의 번호를 1씩 증가!
												
						//if (i != client_num)
						//{
						//	std::string str;							//어떤 client가 말한 것인지를 추가!
						//	str = "[Client #" + std::to_string(i) + "] >> " + buf;

						//	send(client_sock[i], str.c_str(), strlen(str.c_str()), 0);	//서버가 메세지를 클라이언트측에 전달
						//}
					}
				}

				int r = 0;									//투표수가 같은 것이 있는 경우 1로 바꿈
				char res[PACKET_SIZE];						//client에게 보낼 문자열 결과
				int vote_max = -1;							//최대로 받은 투표수
				int vote_client;							//vote_max일 때의 client의 번호

				for (int i = 0; i < MAX; i++)				//투표수를 비교
				{
					for (int j = i + 1; j < MAX; j++)
					{
						if (vote_cnt[i] == vote_cnt[j])		//투표수가 같은 것이 있다면
						{
							sprintf(res, "아무도 퇴출되지 않았습니다.\n이제 밤입니다.\n\n");
							//send(client_sock[i], res, strlen(res), 0);
							r = 1;
							break;
						}

						if (vote_max < v[i])				//가장 많은 투표수와 그 client 번호 정하기
						{
							vote_max = v[i];
							vote_client = i;
						}
					}

					if (r)									//투표수가 같은 것이 있다면
						break;
				}

				if (!r)										//투표수가 같은 것이 없다면
				{
					sprintf(res, "Client[%d]가 퇴출되었습니다.\n이제 밤입니다.\n\n", vote_client);
					v[vote_client] = -1;					//퇴출되면 -1로 전환!
				}

				for (int i = 0; i < MAX; i++)
				{
					send(client_sock[i], res, strlen(res), 0);	//투표 결과 알림
				}

				sw = Time::NIGHT;							//낮의 투표가 끝났으므로 밤으로 전환!
			}
			else if (sw == Time::NIGHT)						//밤인 경우
			{
				for (int i = 0; i < MAX; i++)
				{
					if (v[i] == 1 && role[i] == Role::MAFIA)	//살아있는 마피아인 경우
					{
						//recv(소켓, 수신 정보를 담을 배열주소, 그 배열의 크기, flag)
						//recv 함수는 대상 소켓으로부터 보내온 정보를 받아주는 역할
						//보내준 데이터가 없다면 여기에서 받을때까지 계속 대기
						//flag 값으로는 flag를 활성화시키지 않을것이기에 0을 지정

						if (recv(client_sock[i], buf, PACKET_SIZE, 0) == -1) //클라이언트 종료 감지
							break;


						std::cout << "\nClient #" << i << "의 투표 결과 << " << buf << "\n";

						v[std::stoi(buf)]++;								//투표된 client의 번호를 1씩 증가!

						//if (i != client_num)
						//{
						//	std::string str;							//어떤 client가 말한 것인지를 추가!
						//	str = "[Client #" + std::to_string(i) + "] >> " + buf;

						//	send(client_sock[i], str.c_str(), strlen(str.c_str()), 0);	//서버가 메세지를 클라이언트측에 전달
						//}
					}
				}

				int r = 0;									//투표수가 같은 것이 있는 경우 1로 바꿈
				char res[PACKET_SIZE];						//client에게 보낼 문자열 결과
				int vote_max = -1;							//최대로 받은 투표수
				int vote_client;							//vote_max일 때의 client의 번호

				for (int i = 0; i < MAX; i++)				//투표수를 비교
				{
					for (int j = i + 1; j < MAX; j++)
					{
						if (vote_cnt[i] == vote_cnt[j])					//투표수가 같은 것이 있다면
						{
							sprintf(res, "아무도 퇴출되지 않았습니다.\n이제 밤입니다.\n\n");
							//send(client_sock[i], res, strlen(res), 0);
							r = 1;
							break;
						}

						if (vote_max < v[i])				//가장 많은 투표수와 그 client 번호 정하기
						{
							vote_max = v[i];
							vote_client = i;
						}
					}

					if (r)									//투표수가 같은 것이 있다면
						break;
				}

				if (!r)										//투표수가 같은 것이 없다면
				{
					sprintf(res, "Client[%d]가 퇴출되었습니다.\n이제 밤입니다.\n\n", vote_client);
					v[vote_client] = -1;					//퇴출되면 -1로 전환!
				}

				for (int i = 0; i < MAX; i++)
				{
					send(client_sock[i], res, strlen(res), 0);	//투표 결과 알림
				}

				sw = Time::DAY;							//마피아의 투표가 끝났으므로 낮으로 전환!
			}
		}




	}

	for (int i = 0; i < MAX; i++)				//for문으로 종료 소켓 늘리기
		closesocket(client_sock[i]);			

	closesocket(skt);

	WSACleanup();								//WSAStartup 을 하면서 지정한 내용을 지움

	return;
}

int main()
{
	int PORT;												//포트
	std::cout << "포트설정 >> ";
	std::cin >> PORT;										//포트 설정
	std::cout << "클라이언트 최대수용 수 설정 >> ";
	std::cin >> MAX;										//최대 수용 수 설정

	//동적할당
	client_sock = new SOCKET[MAX];
	client = new SOCKADDR_IN[MAX];
	client_size = new int[MAX];


	//메모리 비우기
	ZeroMemory(client_sock, sizeof(client_sock));
	ZeroMemory(client, sizeof(client));
	ZeroMemory(client_size, sizeof(client_size));

	openSocket(PORT);										//소켓열기

	delete[] client_sock, client, client_size;				//동정할당 해제
	return 0;
}