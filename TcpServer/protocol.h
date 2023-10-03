/*
 *设置通信双方的协议
*/
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
typedef unsigned int uint;

#define REGIST_OK "regist ok"
#define REGIST_FAILED "regist failed : name existed"

#define LOGIN_OK "login ok"
#define LOGIN_FAILED "regist failed : name error or pwd error or relogin"

#define SEARCH_USER_NO "No such people"
#define SEARCH_USER_ONLINE "online"
#define SEARCH_USER_OFFLINE "offline"

#define UNKONOW_ERROR "unknow error"
#define EXISTED_FRIEND "friend exist"
#define ADD_FRIEND_OFFLINE "user offline"
#define ADD_FRIEND_NOEXIST "user not exist"
#define ADD_FRIEND_SELF_ADD_SELF "can not self add self"

#define DEL_FRIEND_OK "delete friend ok"

enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN = 0,
    ENUM_MSG_TYPE_REGIST_REQUEST,    //注册请求
    ENUM_MSG_TYPE_REGIST_RESPOND,    //注册回复

    ENUM_MSG_TYPE_LOGIN_REQUEST,     //登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND,     //登录回复

    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,     //在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,     //在线用户回复

    ENUM_MSG_TYPE_SEARCH_USER_REQUEST,     //搜索用户用户请求
    ENUM_MSG_TYPE_SEARCH_USER_RESPOND,     //搜索用户回复

    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,      //加友好请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,      //加友好回复

    ENUM_MSG_TYPE_ADD_FRIEND_AGGREE,       //同意添加好友
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE,       //拒绝添加好友

    ENUM_MSG_TYPE_FRIEND_AGREE_RESPOND,    //好友同意 回复
    ENUM_MSG_TYPE_FRIEND_REFUSE_RESPOND,   //好友拒绝 回复

    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST,    //刷新好友请求
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND,    //刷新好友回复

    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,   //删除好友请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND,   //删除好友回复

    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,   //私聊请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,   //私聊回复
    ENUM_MSG_TYPE_MAX = 0x00ffffff,
};

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
