// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")
#include <ws2tcpip.h>
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <cstdlib>
# define Port_num "5000"
# define Revc_Time_Out 1000
# define Server_Time_Out 1000
SOCKET ConnectSocket;
struct message {
	int src_ID;
	int des_ID;
	int Type;
	int value;
};
char* intTochar(unsigned int x) {
	char* a = new char[4];
	unsigned int j = 255;
	for (int i = 0; i < 4; i++) {
		unsigned int c = x >> (8*i);
		c = c&j;
		a[i] = char(c);
	}
	return a;
}
char* messageTochar(unsigned int src, unsigned int des, unsigned int type, unsigned int val)
{
	char*Csrc = intTochar(src);
	char*Cdes = intTochar(des);
	char*Ctype = intTochar(type);
	char*Cval = intTochar(val);

	char * a = new char[16];
	int l = 0;
	for (int i = 0; i < 16; i++) 
	{
		if (i % 4 == 0&&i!=0)
			l++;
		if (l == 0)
			a[i] = Csrc[i % 4];
		else if (l == 1)
			a[i] = Cdes[i % 4];
		else if (l==2)
			a[i] = Ctype[i % 4];
		else
			a[i] = Cval[i % 4];
	}
	delete(Csrc), delete(Cdes), delete(Ctype), delete(Cval);
	return a;
}

unsigned int charToint(char*a,int index) 
{
	unsigned int x=0;
	for (int i = index+3; i >= index; i--) {
		x |= (unsigned int) ((a[i])&255);
		if(i!=index)
		x = x << (8);
	}
	return x;
}
void Exiting() {

	closesocket(ConnectSocket);
	WSACleanup();

}

int main(int argc, char **argv)
{
	std::atexit(Exiting);
	WSADATA wsaData;
	int out = 0;

	// Initialize Winsock
	out = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (out != 0) {
		printf("Winsock Intialization failed: %d\n", out);
		return 1;
	}
	addrinfo Address;
	addrinfo *result = NULL;
	ZeroMemory(&Address, sizeof(Address));
	Address.ai_family = AF_INET;
	Address.ai_socktype = SOCK_STREAM;
	Address.ai_protocol = IPPROTO_TCP;
	out = getaddrinfo(NULL, Port_num, &Address, &result);
	if (out != 0) {
		printf("Address failed to be Set: %d\n", out);
		return 1;
	}
	SOCKET ConnectSocket;
	for (addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		out = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (out == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	unsigned int add = (unsigned int)result->ai_addr;
	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	else
		printf("Connection\n");
	printf("x is %x\n", add);
	char*a = messageTochar(add,0,0,0);
	send(ConnectSocket, a, 16, 0);
	recv(ConnectSocket, a, 16, 0);
	unsigned int ID = charToint(a, 12);
	unsigned int Type = charToint(a, 8);
	bool sp = 0;
	printf("ID is %d", ID);
	int status = 1;
	if (Type==1)
	{
		status = 0;
		printf(" I am the Coordinator\n");

	}
	if (Type == 2)
	{
		sp = 1;
		printf(" I am the Interface\n");

	}
	int Time=1000;
	bool b = 0;
	bool bow = 0;
	while(1)
	{
		if (sp) {
			int id;
			std::cin >> id;
			char*a = messageTochar(0, 0, 0, id);
			out = send(ConnectSocket, a, 16, 0);
		}
		else {
			if (status == 0)  // COORDINATOR
			{
				char*a = messageTochar(ID, 0, 0, 0);
				out = send(ConnectSocket, a, 16, 0);
				out= recv(ConnectSocket, a, 16, 0);
				if (charToint(a,8) == 0) 
				{
					status = 1;
				}
				printf("I am %d the Coordinator\n", ID);
			}
			if (status == 1)  //Non COORDINATOR
			{
				out = recv(ConnectSocket, a, 16, 0);
				if (out != -1) 
				{
					unsigned int type = charToint(a, 8);

					if (type == 1)
					{
						status = 2;
					}
					else
						printf("I am %d My Coordinator is :%d\n", ID, charToint(a, 0));
				}
			}
			if (status == 2) // election Process;
			{
				printf(" Election Process \n");
				out = recv(ConnectSocket, a, 16, 0);
				if (out != -1) 
				{
					int type = charToint(a, 8);
					if (type == 2) {
						status = 0;
					}
					else
					{
						unsigned int src = charToint(a, 0);
						printf("%d Sends Ok Message\n", (int)src);
						status = 1;

					}
				}
			}
		}
	}

    return 0;
}

