#ifndef OPERATORDB_H
#define OPERATORDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

class OperatorDB : public QObject
{
    Q_OBJECT
public:
    explicit OperatorDB(QObject *parent = nullptr);
    static OperatorDB& getInstance();
    void init();
    ~OperatorDB();

    bool handleRegist(const char* name, const char* pwd);

signals:
private:
    QSqlDatabase m_db;    //连接数据库
};

#endif // OPERATORDB_H
