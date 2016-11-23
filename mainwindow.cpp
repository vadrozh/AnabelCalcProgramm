// Добавить - если файл защищен от записи, сделать все ячейки нередактируемыми)))
// Добавить - горячие клавиши на открытие конфига и зачистку таблицы

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QList>
#include <QFileDialog>
#include <QtAlgorithms>
#include <QDate>

#include <signal.h>

// Необходимо для смены цвета ячейки таблицы.
// Пока-что не работает, понты фреймворка
#define LRed     QColor(255,120,100,150)
#define LGreen   QColor(120,255,100)
#define LYellow  QColor(255,255,100)
// Необходимо для смены цвета ячейки таблицы.
// Пока-что не работает, понты фреймворка
#define DRed     QColor(125,10,0)
#define DGreen   QColor(10,125,0)
#define DYellow  QColor(125,125,0)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QHBoxLayout *Layout = new QHBoxLayout();
    Layout->addWidget(MainTabWidget);
    MainTabWidget->setGeometry(0,0,1366,768);


    //MainTabWidget->show();

    ui->setupUi(this);
    ui->mainToolBar->hide();
    ui->menuBar->hide();
    ui->statusBar->hide();
    ui->centralWidget->hide();
    ui->wgPreference->show();

    keyOpen = new QShortcut(MainTabWidget);
    keyOpen->setKey(Qt::CTRL + Qt::Key_O);

    connect(keyOpen, SIGNAL(activated()), this, SLOT(OpenConfigFile()));



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::AddNominations()
{
    int NominationCount = 0;
    foreach (QString Nomination, Nominations) {
        NominationCount++;
        MainTabWidget->setObjectName(Nomination.replace(" ","_"));
        MainTabWidget->addTab(AddReferee(NominationCount),Nomination.replace("_"," "));


    }
}

QTabWidget * MainWindow::AddReferee(int NominationCount)
{
//    QStringList Referees = GenerateStringList("referee");

    QTableWidget *TableReferee = new QTableWidget(this);
    QTabWidget *TabReferee = new QTabWidget();
    foreach (QString Referee, Referees) {
        QString Nomination = MainTabWidget->objectName();
        TableReferee = AddTable(NominationCount,Nomination+"-"+Referee);
        TabReferee->addTab(TableReferee,Referee);
    }
    return TabReferee;
}

QTableWidget * MainWindow::AddTable(int TabNum, QString Name)
{

    QStringList MembersList = Members.at(TabNum-1);

    QFont BOLD;
    BOLD.setBold(true);

    QTableWidget *TableWidget = new QTableWidget;
    TableWidget->setObjectName(Name);

    TableWidget->setRowCount(MembersList.size());
    TableWidget->setColumnCount(Criteria.size());

    QStringList FileName = Name.split("-");
    QFile file(ResultsPath + FileName.at(0));
    QStringList stringList;
    QTextStream textStream(&file);
    textStream.reset();
    stringList.clear();

    QStringList List;

    if (file.exists()){
        file.open(QIODevice::ReadOnly);

        QString Results;
        Results.clear();

        Results = textStream.readAll();
        Results.replace("|","").simplified();
        List = Results.split("\n");
    }

    file.close();

    int MembersSize = MembersList.count();
    //int CriteriaSize = Criteria.count() + 2;    // + Столбцы под результаты (текущие и общие)

    int ResultsStringIndex = 0;
    for (int i=0; i<List.count(); i++){
        if (List.at(i) == FileName.at(1)){
            ResultsStringIndex = i+1;
        } else {
            i += MembersSize + 1;
        }
    }

    for (int i=0; i<MembersList.count(); i++){
        QStringList ScoresList ;
        if (file.exists()){
            QString ScoresString = List.at(ResultsStringIndex+i);
            ScoresList = ScoresString.simplified().split(" ");
        }
        for (int j=0; j<Criteria.count(); j++){
            QTableWidgetItem *Item = new QTableWidgetItem("");
            if (file.exists() && ResultsStringIndex != 0){
                QString Score = ScoresList.at(j);
                Item->setText(Score);
            } else {
                Item->setText("0.0");
            }
            TableWidget->setItem(i,j,Item);
            if (j == Criteria.count()-1/* || j == Criteria.count()-2*/){
                Item->setFlags(Qt::ItemIsEditable);
                Item->setFlags(Qt::ItemIsEnabled);
                if (j == Criteria.count()-1 ){
                    Item->setFont(BOLD);
                }
            }
        }
    }   //for

    TableWidget->setVerticalHeaderLabels(MembersList);
    TableWidget->setHorizontalHeaderLabels(Criteria);
    TableWidget->horizontalHeaderItem(Criteria.size()-1)->setFont(BOLD);

    changeItemsBackground(TableWidget);

    connect(TableWidget,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(changeResultValue(QTableWidgetItem*)));
    connect(TableWidget,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(saveResultsValues(QTableWidgetItem*)));
    connect(TableWidget,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(selectWinners(QTableWidgetItem*)));

    return TableWidget;
}

