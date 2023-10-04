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
    m_pFlushDirPB = new QPushButton("刷新文件");

    QVBoxLayout* pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pReNamePB);
    pDirVBL->addWidget(m_pFlushDirPB);

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
