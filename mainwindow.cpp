#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->lineEdit_Interval,SIGNAL(textChanged(const QString)),this,SLOT(intervalChanged()));
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_pushButtonRun_clicked()
{

    QString traceCommand = "traceroute";
    QStringList traceArgument;
    traceArgument << "-q" << "1" << "-n" << ui->lineEdit_IP->text();
//    ui->lineEdit_IP->setText(ip_input);

    pTraceroute = new QProcess;
    connect(pTraceroute,SIGNAL(readyReadStandardOutput()),this,SLOT(readOutput_pTraceroute()));
    pTraceroute->start(traceCommand,traceArgument);
    connect(pTraceroute,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(traceFinished()));
//    p->execute(ip_input);
//    p->waitForFinished(-1);

}

void MainWindow::readOutput()
{
    QString outputLine = p->readAll();
    qDebug() << outputLine;

    output = output + p->readAll();
//    qDebug() << "gg";
    ui->textEditShowResult->setText(output);
}
void MainWindow::readOutput_pTraceroute()
{
    //thread used to read the output of Traceroute
    QString outputLine = pTraceroute->readAllStandardOutput();
    QStringList splitOutputLine = outputLine.trimmed().split(" ");

//    output = output + pTraceroute->readAll();
    if(splitOutputLine[2] == "*")
    {
        //icmp packet filtered
        qDebug() << splitOutputLine[0] + ": filtered";
        QString hopNum = splitOutputLine[0];
        QString ip = "-";
        QString delay = "*";
        QString count = "1";
        QString pl = "100";
        //add next hop line
        ui->tableWidget_hopList->setRowCount(hopNum.toInt());
        //update the hop table
        update_hopTable(hopNum.toInt(),ip,count,delay,delay,delay,pl);

        //store the data
        QList<QString> hopItem;
        hopItem << hopNum << ip << count << delay << delay << pl;
        hopList << hopItem;

        traceHopNode node;
        node.hopNum = hopNum.toInt();
        node.delay = delay;
        time_t curT = time(NULL);
        tm* t = localtime(&curT);
        node.curTime = QString::number(t->tm_hour) + ":" + QString::number(t->tm_min) + ":" + QString::number(t->tm_sec);
        pingResult << node;

    }
    else
    {
        //normal node
        QString hopNum = splitOutputLine[0];
        QString ip = splitOutputLine[2];
        QString delay = splitOutputLine[4];
        QString count = "1";
        QString pl = "*";
        qDebug() << "hopNum: " + hopNum << "ip: " + ip << "delay: " + delay;

        //add next hop line
        ui->tableWidget_hopList->setRowCount(hopNum.toInt());
        //update the hop table
        update_hopTable(hopNum.toInt(),ip,count,delay,delay,delay,pl);


//        store the data
        QList<QString> hopItem;
        hopItem << hopNum << ip << count << delay << delay << pl;
        hopList << hopItem;

        traceHopNode node;
        node.hopNum = hopNum.toInt();
        node.delay = delay;
        time_t curT = time(NULL);
        tm* t = localtime(&curT);
        node.curTime = QString::number(t->tm_hour) + ":" + QString::number(t->tm_min) + ":" + QString::number(t->tm_sec);
        pingResult << node;

    }

    ui->textEditShowResult->setText(outputLine);
}
void MainWindow::pingFinished()
{
    qDebug() << "ping is finished";
//    for (int i = 0; i < pingResult.length(); i++) {
//        qDebug() << pingResult[i].curTime;
//    }

}
void MainWindow::traceFinished()
{
    //traceroute is done
    qDebug() << "traceroute is done";
    for (int i = 0; i < hopList.length(); i++)
    {
        qDebug() << hopList[i];
    }
//    for (int i = 0;i < pingResult.length(); i++)
//    {
//        qDebug() << pingResult[i].hopNum << pingResult[i].curTime->tm_hour << pingResult[i].curTime->tm_min << pingResult[i].curTime->tm_sec;
//    }

    //start to ping
    runPing();
}
void MainWindow::runPing()
{
    //start to ping each hop node within an interval
    QString pingCommand = "ping";
    QString hopNum = "0";
    validHopNum = 0;
    for (int i = 0;i < hopList.length(); i++)
    {
        if(hopList[i][1] != "-")
        {
            //calculate the valid hop num
            validHopNum ++;
        }
    }
    p_threads = new QProcess*[hopList.length()];
    for(int i = 0; i < hopList.length(); i++)
    {
        p_threads[i] = new QProcess();
        if(hopList[i][1] == "-") continue;
        connect(p_threads[i],SIGNAL(readyReadStandardOutput()),this,SLOT(readPingOutput_allThreads()));
        QStringList pingArgument;
        pingArgument << "-i" << interval << hopList[i][1];
        p_threads[i]->start(pingCommand,pingArgument);
        connect(p_threads[i],SIGNAL(finished(int, QProcess::ExitStatus)),this,SLOT(pingFinished()));
    }

}
void MainWindow::readPingOutput_allThreads()
{
    QString outputline;
    QString curOutput;
    for (int i = 0; i < hopList.length(); i++) {
        if(hopList[i][1] == "-") continue;
        outputline = p_threads[i]->readAllStandardOutput().trimmed();
        if(outputline != ""){
            qDebug() << outputline;
        }

        if(outputline.startsWith("64 bytes from"))
        {
            //current process to read
            QStringList outputLineSplited = outputline.split(" ");
            QString delay = outputLineSplited[6].mid(5);
            QStringList ipTemp = outputLineSplited[3].split(":");
            int icmp_seq = outputLineSplited[4].mid(9).toInt();
            QString ip = ipTemp[0];
            qDebug() << ip;
            QString hopNum;
            for (int i = 0;i < hopList.length(); i ++) {
                //match the ip in the hoplist
                if(ip == hopList[i][1])
                {
                    //hop node found
                    hopNum = hopList[i][0];
                    qDebug() << hopNum;
                }
            }
            //calculate the arguments
            double dCount = hopList[hopNum.toInt()-1][2].toDouble() + 1.0;
            double dAvg = (hopList[hopNum.toInt()-1][3].toDouble() * (dCount - 1) + delay.toDouble()) / dCount;
            if(hopList[hopNum.toInt()-1][4].toDouble() > delay.toDouble())
            {
                //update min
                hopList[hopNum.toInt()-1][4] = delay;
            }
            //update the arguments
            hopList[hopNum.toInt()-1][2] = QString::number(dCount);
            hopList[hopNum.toInt()-1][3] = QString::number(dAvg);

            QString count = hopList[hopNum.toInt()-1][2];
            QString avg = hopList[hopNum.toInt()-1][3];
            QString min = hopList[hopNum.toInt()-1][4];
            double pl = (double(icmp_seq + 1) - (count.toDouble())) / double(icmp_seq + 1) * 100;
            update_hopTable(hopNum.toInt(),ip,count,avg,min,delay,QString::number(pl));

            //store the data
            traceHopNode node;
            node.hopNum = hopNum.toInt();
            node.delay = delay;
            time_t curT = time(NULL);
            tm* t = localtime(&curT);
            node.curTime = QString::number(t->tm_hour) + ":" + QString::number(t->tm_min) + ":" + QString::number(t->tm_sec);
            pingResult << node;

        }
        else if(outputline.startsWith("Request timeout"))
        {
            //timeout for this imcp_seq


        }
    }
}

