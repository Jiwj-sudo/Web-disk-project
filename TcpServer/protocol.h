#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
typedef unsigned int uint;

struct PDU
{
    uint uiPDULen;   //总的协议数据单元大小
    uint uiMsgType;  //消息类型
    char caData[64];
    uint uiMsgLen;   //实际的消息长度
    int caMsg[];     //实际消息
};

//动态申请空间
PDU* mkPDU(uint uiMsgLen);

#endif // PROTOCOL_H
