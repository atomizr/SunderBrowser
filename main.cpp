#include <QApplication>
#include "BrowserWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    BrowserWindow window;
    window.show();
    window.raise();
    window.activateWindow();
    return app.exec();
}
