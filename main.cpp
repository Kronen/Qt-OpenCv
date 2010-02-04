#include <QtGui/QApplication>

#include "camerawindow.h"
#include "version.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    CameraWindow *mainWin = new CameraWindow();
    mainWin->setWindowTitle(appName + appVersion);
    mainWin->show();

    return app.exec();
}
