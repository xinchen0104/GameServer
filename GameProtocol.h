#include <stdint.h>
enum MsgType{
    MATCH_REQ = 0x0001,
    READY_CONFIRM = 0x0002,
    QUIT_REQ = 0x0004,
    GAMEOVER_CONFIRM = 0x0008,
    PLAYER_ACTION = 0x000f
}
typedef struct MsgType MsgType
struct Msg{
    MsgType msgType,
    union{
        uint16_t unused,
        {
            uint8_t playerIndex,
            uint8_t action,
        }
    },
    bool isReq,
    bool isEcho
}
struct Tick{
    uint64_t index,
    bool hasMsg,
    uint8_t msgNum,
}
