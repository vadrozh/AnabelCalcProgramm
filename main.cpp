#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.hide();
   // server s;
   // QIcon ico(QApplication::applicationDirPath()+"/ico.ico");
    //s.setWindowIcon(ico);
    return a.exec();
}
