// Bully.cpp : Defines the entry point for the console application.
//Server

#include "stdafx.h"
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")
#include <ws2tcpip.h>
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <conio.h>
# define Port_num "5000"
# define interface_Port_num "5001"

# define Recv_Time_Out 8
# define Client_Time_Out 15

unsigned int* Adds;
int allocated_size = 0;
SOCKET InterFaceconnect;
HANDLE InterFaceProcess;
SOCKET *connections;
HANDLE *Procs;
unsigned int* src;
unsigned int Inter_src;
char * a = new char[16];
int inter_time = 10;
struct message {
	int src_ID;
	int des_ID;
	int Type;
	int value;
};
char* intTochar(unsigned int x) {
	char* b = new char[4];
	unsigned int j = 255;
	for (int i = 0; i < 4; i++) {
		unsigned int c = x >> (8 * i);
		c = c&j;
		b[i] = char(c);
	}
	return b;
}
char* messageTochar(unsigned int src, unsigned int des, unsigned int type, unsigned int val)
{
	char*Csrc = intTochar(src);
	char*Cdes = intTochar(des);
	char*Ctype = intTochar(type);
	char*Cval = intTochar(val);
	int l = 0;
	for (int i = 0; i < 16; i++)
	{
		if (i % 4 == 0&&i!=0)
			l++;
		if (l == 0)
			a[i] = Csrc[i % 4];
		else if (l == 1)
			a[i] = Cdes[i % 4];
		else if (l == 2)
			a[i] = Ctype[i % 4];
		else
			a[i] = Cval[i % 4];
	}
	delete(Csrc), delete(Cdes), delete(Ctype), delete(Cval);
	return a;
}

unsigned int charToint(char*a, int index)
{
	unsigned int x = 0;
	for (int i = index + 3; i >= index; i--) {
		x |= (unsigned int)((a[i]) & 255);
		if (i != index)
			x = x << (8);
	}
	return x;
}

void DeleteFromData(int curID, int *num_of_Pros)
{
	TerminateProcess(Procs[curID], 0);
	closesocket(connections[curID]);
	Procs[curID] = Procs[(*num_of_Pros) - 1];
	src[curID] = src[(*num_of_Pros) - 1];
	Procs[curID] = Procs[(*num_of_Pros) - 1];
	connections[curID] = connections[(*num_of_Pros) - 1];
	Adds[curID] = Adds[--(*num_of_Pros)];
}

