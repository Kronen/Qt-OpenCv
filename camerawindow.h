#ifndef CAMERAWINDOW_H
#define CAMERAWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QToolBar>
#include <QtGui/QStatusBar>
#include <QtGui/QAction>
#include <QtGui/QLabel>

#include <cv.h>
#include <highgui.h>
#include "opencvwidget.h"

class CameraWindow : public QMainWindow {
    Q_OBJECT

    public:
        CameraWindow(QWidget *parent = 0);

    private:
        void createStatusBar();
        void createToolBar();
        void createActions();

    private slots:
        void takeScreenshot();
        void writeVideo();
        bool isWritingVideo();
        void detectFaces();
        void cerrar();

    private:
        OpenCVWidget *cvWidget;
        bool writingVideo;
        bool detectingFaces;

        QLabel *statusLabel;
        QToolBar *toolBar;

        QAction *quitAction;
        QAction *screenshotAction;
        QAction *videoAction;
        QAction *detectfacesAction;
};

#endif // CAMERAWINDOW_H
