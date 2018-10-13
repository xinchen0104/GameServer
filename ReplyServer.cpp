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
    char* service = argv[1];
    // Create socket for incoming connections
    int servSock = SetupTCPServerSocket(service);
    if (servSock < 0)
        DieWithUserMessage("SetupTCPServerSocket() failed", service);

    for (;;) { // Run forever
    // New connection creates a connected client socket
        int clntSock = AcceptTCPConnection(servSock);
        FILE *channel = fdopen(clntSock, "r+");
        FILE *reply = fopen("./reply", "w+");
        if (channel == NULL)
            DieWithSystemMessage("fdopen() failed");
        
        uint8_t inBuf[512];
        Msg msgList[8];
        int msgCount;
        int mSize;
        while(true){
            msgCount = 0;
            for(int i = 0; i < 8; i++){
                mSize = GetNextMsg(channel, sizeof(MsgBin), inBuf, 512);
                PutMsg(inBuf, sizeof(MsgBin), reply);
                if(mSize != -1){
                    Decode(inBuf, 512, &(msgList[i]));
                    msgCount ++;
                }
                else{
                    break;
                }
            }
            for(int i = 0; i < msgCount; i++){
                printf("%d\n", msgList[i].tickIndex);
                if(msgList[i].tickIndex == 999){
                    fclose(channel);
                    fclose(reply);
                    return 0;
                }
            }
        }
    }
    // NOT REACHED
    close(servSock);
}