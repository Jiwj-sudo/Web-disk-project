#ifndef OPERATORDB_H
#define OPERATORDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>

class OperatorDB : public QObject
{
    Q_OBJECT
public:
    explicit OperatorDB(QObject *parent = nullptr);
    static OperatorDB& getInstance();
    void init();
    ~OperatorDB();

    bool handleRegist(const char* name, const char* pwd);
    bool handleLogin(const char* name, const char* pwd);
    void handleOffline(const char* name);
    QStringList handleAllOnline();
    int handleSearchUser(const char* name);

signals:
private:
    QSqlDatabase m_db;    //连接数据库
};

#endif // OPERATORDB_H
