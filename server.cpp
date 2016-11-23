#include "server.h"
#include "ui_server.h"
#include <mainwindow.h>
#include <QNetworkInterface>
#include <qmath.h>
#include <QRegExpValidator>
server::server(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::server)
{
    ui->setupUi(this);
    ui->pbSend->setEnabled(false);
    setTrayIconActions();
    showTrayIcon();
    show();
    ui->textBrowser->append("                                           Welcome to Simple Chat Addon");
    QNetworkAccessManager *manager = new QNetworkAccessManager();
   // QNetworkRequest request(QUrl("http://ipinfo.io/ip"));
    QNetworkRequest request(QUrl("http://www.grio.ru/myip.php"));
    QNetworkReply *reply = manager->get(request);
    connect(reply,SIGNAL(finished()),this,SLOT(replyFinished()));
    ui->tableWidget->setEnabled(false);
    connect(ui->cbMode,SIGNAL(clicked(bool)),ui->tableWidget,SLOT(setEnabled(bool)));
    cli = new QTcpSocket(this);
    srv = new QTcpServer(this);
    QIcon ico(QApplication::applicationDirPath()+"/ico.png");
    setWindowIcon(ico);
    QValidator *valid = new QRegExpValidator(QRegExp("[a-zA-Z0-9А-Яа-я ]+"),this);
    ui->leNick->setValidator(valid);
}
server::~server()
{
    delete ui;
}

void server::replyFinished(){
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
      if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray content= reply->readAll();
            QTextCodec *codec = QTextCodec::codecForName("cp1251");
         // ui->textBrowser->append("Current IP -"+codec->toUnicode(content.data()));
            QString data = codec->toUnicode(content.data());
            //data.chop(1);
            ui->le_Currentip->setText(data);
          //  ui->le_ip->setText(data);
        }
      else ui->textBrowser->append("[SYSTEM]Can't connect to server - "+reply->errorString());
      reply->deleteLater();
}

void server::closeEvent(QCloseEvent *event){
    //if (cli->state() == QTcpSocket::ConnectedState)
   // {
        cli->write("[SERVER]Client [" +ui->leNick->text().toUtf8()+ "] disconnected.");
        cli->close();
        srv->close();
   // }
    event->accept();
}

void server::createServer(int port)
{
    if (!srv->listen(QHostAddress::Any, port)){
        ui->textBrowser->append(QTime::currentTime().toString().toUtf8()+" - [SYSTEM]Failed to start server: ");
        ui->textBrowser->append(srv->errorString());
        srv->close();
    }
    connect(srv,SIGNAL(newConnection()),this,SLOT(addNewClient()));
    QString msg =  QString::number(port);
    ui->textBrowser->append(QTime::currentTime().toString().toUtf8()+" - [SYSTEM]Server started. Your Port - " + msg +". Your IP - "+ui->le_Currentip->text());
    //ui->textBrowser->append(QString(QHostAddress::setAddress));
    ui->pbSend->setEnabled(true);

}

void server::createClient(QString address, int port)
{
    cli->connectToHost(address,port);
    ui->textBrowser->append(QTime::currentTime().toString().toUtf8()+" - [SYSTEM]Client started. Server IP - " + ui->le_Currentip->text() + ", port - " + QString::number(port));
    connect(cli, SIGNAL(connected()), SLOT(serverConnected()));
    connect(cli, SIGNAL(readyRead()), SLOT(readFromServer()));
    connect(ui->leMessage, SIGNAL(returnPressed()), SLOT(sendToServer()));
}

void server::addNewClient()
{
    QTcpSocket* clientSocket = srv->nextPendingConnection();

    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(disconnectClient()));
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(readFromClient()));

    clientList.append(clientSocket);
    ui->textBrowser->append(QTime::currentTime().toString().toUtf8()+" - [SERVER]New client connected.");
    sendToClient(clientSocket, QTime::currentTime().toString().toUtf8()+" - "+"[SERVER]You're connected!");
}

void server::disconnectClient()
{
    QTcpSocket* client = (QTcpSocket*)sender();
    for (int i=0; i<clientList.size(); i++){
        if (client == clientList.at(i)){
            clientList.removeAt(i);
        }
    }
    client->deleteLater();
}

