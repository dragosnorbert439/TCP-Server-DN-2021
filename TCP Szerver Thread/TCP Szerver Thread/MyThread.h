#ifndef MYTHREAD_H
#define MYTHREAD_H
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <string.h>
#include <map>

#include "SysThread.h"

class MyThread : public SysThread
{
private:
	SOCKET* mySocket;
	std::vector<MyThread*>* myThreadList;
	CRITICAL_SECTION* myCritSect;
	std::map<std::string, std::string>* myUsers;
	std::map<std::string, std::pair<std::string, std::vector<std::string>>>* myGroups;

	std::string userId;
	bool init{ false };

	virtual void run(void) override;

public:
	MyThread(SOCKET* mySocket,
		std::vector<MyThread*>* myThreadList,
		CRITICAL_SECTION* myCritSect,
		std::map<std::string, std::string>* myUsers,
		std::map<std::string, std::pair<std::string, std::vector<std::string>>>* myGroups);
};


#endif // !MYTHREAD_H
