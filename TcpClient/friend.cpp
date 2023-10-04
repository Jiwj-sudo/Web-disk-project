#include "friend.h"
#include "protocol.h"
#include "tcpclient.h"
#include "privatechat.h"
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
    connect(m_pPrivateChatPB, &QPushButton::clicked, this, &Friend::privateChat);

    connect(m_pInputMsgLE, &QLineEdit::returnPressed, m_pMsgSendPB, &QPushButton::click);
    connect(m_pMsgSendPB, &QPushButton::clicked, this, &Friend::groupChat);
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

void Friend::updateGroupMsg(PDU *pdu)
{
    QString strMsg = QString("%1 say: %2").arg(pdu->caData).arg(reinterpret_cast<char*>(pdu->caMsg));
    m_pShowMsgTE->append(strMsg);
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

void Friend::privateChat()
{
    if (nullptr == m_pFriendListWidget->currentItem())
        return;

    QString strChatName = m_pFriendListWidget->currentItem()->text();
    PrivateChat::getInstance().setChatName(strChatName);

    if (PrivateChat::getInstance().isHidden())
    {
        PrivateChat::getInstance().show();
    }
}

void Friend::groupChat()
{
    QString strMsg = m_pInputMsgLE->text();
    if (!strMsg.isEmpty())
    {
        PDU* pdu = mkPDU(strMsg.toUtf8().size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        QString strName = TcpClient::getInstance().getLoginName();
        strncpy(pdu->caData, strName.toUtf8().toStdString().c_str(), 32);
        strncpy(reinterpret_cast<char*>(pdu->caMsg), strMsg.toUtf8().toStdString().c_str(), strMsg.toUtf8().size() + 1);
        TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;

        m_pInputMsgLE->clear();
        m_pShowMsgTE->append(QString("I say: %1").arg(strMsg));
    }
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
