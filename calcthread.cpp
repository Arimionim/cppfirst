#include "calcthread.h"
#include <iostream>
#include <QCryptographicHash>

void calcThread::listfilesindir(QString path, QList<QString> *files)
{
    QDir dir(path);

    bool ok = dir.exists();
    if (ok)
    {
        dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks);
        dir.setSorting(QDir::Time | QDir::Reversed);
        QFileInfoList list = dir.entryInfoList();
        int l, r;
        l = 0;
        r = list.size();

        for (int i = l; i < r; ++i){
            numFiles++;
            if (QThread::currentThread()->isInterruptionRequested()) {
                emit finished();
                return;
            }
            QFileInfo fileInfo = list.at(i);
            if (fileInfo.isDir()){
                listfilesindir(fileInfo.filePath(), files);
            }
            else{
                files->append(fileInfo.filePath());
            }
        }
    }
}


void calcThread::md5(const QList<QString> &files){
    for (int i = 0; i < files.size(); ++i){
        emit setProgress((i * 100) / numFiles);
        if (QThread::currentThread()->isInterruptionRequested()) {
            emit finished();
            return;
        }
        std::cout << i << std::endl;
        QString fileName = files[i];
        QFile f(fileName);
        try {
            f.open(QFile::ReadWrite);
            QCryptographicHash hash(QCryptographicHash::Algorithm::Md5);
            if (hash.addData(&f)) {
                sendFile(hash.result(), fileName);
            }
        }
        catch (std::exception e){
            sendFile(nullptr, path, true);
        }
    }
}

void calcThread::run()
{
    QList<QString> files;
    numFiles = 0;
    listfilesindir(path, &files);
    md5(files);
    emit scanFinished();
    std::cout << "want to finish" << std::endl;
    emit finished();
    std::cout << "send signal" << std::endl;
}

