#include "SlagPondWidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SlagPondWidget w;
    w.show();
    return a.exec();
}
