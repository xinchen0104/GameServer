#include <stdint.h>

typedef enum {
    TICK = 0x01,
    MATCH_REQ = 0x02,
    READY_CONFIRM = 0x03,
    QUIT_REQ = 0x04,
    GAMEOVER_CONFIRM = 0x05,
    PLAYER_ACTION = 0x06
} MsgType;

typedef enum {
    ACTION_UP = 0x01,
    ACTION_DOWN = 0x02,
    ACTION_LEFT = 0x04,
    ACTION_RIGHT = 0x08,
    ACTION_ATTACK = 0x10
} ActionType;

typedef struct Msg{
    MsgType msgType;
    uint8_t playerIndex;
    uint32_t tickIndex;
    union{
        uint32_t msgNum;
        ActionType action;
    };
    bool isRes;//unused
} Msg;

typedef struct MsgBin
{
    uint8_t header;
    uint8_t msgType;
    uint8_t playerIndex;
    uint32_t tickIndex;
    union{
        uint8_t msgNum;
        uint8_t action;
    };    
} MsgBin;

void Encode(void* frameData, uint8_t *outBuf, size_t bufSize);
void Decode(uint8_t *inBuf, size_t bufSize, void* frameData);
void GetNextMsg(FILE *in, uint8_t *buf, size_t bufSize);
void PutMsg(uint8_t *buf, size_t msgSize, FILE *out);
