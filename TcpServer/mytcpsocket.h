#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include "operatordb.h"

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString getName();

signals:
    void offline(MyTcpSocket* mysocket);

public slots:
    void recvMsg();
    void clientOffline();

private:
    QString m_strName;
};

#endif // MYTCPSOCKET_H
