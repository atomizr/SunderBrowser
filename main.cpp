#include <QApplication>
#include "BrowserWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    BrowserWindow window;
    window.show();
    return app.exec();
}