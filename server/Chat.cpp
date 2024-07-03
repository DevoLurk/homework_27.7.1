#include "Chat.h"

Chat::Chat()
{
#ifdef __linux__ 
    setvbuf(stdout, NULL, _IONBF, 0);
#endif
}

void Chat::start()
{
    scr_load();
    scr_work();
}

void Chat::first_start(const std::string& db_name)
{
    edit_config(db_name);
    db_connect();

    if (!db_create())
    {
        printf("\n  Data base creation FALUE\n");
        mySleep(350);
        scr_exit();
    }
    else
        scr_work();
}

void Chat::edit_config(const std::string& db_name)
{
    std::ifstream config_in(config_path);
    std::string fileLines[6];

    if (!config_in.is_open())
        return;

    for (int i{ 0 }; i < 6; i++)
        std::getline(config_in, fileLines[i]);

    config_in.close();

    std::ofstream config_out(config_path);
    if (!config_out.is_open())
        return;

    fileLines[3] = "DATABASE=" + db_name + ";";
    for (int i{ 0 }; i < 6; i++)
        config_out << fileLines[i] << std::endl;

    config_out.close();
}

void Chat::clearLine()
{
    printf("\033[A");       // line up
    printf("\33[2K\r");     // clear line + move curor
}

void Chat::mySleep(int time)
{
    std::chrono::milliseconds timespan(time);
    std::this_thread::sleep_for(timespan);
}

void Chat::clear_screen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void Chat::StringReplacer(std::string& inputStr, const std::string& to_replace, const std::string& replace_with)
{
    size_t pos = inputStr.find(to_replace);
    inputStr.replace(pos, to_replace.size(), replace_with);
}

std::string Chat::getTimeStamp()
{
    char buf[20];
    struct tm mytime;

    std::time_t now = std::time(nullptr);
    localtime_s(&mytime, &now);

    strftime(buf, 20, "%Y-%m-%d %H:%M:%S", &mytime);

    std::string time(buf);
    return time;
}

std::string Chat::Hash_to_str(const Hash& hash_class)
{
    std::stringstream ss;
    ss << std::hex << hash_class._part1
        << hash_class._part2
        << hash_class._part3
        << hash_class._part4
        << hash_class._part5;

    return ss.str();
}

Hash Chat::arr_to_Hash(const unsigned char* hash_arr)
{
    Hash result;
    unsigned int newArr[5];

    for (int i = 0; i < 5; i++)
    {
        newArr[i] = (hash_arr[i * 4] << 24) |
            (hash_arr[i * 4 + 1] << 16) |
            (hash_arr[i * 4 + 2] << 8) |
            hash_arr[i * 4 + 3];
    }

    result._part1 = newArr[0];
    result._part2 = newArr[1];
    result._part3 = newArr[2];
    result._part4 = newArr[3];
    result._part5 = newArr[4];

    return result;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DATABASE

void Chat::showSQLError(unsigned int handleType, const SQLHANDLE& handle)
{
    SQLWCHAR SQLState[1024];
    SQLWCHAR message[1024];
    if (SQL_SUCCESS == SQLGetDiagRec(handleType, handle, 1, SQLState, NULL, message, 1024, NULL))
        std::wcout << "\nSQL driver message: " << message << "\nSQL state: " << SQLState << "." << std::endl;
}

bool Chat::db_create()
{
    std::wstring wcommand1 = LR"(
                                create table users(
	                                id integer auto_increment primary key,
	                                login varchar(30) unique
                                );
                                )";
    std::wstring wcommand2 = LR"(
                                create table pass(
	                                id_user integer,
                                    hash BINARY(20) not null,
	                                FOREIGN key (id_user) references users(id)
                                );
                                )";

    std::wstring wcommand3 = LR"(
                                create table public_msg(
	                                id integer auto_increment primary key,
	                                message text,
	                                id_sender integer,
	                                FOREIGN key (id_sender) references users(id)
                                );
                                )";

    std::wstring wcommand4 = LR"(
                                create table private_msg(
                                    id integer auto_increment primary key,
	                                id_sender integer,
	                                id_reciever integer,
	                                message text,
	                                flag bool,
	                                FOREIGN key (id_sender) references users(id),
	                                FOREIGN key (id_reciever) references users(id)
                                );
                                )";


    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand1.c_str(), SQL_NTS))
        return false;
    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand2.c_str(), SQL_NTS))
        return false;
    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand3.c_str(), SQL_NTS))
        return false;
    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand4.c_str(), SQL_NTS))
        return false;

    return true;
}

