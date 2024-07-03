#ifndef CHAT
#define CHAT

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include<Windows.h>
#include<conio.h>
#else
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#endif

#include<iostream>
#include<string>
#include<chrono>
#include<thread>
#include<vector>
#include<deque>

#include"User.h"
#include"myCrypt.h"

#ifdef max // std::numeric_limits<std::streamsize>::max()
#undef max
#endif

#define PORT 50306

class Chat
{
	enum Screens
	{
		NOSCR,          // not set
		LOAD,           // scr_exit
		EXIT,           // scr_exit
		LOGIN,          // scr_login
		PUBLIC,         // scr_public
		WELLCOME,       // scr_welcome
		PRIVATE,        // scr_private
		PROFILE,        // scr_profile
		COMMANDS,       // scr_commands
		NEWACCOUNT,     // scr_newAccount
		MESSAG          // scr_message
	};

private:
	User current_user;
	Screens current_screen;
	Screens previos_screen;
	std::string salt{ "Y6sEgFOSx38RM8bHnSteSOsEjqZKZgDuLBY5AecmPFMT000bBlRBmxMCiYKN" }; // 30+ characters
	std::vector<std::string> Usernames;
	std::deque<std::string> public_msgArr;

#ifdef _WIN32
	WSADATA wsaData;
	SOCKET clientSocket;
	sockaddr_in serverAddr;
#else
	int clientSocket;
    struct sockaddr_in serverAddr;
#endif

	// screens
	void scr_load();
	void scr_exit();
	void scr_login();
	void scr_public();
	void scr_welcome();
	void scr_private();
	void scr_profile();
	void scr_commands();
	void scr_newAccount();
	void scr_message(Message& m);

	// common functions
	void clear_screen();
	void showHistory();
	void showMailbox();
	void showUsers();
	void clearLine();
	void cmdProcessing();
	void keyProcessing();
	void rememberMail(const std::string& msg, const std::string& author);
	void mySleep(int time = 120);
	void cmd_default(std::string& str);
	bool check_login(const std::string& login);
	bool strCmp_read(std::string& str, int& num);
	bool strCmp_pm(std::string& str_cmd, std::string& str_msg);

	// server functions
	void sv_timeout();
	bool sv_setConnection();
	void sv_updatePrivate();
	void sv_setMsgReaded(const int& pos);
	void sv_addUser(std::string& login, Hash& hash);
	void sv_closeConnection();
	void sv_updatePublic();
	void sv_sendMessage(std::string& reciever, std::string& message);
	bool sv_userExists(std::string& login); // true if exists
	bool sv_checkPass(std::string& login, Hash& hash);
	void sv_sendtask(const int& connection_descriptor, const std::string& username, const std::string& task);
	void sv_rememberMail(const std::string& str);
	void sv_updateUsernames();

	void sv_sendHash(const int& connection_descriptor, const Hash& hash);
	void sv_sendBool(const int& connection_descriptor , const bool& flag);
	void sv_sendInt(const int& connection_descriptor, const int& integer);
	void sv_sendStr(const int& connection_descriptor, const std::string& str);

	void sv_readHash(const int& connection_descriptor, Hash& hash);
	void sv_readBool(const int& connection_descriptor, bool& flag);
	void sv_readInt(const int& connection_descriptor, int& integer);
	void sv_readStr(const int& connection_descriptor, std::string& str);

public:
	Chat();
	Chat(Chat& other) = delete;
	Chat& operator=(Chat& other) = delete;

	void start();
};

#endif // CHAT