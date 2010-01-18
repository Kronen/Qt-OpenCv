#include <QtGui/QApplication>

#include "camerawindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    CameraWindow *mainWin = new CameraWindow();
    mainWin->setWindowTitle("Playing with OpenCV");
    mainWin->show();

    return app.exec();
}
