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

QStringList OperatorDB::handleAllOnline()
{
    QString data = QString("select name from userInfo where online=1");
    QSqlQuery query;
    query.exec(data);

    QStringList result;  //结果保存在此
    result.clear();

    while (query.next())
    {
        result.append(query.value(0).toString());
    }
    return result;
}

int OperatorDB::handleSearchUser(const char *name)
{
    if (nullptr == name)
        return -1;

    QString data = QString("select online from userInfo where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    if (query.next())
    {
        int ret = query.value(0).toInt();
        if (1 == ret)
        {
            return 1;
        }
        else if (0 == ret)
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }
}

int OperatorDB::handleAddFriend(const char *perName, const char *Name)
{
    if (nullptr == perName || nullptr == Name)
        return -1;

    if (0 == strcmp(perName, Name))  //自己加自己
        return -2;

    //是否是自己的好友
    QString data = QString("select * from friend where (id=(select id from userInfo where name=\'%1\') and friendId=(select id from userInfo where name=\'%2\')) "
                           "or (id=(select id from userInfo where name=\'%3\') and friendId=(select id from userInfo where name=\'%4\'))").arg(perName).arg(Name).arg(Name).arg(perName);

    // qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    if (query.next())
    {
        return 0;   //双方已经是好友
    }
    else
    {
        data = QString("select online from userInfo where name=\'%1\'").arg(perName);
        QSqlQuery query;
        query.exec(data);
        if (query.next())
        {
            int ret = query.value(0).toInt();
            if (1 == ret)
            {
                return 1;  //在线
            }
            else if (0 == ret)
            {
                return 2;  //不在线
            }
        }
        else
        {
            return 3;    //不存在
        }
    }
}

void OperatorDB::handleAddFriendAgree(const char *perName, const char *Name)
{
    if (nullptr == perName || nullptr == Name)
    {
        qInfo()<<"handleADDFriendAgree function name or pername is nullptr ";
        return ;
    }
    // qDebug() << perName << Name;
    QString data = QString("insert into friend (id,friendId) values((select id from userInfo where name=\'%1\') "
                           ",(select id from userInfo where name=\'%2\')) ").arg(perName).arg(Name);

    // qDebug() << data;
    QSqlQuery query;
    query.exec(data);
}

QStringList OperatorDB::handleFlushFriend(const char *name)
{
    QStringList strFriendList;
    strFriendList.clear();

    if (nullptr == name)
        return strFriendList;

    QString data = QString("select name from userInfo where online=1 and id in (select id from friend where friendId=(select id from userInfo where name=\'%1\'))").arg(name);
    QSqlQuery query;
    query.exec(data);
    while (query.next())
    {
        strFriendList.append(query.value(0).toString());
        // qDebug() << query.value(0).toString();
    }

    data = QString("select name from userInfo where online=1 and id in (select friendId from friend where id=(select id from userInfo where name=\'%1\'))").arg(name);
    query.exec(data);
    while (query.next())
    {
        strFriendList.append(query.value(0).toString());
        // qDebug() << query.value(0).toString();
    }

    return strFriendList;
}

bool OperatorDB::handleDelFriend(const char *name, const char *friendName)
{
    if (nullptr == name || nullptr == friendName)
        return false;

    QString name_id = QString("select id from userInfo where name=\'%1\'").arg(name);
    QString friendName_id = QString("select id from userInfo where name=\'%1\'").arg(friendName);

    QSqlQuery query;

    query.exec(name_id);
    if(query.next())
        name_id = query.value(0).toString();
    else
        return false;

    query.exec(friendName_id);
    if(query.next())
        friendName_id = query.value(0).toString();
    else
        return false;

    QString data = QString("delete from friend where id=%1 and friendId=%2").arg(name_id).arg(friendName_id);
    query.exec(data);

    data = QString("delete from friend where id=%1 and friendId=%2").arg(friendName_id).arg(name_id);
    query.exec(data);
    return true;
}
