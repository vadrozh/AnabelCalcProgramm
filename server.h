#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QMainWindow>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTextCodec>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
#include <QStringList>
#include <QValidator>
#include <QNetworkInterface>
#include <qmath.h>
#include <QRegExpValidator>
#include <QTableWidgetItem>
namespace Ui {
class server;
}

class server : public QMainWindow
{
    Q_OBJECT

public:
    explicit server(QWidget *parent = 0);
    ~server();

    QTcpServer *srv = new QTcpServer(this);
    QTcpSocket *cli = new QTcpSocket(this);
    QList<QTcpSocket *> clientList;
    QStringList nickList;
public slots:
    void addNewClient();
    void disconnectClient();
    void readFromClient();
    void sendToClient(QTcpSocket* client, QString message);

    void readFromServer();
    void serverConnected();
    void sendToServer();
    void createClient(QString address, int port);
    void createServer(int port);
private slots:
    void on_pbServer_clicked();

    void on_pbServer_create_clicked();
    void replyFinished();
    void on_pbSend_clicked();

    void changeEvent(QEvent*);
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void trayActionExecute();
    void setTrayIconActions();
    void showTrayIcon();

protected:
    virtual void closeEvent(QCloseEvent *event);
private:
    Ui::server *ui;
    QMenu *trayIconMenu;
    QAction *minimizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    QSystemTrayIcon *trayIcon;
    QStringList data;
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QValidator *valid = new QRegExpValidator(QRegExp("[a-zA-Z0-9А-Яа-я ]+"),this);
};

#endif // SERVER_H