void MainWindow::changeItemsBackground(QTableWidget *Table)
{
    int col = Table->columnCount()-1;
    int row = Table->rowCount();

    QList<double> Results;
    Results.clear();
    for (int i=0; i<row; i++){
        QString RES = Table->item(i,col)->text();
        double RESD;
        if (RES == "0" || RES == "0.0")
        {
            RESD = 0;
        } else {
            RESD = RES.toDouble();
        }
        Results.append(RESD);
    }

    QList<int> Winners;
    Winners.clear();

    int placesMax = 3;  //1
//    if (Results.size() > 1) placesMax = 2;
//    if (Results.size() > 2) placesMax = 3;

    for (int place=0; place<placesMax; place++){
        double maxResult = 0;
        double maxResultIndex = 0;
        for (int i=0; i<Results.size(); i++){
            if (Results[i]>maxResult){
                maxResult = Results[i];
                maxResultIndex = i;
            }
        }
        Winners << maxResultIndex;
        Results[maxResultIndex] = -1;
    }

    QList<float> Place;
    Place << Table->item(Winners[2],col)->text().toFloat();
    Place << Table->item(Winners[1],col)->text().toFloat();
    Place << Table->item(Winners[0],col)->text().toFloat();

    QFont Bold,Regular;
    Bold.setBold(true);
    Regular.setBold(false);

    for(int i=0; i<row; i++){
        QTableWidgetItem *CurrentItem = Table->item(i,col);
        if(CurrentItem){
            float CurrentItemValue = CurrentItem->text().toFloat();
            if (CurrentItemValue == Place.at(2)){
                Table->verticalHeaderItem(i)->setForeground(QBrush(DGreen));
                Table->verticalHeaderItem(i)->setFont(Bold);
            } else
            if (CurrentItemValue == Place.at(1)){
                Table->verticalHeaderItem(i)->setForeground(QBrush(DYellow));
                Table->verticalHeaderItem(i)->setFont(Bold);
            } else
            if (CurrentItemValue == Place.at(0)){
                Table->verticalHeaderItem(i)->setForeground(QBrush(DRed));
                Table->verticalHeaderItem(i)->setFont(Bold);
            } else {
                Table->verticalHeaderItem(i)->setForeground(Qt::black);
                Table->verticalHeaderItem(i)->setFont(Regular);
            }
            if(CurrentItemValue == 0){
                Table->verticalHeaderItem(i)->setForeground(Qt::black);
                Table->verticalHeaderItem(i)->setFont(Regular);
            }
            CurrentItem->setForeground(Table->verticalHeaderItem(i)->foreground());
        }
//        if (!(Table->verticalHeaderItem(i)->foreground() == Qt::black))
//        {
//            CurrentItem->setBackground(Table->verticalHeaderItem(i)->foreground());
//        } else {
//            CurrentItem->setBackground(Table->verticalHeaderItem(i)->background());
//        }
    }
}



bool MainWindow::openFile()
{
    QString FileName = QFileDialog::getOpenFileName(this,
                                QString::fromUtf8("Открыть файл соревнования"),
                                QDir::currentPath(),
                                "Text files (*.txt)");
    QFile File(FileName);
    if (File.exists() && File.open(QIODevice::ReadOnly)){
        QString tempString;

        tempString.clear();

        while(!File.atEnd()){
            // Заипсываем построчно в tempString данные из файла.
            tempString = QString(File.readLine().simplified());
            if (!tempString.isEmpty() && !tempString.startsWith("#"))
            {
                // Название соревнования
                if(tempString == "[competition]"){
                    // Читаем следующую за идентификатором строку.
                    tempString = QString(File.readLine().simplified());
                    if (!tempString.isEmpty() && !tempString.startsWith("#")){
                        CompetitionName = tempString;
                    }
                    if (CompetitionName.isEmpty()){
                        CompetitionName = QString(QDate::currentDate().toString("Competition_dd_MM_yyyy"));
                    }
                    ResultsPath = QDir::currentPath()+"/"+CompetitionName+"/";
                    if (!QDir().mkdir(ResultsPath) && !QDir().exists(ResultsPath)){
                        ResultsPath = QDir::currentPath()+"/";
                    }
                    tempString = QString(File.readLine().simplified());
                }
                // Список критериев
                if(tempString == "[criteria]"){
                    QStringList _criteriaList;
                    // Читаем следующую за идентификатором строку
                    tempString = QString(File.readLine().simplified());
                    while(tempString != "[/criteria]"){
                        if (!tempString.isEmpty() && !tempString.startsWith("#")){
                            _criteriaList = tempString.split(" ");
                            Criteria.   append(_criteriaList.at(0));
                            CriteriaMin.append(QString(_criteriaList.at(1)).toInt());
                            CriteriaMax.append(QString(_criteriaList.at(2)).toInt());
                        }
                        tempString = QString(File.readLine().simplified());
                    }
                    Criteria.   append("Всего");
                    CriteriaMin.append(-100);
                    CriteriaMax.append(100);
                }
                // #Список критериев
                // Список Судей
                if(tempString == "[referee]"){
                    // Читаем следующую за идентификатором строку
                    tempString = QString(File.readLine().simplified());
                    while(tempString != "[/referee]"){
                        if (!tempString.isEmpty() && !tempString.startsWith("#")){
                            Referees.   append(tempString);
                        }
                        tempString = QString(File.readLine().simplified());
                    }
                }
                // #Список Судей
                // Список участников
                if(tempString == "[members]"){
                    int startIndex = true;
                    // Читаем следующую за идентификатором строку
                    tempString = QString(File.readLine().simplified());
                    // Список участников для данной номинации
                    while(tempString != "[/members]"){
                        if (tempString.isEmpty() || tempString == "[category]"){
                            tempString = QString(File.readLine().simplified());
                            startIndex = true;
                        }
                        if (startIndex){
                            Nominations.append(tempString);
                            startIndex = false;
                            Members.append(QStringList());
                        } else {
                            Members[Nominations.size()-1].append(tempString);
                        }
                        tempString = QString(File.readLine().simplified());
                    }
                }
                // #Список участников
            }
        }
    } else {
        return false;
    }
    return true;
}

