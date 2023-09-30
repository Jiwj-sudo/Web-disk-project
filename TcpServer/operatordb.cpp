#include "operatordb.h"
#include <QMessageBox>
#include <QDebug>

OperatorDB::OperatorDB(QObject *parent)
    : QObject{parent}
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

OperatorDB &OperatorDB::getInstance()
{
    static OperatorDB instance;
    return instance;
}

void OperatorDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("E:\\code\\Web-disk-project\\TcpServer\\cloud.db");
    if (m_db.open())
    {
        QSqlQuery query;
        query.exec("select * from userInfo");
        while (query.next())
        {
            QString data = QString("%1,%2,%3").arg(query.value(0).toString())
                               .arg(query.value(1).toString())
                               .arg(query.value(2).toString());
            qDebug() << data;
        }
    }
    else
    {
        QMessageBox::critical(NULL, "打开数据库", "打开数据库失败!");
    }
}

OperatorDB::~OperatorDB()
{
    m_db.close();
}

bool OperatorDB::handleRegist(const char *name, const char *pwd)
{
    if (nullptr == name || nullptr == pwd)
        return false;

    QString data = QString("insert into userInfo(name, pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);
    QSqlQuery query;
    return query.exec(data);
}

bool OperatorDB::handleLogin(const char *name, const char *pwd)
{
    if (nullptr == name || nullptr == pwd)
        return false;
    QString data = QString("select * from userInfo where name=\'%1\' and pwd=\'%2\' and online=0").arg(name).arg(pwd);
    QSqlQuery query;
    query.exec(data);
    if (query.next())
    {
        data = QString("update userInfo set online=1 where name=\'%1\' and pwd=\'%2\'").arg(name).arg(pwd);
        // QSqlQuery query;
        query.exec(data);

        return true;
    }
    else
    {
        return false;
    }
}

void OperatorDB::handleOffline(const char *name)
{
    if (nullptr == name)
        return;

    QString data = QString("update userInfo set online=0 where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
}
