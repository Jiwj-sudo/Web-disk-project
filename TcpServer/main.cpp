#include "tcpserver.h"
#include "operatordb.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    OperatorDB::getInstance().init();

    TcpServer w;
    w.show();
    return a.exec();
}
