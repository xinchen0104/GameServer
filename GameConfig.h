#include <stdint.h>
#include <pthread.h>

#define MAX_PLAYER 2
#define MAX_CHANNEL 2
#define TICK_INTERVAL 200

struct Player;
struct Channel;
typedef enum {
    WAITING_CONNECTION = 0x01,
    WAITING_READY = 0x02,
    RUNNING = 0x03,
    OTHER = 0x04
}STATUS;

typedef struct Player{
    int index;
    Channel* channel;
    bool ready;
    FILE* fSock;
//    pthread_rwlock_t lock;
}Player;

typedef struct Channel{
    int sock;
    bool busy;
    Player* player;
//    pthread_rwlock_t lock;
}Channel;