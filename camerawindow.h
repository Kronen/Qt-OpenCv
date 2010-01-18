#ifndef CAMERAWINDOW_H
#define CAMERAWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
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
        void createActions();
        void createMenu();
        void createToolBar();
        void createStatusBar();

    private slots:
        void takeScreenshot();
        void writeVideo();
        bool isWritingVideo();
        void detectFaces();
        void setCascadeFile();
        void cerrar();

    private:
        OpenCVWidget *cvWidget;
        bool writingVideo;

        QMenu *optionsMenu;
        QToolBar *toolBar;
        QLabel *statusLabel;        

        QAction *quitAction;
        QAction *screenshotAction;
        QAction *videoAction;
        QAction *detectFacesAction;
        QAction *cascadeFileAction;
        QAction *findBiggestObjectAction;
        QAction *doRoughSearchAction;
        QAction *doCannyPruningAction;
        QAction *scaleImageAction;
};

#endif // CAMERAWINDOW_H
