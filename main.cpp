#include "wingate.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WinGate w;
    w.show();

    return a.exec();
}
