#include <QtGui/QApplication>

#include "camerawindow.h"
#include "version.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    CameraWindow *mainWin = new CameraWindow();
    mainWin->setWindowTitle(QString("Qt + OpenCV v%1").arg(version));
    mainWin->show();

    return app.exec();
}
