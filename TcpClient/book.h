﻿#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);

signals:

private:
    QListWidget* m_pBookListW;
    QPushButton* m_pReturnPB;
    QPushButton* m_pCreateDirPB;
    QPushButton* m_pDelDirPB;
    QPushButton* m_pReNamePB;
    QPushButton* m_pFlushDirPB;
    QPushButton* m_pUploadPB;
    QPushButton* m_pDownLoadPB;
    QPushButton* m_pDelFilePB;
    QPushButton* m_pShareFilePB;
};

#endif // BOOK_H
