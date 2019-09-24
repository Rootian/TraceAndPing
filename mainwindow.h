#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <iostream>
#include <QDebug>
#include <time.h>
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
    void on_pushButtonRun_clicked();
    void readOutput();
    void readOutput_pTraceroute();
    void pingFinished();
    void traceFinished();
    void runPing();
    void intervalChanged();
    void readPingOutput_allThreads();
    void update_hopTable(int hopNum,QString ip, QString count, QString avg,QString min,QString cur,QString pl);

private:
    Ui::MainWindow *ui;
    QProcess *p;
    QProcess **p_threads;
    QProcess *pTraceroute;
    QString output;
    QString interval;
    QList<QList<QString>> hopList; //(hopNum,ip,count,avg,min,PL)
    typedef struct
    {
        int hopNum;
        QString curTime;
        QString delay;
    }traceHopNode;
    QList<traceHopNode> pingResult;
    int validHopNum;


signals:
    void sig(QString hopN);

};

#endif // MAINWINDOW_H
