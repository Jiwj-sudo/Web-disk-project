#include "mytcpsocket.h"
#include "operatordb.h"
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
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = nullptr;
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST:   //登录请求
    {
        char caName[32] = {0};
        char caPwd[32] = {0};
        strncpy(caName, pdu->caData, 32);
        strncpy(caPwd, pdu->caData+32, 32);
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
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = nullptr;
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:   //查看在线用户请求
    {
        QStringList ret = OperatorDB::getInstance().handleAllOnline();

        uint uiMsgLen = ret.size() * 32;   //消息长度
        PDU* respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
        for(int i = 0; i < ret.size(); i++)
        {
            memcpy((char*)(respdu->caMsg)+i*32, ret[i].toStdString().c_str(), ret[i].size());
        }

        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = nullptr;
        break;
    }
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
