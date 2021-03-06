#ifndef CALCTHREAD_H
#define CALCTHREAD_H

#include "mainwindow.h"

#include <QThread>
#include <QMainWindow>
#include <QFileSystemModel>
#include <QObject>

class calcThread : public QObject
 {
    Q_OBJECT
public:
    QString path;

public slots:
    void run();

signals:
    void finished(int );
    void setProgress(int v);
    void sendFile(QByteArray hash, QString path, bool error = false);

private:
    int numFiles;
    void listfilesindir(QString, QList<QString>*);
    void compare(const QList<QString>&);

};
#endif // CALCTHREAD_H
