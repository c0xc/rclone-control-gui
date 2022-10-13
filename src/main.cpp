#define DEFINE_GLOBALS
#include "main.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("c0xc");
    app.setApplicationName(PROGRAM);
    QString program = QString(PROGRAM).toLower();
    SettingsManager::setInitVariantPrefix(true);
    SettingsManager::setDefaultGroup("main");

    MainWindow *gui = 0;
    gui = new MainWindow;
    gui->show();

    int code = app.exec();
    delete gui;
    return code;
}