bool Chat::db_connect()
{
    std::wstring config;

    if (!db_getConf(config))
        return false;

    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvHandle))
    {
        db_disconnect();
        return false;
    }

    if (SQL_SUCCESS != SQLSetEnvAttr(sqlEnvHandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
    {
        db_disconnect();
        return false;
    }

    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvHandle, &sqlConnHandle))
    {
        db_disconnect();
        return false;
    }

    switch (SQLDriverConnect(sqlConnHandle,
        GetDesktopWindow(),
        (SQLWCHAR*)config.c_str(),
        SQL_NTS,
        retconstring,
        1024,
        NULL,
        SQL_DRIVER_COMPLETE))
    {
    case SQL_INVALID_HANDLE:
        showSQLError(SQL_HANDLE_DBC, sqlConnHandle);
        break;
    case SQL_ERROR:
        showSQLError(SQL_HANDLE_DBC, sqlConnHandle);
        db_disconnect();
        return false;
    default:
        break;
    }

    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle))
        return false;

    return true;
}

void Chat::db_disconnect()
{
    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
    SQLDisconnect(sqlConnHandle);
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnHandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvHandle);
}

bool Chat::db_getConf(std::wstring& config)
{
    std::ifstream conf_in(config_path);
    if (!conf_in.is_open())
        return false;

    std::string conf, buff;
    for (int i{ 0 }; i < 6; i++)
    {
        std::getline(conf_in, buff);
        conf += buff;
    }
    conf_in.close();

    std::wstring wconf(conf.begin(), conf.end());
    config = wconf;
    return true;

    // config should be like ->
    /*
    DRIVER={MySQL ODBC 8.4 ANSI Driver};
    SERVER=localhost;
    PORT=3306;
    DATABASE=testdb;
    UID=root;
    PWD=root;
    */
}

bool Chat::db_setMsgReaded(const std::string& username, const int& poz)
{
    std::string command = R"(
                update private_msg
                set flag = true
                where private_msg.id =
                    (
                        select msg_for_user.id
                        from(
                                select ROW_NUMBER() over(order by id_sender) as total, private_msg.*
                                from private_msg
                                join(
                                    select id
                                    from users
                                    where login = '--username--'
                                    ) as userid
                                    on userid.id = private_msg.id_reciever
                                ) as msg_for_user
                        where msg_for_user.total = --msgpoz--
                    );
                )";
    // --username-- replace with userName (string)
    // --msgpoz-- replace with poz (int)

    StringReplacer(command, "--username--", username);
    StringReplacer(command, "--msgpoz--", std::to_string(poz));
    std::wstring wcommand(command.begin(), command.end());

    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand.c_str(), SQL_NTS))
        return false;
    else
        return true;
}

bool Chat::db_getUsersCount(int& users_count)
{
    SQLINTEGER users_cnt;
    std::string command = R"(
                            select count(id)
                            from users;
                            )";
    std::wstring wcommand(command.begin(), command.end());

    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand.c_str(), SQL_NTS))
        return false;

    SQLBindCol(sqlStmtHandle, 1, SQL_INTEGER, &users_cnt, sizeof(users_cnt), NULL);
    SQLFetch(sqlStmtHandle);

    users_count = (int)users_cnt;
    return true;
}

int Chat::db_getUserID(const std::string& user)
{
    SQLINTEGER users_id;
    std::string command = R"(
                            select id
                            from users
                            where login = '--username--';
                            )";

    StringReplacer(command, "--username--", user);
    std::wstring wcommand(command.begin(), command.end());

    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand.c_str(), SQL_NTS))
        printf("\033[31m  ERROR db_getUserID()\033[0m");

    SQLBindCol(sqlStmtHandle, 1, SQL_INTEGER, &users_id, sizeof(users_id), NULL);
    SQLFetch(sqlStmtHandle);

    return (int)users_id;
}

bool Chat::db_addPrivateMsg(const std::string& sender, const std::string& reciever, const std::string& mesage)
{
    std::string command = R"(
                            insert into private_msg(id_sender, id_reciever, message, flag) 
                                        values(--user_1_id--, --user_2_id--, '--message--', false);
                            )";

    StringReplacer(command, "--user_1_id--", std::to_string(db_getUserID(sender)));
    StringReplacer(command, "--user_2_id--", std::to_string(db_getUserID(reciever)));
    StringReplacer(command, "--message--", mesage);
    std::wstring wcommand(command.begin(), command.end());

    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand.c_str(), SQL_NTS))
        return false;
    else
        return true;
}

