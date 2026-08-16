#include <QApplication>
#include "BrowserWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Метаданные приложения
    app.setOrganizationName("Atomizr");
    app.setApplicationName("SunderBrowser");
    app.setApplicationVersion("1.0.0");

    // Устанавливаем иконку окна из ресурсов
    app.setWindowIcon(QIcon(":/app.ico"));

    BrowserWindow window;
    window.show();
    window.raise();
    window.activateWindow();

    return app.exec();
}
