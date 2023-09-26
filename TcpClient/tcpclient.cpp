#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    // 加载配置
    loadConfig();
    // 连接服务器
    connectionServer();
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


    // 连接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);
}

void TcpClient::showConnect()
{
    QMessageBox::information(this, "连接服务器", "连接服务器成功");
}