bool Chat::db_addPublicMsg(const std::string& sender, const std::string& mesage)
{
    std::string command = R"(
                            insert into public_msg(message, id_sender)
                                        values('--message--', --sender--);
                            )";

    StringReplacer(command, "--message--", mesage);
    StringReplacer(command, "--sender--", std::to_string(db_getUserID(sender)));
    std::wstring wcommand(command.begin(), command.end());

    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand.c_str(), SQL_NTS))
        return false;
    else
        return true;
}

bool Chat::db_userExists(const std::string& username)
{
    SQLINTEGER users_cnt;
    std::string command = R"(
                            select count(id)
                            from users
                            where login = '--username--';
                            )";

    StringReplacer(command, "--username--", username);
    std::wstring wcommand(command.begin(), command.end());

    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand.c_str(), SQL_NTS))
    {
        printf("\033[31m  ERROR db_userExists()\033[0m");
        return false;
    }

    SQLBindCol(sqlStmtHandle, 1, SQL_INTEGER, &users_cnt, sizeof(users_cnt), NULL);
    SQLFetch(sqlStmtHandle);

    printf("\033[32m  DONE\033[0m");

    if ((int)users_cnt == 0)
        return false;
    else
        return true;
}

bool Chat::db_getPublicMsgCnt(int& cnt)
{
    SQLINTEGER messages_cnt;
    std::string command = R"(
                            select count(id)
                            from public_msg;
                            )";
    std::wstring wcommand(command.begin(), command.end());

    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand.c_str(), SQL_NTS))
        return false;

    SQLBindCol(sqlStmtHandle, 1, SQL_INTEGER, &messages_cnt, sizeof(messages_cnt), NULL);
    SQLFetch(sqlStmtHandle);

    cnt = (int)messages_cnt;
    return true;
}

bool Chat::db_getPrivateMsgCnt(int& cnt, const std::string& username)
{
    SQLINTEGER messages_cnt;
    std::string command = R"(
                             select count(private_msg.id)
                             from private_msg
                             join (
		                            select id
		                            from users
		                            where login = '--username--'
	                              ) as userid
                                  on userid.id = private_msg.id_reciever;
                            )";
    StringReplacer(command, "--username--", username);
    std::wstring wcommand(command.begin(), command.end());

    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand.c_str(), SQL_NTS))
        return false;

    SQLBindCol(sqlStmtHandle, 1, SQL_INTEGER, &messages_cnt, sizeof(messages_cnt), NULL);
    SQLFetch(sqlStmtHandle);

    cnt = (int)messages_cnt;
    return true;
}

bool Chat::db_getUserHash(Hash& server_hash, const std::string& username)
{
    SQLCHAR hash[20]; // SQLCHAR is unsigned char
    std::string command = R"(
                            with userid as
                            (
		                        select id
		                        from users
		                        where login = '--username--'
                            )
                            select pass.hash
                            from pass
                            join userid on pass.id_user = userid.id
                            )";
    StringReplacer(command, "--username--", username);
    std::wstring wcommand(command.begin(), command.end());

    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand.c_str(), SQL_NTS))
        return false;

    SQLBindCol(sqlStmtHandle, 1, SQL_C_BINARY, &hash, sizeof(hash), NULL);
    SQLFetch(sqlStmtHandle);

    server_hash = arr_to_Hash(hash);
    return true;
}

bool Chat::db_addUser(const Hash& user_hash, const std::string& username)
{
    std::string userhash = Hash_to_str(user_hash);

    std::string command1 = "insert into users(login) values('--username--');";
    std::string command2 = "insert into pass(id_user, hash) values(--usernid--, 0x--usergash--);";
    StringReplacer(command1, "--username--", username);
    StringReplacer(command2, "--usernid--", std::to_string(db_getMaxUserID() + 1));
    StringReplacer(command2, "--usergash--", userhash);
    std::wstring wcommand1(command1.begin(), command1.end());
    std::wstring wcommand2(command2.begin(), command2.end());

    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand1.c_str(), SQL_NTS))
        return false;

    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand2.c_str(), SQL_NTS))
        return false;

    return true;
}

