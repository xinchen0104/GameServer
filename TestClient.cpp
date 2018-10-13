#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <malloc.h>
#include "Practical.h"
#include "GameProtocol.h"

int main(int argc, char* argv[]){
    char *server = argv[1];    // First arg: server address/name
    char *service = argv[2];   // Second arg: string to echo
    int sock = SetupTCPClientSocket(server, service);
    if (sock < 0)
        DieWithUserMessage("SetupTCPClientSocket() failed", "unable to connect");

    FILE *str = fdopen(sock, "r+"); // Wrap for stream I/O
    if (str == NULL)
        DieWithSystemMessage("fdopen() failed");
    Msg msg;
    memset((void*)(&msg), 0, sizeof(Msg));
    msg.msgType = READY_CONFIRM;
    uint8_t buf[8];
    memset(buf, 0, sizeof(MsgBin));
    Encode(&msg, buf, sizeof(MsgBin));
    PutMsg((MsgBin*)buf, sizeof(MsgBin), 1, str);
    while(true){}
}