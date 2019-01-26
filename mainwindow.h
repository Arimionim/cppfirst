#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QTreeWidgetItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();    

private slots:
    void on_dir_doubleClicked(const QModelIndex &index);

    void on_goButton_clicked();
    void on_findButton_clicked();

    void on_cancelButton_clicked();

    void setProgress(int);
    void scanFinished(int);
    void getFile(QByteArray hash, QString path, bool error);

    void on_deleteButton_clicked();

private:
    QMap<QByteArray, QTreeWidgetItem*> nodes;

    QString rootPath;
    Ui::MainWindow *ui;
    QThread* job;
    QFileSystemModel *model;
};
#endif // MAINWINDOW_H
