#include "mytcpsocket.h"
#include "operatordb.h"
#include "mytcpserver.h"
#include <QDebug>
#include <QStringList>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this, &QTcpSocket::readyRead, this, &MyTcpSocket::recvMsg);
    connect(this, &QTcpSocket::disconnected, this, &MyTcpSocket::clientOffline);
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::RegistrationRequest(PDU *pdu)
{
    char caName[32] = {0};
    char caPwd[32] = {0};
    strncpy(caName, pdu->caData, 32);
    strncpy(caPwd, pdu->caData+32, 32);
    bool ret = OperatorDB::getInstance().handleRegist(caName, caPwd);
    PDU* respdu = mkPDU(0);
    respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
    if (ret)
    {
        strcpy(respdu->caData, REGIST_OK);
    }
    else
    {
        strcpy(respdu->caData, REGIST_FAILED);
    }

    write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::LoginRequest(PDU *pdu)
{
    char caName[32] = {0};
    char caPwd[32] = {0};
    strncpy(caName, pdu->caData, 32);
    strncpy(caPwd, (pdu->caData)+32, 32);
    bool ret = OperatorDB::getInstance().handleLogin(caName, caPwd);
    PDU* respdu = mkPDU(0);
    respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
    if (ret)
    {
        strcpy(respdu->caData, LOGIN_OK);
        m_strName = caName;
    }
    else
    {
        strcpy(respdu->caData, LOGIN_FAILED);
    }

    write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::OnlineUserRequests()
{
    QStringList ret = OperatorDB::getInstance().handleAllOnline();

    uint uiMsgLen = ret.size() * 32;   //消息长度
    PDU* respdu = mkPDU(uiMsgLen);
    respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
    for(int i = 0; i < ret.size(); i++)
    {
        memcpy(reinterpret_cast<char*>(respdu->caMsg)+i*32, ret[i].toStdString().c_str(), ret[i].size());
    }

    write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::FindUserRequest(PDU *pdu)
{
    int ret = OperatorDB::getInstance().handleSearchUser(pdu->caData);
    PDU* respdu = mkPDU(0);
    respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_RESPOND;
    if (-1 == ret)
    {
        strncpy(respdu->caData, SEARCH_USER_NO, 64);
    }
    else if (1 == ret)
    {
        strncpy(respdu->caData, SEARCH_USER_ONLINE, 64);
    }
    else if (0 == ret)
    {
        strncpy(respdu->caData, SEARCH_USER_OFFLINE, 64);
    }
    else
    {
        strncpy(respdu->caData, UNKONOW_ERROR, 64);
    }

    write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::AddFriendRequest(PDU *pdu)
{
    char perName[32] = {0};  //被加好友的一方
    char Name[32] = {0};     //主动申请加好友的一方
    strncpy(perName, pdu->caData, 32);
    strncpy(Name, (pdu->caData)+32, 32);

    int ret = OperatorDB::getInstance().handleAddFriend(perName, Name);

    PDU* respdu = nullptr;

    if (-2 == ret)
    {
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        strncpy(respdu->caData, ADD_FRIEND_SELF_ADD_SELF, 64);
    }
    else if (-1 == ret)
    {
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        strncpy(respdu->caData, UNKONOW_ERROR, 64);
    }
    else if (0 == ret)
    {
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        strncpy(respdu->caData, EXISTED_FRIEND, 64);
    }
    else if (1 == ret)
    {
        MyTcpServer::getInstance().resend(perName, pdu);
        return;
    }
    else if (2 == ret)
    {
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        strncpy(respdu->caData, ADD_FRIEND_OFFLINE, 64);
    }
    else if (3 == ret)
    {
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        strncpy(respdu->caData, ADD_FRIEND_NOEXIST, 64);
    }

    write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::AddFriendAgree(PDU *pdu)
{
    //好友同意时，将数据添加到数据库
    char caName[32] = {0};  //主动加好友一方的名字
    char caPerName[32] {0}; //被加一方的名字
    strncpy(caPerName, pdu->caData, 32);
    strncpy(caName, (pdu->caData) + 32, 32);

    OperatorDB::getInstance().handleAddFriendAgree(caPerName, caName);

    PDU* respdu = mkPDU(0);
    respdu->uiMsgType = ENUM_MSG_TYPE_FRIEND_AGREE_RESPOND;

    MyTcpServer::getInstance().resend(caName,respdu);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::AddFriendRefuse(PDU *pdu)
{
    char caName[32] = {0};  //主动加好友一方的名字
    strncpy(caName, pdu->caData + 32, 32);

    PDU* respdu = mkPDU(0);
    respdu->uiMsgType = ENUM_MSG_TYPE_FRIEND_AGREE_RESPOND;

    MyTcpServer::getInstance().resend(caName,respdu);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::recvMsg()
{
    qDebug() << this->bytesAvailable();
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen, sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);

    PDU* pdu = mkPDU(uiMsgLen);
    this->read((char*)pdu+sizeof(uint), uiPDULen - sizeof(uint));
    switch(pdu->uiMsgType)
    {
    case ENUM_MSG_TYPE_REGIST_REQUEST:   //注册请求
        RegistrationRequest(pdu);
        break;
    case ENUM_MSG_TYPE_LOGIN_REQUEST:   //登录请求
        LoginRequest(pdu);
        break;
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:   //查看在线用户请求
        OnlineUserRequests();
        break;
    case ENUM_MSG_TYPE_SEARCH_USER_REQUEST:   //查找用户请求
        FindUserRequest(pdu);
        break;
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:   //加好友请求
        AddFriendRequest(pdu);
        break;
    case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE:   //加好友同意
        AddFriendAgree(pdu);
        break;
    case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:   //加好友拒绝
        AddFriendRefuse(pdu);
    default:
        break;
    }

    free(pdu);
    pdu = nullptr;

    //qDebug() << caName << caPwd << pdu->uiMsgType;
    // qDebug() << pdu->uiMsgType << (char*)pdu->caMsg;
}

void MyTcpSocket::clientOffline()
{
    OperatorDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this);
}