int Chat::db_getMaxUserID()
{
    SQLINTEGER user_max_id;
    std::wstring wcommand = LR"(
                            select count(id)
                            from users;
                            )";

    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand.c_str(), SQL_NTS))
    {
        printf("\033[31m  ERROR db_getUserID()\033[0m");
        return -1;
    }

    SQLBindCol(sqlStmtHandle, 1, SQL_INTEGER, &user_max_id, sizeof(user_max_id), NULL);
    SQLFetch(sqlStmtHandle);

    return (int)user_max_id;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SCREENS

void Chat::scr_exit()
{
    clear_screen();
    printf("\n\n\n");
    printf("\t\t\033[36m     Exiting");

    for (auto i{ 0 }; i < 3; ++i)
    {
        mySleep();
        printf(".");
    }

    sv_closeConnection();
    db_disconnect();
    printf("\n\n\n\n\n\n\033[0m");
    exit(0);
}

void Chat::scr_load()
{
    clear_screen();
    printf("\n\n\n");
    printf("\033[36m");
    printf("\t\t      Connecting to database");

    for (auto i{ 0 }; i < 3; ++i)
    {
        mySleep(200);
        printf(".");
    }

    if (db_connect())
    {
        mySleep(200);
        printf("\n\t\t       success\n");
        mySleep(500);
        clearLine();
    }
    else
    {
        mySleep(200);
        printf("\n\t\t        falue\n");
        mySleep(5000);
        clearLine();
        scr_exit();
    }
    printf("\033[0m");
}

