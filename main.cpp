#include "wingate.h"
#include <QApplication>
#include <QPixmap>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WinGate w;

    QPixmap wingate_image("wingate.ico");
    QIcon wingate_icon(wingate_image);

    w.setWindowIcon(wingate_icon);

    w.show();

    return a.exec();
}
