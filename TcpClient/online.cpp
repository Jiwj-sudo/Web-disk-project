﻿#include "online.h"
#include "ui_online.h"
#include "tcpclient.h"
#include <string.h>
#include <QListWidgetItem>

Online::Online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showUser(PDU *pdu)
{
    if (nullptr == pdu)
        return;
    //ui->online_lw->clear();
    uint uiSize = pdu->uiMsgLen / 32;
    char caTmp[32];
    for (uint i = 0 ; i < uiSize; i++)
    {
        memcpy(caTmp, reinterpret_cast<char*>(pdu->caMsg) + i*32, 32);
        if ((ui->online_lw->findItems(caTmp, Qt::MatchFixedString | Qt::MatchCaseSensitive)).size() == 0)
            ui->online_lw->addItem(caTmp);
    }
}

void Online::on_add_pb_clicked()
{
    QListWidgetItem* pItem = ui->online_lw->currentItem();
    QString strPerUserName = pItem->text();
    QString strLoginName = TcpClient::getInstance().getLoginName();
    //qDebug() << strPerUserName << " " << strLoginName;

    PDU* pdu = mkPDU(0);

    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    strncpy(pdu->caData, strPerUserName.toStdString().c_str(), 32);
    strncpy(pdu->caData + 32, strLoginName.toStdString().c_str(), 32);
    //qDebug() << pdu->caData << " " << pdu->caData+32;
    TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

