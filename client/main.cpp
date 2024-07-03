#include"Chat.h"

using namespace std;

int main()
{
    setlocale(LC_ALL, "");

    Chat client;

    //client.sv_setConnection();
    client.start();

    return 0;
}
