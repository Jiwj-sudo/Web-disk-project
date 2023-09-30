#include "opewidget.h"

OpeWidget::OpeWidget(QWidget *parent)
    : QWidget{parent}
{
    this->resize(820, 610);
    m_pListW = new QListWidget(this);
    m_pListW->addItem("好友");
    m_pListW->addItem("图书");

    m_pFriend = new Friend;
    m_pBook = new Book;

    m_pSW =  new QStackedWidget;
    m_pSW->addWidget(m_pFriend);
    m_pSW->addWidget(m_pBook);

    QHBoxLayout* pMain = new QHBoxLayout;
    pMain->addWidget(m_pListW);
    pMain->addWidget(m_pSW);

    setLayout(pMain);

    connect(m_pListW, &QListWidget::currentRowChanged, m_pSW, &QStackedWidget::setCurrentIndex);
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