void Chat::scr_work()
{
    std::string task, userName, log;

    clear_screen();
    sv_create();

    while (true)
    {
        bool flag;
        int foo, poz;

        sv_gettask(clientSocket, userName, task);

        if (task.length() == 0)
            break;

        log.clear();
        log = "[" + getTimeStamp() + "] userName: ";
        log.append(userName, 0, 10);
        log += "  task: " + task;

        printf("\n%s", log.c_str());

        if(logger.is_active())
            logger.writeStr(log);

        if (task == "updatePrivate")
        {
            int client_msg_cnt, server_msg_cnt, messages_to_send;

            sv_readInt(clientSocket, client_msg_cnt);

            if (!db_getPrivateMsgCnt(server_msg_cnt, userName))
                printf("\033[31m  ERROR db_getPrivateMsgCnt()\033[0m");
            else
            {
                if (client_msg_cnt == server_msg_cnt)
                {
                    flag = false;
                    sv_sendBool(clientSocket, flag);
                }
                else
                {
                    flag = true;
                    sv_sendBool(clientSocket, flag);

                    // send difference in messegaes to client
                    // notice! we mean server_msg_cnt allways bigger or simular to client_msg_cnt
                    messages_to_send = server_msg_cnt - client_msg_cnt;
                    sv_sendInt(clientSocket, messages_to_send);

                    std::string command = R"(
                                     with userid as
                                     (
		                                    select id
		                                    from users
		                                    where login = '--reciever--'
                                     )
                                     select users.login, private_msg.message, private_msg.flag
                                     from private_msg
                                     join userid on private_msg.id_reciever = userid.id
                                     join users on private_msg.id_sender = users.id;
                                     )";
                    StringReplacer(command, "--reciever--", userName);
                    std::wstring wcommand(command.begin(), command.end());

                    SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
                    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand.c_str(), SQL_NTS))
                        printf("\033[31m  ERROR updatePrivate\033[0m");

                    SQLCHAR author[30];
                    SQLCHAR message[SQL_TEXT_SIZE];
                    SQLCHAR msgFlag;
                    SQLLEN indicator;


                    SQLBindCol(sqlStmtHandle, 1, SQL_CHAR, &author, sizeof(author), &indicator);
                    SQLBindCol(sqlStmtHandle, 2, SQL_CHAR, &message, sizeof(message), &indicator);
                    SQLBindCol(sqlStmtHandle, 3, SQL_C_UTINYINT, &msgFlag, sizeof(msgFlag), &indicator);

                    for (auto i{ client_msg_cnt }; i < server_msg_cnt; i++)
                    {
                        SQLFetch(sqlStmtHandle);

                        std::string str_author(reinterpret_cast<char*>(author));
                        std::string str_message(reinterpret_cast<char*>(message));

                        sv_sendStr(clientSocket, str_author);
                        sv_sendStr(clientSocket, str_message);
                        sv_sendBool(clientSocket, (bool)msgFlag);
                    }
                }
                printf("\033[32m  DONE\033[0m");
            }
        }
        else if (task == "updatePublic")
        {
            int pubMsg_cnt;
            if (!db_getPublicMsgCnt(pubMsg_cnt))
                printf("\033[31m  ERROR db_getPublicMsgCnt()\033[0m");

            sv_sendInt(clientSocket, pubMsg_cnt);

            SQLCHAR author[30];
            SQLCHAR message[SQL_TEXT_SIZE];
            SQLLEN indicator;

            std::string command = R"(
                                         with userid as
                                         (
		                                        select id
		                                        from users
		                                        where login = '--reciever--'
                                         )
                                         select users.login , public_msg.message 
                                         from public_msg
                                         join userid on public_msg.id_sender = userid.id
                                         join users on users.id = userid.id;
                                     )";

            StringReplacer(command, "--reciever--", userName);
            std::wstring wcommand(command.begin(), command.end());

            SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
            if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand.c_str(), SQL_NTS))
                printf("\033[31m  ERROR updatePrivate\033[0m");

            SQLBindCol(sqlStmtHandle, 1, SQL_CHAR, &author, sizeof(author), &indicator);
            SQLBindCol(sqlStmtHandle, 2, SQL_CHAR, &message, sizeof(message), &indicator);


            for (auto i{ 0 }; i < pubMsg_cnt; i++)
            {
                SQLFetch(sqlStmtHandle);

                std::string str_author(reinterpret_cast<char*>(author));
                std::string str_message(reinterpret_cast<char*>(message));

                if (str_author.length() > 11)      // client store messages in one sting with shortening of long names 
                {
                    str_author.append(str_author, 0, 9);
                    str_author += "..";
                }
                std::string result = str_author + " : " + str_message;

                sv_sendStr(clientSocket, result);
            }

            printf("\033[32m  DONE\033[0m");
        }
        else if (task == "userExists")
        {
            flag = db_userExists(userName);
            sv_sendBool(clientSocket, flag);

            //  db_userExists() will print error or done
        }
        else if (task == "checkPass")
        {
            Hash client_hash, server_hash;
            sv_readHash(clientSocket, client_hash);

            if (!db_getUserHash(server_hash, userName))
                printf("\033[31m  ERROR db_getUserHash()\033[0m");

            if (server_hash == client_hash)
                flag = true;
            else
                flag = false;

            sv_sendBool(clientSocket, flag);

            printf("\033[32m  DONE\033[0m");
        }
        else if (task == "addUser")
        {
            Hash client_hash;

            sv_readHash(clientSocket, client_hash);

            if (!db_addUser(client_hash, userName))
                printf("\033[31m  ERROR db_addUser()\033[0m");
            else
                printf("\033[32m  DONE\033[0m");
        }
        else if (task == "rememberPUBMail")
        {
            std::string pub_message;
            sv_readStr(clientSocket, pub_message);

            if (!db_addPublicMsg(userName, pub_message))
                printf("\033[31m  ERROR db_addPublicMsg()\033[0m");
            else
                printf("\033[32m  DONE\033[0m");
        }
        else if (task == "sendMessage")
        {
            std::string private_message, reciever;
            sv_readStr(clientSocket, reciever);
            sv_readStr(clientSocket, private_message);

            if (!db_addPrivateMsg(userName, reciever, private_message))
                printf("\033[31m  ERROR db_addPrivateMsg()\033[0m");
            else
                printf("\033[32m  DONE\033[0m");
        }
        else if (task == "updateUsernames")
        {
            int users_cnt;

            if (!db_getUsersCount(users_cnt))
                printf("\033[31m  ERROR db_setMsgReaded()\033[0m");

            sv_sendInt(clientSocket, users_cnt);

            std::string command = R"(
                                    select login
                                    from users;
                                     )";
            std::wstring wcommand(command.begin(), command.end());

            SQLFreeStmt(sqlStmtHandle, SQL_CLOSE);
            if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)wcommand.c_str(), SQL_NTS))
                printf("\033[31m  ERROR updateUsernames\033[0m");

            SQLCHAR buffer[30];
            SQLLEN indicator;
            SQLBindCol(sqlStmtHandle, 1, SQL_CHAR, &buffer, sizeof(buffer), &indicator);

            for (auto i{ 0 }; i < users_cnt; i++)
            {
                SQLFetch(sqlStmtHandle);

                std::string str(reinterpret_cast<char*>(buffer));
                sv_sendStr(clientSocket, str);
            }

            printf("\033[32m  DONE\033[0m");
        }
        else if (task == "setMsgReaded")
        {
            sv_readInt(clientSocket, poz); // poz = msg position in msg_arr --starts from 0 in db we count from 1
            if (!db_setMsgReaded(userName, poz + 1))
                printf("\033[31m  ERROR db_setMsgReaded()\033[0m");
            else
                printf("\033[32m  DONE\033[0m");
        }
    }

    printf("\nUSER DISCONNECTED");
    mySleep(300);
    scr_exit();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SERVER