SOCKET CreateListener(bool Port_number) 
{
	addrinfo Address;
	addrinfo *result = NULL;
	ZeroMemory(&Address, sizeof(Address));
	Address.ai_family = AF_INET;
	Address.ai_socktype = SOCK_STREAM;
	Address.ai_protocol = IPPROTO_TCP;
	int out;
	 out = getaddrinfo(NULL, Port_num, &Address, &result);
	if (out != 0) {
		printf("Address failed to be Set: %d\n", out);
		exit(0);
	}
	SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(0);
	}

	// Setup the TCP listening socket
	out = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	printf("%x", result->ai_addr);
	if (out == SOCKET_ERROR) {
		printf("bind failed error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		exit(0);
	}
	freeaddrinfo(result);
	out = listen(ListenSocket, 64);
	if (out == SOCKET_ERROR) {
		printf("listen failed error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(0);
	}
	return ListenSocket;
}

int main(int argc, TCHAR *argv[])
{
	WSADATA wsaData;
	int out=0;
	int num_of_Pros;
	std::cin >> num_of_Pros;
	// Initialize Winsock
	out=WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (out != 0) {
		printf("Winsock Intialization failed: %d\n", out);
		return 1;
	}
	SOCKET ListenSocket = CreateListener(1);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	connections = new SOCKET[num_of_Pros];
    Procs = new HANDLE[num_of_Pros];
	Adds = new unsigned int[num_of_Pros];
	src = new unsigned int[num_of_Pros];

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	for (int i = 0; i <= num_of_Pros; i++) {
		CreateProcess(TEXT("Client\\Debug\\Client.exe"), NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL,  &si, &pi);
		SOCKET connection = accept(ListenSocket, (sockaddr*)NULL, NULL);
		int RecvTime = Recv_Time_Out;
		int ClientTime = Client_Time_Out;
		setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (const char*)&RecvTime, sizeof(RecvTime));
		setsockopt(connection, SOL_SOCKET, SO_SNDTIMEO, (const char*)&ClientTime, sizeof(ClientTime));
		if (connection == INVALID_SOCKET) {
			printf("accept failed error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		else
		{

			char* a=new char[16];
			recv(connection, a, 16, 0);
			int srcAdd = charToint(a, 0);
			if (i == num_of_Pros) {
				setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (const char*)&inter_time, sizeof(inter_time));
				printf("Connection Success id is %d\n", 0);
				a = messageTochar(0, 0, 2, 0);
				sendto(connection, a, 16, 0, (const sockaddr*)srcAdd, sizeof(srcAdd));
				InterFaceconnect = connection;
				InterFaceProcess = pi.hProcess;
				Inter_src = srcAdd;
			}
			else {
				printf("Connection Success id is %d\n", i + 1);
				Adds[i] = i + 1;
				printf("Address of id is %x\n", Adds[i]);
				int TypeofNode = 0;
				if (i == num_of_Pros - 1)
					TypeofNode = 1;
				a = messageTochar(0, 0, TypeofNode, Adds[i]);
				sendto(connection, a, 16, 0,(const sockaddr*)srcAdd,sizeof(srcAdd));
				src[i] = srcAdd;
				connections[i] = connection;
				Procs[i] = pi.hProcess;
			}
		}
	}
	unsigned int CoordinatorID = num_of_Pros-1 ;
	int curID=0;
	int eleProcess = -1;
	int minInd = -1;
	bool ka = 0;
	while (1) 
	{
		int size = sizeof(Inter_src);
		if (recvfrom(InterFaceconnect, a, 16, 0, ( sockaddr*)Inter_src,&size) != -1)
		{
			int type = charToint(a, 8);
			if (type == 0) {
				int val = charToint(a, 12);
				printf("%d entered \n", val);
				bool b = 0;
				for (int i = 0; i < num_of_Pros; i++)
				{
					if (Adds[i] == val)
					{
						TerminateProcess(Procs[i], 0);
						b = 1;
					}
				}
				if (!b&&num_of_Pros != allocated_size)
				{
					CreateProcess(TEXT("Client\\Debug\\Client.exe"), NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
					SOCKET connection = accept(ListenSocket, (sockaddr*)NULL, NULL);
					int *RecvTime = new int(Recv_Time_Out);
					int *ClientTime =new int( 1000);
					setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (const char*)RecvTime, sizeof(RecvTime));
					setsockopt(connection, SOL_SOCKET, SO_SNDTIMEO, (const char*)&ClientTime, sizeof(ClientTime));
					recv(connection, a, 16, 0);
					int srcAdd = charToint(a, 0);
					printf("Connection Success id is %d\n", val);
					Adds[num_of_Pros] = val;
					src[num_of_Pros] = srcAdd;
					a = messageTochar(0, 0, 0, Adds[num_of_Pros]);
					sendto(connection, a, 16, 0,(const sockaddr*)srcAdd,sizeof(srcAdd));
					connections[num_of_Pros] = connection;
					Procs[num_of_Pros++] = pi.hProcess;
				}


			}
		}
		if (CoordinatorID == -1) 
		{
			bool b1 = 0;
			unsigned int min=MAXINT;
			minInd = -1;
			for (int i = 0; i < num_of_Pros; i++) 
			{
				if (Adds[eleProcess] < Adds[i]) 
				{
						if (min > Adds[i]) 
						{
							min = Adds[i];
							minInd = i;
						}	
				}
			}
			if (minInd==-1) {
				printf("%d ", eleProcess);
					CoordinatorID = eleProcess;
					a = messageTochar(0, 0, 2, Adds[eleProcess]);
					out=sendto(connections[eleProcess], a, 16, 0, (const sockaddr*)src[eleProcess], sizeof(src[eleProcess]));
			}
			else
			{
				eleProcess = minInd;
				a = messageTochar(0, 0, 1, Adds[eleProcess]);
				sendto(connections[eleProcess], a, 16, 0, (const sockaddr*)src[eleProcess], sizeof(src[eleProcess]));
			}
			

		}
	   else 
	   {
		   if (curID == CoordinatorID) {
			   curID++;
			   if (curID == num_of_Pros)
				   curID = 0;
		   }
		   size = sizeof(src[CoordinatorID]);
		   int out=recvfrom(connections[CoordinatorID], a, 16, 0,(sockaddr*)src[CoordinatorID],&size);
		   if (out == -1|| Adds[curID]>Adds[CoordinatorID])
		   {
			   if (out == -1) 
			   {
				   CoordinatorID = -1;
				   eleProcess = curID;
			   }
			   else
			   {
				   a = messageTochar(0, 0, 0, 0);
				   size = sizeof(src[CoordinatorID]);
				   out = sendto(connections[CoordinatorID], a, 16, 0, (const sockaddr*)src[CoordinatorID], size);
			   }

			 
		   }
		   else 
		   {
			   a = messageTochar(Adds[curID], Adds[CoordinatorID], 1, 0);
			   out=sendto(connections[CoordinatorID], a, 16, 0, (const sockaddr*)src[CoordinatorID], size);
			   if (out == -1 )
			   {
				   CoordinatorID = -1;
				   eleProcess = curID;
			   }
			   else
			   {
				   size = sizeof(src[curID]);
				   a = messageTochar(Adds[CoordinatorID], Adds[curID], 0, 0);
				   out = sendto(connections[curID], a, 16, 0, (const sockaddr*)src[curID], size);
				   if (out == -1) {
					   DeleteFromData(curID, &num_of_Pros);
				   }
				   else
				   {
					   curID++;
				   }
			   }

		   }

		   if (curID == num_of_Pros)
			   curID = 0;

	   }
	}


	printf("done");
	return 0;
}

