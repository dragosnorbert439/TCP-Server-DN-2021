#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

#include "winsock2.h"
#include "MyThread.h"
#include "utils.h"

#pragma comment(lib, "ws2_32.lib")

void main()
{
	// [EN] Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		std::cout << "Error at WSAStartup()." << std::endl; return;
	}

	//----------------------------------------------------------------------------------------
	// [EN] Create a SOCKET for listening for incoming connection requests.
	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
		WSACleanup(); return;
	}

	//----------------------------------------------------------------------------------------
	// [EN]	The sockaddr_in structure specifies the address family, IP address,
	//		and port for the socket that is being bound.
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr("127.0.0.1");
	service.sin_port = htons(13000);
	if (bind(ListenSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
	{
		std::cout << "bind() failed." << std::endl;
		closesocket(ListenSocket); WSACleanup(); return;
	}

	//----------------------------------------------------------------------------------------
	// [EN] Listen for incoming connection requests on the created socket
	if (listen(ListenSocket, 1) == SOCKET_ERROR)
	{
		std::cout << "Error listening on socket" << std::endl;
		closesocket(ListenSocket); WSACleanup(); return;
	}
	
	//----------------------------------------------------------------------------------------
	// [EN] Initial MyThread list and CRITICAL_SECTION
	std::vector<MyThread*> myThreadList;
	CRITICAL_SECTION myCritSect;
	InitializeCriticalSection(&myCritSect);

	//----------------------------------------------------------------------------------------
	// [EN] Initial users ID/NAME map
	std::map<std::string, std::string> users;
	std::ifstream fin(FILE_NAME);
	if (!fin.is_open()) { std::cout << "Could not open users id-name file." << std::endl; return; }

	std::string line;
	std::vector<std::string> holder;
	while (std::getline(fin, line))
	{
		std::stringstream ss(line);
		while (ss.good())
		{
			std::string substr;
			getline(ss, substr, ',');
			holder.push_back(substr);
		}

		users.insert({holder[0], holder[1]});
		holder.clear();
	}
	fin.close();

	/*
	// [EN] for testing
	for (std::map<std::string, std::string>::const_iterator it = users.begin(); it != users.end(); ++it)
	{
		std::cout << it->first << " " << it->second << std::endl;
	}
	*/

	//----------------------------------------------------------------------------------------
	// [EN] Initial groups ID/NAME/userIds map
	std::map<std::string, std::pair<std::string, std::vector<std::string>>> groups;
	std::vector<std::string> userIds;
	fin.open(FILE_NAME_GROUPS);
	if (!fin.is_open()) { std::cout << "Could not open groups file." << std::endl; return; }

	while (std::getline(fin, line))
	{
		std::stringstream ss(line);
		while (ss.good())
		{
			std::string substr;
			getline(ss, substr, ',');
			holder.push_back(substr);
		}

		for (int i = 2; i < holder.size(); ++i)
		{
			userIds.push_back(holder.at(i));
		}

		groups.insert({ holder[0], { holder[1], userIds } });
		userIds.clear();
		holder.clear();
	}

	fin.close();

	/*
	// [EN] for testing
	for (std::map<std::string, std::pair<std::string, std::vector<std::string>>>::const_iterator it = groups.begin();
		it != groups.end(); ++it)
	{
		std::cout << it->first << " " << it->second.first << ":" << std::endl;
		for (int i = 0; i < it->second.second.size(); ++i)
		{
			std::cout << it->second.second.at(i) << " ";
		}
		std::cout << std::endl;
	}
	*/

	while (true)
	{ 
		//------------------------------------------------------------------------------------
		// [EN] Create a SOCKET for accepting incoming requests.
		SOCKET* AcceptSocket = new SOCKET();
		std::cout << "Waiting for client to connect..." << std::endl;

		//------------------------------------------------------------------------------------
		// [EN] Accept the connection.
		*AcceptSocket = accept(ListenSocket, NULL, NULL);
		if (*AcceptSocket == INVALID_SOCKET)
		{
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return;
		}
		else std::cout << "Client connected." << std::endl;
	
		//------------------------------------------------------------------------------------
		// [EN] New thread
		MyThread* th = new MyThread(AcceptSocket, &myThreadList, &myCritSect, &users, &groups);
		EnterCriticalSection(&myCritSect);
		myThreadList.push_back(th);
		LeaveCriticalSection(&myCritSect);
		th->start();
	}

	//----------------------------------------------------------------------------------------
	// [EN] When the application is finished sending, close the socket
	std::cout << "Finished sending. Closing socket." << std::endl;
	closesocket(ListenSocket);

	//----------------------------------------------------------------------------------------
	// [EN] Clean up and quit
	std::cout << "Exiting." << std::endl;
	WSACleanup(); return;
}