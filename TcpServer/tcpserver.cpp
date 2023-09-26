#include "tcpserver.h"
#include "ui_tcpserver.h"
#include "mytcpserver.h"
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    this->setFixedSize(380,250);
    ui->setupUi(this);
    loadConfig();
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP), m_usPort);
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config");
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
    qDebug() << m_strIP << ":" << m_usPort;
}

