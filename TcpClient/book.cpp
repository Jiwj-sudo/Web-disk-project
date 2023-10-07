#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QVariant>
#include <QFileDialog>

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    m_strEnterDir.clear();

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
    connect(m_pDelDirPB, &QPushButton::clicked, this, &Book::delDir);
    connect(m_pReNamePB, &QPushButton::clicked, this, &Book::reName);
    connect(m_pBookListW, &QListWidget::doubleClicked, this, &Book::enterDir);
    connect(m_pReturnPB, &QPushButton::clicked, this, &Book::returnPre);
    connect(m_pDelFilePB, &QPushButton::clicked, this, &Book::delRegFile);
    connect(m_pUploadPB, &QPushButton::clicked, this, &Book::uploadFile);
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

QPushButton *Book::getFlushPB()
{
    return m_pFlushFilePB;
}

void Book::clearEnterDir()
{
    m_strEnterDir.clear();
}

QString Book::enterDirName()
{
    return m_strEnterDir;
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

void Book::delDir()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QListWidgetItem* pItem = m_pBookListW->currentItem();
    if(pItem)
    {
        QString strDelName = pItem->text();
        PDU* pdu = mkPDU(strCurPath.toUtf8().size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData, strDelName.toUtf8().toStdString().c_str(), 32);
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.toUtf8().size()+1);

        TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
}

void Book::reName()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QListWidgetItem* pItem = m_pBookListW->currentItem();
    if (nullptr == pItem)
    {
        QMessageBox::warning(this, "重命名文件", "请选择文件");
    }
    else
    {
        QString strOldName = pItem->text();
        QString strNewName = QInputDialog::getText(this, "重命名文件", "请输入新的文件名");
        if (!strNewName.isEmpty())
        {
            PDU* pdu = mkPDU(strCurPath.toUtf8().size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strcpy(pdu->caData, strOldName.toUtf8().toStdString().c_str());
            strcpy(pdu->caData + 32, strNewName.toUtf8().toStdString().c_str());
            memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.toUtf8().size()+1);

            TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
            free(pdu);
            pdu = nullptr;
        }
    }
}

void Book::enterDir(const QModelIndex &index)
{
    QString strDirName = index.data().toString();
    m_strEnterDir = strDirName;
    QString strCurPath = TcpClient::getInstance().getCurPath();
    PDU* pdu = mkPDU(strCurPath.toUtf8().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strcpy(pdu->caData, strDirName.toUtf8().toStdString().c_str());
    memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.toUtf8().size()+1);

    TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Book::returnPre()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QString strRootPath = "../home/" + TcpClient::getInstance().getLoginName();
    if (strCurPath == strRootPath)
    {
        QMessageBox::warning(this, "返回", "返回失败: 已经在根目录");
    }
    else
    {
        int index = strCurPath.lastIndexOf("/");
        strCurPath.remove(index, strCurPath.toUtf8().size()-index);
        // qDebug() << "returnPre: " << strCurPath;
        TcpClient::getInstance().setCurPath(strCurPath);

        clearEnterDir();
        flushFile();
    }
}

void Book::delRegFile()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QListWidgetItem* pItem = m_pBookListW->currentItem();
    if(pItem)
    {
        QString strDelName = pItem->text();
        PDU* pdu = mkPDU(strCurPath.toUtf8().size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
        strncpy(pdu->caData, strDelName.toUtf8().toStdString().c_str(), 32);
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.toUtf8().size()+1);

        TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
}

void Book::uploadFile()
{
    QString strUploadFilePath = QFileDialog::getOpenFileName();
    if (!strUploadFilePath.isEmpty())
    {
        int index = strUploadFilePath.lastIndexOf("/");
        QString strFileName = strUploadFilePath.right(strUploadFilePath.toUtf8().size()-index-1);

        QFile file(strUploadFilePath);
        qint64 fileSize = file.size();   //获得文件大小

        QString strCurPath = TcpClient::getInstance().getCurPath();

        PDU* pdu = mkPDU(strCurPath.toUtf8().size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.toUtf8().size()+1);
        sprintf(pdu->caData, "%s %lld", strFileName.toUtf8().toStdString().c_str(), fileSize);

        TcpClient::getInstance().getTcpSocket().write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    else
    {
        QMessageBox::warning(this, "上传文件", "请选择一个文件");
    }
}
