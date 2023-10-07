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
    void RegistrationRequest(PDU* pdu);
    void LoginRequest(PDU* pdu);
    void OnlineUserRequests();
    void FindUserRequest(PDU* pdu);
    void AddFriendRequest(PDU* pdu);
    void AddFriendAgree(PDU* pdu);
    void AddFriendRefuse(PDU* pdu);
    void FlushFriendRequest(PDU* pdu);
    void DeleteFriendRequest(PDU* pdu);
    void PrivateRequest(PDU* pdu);
    void GroupRequest(PDU* pdu);
    void CreateDirRequest(PDU* pdu);
    void FlushFileRequest(PDU* pdu);
    void DelDirRequest(PDU* pdu);
    void ReNameFileRequest(PDU* pdu);
    void EnterDirRequest(PDU* pdu);
    void DelFileRequest(PDU* pdu);

signals:
    void offline(MyTcpSocket* mysocket);

public slots:
    void recvMsg();
    void clientOffline();

private:
    QString m_strName;
};

#endif // MYTCPSOCKET_H
