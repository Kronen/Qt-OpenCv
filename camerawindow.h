/*
    Author: Alberto G. Lagos (Kronen)
    Copyright (C) 2010  Alberto G. Lagos (Kronen)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
*/


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
        void createCamShiftDialog();
        void flipHorizontally();
        void flipVertically();
        void setFlags();
        void unsetFlags();

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
        QAction *flipHorizontallyAction;
        QAction *flipVerticallyAction;

        // Flags Menu
        QAction *findBiggestObjectAction;
        QAction *doRoughSearchAction;
        QAction *doCannyPruningAction;
        QAction *scaleImageAction;
        QAction *unsetFlagsAction;
};

#endif // CAMERAWINDOW_H
