#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Sunglasses");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("FCAI Team");

    app.setStyle("Fusion");

    MainWindow window;
    window.show();

    return app.exec();
}
