#include "privatechat.h"
#include "ui_privatechat.h"
#include "protocol.h"
#include "tcpclient.h"

PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);

    connect(ui->inputMsg_le, &QLineEdit::returnPressed, this, &PrivateChat::on_sendMsg_pb_clicked);
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

void PrivateChat::updateMsg(const PDU *pdu)
{
    if (nullptr == pdu)
        return ;

    char sendName[32] = {0};
    strncpy(sendName, pdu->caData, 32);

    QString strMsg = QString("%1 say: %2").arg(sendName).arg(reinterpret_cast<const char*>(pdu->caMsg));
    ui->showMsg_te->append(strMsg);
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

        ui->showMsg_te->append(QString("I say: %1").arg(strMsg));
        ui->inputMsg_le->clear();
    }
}

