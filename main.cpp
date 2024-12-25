#include "widget.h"
#include <QtGlobal>
#include <QTime>
#include <QCoreApplication>
#include <QApplication>
#include <cstdlib>
#include <ctime>
#include <iostream>

int main(int argc, char *argv[])
{
    srand(static_cast<unsigned int>(time(0)));

    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
