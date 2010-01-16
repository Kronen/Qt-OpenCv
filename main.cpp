#include <QtGui/QApplication>
#include <cv.h>
#include <highgui.h>
#include "opencvwidget.h"
#include "camerawindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    CameraWindow *mainWin = new CameraWindow();
    mainWin->setWindowTitle("Playing with OpenCV");
    mainWin->show();

    return app.exec();
}
