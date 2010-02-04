#ifndef CAMERAWINDOW_H
#define CAMERAWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QToolBar>
#include <QtGui/QStatusBar>
#include <QtGui/QAction>
#include <QtGui/QLabel>
#include <QtGui/QCloseEvent>

#include "opencvwidget.h"
#include "camshiftdialog.h"

class CameraWindow : public QMainWindow {
    Q_OBJECT

    public:
        CameraWindow(QWidget *parent = 0);

    private:
        void createActions();
        void createMenu();
        void createToolBar();
        void createStatusBar();
        void readSettings();

    private slots:
        void writeSettings();
        void saveScreenshot();
        void writeVideo();
        void detectFaces();
        void trackFace();
        void setCascadeFile();
        void setFlags();
        void unsetFlags();
        void dialogCamShift();

    protected:
        virtual void closeEvent(QCloseEvent *event);

    private:
        OpenCVWidget *cvWidget;
        CamShiftDialog *mCamShiftDialog;

        QMenu *fileMenu;
        QMenu *settingsMenu;
        QMenu *faceDetectMenu;
        QMenu *flagsMenu;
        QToolBar *toolBar;
        QLabel *statusLabel;

        QAction *quitAction;
        QAction *screenshotAction;
        QAction *videoAction;
        QAction *detectFacesAction;
        QAction *trackFaceAction;

        // Settings Menu
        QAction *cascadeFileAction;
        QAction *camshiftDialogAction;

        // Flags Menu
        QAction *findBiggestObjectAction;
        QAction *doRoughSearchAction;
        QAction *doCannyPruningAction;
        QAction *scaleImageAction;
        QAction *unsetFlagsAction;
};

#endif // CAMERAWINDOW_H
