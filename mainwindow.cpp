#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "calcthread.h"
#include <vector>
#include <QDir>
#include <QList>
#include <iostream>
#include <QCryptographicHash>
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    model = new QFileSystemModel(this);
    model->setFilter(QDir::QDir::AllEntries);
    model->setRootPath("");
    ui->dir->setModel(model);
    ui->progressBar->setValue(0);
    job = nullptr;
    ui->duples->setHeaderHidden(1);
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_dir_doubleClicked(const QModelIndex &index){
    QListView *listView = (QListView*)sender();
    QFileInfo fileInfo = model->fileInfo(index);
    if (fileInfo.fileName() == ".."){
        QDir dir = fileInfo.dir();
        dir.cdUp();
        listView->setRootIndex(model->index(dir.absolutePath()));
    }
    else if (fileInfo.fileName() == "."){
        listView->setRootIndex(model->index(""));
    }
    else if (fileInfo.isDir()){
        listView->setRootIndex(index);
    }
    ui->path->setText(model->filePath(listView->rootIndex()));
}

void MainWindow::on_goButton_clicked()
{
    QString path = ui->path->text();
    QListView *listView = ui->dir;

    if ((model->index(path)).isValid()){
        listView->setRootIndex(model->index(path));
    }
    else{
        ui->statusBar->showMessage("Incorrect path", 2000);
    }
}

void MainWindow::on_findButton_clicked()
{
    if (job && job->isRunning() && !job->isFinished()){
        ui->statusBar->showMessage("Wait please", 2000);
        return;
    }
    calcThread *calc = new calcThread();
    nodes.clear();
    ui->duples->clear();
    job = new QThread;
    calc->path = model->filePath(ui->dir->rootIndex());
    rootPath = model->filePath(ui->dir->rootIndex());
    connect(job, &QThread::started, calc, &calcThread::run);

    connect(calc, &calcThread::setProgress, this, &MainWindow::setProgress);
    connect(calc, &calcThread::scanFinished, this, &MainWindow::scanFinished);
    connect(calc, &calcThread::sendFile, this, &MainWindow::getFile);

    connect(job, &QThread::finished, calc, &calcThread::deleteLater);
    connect(job, &QThread::finished, job, &QThread::deleteLater);

    calc->moveToThread(job);
    ui->progressBar->setValue(0);
    ui->statusLabel->setText("Searching...");
    job->start();
}

void MainWindow::on_cancelButton_clicked()
{
    if (job && job->isRunning() && !job->isFinished()){
        job->requestInterruption();
    }
}

void MainWindow::getFile(QByteArray hash, QString path, bool error){
    if (error){
        QTreeWidgetItem *item = new QTreeWidgetItem(
            ui->duples,
            QStringList(path.remove(0, rootPath.size()))
        );
        item->setTextColor(0, Qt::red);
        ui->duples->expandItem(item);
        return;
    }

    bool newItem = false;
    if (nodes.find(hash) == nodes.end()) {
        QTreeWidgetItem *node = new QTreeWidgetItem(ui->duples, QStringList{"if you see it smth go wrong"});
        nodes[hash] = node;
        newItem = true;
        ui->duples->expandItem(node);
        node->setHidden(true);
    }
    else{
        if (nodes[hash]->isHidden()){
            nodes[hash]->setHidden(false);
        }
        nodes[hash]->setText(0, QString::number(nodes[hash]->childCount() + 1) + " files");
    }

    QTreeWidgetItem *item = new QTreeWidgetItem(
        nodes[hash],
        QStringList(path.remove(0, rootPath.size()))
    );

    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(0, !newItem ? Qt::Checked : Qt::Unchecked);
}

void MainWindow::scanFinished(){
    emit setProgress(100);
    ui->statusLabel->setText("Finished");
}

void MainWindow::setProgress(int v){
    ui->progressBar->setValue(v);
}

void MainWindow::on_deleteButton_clicked()
{
    ui->progressBar->setValue(0);
    QTreeWidgetItemIterator it(ui->duples);
    std::cout << rootPath.toStdString() << std::endl;
    while (*it) {
        if (!(*it)->isHidden() && (*it)->checkState(0) == Qt::Checked) {
            QFile file(rootPath + (*it)->text(0));
            if (!file.remove()) {
                (*it)->setDisabled(true);
                (*it)->setCheckState(0, Qt::Unchecked);
            } else {
                (*it)->setHidden(true);
            }
        }
        it++;
    }
    ui->progressBar->setValue(100);
}
