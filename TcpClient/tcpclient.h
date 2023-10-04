#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>
#include "protocol.h"
#include "opewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();
    void connectionServer();

    static TcpClient& getInstance();
    QTcpSocket& getTcpSocket();

    void RegistrationReply(PDU* pdu);
    void LoginReply(PDU* pdu);
    void AllOnlineReply(PDU* pdu);
    void SearchUserReply(PDU* pdu);
    void AddFriendReqReply(PDU* pdu);
    void FlushFriendReply(PDU* pdu);
    void DelFriendReply();
    void PrivateRequest(PDU* pdu);
    void GroupRequest(PDU* pdu);
    void CreateDirReply(PDU* pdu);

    QString getLoginName();
    QString getCurPath();

public slots:
    void showConnect();
    void recvMsg();

private slots:
    // void on_send_pb_clicked();
    void on_login_pb_clicked();
    void on_regist_pb_clicked();
    void on_cancel_pb_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIP;
    quint16 m_usPort;
    //连接服务器,和服务器进行数据交互
    QTcpSocket m_tcpSocket;

    QString m_strLoginName;

    QString m_strCurPath;   //记录路径
};
#endif // TCPCLIENT_H
