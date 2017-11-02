#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "Practical.h"
#include "GameProtocol.h"
const uint8_t MAGIC = 0x54;
const uint8_t MAGIC_MASK = 0xfc;
const uint8_t RES_FLAG = 0x01;
void Encode(Msg* frameData, uint8_t *outBuf, size_t bufSize){
  	if (bufSize < sizeof(MsgBin))
    	DieWithUserMessage("Output buffer too small", "");
  	MsgBin *mb = (MsgBin *) outBuf;
  	memset(outBuf, 0, sizeof(MsgBin)); // Be sure
  	mb->header = MAGIC;
  	if (frameData->isRes)
  		mb->header |= RES_FLAG;
  	mb->msgType = (uint8_t)(frameData->msgType); 
  	mb->playerIndex = frameData->playerIndex;
  	mb->tickIndex = htonl(frameData->tickIndex); 
  	if (frameData->msgType == TICK)
  		mb->msgNum = (uint8_t)(frameData->msgNum);
  	if (frameData->msgType == PLAYER_ACTION)
  		mb->action = (uint8_t)(frameData->action);

}
void Decode(uint8_t *inBuf, size_t bufSize, Msg* frameData){
	MsgBin *mb = (MsgBin *) inBuf;
	memset(frameData, 0, sizeof(Msg));
	if((mb->header & RES_FLAG) != 0)
		frameData->isRes = true;
	frameData->msgType = (MsgType)((uint32_t)(mb->msgType));
	frameData->playerIndex = mb->playerIndex;
  	frameData->tickIndex = ntohl(mb->tickIndex);
  	if (frameData->msgType == TICK)
  		frameData->msgNum = (uint32_t)(mb->msgNum);
  	if (frameData->msgType == PLAYER_ACTION)
  		frameData->action = (ActionType)((uint32_t)(mb->action));
}
void GetNextMsg(FILE *in, uint8_t *buf, size_t bufSize){

}
void PutMsg(uint8_t *buf, size_t msgSize, FILE *out){

}