void MainWindow::update_hopTable(int hopNum,QString ip, QString count, QString avg,QString min,QString cur,QString pl)
{
    //update the hop table
    //update ip
    QTableWidgetItem *ipItem = new QTableWidgetItem(ip);
    ui->tableWidget_hopList->setItem(hopNum-1,1,ipItem);
    //update Cur
    QTableWidgetItem *curItem = new QTableWidgetItem(cur);
    ui->tableWidget_hopList->setItem(hopNum-1,4,curItem);
    //update Avg
    QTableWidgetItem *avgItem = new QTableWidgetItem(avg);
    ui->tableWidget_hopList->setItem(hopNum-1,2,avgItem);
    //update Min
    QTableWidgetItem *minItem = new QTableWidgetItem(min);
    ui->tableWidget_hopList->setItem(hopNum-1,3,minItem);
    //update count
    QTableWidgetItem *countItem = new QTableWidgetItem(count);
    ui->tableWidget_hopList->setItem(hopNum-1,0,countItem);
    //update PL
    QTableWidgetItem *plItem = new QTableWidgetItem(pl);
    ui->tableWidget_hopList->setItem(hopNum-1,5,plItem);
}
void MainWindow::intervalChanged()
{
    //interval changed
    interval = ui->lineEdit_Interval->text();
//    qDebug() << interval;
}
