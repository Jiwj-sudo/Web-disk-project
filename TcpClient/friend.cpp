﻿#include "friend.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QDebug>

Friend::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pShowMsgTE = new QTextEdit;
    m_pFriendListWidget = new QListWidget;
    m_pInputMsgLE = new QLineEdit;

    m_pDelFriendPB = new QPushButton("删除好友");
    m_pFlushFriendPB = new QPushButton("刷新好友");
    m_pShowOnlineUserPB = new QPushButton("显示在线好友");
    m_pSearchUserPB = new QPushButton("查找用户");
    m_pMsgSendPB = new QPushButton("信息发送");
    m_pPrivateChatPB = new QPushButton("私聊");

    QVBoxLayout* pRightPBVBL = new QVBoxLayout;
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUserPB);
    pRightPBVBL->addWidget(m_pSearchUserPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout* pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(pRightPBVBL);

    QHBoxLayout* pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline = new Online;

    QVBoxLayout* pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();

    setLayout(pMain);

    connect(m_pShowOnlineUserPB, &QPushButton::clicked, this, &Friend::showOnline);
    connect(m_pSearchUserPB, &QPushButton::clicked, this, &Friend::searchUser);
    connect(m_pFlushFriendPB, &QPushButton::clicked, this, &Friend::flushFriend);
    connect(m_pDelFriendPB, &QPushButton::clicked, this, &Friend::delFriend);
}

void Friend::showAllOnlineUser(PDU *pdu)
{
    if (nullptr == pdu)
        return;

    m_pOnline->showUser(pdu);
}

void Friend::updateFriendList(PDU *pdu)
{
    if (nullptr == pdu)
        return;

    m_pFriendListWidget->clear();
    uint uisize = pdu->uiMsgLen / 32;
    char caName[32] = {0};
    for (uint i = 0; i < uisize; i++)
    {
        strncpy(caName, reinterpret_cast<char*>(pdu->caMsg) + i*32, 32);
        m_pFriendListWidget->addItem(caName);
    }
}

void Friend::searchUser()
{
    m_strSearchName = QInputDialog::getText(this, "搜索", "用户名: ");
    if (!m_strSearchName.isEmpty())
    {
        qDebug() << m_strSearchName;
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_REQUEST;
        strncpy(pdu->caData, m_strSearchName.toUtf8().toStdString().c_str(), 32);
        TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
}

void Friend::flushFriend()
{
    QString strName = TcpClient::getInstance().getLoginName();
    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    strncpy(pdu->caData, strName.toUtf8().toStdString().c_str(), strName.toUtf8().size());
    TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Friend::delFriend()
{
    if (nullptr == m_pFriendListWidget->currentItem())
        return;

    //要删除的好友名字
    QString delName = m_pFriendListWidget->currentItem()->text();
    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;

    //自己的名字
    QString MyName = TcpClient::getInstance().getLoginName();

    strncpy(pdu->caData, MyName.toUtf8().toStdString().c_str(), 32);
    strncpy(pdu->caData + 32, delName.toUtf8().toStdString().c_str(), 32);

    TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Friend::showOnline()
{
    if (m_pOnline->isHidden())
    {
        m_pOnline->show();

        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    else
    {
        m_pOnline->hide();
    }
}
