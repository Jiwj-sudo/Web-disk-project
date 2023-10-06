#include "opewidget.h"
#include "tcpclient.h"

OpeWidget::OpeWidget(QWidget *parent)
    : QWidget{parent}
{
    this->setWindowTitle(QString("你好, %1").arg(TcpClient::getInstance().getLoginName()));
    this->resize(820, 610);
    m_pListW = new QListWidget(this);
    m_pListW->addItem("好友");
    m_pListW->addItem("图书");

    m_pFriend = new Friend;
    m_pBook = new Book;

    emit m_pFriend->getFlushPB()->click();
    // emit m_pBook->getFlushPB()->click();

    m_pSW =  new QStackedWidget;
    m_pSW->addWidget(m_pFriend);
    m_pSW->addWidget(m_pBook);

    QHBoxLayout* pMain = new QHBoxLayout;
    pMain->addWidget(m_pListW);
    pMain->addWidget(m_pSW);

    setLayout(pMain);

    connect(m_pListW, &QListWidget::currentRowChanged, m_pSW, &QStackedWidget::setCurrentIndex);
    connect(m_pSW, &QStackedWidget::currentChanged, this, [=](int index)
    {
        if (index == 0)
            emit m_pFriend->getFlushPB()->click();
        else if (index == 1)
            emit m_pBook->getFlushPB()->click();
    });
}

OpeWidget &OpeWidget::getInstance()
{
    static OpeWidget instance;
    return instance;
}

Friend *OpeWidget::getFriend()
{
    return m_pFriend;
}

Book *OpeWidget::getBook()
{
    return m_pBook;
}
