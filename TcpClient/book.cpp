#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    m_pBookListW = new QListWidget;

    m_pReturnPB = new QPushButton("返回");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDelDirPB = new QPushButton("删除文件夹");
    m_pReNamePB = new QPushButton("重命名");
    m_pFlushFilePB = new QPushButton("刷新文件");

    QVBoxLayout* pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pReNamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    m_pUploadPB = new QPushButton("上传文件");
    m_pDownLoadPB = new QPushButton("下载文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pShareFilePB = new QPushButton("分享文件");

    QVBoxLayout* pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);

    QHBoxLayout* pMain = new QHBoxLayout;
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);

    connect(m_pCreateDirPB, &QPushButton::clicked, this, &Book::createDir);
    connect(m_pFlushFilePB, &QPushButton::clicked, this, &Book::flushFile);
}

void Book::updateFileList(const PDU *pdu)
{
    if (nullptr == pdu)
        return;

    QListWidgetItem* pItemTmp = nullptr;
    int row = m_pBookListW->count();
    while (m_pBookListW->count() > 0)
    {
        pItemTmp = m_pBookListW->item(row-1);
        m_pBookListW->removeItemWidget(pItemTmp);
        delete pItemTmp;
        row--;
    }

    const FileInfo* pFileInfo = nullptr;
    int iCount = pdu->uiMsgLen / sizeof(FileInfo);

    for (int i = 0; i < iCount; i++)
    {

        pFileInfo = reinterpret_cast<const FileInfo*>(pdu->caMsg) + i;
        if(0 == strcmp(pFileInfo->caFileName, ".") || 0 == strcmp(pFileInfo->caFileName, ".."))
            continue;

        QListWidgetItem* pItem = new QListWidgetItem;
        if(0 == pFileInfo->iFileType)
            pItem->setIcon(QIcon(QPixmap(":/map/dir.jpg")));
        else if (1 == pFileInfo->iFileType)
            pItem->setIcon(QIcon(QPixmap(":/map/file.jpg")));

        pItem->setText(pFileInfo->caFileName);
        m_pBookListW->addItem(pItem);
    }
}

void Book::createDir()
{
    QString strNewDir = QInputDialog::getText(this, "新建文件夹", "新文件夹名字");
    if (strNewDir.toUtf8().size() >= 32)
        QMessageBox::warning(this, "警告", "文件夹名称长度不能超过32");

    if (!strNewDir.isEmpty())
    {
        QString strName = TcpClient::getInstance().getLoginName();
        QString strCurPath = TcpClient::getInstance().getCurPath();
        PDU* pdu = mkPDU(strCurPath.toUtf8().size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
        strncpy(pdu->caData, strName.toUtf8().toStdString().c_str(), 32);
        strncpy(pdu->caData+32, strNewDir.toUtf8().toStdString().c_str(), 32);

        strcpy(reinterpret_cast<char*>(pdu->caMsg), strCurPath.toUtf8().toStdString().c_str());

        TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    else
    {
        QMessageBox::warning(this, "警告", "文件夹名称不能为空");
    }
}

void Book::flushFile()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    PDU* pdu = mkPDU(strCurPath.toUtf8().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strcpy(reinterpret_cast<char*>(pdu->caMsg), strCurPath.toUtf8().toStdString().c_str());

    TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}
