#include "mytcpserver.h"
#include <QDebug>

MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "new client connected";
    MyTcpSocket* pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket, &MyTcpSocket::offline, this, &MyTcpServer::deleteSocket);
}

void MyTcpServer::deleteSocket(MyTcpSocket *mysocket)
{
    QList<MyTcpSocket*>::Iterator iter = m_tcpSocketList.begin();
    for(; iter != m_tcpSocketList.end(); iter++)
    {
        if (mysocket == *iter)
        {
            //delete *iter;
            (*iter)->deleteLater();
            *iter = nullptr;
            m_tcpSocketList.erase(iter);
            break;
        }
    }

    for (int i = 0; i < m_tcpSocketList.size(); i++)
    {
        qDebug() << m_tcpSocketList[i]->getName();
    }
}