void Chat::sv_create()
{
#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed\n");
        exit(1);
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET)
    {
        printf("Socket creation failed!\n");
        WSACleanup();
        exit(1);
    }

    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_family = AF_INET;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("Socket binding failed!\n");
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("Socket is unable to listen for new connections!\n");
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }
    else
    {
        printf("Server is listening for new connection\n");
    }

    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

    if (clientSocket == INVALID_SOCKET)
    {
        printf("Server is unable to accept the data from client!");
        closesocket(serverSocket);
        WSACleanup();
    }
    else
    {
        printf("Connection created");
    }
#else
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        printf("Socket creation failed!\n");
        close(serverSocket);
        exit(1);
    }

    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_family = AF_INET;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        printf("Socket binding failed!\n");
        close(serverSocket);
        exit(1);
    }

    if (listen(serverSocket, SOMAXCONN) == -1)
    {
        printf("Socket is unable to listen for new connections!\n");
        close(serverSocket);
        exit(1);
    }
    else
    {
        printf("Server is listening for new connection\n");
    }

    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == -1)
        printf("Server is unable to accept the data from client!");
    else
        printf("Connection created");

#endif
}

void Chat::sv_closeConnection()
{
#ifdef _WIN32 
    closesocket(serverSocket);
    WSACleanup();
#else
    close(serverSocket);
#endif
}

void Chat::sv_gettask(const int& connection_descriptor, std::string& name, std::string& task)
{
    sv_readStr(connection_descriptor, name);
    sv_readStr(connection_descriptor, task);
}

void Chat::sv_sendStr(const int& connection_descriptor, const std::string& str)
{
    send(connection_descriptor, str.c_str(), str.length(), 0);
    sv_timeout();
}

void Chat::sv_readStr(const int& connection_descriptor, std::string& str)
{
    char buffer[1024];
    memset(buffer, 0, 1024);
    recv(connection_descriptor, buffer, 1024, 0);
    std::string foo(buffer);
    str = foo;
}

void Chat::sv_sendHash(const int& connection_descriptor, const Hash& hash)
{
    send(connection_descriptor, (char*)&hash._part1, sizeof(uint), 0);
    send(connection_descriptor, (char*)&hash._part2, sizeof(uint), 0);
    send(connection_descriptor, (char*)&hash._part3, sizeof(uint), 0);
    send(connection_descriptor, (char*)&hash._part4, sizeof(uint), 0);
    send(connection_descriptor, (char*)&hash._part5, sizeof(uint), 0);
}

void Chat::sv_readHash(const int& connection_descriptor, Hash& hash)
{
    recv(connection_descriptor, (char*)&hash._part1, sizeof(uint), 0);
    recv(connection_descriptor, (char*)&hash._part2, sizeof(uint), 0);
    recv(connection_descriptor, (char*)&hash._part3, sizeof(uint), 0);
    recv(connection_descriptor, (char*)&hash._part4, sizeof(uint), 0);
    recv(connection_descriptor, (char*)&hash._part5, sizeof(uint), 0);
}

void Chat::sv_sendBool(const int& connection_descriptor, const bool& flag)
{
    send(connection_descriptor, (char*)&flag, sizeof(bool), 0);
}

void Chat::sv_readBool(const int& connection_descriptor, bool& flag)
{
    recv(connection_descriptor, (char*)&flag, sizeof(bool), 0);
}

void Chat::sv_sendInt(const int& connection_descriptor, const int& integer)
{
    send(connection_descriptor, (char*)&integer, sizeof(int), 0);
}

void Chat::sv_readInt(const int& connection_descriptor, int& integer)
{
    recv(connection_descriptor, (char*)&integer, sizeof(int), 0);
}

void Chat::sv_timeout()
{
    mySleep(90);
}