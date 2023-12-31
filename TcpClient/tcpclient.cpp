﻿#include "tcpclient.h"
#include "ui_tcpclient.h"
#include "privatechat.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    this->resize(370, 270);
    // 加载配置
    loadConfig();
    // 连接服务器
    connectionServer();

    //回车键登录
    connect(ui->pwd_le, &QLineEdit::returnPressed, ui->login_pb, &QPushButton::click);
}

TcpClient::~TcpClient()
{
    delete ui;
}
/*
 加载配置文件，配置文件中存有ip地址和端口号
*/
void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "open config", "open config failed");
    }

    QByteArray baData = file.readAll();
    QString strData = baData.toStdString().c_str();
    // qDebug() << strData;
    file.close();

    strData.replace("\r\n", " ");

    QStringList strList = strData.split(" ");
    m_strIP = strList[0];
    m_usPort = strList[1].toUShort();
}

void TcpClient::connectionServer()
{
    //connect(&m_tcpSocket, SIGNAL(connectd()), this, SLOT(showConnect()));
    connect(&m_tcpSocket, &QTcpSocket::connected, this, &TcpClient::showConnect);
    connect(&m_tcpSocket, &QTcpSocket::readyRead, this, &TcpClient::recvMsg);

    // 连接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

void TcpClient::RegistrationReply(PDU *pdu)
{
    if(0 == strcmp(pdu->caData, REGIST_OK))
    {
        QMessageBox::information(this, "注册", REGIST_OK);
    }
    else if (0 == strcmp(pdu->caData, REGIST_FAILED))
    {
        QMessageBox::warning(this, "注册", REGIST_FAILED);
    }
}

void TcpClient::LoginReply(PDU* pdu)
{
    if(0 == strcmp(pdu->caData, LOGIN_OK))
    {
        m_strCurPath = QString("../home/%1").arg(m_strLoginName);
        // QMessageBox::information(this, "登录", LOGIN_OK);
        OpeWidget::getInstance().show();
        //隐藏登录界面
        this->hide();
        // 登录成功后，直接刷新在线好友列表和文件列表
        // OpeWidget::getInstance().getFriend()->flushFriend();
        // OpeWidget::getInstance().getBook()->flushFile();
    }
    else if (0 == strcmp(pdu->caData, LOGIN_FAILED))
    {
        QMessageBox::warning(this, "登录", LOGIN_FAILED);
    }
}

void TcpClient::AllOnlineReply(PDU *pdu)
{
    OpeWidget::getInstance().getFriend()->showAllOnlineUser(pdu);
}

void TcpClient::SearchUserReply(PDU *pdu)
{
    if (0 == strcmp(SEARCH_USER_NO, pdu->caData))
    {
        QMessageBox::information(this, "搜索",
                                QString("%1: not exist").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
    }
    else if (0 == strcmp(SEARCH_USER_ONLINE, pdu->caData))
    {
        int ret = QMessageBox::information(this, "搜索",
                                QString("%1: online, 需要添加对方为好友吗?").arg(OpeWidget::getInstance().getFriend()->m_strSearchName),
                                QMessageBox::Yes, QMessageBox::No);

        PDU* respdu = mkPDU(0);
        strncpy(respdu->caData, OpeWidget::getInstance().getFriend()->m_strSearchName.toStdString().c_str(), 32);
        strncpy(respdu->caData + 32, m_strLoginName.toStdString().c_str(), 32);
        if (QMessageBox::Yes == ret)
        {
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
            m_tcpSocket.write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
        }
        free(respdu);
        respdu = nullptr;
    }
    else if (0 == strcmp(SEARCH_USER_OFFLINE, pdu->caData))
    {
        QMessageBox::information(this, "搜索",
                                QString("%1: offline").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
    }
    else
    {
        QMessageBox::information(this, "搜索",
                                QString("%1: Unknown error occurred").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
    }
}

void TcpClient::AddFriendReqReply(PDU *pdu)
{
    char caName[32] = {0};  //对方的名字
    strncpy(caName, (pdu->caData) + 32, 32);

    int ret = QMessageBox::information(this, "添加好友", QString("%1 want to add you as friend ?").arg(caName),
                                                   QMessageBox::Yes, QMessageBox::No);

    PDU* respdu = mkPDU(0);
    strncpy(respdu->caData, pdu->caData, 32);
    strncpy(respdu->caData + 32, pdu->caData + 32, 32);
    if (QMessageBox::Yes == ret)
    {
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;
    }
    else
    {
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
    }

    m_tcpSocket.write(reinterpret_cast<char*>(respdu), respdu->uiPDULen);
    free(respdu);
    respdu = nullptr;

    // OpeWidget::getInstance().getFriend()->flushFriend();
    emit OpeWidget::getInstance().getFriend()->getFlushPB()->click();
}

void TcpClient::FlushFriendReply(PDU *pdu)
{
    OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
}

void TcpClient::DelFriendReply()
{
    QMessageBox::information(this, "删除", "删除好友成功");
    // 收到回复后直接刷新好友列表
    // OpeWidget::getInstance().getFriend()->flushFriend();
    emit OpeWidget::getInstance().getFriend()->getFlushPB()->click();
}

void TcpClient::PrivateRequest(PDU *pdu)
{
    if (PrivateChat::getInstance().isHidden())
        PrivateChat::getInstance().show();

    char sendName[32] = {0};
    strncpy(sendName, pdu->caData, 32);
    PrivateChat::getInstance().setChatName(sendName);
    PrivateChat::getInstance().updateMsg(pdu);
}

void TcpClient::GroupRequest(PDU *pdu)
{
    OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);
}

void TcpClient::CreateDirReply(PDU *pdu)
{
    if (strlen(pdu->caData) != 0)
    {
        QMessageBox::warning(this, "创建文件夹", pdu->caData);
    }
    else
    {
        emit OpeWidget::getInstance().getBook()->getFlushPB()->click();
    }
}

void TcpClient::FlushFileReply(PDU *pdu)
{
    OpeWidget::getInstance().getBook()->updateFileList(pdu);
    if (0 == strcmp(pdu->caData, "enter dir"))
    {
        QString strEnterDir = OpeWidget::getInstance().getBook()->enterDirName();
        if (!strEnterDir.isEmpty())
        {
            m_strCurPath = m_strCurPath + "/" + strEnterDir;
        }
    }
}

void TcpClient::DelDirReply(PDU *pdu)
{
    if (0 == strcmp(pdu->caData, DEL_DIR_OK))
    {
        emit OpeWidget::getInstance().getBook()->getFlushPB()->click();
    }
    else
    {
        QMessageBox::information(this, "删除文件夹", pdu->caData);
    }
}

void TcpClient::ReNameFileReply(PDU *pdu)
{
    if (0 == strcmp(pdu->caData, RENAME_FILE_FAILED))
        QMessageBox::warning(this, "重命名", pdu->caData);
    else
        emit OpeWidget::getInstance().getBook()->getFlushPB()->click();
}

void TcpClient::EnterDirReply(PDU *pdu)
{
    OpeWidget::getInstance().getBook()->clearEnterDir();
    QMessageBox::warning(this, "进入文件夹", pdu->caData);
}

void TcpClient::DelFileReply(PDU *pdu)
{

    if (0 == strcmp(pdu->caData, DEL_FILE_OK))
    {
        emit OpeWidget::getInstance().getBook()->getFlushPB()->click();
    }
    else
    {
        QMessageBox::information(this, "删除文件夹", pdu->caData);
    }
}

QString TcpClient::getLoginName()
{
    return m_strLoginName;
}

QString TcpClient::getCurPath()
{
    return m_strCurPath;
}

void TcpClient::setCurPath(QString strCurPath)
{
    m_strCurPath = strCurPath;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this, "连接服务器", "连接服务器成功");
}

void TcpClient::recvMsg()
{
    //qDebug() << m_tcpSocket.bytesAvailable();
    uint uiPDULen = 0;
    m_tcpSocket.read((char*)&uiPDULen, sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);

    PDU* pdu = mkPDU(uiMsgLen);
    m_tcpSocket.read((char*)pdu+sizeof(uint), uiPDULen - sizeof(uint));

    switch(pdu->uiMsgType)
    {
    case ENUM_MSG_TYPE_REGIST_RESPOND: //注册回复
        RegistrationReply(pdu);
        break;
    case ENUM_MSG_TYPE_LOGIN_RESPOND:  //登录回复
        LoginReply(pdu);
        break;
    case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:  //显示在线用户回复
        AllOnlineReply(pdu);
        break;
    case ENUM_MSG_TYPE_SEARCH_USER_RESPOND: //搜索用户回复
        SearchUserReply(pdu);
        break;
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:    //添加好友请求
        AddFriendReqReply(pdu);
        break;
    case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:    //添加好友回复
        QMessageBox::information(this, "添加好友", pdu->caData);
        break;
    case ENUM_MSG_TYPE_FRIEND_AGREE_RESPOND:
        QMessageBox::information(this, QString("%1回复").arg(pdu->caData), QString("对方同意了你的好友请求"));
        OpeWidget::getInstance().getFriend()->flushFriend();
        break;
    case ENUM_MSG_TYPE_FRIEND_REFUSE_RESPOND:
        QMessageBox::information(this, QString("%1回复").arg(pdu->caData), QString("对方拒绝了你的好友请求"));
        break;
    case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:    //刷新好友回复
        FlushFriendReply(pdu);
        break;
    case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:   //删除好友回复
        DelFriendReply();
        break;
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:   //删除好友的申请提示
        QMessageBox::information(this, "删除", QString("%1 已将你删除").arg(reinterpret_cast<char*>(pdu->caData)));
        OpeWidget::getInstance().getFriend()->flushFriend();
        break;
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        PrivateRequest(pdu);
        break;
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        GroupRequest(pdu);
        break;
    case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
        CreateDirReply(pdu);
        break;
    case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
        FlushFileReply(pdu);
        break;
    case ENUM_MSG_TYPE_DEL_DIR_RESPOND:
        DelDirReply(pdu);
        break;
    case ENUM_MSG_TYPE_RENAME_FILE_RESPOND:
        ReNameFileReply(pdu);
        break;
    case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
        EnterDirReply(pdu);
        break;
    case ENUM_MSG_TYPE_DEL_FILE_RESPOND:
        DelFileReply(pdu);
        break;
    default:
        break;
    }

    free(pdu);
    pdu = nullptr;
}

#if 0
void TcpClient::on_send_pb_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if (!strMsg.isEmpty())
    {
        PDU* pdu = mkPDU(strMsg.toUtf8().size() + 1);
        pdu->uiMsgType = 8888;
        memcpy(pdu->caMsg, strMsg.toStdString().c_str(), strMsg.toUtf8().size());
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    else
    {
        QMessageBox::warning(this, "信息发送", "发送的信息不能为空");
    }
}
#endif


void TcpClient::on_login_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if (!strName.isEmpty() && !strPwd.isEmpty())
    {
        m_strLoginName = strName;
        //qDebug() << m_strLoginName;
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        //将用户名和密码放到caData中，服务器通过caData获取用户名和密码
        strncpy(pdu->caData, strName.toUtf8().toStdString().c_str(), 32);
        strncpy(pdu->caData+32, strPwd.toUtf8().toStdString().c_str(), 32);
        m_tcpSocket.write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    else
    {
        QMessageBox::critical(this, "登录", "登录失败,用户名或密码错误!");
    }
}

void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if (!strName.isEmpty() && !strPwd.isEmpty())
    {
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData, strName.toUtf8().toStdString().c_str(), 32);
        strncpy(pdu->caData+32, strPwd.toUtf8().toStdString().c_str(), 32);
        m_tcpSocket.write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    else
    {
        QMessageBox::critical(this, "注册", "注册失败,用户名或密码为空!");
    }
}

void TcpClient::on_cancel_pb_clicked()
{

}
