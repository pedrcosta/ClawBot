#include "mainwindow.h"
#include <QApplication>


/*
Hue values of basic colors
Yellow 22- 38
Green 38-75
Blue 75-130
Red 160-179
*/


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    /*cout << "iLowH-> " << iLowH << " iHighH-> " << iHighH << endl;
    cout << "iLowS-> " << iLowS << " iHighS-> " << iHighS << endl;
    cout << "iLowV-> " << iLowV << " iHighV-> " << iHighV << endl;*/


    return a.exec();
}
