#include "MyThread.h"

void MyThread::run(void)
{
    // [EN] Variables
    int iResult = 0;
    int bufLen = 1024;
    int bytes = 0;
    unsigned short messageSize = 0;
    unsigned short messageOrFile = 0;
    unsigned short groupOrPrivate = 0;
    std::string fromWhoId;
    std::string toWhomId;
    char recvBuf[1024] = " ";
    char helpBuf[1024] = " ";
    char sendBuf[1024] = " ";
    char holder;
    bool informationIsRead = false;

	while (true)
	{
        // [EN] Receive
        iResult = recv(*mySocket, recvBuf, bufLen, 0);
        if (iResult <= 0) break;

        std::cout << "We got sent " << iResult - 1 << " bytes" << std::endl;
        std::cout << "Message: [" << recvBuf << "]" << std::endl;

        // [EN] if it's a userId init message (first)
        if (!init)
        {
            for (int i = 0; i < iResult - 1; ++i)
            {
                userId.push_back(recvBuf[bytes++]);
            }
            if (userId.size() == 2)
            {
                std::cout << "User identified by ID: " << userId << std::endl;
                init = true;
                bytes = 0;
            }
            continue;
        }

        if (bytes < 10)
        {
            for (int i = 0; i < iResult - 1; ++i)
            {
                holder = recvBuf[i];

                // [EN] message length
                if (i == 0) messageSize += atoi(&holder) * 1000;
                if (i == 1) messageSize += atoi(&holder) * 100;
                if (i == 2) messageSize += atoi(&holder) * 10;
                if (i == 3) messageSize += atoi(&holder);

                // [EN] message or file
                if (i == 4) messageOrFile = atoi(&holder);

                // [EN] group or private
                if (i == 5) groupOrPrivate = atoi(&holder);

                // [EN] from who id
                if (i == 6) fromWhoId.push_back(holder);
                if (i == 7) fromWhoId.push_back(holder);

                // [EN] to whom id
                if (i == 8) toWhomId.push_back(holder);
                if (i == 9) toWhomId.push_back(holder);
            }
        }

        bytes += iResult - 1;

        if (bytes > 9 && !informationIsRead)
        {
            strcpy_s(helpBuf, recvBuf);
            informationIsRead = true;
        }
        else if (bytes > 9 && informationIsRead)
        {
            strcat_s(helpBuf, recvBuf);
        }

        std::cout << "bytes received: " << bytes << std::endl;
        std::cout << "message length: " << messageSize << std::endl;

        if (messageSize + 10 == bytes)
        {
            std::cout << "(final) help buffer is [" << helpBuf << "]" << std::endl;

            // [EN] entering critical section
            EnterCriticalSection(myCritSect);

            // [EN] Sending to everyone
            for (int i = 0; i < myThreadList->size(); ++i)
            {
                if (!myThreadList->at(i)->isExited())
                {
                    if (groupOrPrivate == 1)
                    {
                        if (messageOrFile == 0)
                        {
                            if (myThreadList->at(i)->userId == toWhomId ||
                                myThreadList->at(i)->userId == fromWhoId)
                            {
                                strcpy(sendBuf, helpBuf);
                                iResult = send(*myThreadList->at(i)->mySocket, sendBuf, strlen(sendBuf) + 1, 0);
                                std::cout << "We sent: [" << sendBuf << "] to: [" << myThreadList->at(i)->userId << "]" << std::endl;
                            }
                        }
                        else
                        {
                            if (myThreadList->at(i)->userId == toWhomId)
                            {
                                strcpy(sendBuf, helpBuf);
                                iResult = send(*myThreadList->at(i)->mySocket, sendBuf, strlen(sendBuf) + 1, 0);
                                std::cout << "We sent: [" << sendBuf << "] to: [" << myThreadList->at(i)->userId << "]" << std::endl;
                            }
                        }
                    }
                    else
                    {
                        for (int j = 0; j < myGroups->at(toWhomId).second.size(); ++j)
                        {
                            if (myThreadList->at(i)->userId == myGroups->at(toWhomId).second.at(j))
                            {
                                strcpy(sendBuf, helpBuf);
                                iResult = send(*myThreadList->at(i)->mySocket, sendBuf, strlen(sendBuf) + 1, 0);
                                std::cout << "We sent: [" << sendBuf << "] to: [" << myThreadList->at(i)->userId << "]" << std::endl;
                            }
                            
                        }
                    }
                }
                else
                {
                    MyThread* tthp = myThreadList->at(i);
                    myThreadList->erase(myThreadList->begin() + i--);
                    delete tthp;
                }
            }

            // [EN] leaving critical section
            LeaveCriticalSection(myCritSect);

            // [EN] clearing all the used variables
            messageSize = 0;
            informationIsRead = false;
            fromWhoId.clear();
            toWhomId.clear();
            bytes = 0;
        }

        strcpy_s(helpBuf, "");
	}
}

MyThread::MyThread(SOCKET* mySocket,
    std::vector<MyThread*>* myThreadList,
    CRITICAL_SECTION* myCritSect,
    std::map<std::string, std::string>* myUsers,
    std::map<std::string, std::pair<std::string, std::vector<std::string>>>* myGroups)
{
	this->mySocket = mySocket;
	this->myCritSect = myCritSect;
	this->myThreadList = myThreadList;
    this->myUsers = myUsers;
    this->myGroups = myGroups;
}
