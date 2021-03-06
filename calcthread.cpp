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
                emit finished(0);
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


void calcThread::compare(const QList<QString> &files){
    for (int i = 0; i < files.size(); ++i){
        emit setProgress((i * 100) / numFiles);
        if (QThread::currentThread()->isInterruptionRequested()) {
            emit finished(i);
            return;
        }
        QString fileName = files[i];
        QFile f(fileName);
        try {
            f.open(QFile::ReadOnly);
            QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);
            if (hash.addData(&f)) {
                sendFile(hash.result(), fileName);
            }
        }
        catch (std::exception const &e){
            sendFile(nullptr, path, true);
        }
    }
}

void calcThread::run()
{
    QList<QString> files;
    numFiles = 0;
    listfilesindir(path, &files);
    compare(files);
    emit finished(numFiles);
}

