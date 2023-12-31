﻿#include "mytcpsocket.h"
#include "operatordb.h"
#include "mytcpserver.h"
#include <QDebug>
#include <QDir>
#include <QStringList>
#include <QFileInfoList>

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
        QDir dir;
        //qDebug() << "create dir: " <<
        dir.mkdir(QString("../home/%1").arg(caName));
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
        memcpy(reinterpret_cast<char*>(respdu->caMsg)+i*32, ret[i].toStdString().c_str(), ret[i].toUtf8().size()+1);
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

    /*
     *判断好友是否已经是好友，加的是否是自己本身
     * 判断是否是在线
     * 在线处理
    */
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
    respdu->uiMsgType = ENUM_MSG_TYPE_FRIEND_REFUSE_RESPOND;

    MyTcpServer::getInstance().resend(caName,respdu);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::FlushFriendRequest(PDU *pdu)
{
    char caName[32] = {0};
    strncpy(caName, pdu->caData, 32);
    QStringList ret = OperatorDB::getInstance().handleFlushFriend(caName);

    uint uiMsgLen = ret.size() * 32;
    PDU* respdu = mkPDU(uiMsgLen);
    respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
    for (int i = 0; i < ret.size(); i++)
    {
        strncpy(reinterpret_cast<char*>(respdu->caMsg) + i*32, ret[i].toUtf8().toStdString().c_str(), 32);
    }

    write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::DeleteFriendRequest(PDU *pdu)
{
    char name[32] = {0};
    char delName[32] = {0};
    strncpy(name, pdu->caData, 32);
    strncpy(delName, pdu->caData + 32, 32);
    OperatorDB::getInstance().handleDelFriend(name, delName);

    PDU* respdu = mkPDU(0);
    respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
    strcpy(respdu->caData, DEL_FRIEND_OK);

    write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
    free(respdu);
    respdu = nullptr;

    MyTcpServer::getInstance().resend(delName, pdu);
}

void MyTcpSocket::PrivateRequest(PDU *pdu)
{
    char strChatName[32] = {0};
    strncpy(strChatName, pdu->caData + 32, 32);

    MyTcpServer::getInstance().resend(strChatName, pdu);
}

void MyTcpSocket::GroupRequest(PDU *pdu)
{
    char caName[32] = {0};
    strncpy(caName, pdu->caData, 32);
    // 将所有在线的好友找出来
    QStringList onlineFriend = OperatorDB::getInstance().handleFlushFriend(caName);
    QString tmpName;
    for (int i = 0; i < onlineFriend.size(); i++)
    {
        tmpName = onlineFriend[i];
        MyTcpServer::getInstance().resend(tmpName.toUtf8().toStdString().c_str(), pdu);
    }
}

void MyTcpSocket::CreateDirRequest(PDU *pdu)
{
    QDir dir;
    QString strCurPath = QString("%1").arg(reinterpret_cast<char*>(pdu->caMsg));
    bool ret = dir.exists(strCurPath);
    PDU* respdu = nullptr;
    if (ret)
    {
        char caNewDir[32] = {0};
        strncpy(caNewDir, pdu->caData+32, 32);
        QString strNewPath = strCurPath + "/" + caNewDir;
        ret = dir.exists(strNewPath);

        if (ret)   //创建的文件名已存在
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
            strcpy(respdu->caData, FILE_NAME_EXIST);
        }
        else
        {
            dir.mkdir(strNewPath);
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
        }
    }
    else  //当前目录不存在
    {
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
        strcpy(respdu->caData, DIR_NOEXIST);
    }

    write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::FlushFileRequest(PDU *pdu)
{
    char* pCurPath = new char[pdu->uiMsgLen+1];
    strcpy(pCurPath, reinterpret_cast<char*>(pdu->caMsg));
    QDir dir(pCurPath);
    QFileInfoList fileInfoList = dir.entryInfoList();

    int iFileCount = fileInfoList.size();
    PDU* respdu = mkPDU(sizeof(FileInfo)*(iFileCount));
    respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
    FileInfo* pFileInfo = nullptr;
    QString strFileName;
    for(int i = 0; i < iFileCount; i++)
    {
        pFileInfo = reinterpret_cast<FileInfo*>(respdu->caMsg) + i;
        strFileName = fileInfoList[i].fileName();

        strncpy(pFileInfo->caFileName, strFileName.toUtf8().toStdString().c_str(), 32);
        if (fileInfoList[i].isDir())
        {
            pFileInfo->iFileType = 0;
        }
        else if (fileInfoList[i].isFile())
        {
            pFileInfo->iFileType = 1;
        }
    }

    write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::DelDirRequest(PDU *pdu)
{
    char caName[32] = {0};
    strcpy(caName, pdu->caData);
    char* pPath = new char[pdu->uiMsgLen];
    memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);

    QString strPath = QString("%1/%2").arg(pPath).arg(caName);


    QFileInfo fileInfo(strPath);
    bool flag = false;
    if (fileInfo.isDir())
    {
        QDir dir;
        dir.setPath(strPath);
        flag = dir.removeRecursively();
    }
    else if (fileInfo.isFile())
    {
        flag = false;
    }

    PDU* respdu = nullptr;
    if (flag)
    {
        respdu = mkPDU(strlen(DEL_DIR_OK)+1);
        respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
        memcpy(respdu->caData, DEL_DIR_OK, strlen(DEL_DIR_OK)+1);
    }
    else
    {
        respdu = mkPDU(strlen(DEL_DIR_FAILED)+1);
        respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
        memcpy(respdu->caData, DEL_DIR_FAILED, strlen(DEL_DIR_FAILED)+1);
    }

    write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::ReNameFileRequest(PDU *pdu)
{
    char oldName[32] = {0};
    char newName[32] = {0};
    strcpy(oldName, pdu->caData);
    strcpy(newName, pdu->caData + 32);

    char* pPath = new char[pdu->uiMsgLen];
    memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);

    QString strOldPath = QString("%1/%2").arg(pPath).arg(oldName);
    QString strNewPath = QString("%1/%2").arg(pPath).arg(newName);
    // qDebug() << strOldPath << " " << strNewPath;

    QDir dir;
    bool ret = dir.rename(strOldPath, strNewPath);

    PDU* respdu = mkPDU(0);
    respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
    if (ret)
    {
        strcpy(pdu->caData, RENAME_FILE_OK);
    }
    else
    {
        strcpy(pdu->caData, RENAME_FILE_FAILED);
    }

    write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::EnterDirRequest(PDU *pdu)
{
    char strEnterName[32] = {0};
    strcpy(strEnterName, pdu->caData);

    char* pPath = new char[pdu->uiMsgLen];
    memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);

    QString strPath = QString("%1/%2").arg(pPath).arg(strEnterName);
    qDebug() << strPath;
    QFileInfo fileInfo(strPath);
    PDU* respdu = nullptr;
    if (fileInfo.isDir())
    {
        QDir dir(strPath);
        QFileInfoList fileInfoList = dir.entryInfoList();

        int iFileCount = fileInfoList.size();
        respdu = mkPDU(sizeof(FileInfo)*(iFileCount));
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
        strcpy(respdu->caData, "enter dir");
        FileInfo* pFileInfo = nullptr;
        QString strFileName;
        for(int i = 0; i < iFileCount; i++)
        {
            pFileInfo = reinterpret_cast<FileInfo*>(respdu->caMsg) + i;
            strFileName = fileInfoList[i].fileName();

            strncpy(pFileInfo->caFileName, strFileName.toUtf8().toStdString().c_str(), 32);
            if (fileInfoList[i].isDir())
            {
                pFileInfo->iFileType = 0;
            }
            else if (fileInfoList[i].isFile())
            {
                pFileInfo->iFileType = 1;
            }
        }

        write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
        free(respdu);
        respdu = nullptr;
    }
    else if (fileInfo.isFile())
    {
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
        strcpy(respdu->caData, ENTER_DIR_FAILED);

        write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
        free(respdu);
        respdu = nullptr;
    }
}

void MyTcpSocket::DelFileRequest(PDU *pdu)
{
    char caName[32] = {0};
    strcpy(caName, pdu->caData);
    char* pPath = new char[pdu->uiMsgLen];
    memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);

    QString strPath = QString("%1/%2").arg(pPath).arg(caName);


    QFileInfo fileInfo(strPath);
    bool flag = false;
    if (fileInfo.isDir())
    {
        flag = false;

    }
    else if (fileInfo.isFile())
    {
        QDir dir;
        flag = dir.remove(strPath);
    }

    PDU* respdu = nullptr;
    if (flag)
    {
        respdu = mkPDU(strlen(DEL_FILE_OK)+1);
        respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
        memcpy(respdu->caData, DEL_FILE_OK, strlen(DEL_FILE_OK)+1);
    }
    else
    {
        respdu = mkPDU(strlen(DEL_FILE_FAILED)+1);
        respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
        memcpy(respdu->caData, DEL_FILE_FAILED, strlen(DEL_FILE_FAILED)+1);
    }

    write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
    free(respdu);
    respdu = nullptr;
}

void MyTcpSocket::recvMsg()
{
    // qDebug() << this->bytesAvailable();
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
    case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:   //刷新用户请求
        FlushFriendRequest(pdu);
        break;
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:   //删除好友请求
        DeleteFriendRequest(pdu);
        break;
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:    //私聊请求
        PrivateRequest(pdu);
        break;
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:      //群聊请求
        GroupRequest(pdu);
        break;
    case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:      //新建文件夹请求
        CreateDirRequest(pdu);
        break;
    case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        FlushFileRequest(pdu);
        break;
    case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
        DelDirRequest(pdu);
        break;
    case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
        ReNameFileRequest(pdu);
        break;
    case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
        EnterDirRequest(pdu);
        break;
    case ENUM_MSG_TYPE_DEL_FILE_REQUEST:
        DelFileRequest(pdu);
        break;
    default:
        break;
    }

    free(pdu);
    pdu = nullptr;
}

void MyTcpSocket::clientOffline()
{
    OperatorDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this);
}
