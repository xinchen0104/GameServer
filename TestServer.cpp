#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <malloc.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include "Practical.h"
#include "GameProtocol.h"
#include "GameConfig.h"

typedef struct ThreadArgs{
    Player* player;
}ThreadArgs;

STATUS gStatus;
Channel channel[MAX_CHANNEL];
Player player[MAX_PLAYER];
int servSock;

Channel* GetFreeChannel();
Player* GetFreePlayer();
int GetOnlinePlayerNum();
int ServerInit();
void CheckStatus();
void WaitPlayerConnection();
bool AllPlayerReady();
void* ThreadMain(void* threadArgs);
void PrintPlayer(Player player);
bool AllPlayerReady();
void GameServerMain();
uint64_t GetTick();
bool CheckInterval();
bool GameInit();

bool GameInit(){
    printf("GameInit\n");
    fflush(stdout);
    int flags;
    int ret;
    for(int i = 0; i < MAX_PLAYER; i++){
        flags = fcntl((player[i].channel)->sock, F_GETFL);  
        flags |= O_NONBLOCK;  
        ret = fcntl((player[i].channel)->sock, F_SETFL, flags);  
    }
}

uint64_t GetTick(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

bool CheckInterval(){
    static uint64_t realTick;
    static uint64_t realTickLast = GetTick();
    realTick = GetTick();
    if(realTick - realTickLast < TICK_INTERVAL){
        return false;
    }else{
        realTickLast = realTick;
        return true;
    }
}

bool AllPlayerReady(){
    for(int i = 0; i < MAX_PLAYER; i++){
        if(!(player[i].ready)){
            return false;
        }
    }
    return true;
}

void PrintPlayer(Player player){
    printf("index:%d\n", player.index);
    printf("channel address:%p\n", player.channel);
    printf("ready:%d\n", player.ready);
    fflush(stdout);
}

Channel* GetFreeChannel(){
    int index;
    for(index = 0; index < MAX_CHANNEL; index++){
        if(!channel[index].busy){
            return &(channel[index]);
        }
    }
    return NULL;
}
Player* GetFreePlayer(){
    int index;
    for(index = 0; index < MAX_PLAYER; index++){
        if(player[index].index == -1){
            return &(player[index]);
        }
    }
    return NULL;
}
int GetOnlinePlayerNum(){
    int count = 0;
    for(int index = 0; index < MAX_PLAYER; index++){
        if(player[index].index != -1){
            count++;   
        }
    }
    return count;
}
int ServerInit(){
    servSock = -1;
    for(int i = 0; i < MAX_CHANNEL; i++){
        channel[i].sock = -1;
        channel[i].busy = false;
        channel[i].player = NULL;
//        pthread_rwlock_init(&(channel[i].lock), NULL);
    }
    for(int i = 0; i < MAX_PLAYER; i++){
        player[i].index = -1;
        player[i].channel = NULL;
        player[i].ready = false;
        player[i].fSock = NULL;
//        pthread_rwlock_init(&(player[i].lock), NULL);
    }
    gStatus = WAITING_CONNECTION;
    return 0;
}
int main(int argc, char* argv[]){
    ServerInit();
    char* service = argv[1];
    // Create socket for incoming connections
    servSock = SetupTCPServerSocket(service);
    if (servSock < 0)
        DieWithUserMessage("SetupTCPServerSocket() failed", service);
    while(true){
        CheckStatus();
        if(gStatus == WAITING_CONNECTION){
            WaitPlayerConnection();
        }
        if(gStatus == WAITING_READY){
            
        }
        if(gStatus == RUNNING){
            GameInit();
            GameServerMain();
        }
    }
    printf("final:\n");
    for(int i = 0; i < MAX_PLAYER; i++){
        PrintPlayer(player[i]);
    }
    close(servSock);
}
void CheckStatus(){
//    for(int i = 0; i < MAX_PLAYER; i++){
//        PrintPlayer(player[i]);
//    }
    if(GetOnlinePlayerNum() < MAX_PLAYER){
        gStatus = WAITING_CONNECTION;
        return;
    }
    if(GetOnlinePlayerNum() == MAX_PLAYER && !AllPlayerReady()){
        gStatus = WAITING_READY;
        return;
    }
    if(AllPlayerReady()){
        gStatus = RUNNING;
        return;
    }
}
void WaitPlayerConnection(){
    Player* player = GetFreePlayer();
    Channel* channel = GetFreeChannel();
    pthread_t tid;
    ThreadArgs* threadArgs;
    if(player != NULL && channel != NULL){
        channel->sock = AcceptTCPConnection(servSock);
        channel->busy = true;
        channel->player = player;
        player->index = GetOnlinePlayerNum() + 1;
        player->channel = channel;
        threadArgs = (ThreadArgs*) malloc (sizeof(ThreadArgs));
        if(threadArgs == NULL){
            DieWithSystemMessage("malloc failed");
        }
        threadArgs->player = player;
        pthread_create(&tid, NULL, ThreadMain, threadArgs);
    }else{
        DieWithUserMessage("No enough player or channel address", "");
    }
}

void GameServerMain(){
    uint32_t tickIndex = 0;
    uint32_t playActionNum = 0;
    uint8_t buf[8];
    Msg msg;
    MsgBin msgList[16];
    //录像
    FILE *reply = fopen("./reply", "w+");
    while(true){
        if(!CheckInterval()){
            continue;
        }
        printf("tick\n");
        fflush(stdout);
        for(int i = 0; i < MAX_PLAYER; i++){
            while(GetNextMsg(player[i].fSock, sizeof(MsgBin), buf, 8) != -1){
                printf("!=-1\n");
                fflush(stdout);
                Decode(buf, 8, &msg);
                if(msg.msgType == PLAYER_ACTION){
                    playActionNum ++;
                    msg.tickIndex = tickIndex;
                    msg.playerIndex = player[i].index;
                    msg.isRes = true;
                    Encode(&msg, buf, 8);
                    memcpy(&msgList[playActionNum], buf, sizeof(MsgBin));
                }
            }
        }
        MsgInit(&msg, TICK, -1, tickIndex, playActionNum);
        Encode(&msg, buf, 8);
        memcpy(msgList, buf, sizeof(MsgBin));
        printf("%d", playActionNum);
        fflush(stdout);
        //录像
        PutMsg(msgList, sizeof(MsgBin), playActionNum+1, reply);
        for(int i = 0; i < MAX_PLAYER; i++){
            PutMsg(msgList, sizeof(MsgBin), playActionNum+1, player[i].fSock);
        }
    }
}

void* ThreadMain(void* threadArgs){
    Player* player = ((ThreadArgs*)threadArgs)->player;
    player->fSock = fdopen(player->channel->sock, "r+");
    if (player->fSock == NULL)
        DieWithSystemMessage("fdopen() failed");
    uint8_t buf[8];
    Msg msg;
    while(!(player->ready)){
        if(GetNextMsg(player->fSock, sizeof(MsgBin), buf, 8) != -1){
            Decode(buf, 8, &msg);
            if(msg.msgType == READY_CONFIRM){
                player->ready = true;
            }else{
                //do nothing
            }
        }
    }
    return NULL;
}