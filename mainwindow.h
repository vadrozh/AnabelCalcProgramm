#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <server.h>
#include <QMainWindow>
#include <QLayout>
#include <QTableWidget>
#include <QDir>
#include <QAction>
#include <QKeyEvent>
#include <QtGlobal>
#include <QShortcut>

#include <QTcpServer>
#include <QTcpSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QTabWidget * AddReferee(int NominationCount);
    QTableWidget * AddTable(int TabNum, QString Name);

    void changeItemsBackground(QTableWidget* Table);

    void AddNominations();

    bool openFile();
    server *srv = new server();

signals:
    void configurationComplited();

private slots:
    void changeResultValue(QTableWidgetItem* item);
    void saveResultsValues(QTableWidgetItem* item);
    void selectWinners(QTableWidgetItem* item);

    void OpenConfigFile();



private:
    QTabWidget * MainTabWidget = new QTabWidget();
    QStringList     Referees;
    QList<QString>  Criteria;
    QList<int>      CriteriaMin;
    QList<int>      CriteriaMax;
    QStringList     Nominations;
    QVector<QStringList> Members;

    QString         CompetitionName;
    QString         ResultsPath;

    QShortcut       *keyOpen;


};

#endif // MAINWINDOW_H