void server::readFromClient()
{
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
    QString message;
    if(clientSocket->bytesAvailable()){
        message = clientSocket->readAll();
    }
    if (ui->cbMode->isChecked())
    {
    QString remMessage = message;
    remMessage.remove(0,remMessage.indexOf("]")+1);
    data = remMessage.split(" ");
    if (data.size() == 4) {
        QString nick = message.mid(1,message.indexOf("]")-1);
        if (!nickList.contains(nick)){
            nickList << nick;
            if (nickList.size() != ui->tableWidget->columnCount()){
                ui->tableWidget->setColumnCount(ui->tableWidget->columnCount()+1);
            }
            ui->tableWidget->setHorizontalHeaderLabels(nickList);
        }
        QString numOfRow = data.at(0);
        QString x = data.at(1);
        QString y = data.at(2);
        QString z = data.at(3);
        double i = (sqrt(x.toInt()^2 +y.toInt()^2 +z.toInt()^2));
        //ui->tableWidget->item(numOfRow.toInt(),nickList.indexOf(nick))->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsDragEnabled|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
        QTableWidgetItem *itab = new QTableWidgetItem;
        itab->setText(QString::number(i));
        ui->tableWidget->setItem(numOfRow.toInt(),nickList.indexOf(nick),itab);
       // ui->tableWidget->item(numOfRow.toInt(),nickList.indexOf(nick))->setText(QString::number(i));
        }
    }
    foreach (QTcpSocket* client, clientList) {
            sendToClient(client, QTime::currentTime().toString().toUtf8()+" - "+message);
    }
}

void server::sendToClient(QTcpSocket *client, QString message)
{
    if (ui->cbMode->isChecked()){client->write(message.toUtf8());} else {client->write(message.toUtf8());}
}

void server::readFromServer()
{
    QString message;
    if(cli->bytesAvailable()){
        message = cli->readAll();
        ui->textBrowser->append(message);
        trayIcon->showMessage("Новое сообщение",message,QSystemTrayIcon::Information,1000);
    }
}

void server::serverConnected()
{
    ui->textBrowser->append(QTime::currentTime().toString().toUtf8()+" - [CLIENT]Connected to server.");
    ui->pbSend->setEnabled(true);
}

void server::sendToServer()
{
//    if (ui->cbMode->isChecked()) {
//        QString message = ui->leMessage->text();
//        cli->write(message.toUtf8());
//        ui->leMessage->clear();
//    } else {
    if (!ui->leMessage->text().isEmpty()){
        if (!ui->leNick->text().isEmpty())
    {
    QString message = "["+ui->leNick->text()+"]"+ui->leMessage->text();
   // QString message = ui->leMessage->text();
    cli->write(message.toUtf8());
    ui->leMessage->clear();
    } else {
        ui->textBrowser->append("[ERROR]\"Nickname\" or can't be empty");
    }
    } else {
        ui->textBrowser->append("[ERROR]Message can't be empty");
    }
    //}
}

void server::on_pbServer_clicked()
{
    createClient(ui->le_ip->text(),ui->lePort->text().toInt());
    ui->pbServer_create->setEnabled(false);
    ui->pbServer->setEnabled(false);
}

void server::on_pbServer_create_clicked()
{
    createServer(ui->lePort_create->text().toInt());
    createClient("localhost",ui->lePort_create->text().toInt());
    ui->pbServer_create->setEnabled(false);
    ui->pbServer->setEnabled(false);
}


void server::on_pbSend_clicked()
{
    sendToServer();
}

void server::showTrayIcon()
{
    trayIcon = new QSystemTrayIcon(this);
    QIcon trayImage(QApplication::applicationDirPath()+"/img/ico.png");
    trayIcon->setIcon(trayImage);
    trayIcon->setContextMenu(trayIconMenu);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
}

void server::trayActionExecute()
{
    showNormal();
    setWindowState(Qt::WindowActive);
}

void server::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
            trayActionExecute();
            break;
        default:
            break;
    }
}

void server::setTrayIconActions()
{
    restoreAction = new QAction("Восстановить", this);
    quitAction = new QAction("Выход", this);
    connect (restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    connect (quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(quitAction);
}

void server::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange)
    {
        if (isMinimized())
        {
            hide();
            trayIcon->show();
        }
        if (isActiveWindow()){
            trayIcon->hide();
        }
    }
}

