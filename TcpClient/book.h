#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "protocol.h"

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);

    void updateFileList(const PDU* pdu);
    QPushButton* getFlushPB();

signals:
public slots:
    void createDir();
    void flushFile();
    void delDir();

private:
    QListWidget* m_pBookListW;
    QPushButton* m_pReturnPB;
    QPushButton* m_pCreateDirPB;
    QPushButton* m_pDelDirPB;
    QPushButton* m_pReNamePB;
    QPushButton* m_pFlushFilePB;
    QPushButton* m_pUploadPB;
    QPushButton* m_pDownLoadPB;
    QPushButton* m_pDelFilePB;
    QPushButton* m_pShareFilePB;
};

#endif // BOOK_H