void MainWindow::changeResultValue(QTableWidgetItem *_item)
{
    int Row = _item->row();
    int Col = _item->tableWidget()->columnCount()-1;
    QTableWidget * Table = _item->tableWidget();

    QList<QTableWidget *> TableList= Table->parent()->findChildren<QTableWidget *>();

    float Result = 0;
    foreach (QTableWidget *Table, TableList) {
        for (int i=0; i<Col; i++){
            QTableWidgetItem *Item = Table->item(Row,i);
            QString ItemText = Item->text();
            double ItemValue = ItemText.toDouble();
            double ItemValueRest = ItemValue - (int)ItemValue;
            // Возможно неправильно обрабатываются отрицательные значения!!!
//            if (ItemValueRest != 0 && (ItemValueRest != 0.5 && ItemValueRest != -0.5)){
//                if (ItemValueRest > 0.5){
//                    ItemValue = (int)ItemValue + 1.0;
//                } else {
//                    ItemValue = (int)ItemValue + 0.5;
//                }
//            }
            if (ItemValue > CriteriaMax.at(i)){
                ItemValue = CriteriaMax.at(i);
            }
            if (ItemValue < CriteriaMin.at(i)){
                ItemValue = CriteriaMin.at(i);
            }
            if (ItemValueRest == 0){
                Item->setText(QString::number(ItemValue)+".0");
            } else {
                Item->setText(QString::number(ItemValue));
            }

            Result = Result + Table->item(Row,i)->text().toDouble();
        }
    }
    foreach (QTableWidget *Table, TableList) {
        Table->item(Row,Col)->setText(QString::number(Result));
    }

}

// Сохранение всех изменений таблицы в файл
void MainWindow::saveResultsValues(QTableWidgetItem *_item)
{
    QTableWidget * Table = _item->tableWidget();
    QStringList FileName = Table->objectName().split("-");

    QFile file(ResultsPath + FileName.at(0));
    QStringList stringList;
    QTextStream textStream(&file);
    textStream.reset();
    stringList.clear();

    // Если файл существует и в него возможно записывать данные, происходит добавление информаци.
    if (file.open(QIODevice::WriteOnly) && file.isWritable()){

        QString Results;
        Results.clear();

        QList<QTableWidget *> TableList= Table->parent()->findChildren<QTableWidget *>();
        foreach (QTableWidget *Table, TableList) {
            FileName = Table->objectName().split("-");
            Results += FileName.at(1) + "\n";
            for(int row=0; row<Table->rowCount(); row++){
                for (int col=0; col<Table->columnCount(); col++){
                    Results += Table->item(row,col)->text()+" ";
                    if (col > Table->columnCount()-3) Results += "| ";
                }
                Results += Table->verticalHeaderItem(row)->text()+"\n";
            }
            Results += "\n";
        }
        textStream << Results;
    }
}

void MainWindow::selectWinners(QTableWidgetItem *_item)
{
    changeItemsBackground(_item->tableWidget());
}

void MainWindow::OpenConfigFile()
{
    MainTabWidget->clear();

    Referees.clear();
    Criteria.clear();
    CriteriaMin.clear();
    CriteriaMax.clear();
    Nominations.clear();
    Members.clear();

    CompetitionName.clear();
    ResultsPath.clear();

    if (openFile()){
        AddNominations();
    }

}
