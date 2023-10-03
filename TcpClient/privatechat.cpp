#include "privatechat.h"
#include "ui_privatechat.h"
#include "protocol.h"
#include "tcpclient.h"

PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::setChatName(QString strName)
{
    m_strChatName = strName;
    m_strLoginName = TcpClient::getInstance().getLoginName();
}

void PrivateChat::on_sendMsg_pb_clicked()
{
    QString strMsg = ui->inputMsg_le->text();
    if(!strMsg.isEmpty())
    {
        PDU* pdu = mkPDU(strMsg.toUtf8().size()+1);

        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        strncpy(reinterpret_cast<char*>(pdu->caData), m_strLoginName.toUtf8().toStdString().c_str(), 32);
        strncpy(reinterpret_cast<char*>(pdu->caData) +32, m_strChatName.toUtf8().toStdString().c_str(), 32);
        strncpy(reinterpret_cast<char*>(pdu->caMsg), strMsg.toUtf8().toStdString().c_str(), strMsg.toUtf8().size()+1);

        TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu),pdu->uiPDULen);

        free(pdu);
        pdu = nullptr;
    }
}

