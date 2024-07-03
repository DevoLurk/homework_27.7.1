#ifndef CHAT
#define CHAT

#ifdef _WIN32
#include<WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include<Windows.h>
#include<conio.h>
#else
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#endif

#include<fstream>
#include<iostream>
#include<string>
#include<chrono>
#include<thread>
#include<sstream>

#include<sqlext.h>
#include<sqltypes.h>
#include<sql.h>

#include"myCrypt.h"
#include"Logger.h"

#define PORT 50306
#define SQL_TEXT_SIZE 65535

class Chat
{
private:
	Logger logger;
	std::string config_path{ "config/dbconnection.conf" };

	SQLHANDLE sqlConnHandle{ nullptr };
	SQLHANDLE sqlEnvHandle{ nullptr };
	SQLHANDLE sqlStmtHandle{ nullptr };
	SQLWCHAR retconstring[1024]{};

#ifdef _WIN32
	WSADATA wsaData;
	SOCKET serverSocket, clientSocket;
	sockaddr_in serverAddr, clientAddr;
	int clientAddrSize = sizeof(clientAddr);
#else
	int serverSocket, clientSocket;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t clientAddrSize = sizeof(clientAddr);
#endif

	// screens
	void scr_load();
	void scr_exit();
	void scr_work();

	// common commands
	void edit_config(const std::string& db_name);
	void clear_screen();
	void clearLine();
	void myCinClear();
	void rememberMail(std::string str);
	void mySleep(int time = 120);
	void StringReplacer(std::string& inputStr, const std::string& to_replace, const std::string& replace_with);
	std::string getTimeStamp(); // returns current time like -> YYYY-MM-DD HH:MM:SS
	std::string Hash_to_str(const Hash& hash_class);
	Hash arr_to_Hash(const unsigned char* hash_arr);

	// database
	void showSQLError(unsigned int handleType, const SQLHANDLE& handle);
	int db_getUserID(const std::string& user);
	int db_getMaxUserID();
	void db_disconnect();
	bool db_addUser(const Hash& user_hash, const std::string& username);
	bool db_getUserHash(Hash& server_hash, const std::string& username);
	bool db_getPrivateMsgCnt(int& cnt, const std::string& username);
	bool db_create();
	bool db_connect();
	bool db_getConf(std::wstring& config);
	bool db_setMsgReaded(const std::string& username, const int& poz);
	bool db_getUsersCount(int& users_count);
	bool db_addPrivateMsg(const std::string& sender, const std::string& reciever, const std::string& mesage);
	bool db_addPublicMsg(const std::string& sender, const std::string& mesage);
	bool db_userExists(const std::string& username);
	bool db_getPublicMsgCnt(int& cnt);

	// server functions
	void sv_timeout();
	void sv_create();
	void sv_closeConnection();
	void sv_gettask(const int& connection_descriptor, std::string& name, std::string& task);

	void sv_sendHash(const int& connection_descriptor, const Hash& hash);
	void sv_sendBool(const int& connection_descriptor, const bool& flag);
	void sv_sendInt(const int& connection_descriptor, const int& integer);
	void sv_sendStr(const int& connection_descriptor, const std::string& str);

	void sv_readHash(const int& connection_descriptor, Hash& hash);
	void sv_readBool(const int& connection_descriptor, bool& flag);
	void sv_readInt(const int& connection_descriptor, int& integer);
	void sv_readStr(const int& connection_descriptor, std::string& str);

public:
	Chat();
	Chat(std::string path);
	Chat(Chat& other) = delete;
	Chat& operator=(Chat& other) = delete;

	void start();
	void first_start(const std::string& db_name);
};

#endif // CHAT