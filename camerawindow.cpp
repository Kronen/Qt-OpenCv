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

#include "camerawindow.h"

#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>

CameraWindow::CameraWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowIcon(QIcon(":/images/OpenCV.ico"));
    setMinimumSize(320, 240);

    cvWidget = new OpenCVWidget(this);
    createCamShiftDialog();

    createActions();
    createMenu();
    createToolBar();
    createStatusBar();

    // We check if OpenCV was able to detect a compatible device
    if(!cvWidget->isCaptureActive()) {
        QMessageBox::warning(this, tr("Qt + OpenCV"),
                             tr("Can't detect a camera connected to the PC.\n"
                                "This program doesn't provide any option\n"
                                "to configure the device."), QMessageBox::Close);

        videoAction->setEnabled(false);
        screenshotAction->setEnabled(false);
        settingsMenu->setEnabled(false);
        flagsMenu->setEnabled(false);
    } else {
        readSettings();
        statusLabel->setText(QString("OpenCV Face Detection. (w:%1 h:%2)").arg(cvWidget->width()).arg(cvWidget->height()));
    }

    setCentralWidget(cvWidget);
}

void CameraWindow::closeEvent(QCloseEvent *event) {
    writeSettings();
    delete cvWidget;
    if(mCamShiftDialog) delete mCamShiftDialog;
    event->accept();
}

void CameraWindow::saveScreenshot() {
    cvWidget->saveScreenshot();
}

// Start/Stop writing the webcam frames to a video file
void CameraWindow::writeVideo() {
    if(videoAction->isChecked()) {
        videoAction->setIcon(QIcon(":/images/icon_stopvideo.png"));
        cvWidget->videoWrite();
    } else {
        videoAction->setIcon(QIcon(":/images/icon_video.png"));
        cvWidget->videoStop();
    }
    statusLabel->setText("Writing Video");
}

// Start/Stop detect face mode
void CameraWindow::detectFaces() {
    // We don't track and detect at the same time
    if(detectFacesAction->isChecked()) {
        // Call setCascadeFile if there isn't a default cascade file
        if(!cvWidget->isFaceDetectAvalaible()) setCascadeFile();

        // Don't track and detect at the same time
        cvWidget->setTrackFace(false);
        cvWidget->setDetectFaces(true);
        trackFaceAction->setChecked(false);

        statusLabel->setText("Detecting Faces");
    } else cvWidget->setDetectFaces(false);
}

// Start/Stop track face mode
void CameraWindow::trackFace() {    
    if(trackFaceAction->isChecked()) {
        // Call setCascadeFile if there isn't a default cascade file
        if(!cvWidget->isFaceDetectAvalaible()) setCascadeFile();

        // Activate/deativate the corresponding menus and actions
        detectFacesAction->setChecked(false);
        flagsMenu->setEnabled(false);

        // Don't track and detect at the same time
        cvWidget->setDetectFaces(false);
        cvWidget->setTrackFace(true);

        statusLabel->setText("Tracking Face");
    } else {
        cvWidget->setTrackFace(false);
        flagsMenu->setEnabled(true);
    }
}

// Dialog to calibrate CamShift through vMin and sMin.
// These variables define thresholds for ignoring pixels that are too close to neutral.
// vMin sets the threshold for "almost black," and sMin for "almost gray."
void CameraWindow::createCamShiftDialog() {
    mCamShiftDialog = new CamShiftDialog(this);

    connect(mCamShiftDialog->vMinSlider, SIGNAL(valueChanged(int)), cvWidget, SLOT(setCamShiftVMin(int)));
    connect(mCamShiftDialog->sMinSlider, SIGNAL(valueChanged(int)), cvWidget, SLOT(setCamShiftSMin(int)));
}

void CameraWindow::readSettings() {
    QSettings settings("Kronen Software", "Qt + OpenCV");
    if(settings.value("FlipH").toBool()) {
        cvWidget->switchFlipH();
        flipHorizontallyAction->setChecked(true);
    }
    if(settings.value("FlipV").toBool()) {
        cvWidget->switchFlipV();
        flipVerticallyAction->setChecked(true);
    }
    QString cascadeFile = settings.value("CascadeFile").toString();
    if(QFileInfo(cascadeFile).exists()) cvWidget->setCascadeFile(cascadeFile);

    settings.beginGroup("CamShift");
    mCamShiftDialog->vMinSlider->setValue(settings.value("Vmin").toInt());
    mCamShiftDialog->sMinSlider->setValue(settings.value("Smin").toInt());
    settings.endGroup();
}

void CameraWindow::writeSettings() {
    QSettings settings("Kronen Software", "Qt + OpenCV");
    settings.setValue("FlipH", cvWidget->flipH());
    settings.setValue("FlipV", cvWidget->flipV());
    if(!(cvWidget->cascadeFile().isEmpty())) settings.setValue("CascadeFile", cvWidget->cascadeFile());

    settings.beginGroup("CamShift");
    settings.setValue("Vmin", cvWidget->camshiftVMin());
    settings.setValue("Smin", cvWidget->camshiftSMin());
    settings.endGroup();
}

// First check if there is a file
// Show a dialog to choose the haarcascade file to use on face detecting.
// Face tracking also needs a haarcascade file because we first detect a face to start tracking.
void CameraWindow::setCascadeFile() {
    QString cascadeFile = QFileDialog::getOpenFileName(this, tr("Choose Cascade File"), "./haarcascades",
                                                       tr("Cascade Files (*.xml)"));
    if(!cascadeFile.isNull()) {
        cvWidget->setCascadeFile(cascadeFile);
        statusLabel->setText(cascadeFile + "loaded");
    }
}

void CameraWindow::flipHorizontally() {
    cvWidget->switchFlipH();
}

void CameraWindow::flipVertically() {
    cvWidget->switchFlipV();
}

// Sends the checked flags to the widget to actualize the face detect mode
void CameraWindow::setFlags() {
    int flags = 0;

    findBiggestObjectAction->setEnabled(!(scaleImageAction->isChecked() || doCannyPruningAction->isChecked()));
    doRoughSearchAction->setEnabled(findBiggestObjectAction->isChecked());
    doCannyPruningAction->setEnabled(!(findBiggestObjectAction->isChecked() || scaleImageAction->isChecked()));
    scaleImageAction->setEnabled(!(findBiggestObjectAction->isChecked() || doCannyPruningAction->isChecked()));


    if(scaleImageAction->isChecked()) {
        flags |= CV_HAAR_SCALE_IMAGE;
    } else {
        if(findBiggestObjectAction->isChecked()) {
            flags |= CV_HAAR_FIND_BIGGEST_OBJECT;
            if(doRoughSearchAction->isChecked()) {
                flags |= CV_HAAR_DO_ROUGH_SEARCH;
            }
        } else {
            if(doCannyPruningAction->isChecked()) {
                flags |= CV_HAAR_DO_CANNY_PRUNING;
            }
        }
    }

    cvWidget->setFaceDetectFlags(flags);
}

void CameraWindow::unsetFlags() {
    findBiggestObjectAction->setChecked(false);
    doRoughSearchAction->setChecked(false);
    doCannyPruningAction->setChecked(false);
    scaleImageAction->setChecked(false);
    setFlags();
}

void CameraWindow::createMenu() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(quitAction);

    settingsMenu = menuBar()->addMenu(tr("&Settings"));

    faceDetectMenu = settingsMenu->addMenu(tr("&DetectFace"));
    faceDetectMenu->addAction(cascadeFileAction);
    flagsMenu = faceDetectMenu->addMenu(tr("&Flags"));
    flagsMenu->addAction(findBiggestObjectAction);
    flagsMenu->addAction(doRoughSearchAction);
    flagsMenu->addSeparator();
    flagsMenu->addAction(doCannyPruningAction);
    flagsMenu->addSeparator();
    flagsMenu->addAction(scaleImageAction);
    flagsMenu->addSeparator();
    flagsMenu->addAction(unsetFlagsAction);

    settingsMenu->addSeparator();
    settingsMenu->addAction(camshiftDialogAction);
    settingsMenu->addSeparator();
    settingsMenu->addAction(flipHorizontallyAction);
    settingsMenu->addAction(flipVerticallyAction);
}

void CameraWindow::createToolBar() {
    toolBar = addToolBar(tr("&File"));
    toolBar->setIconSize(QSize(32,32));
    toolBar->setMovable(false);
    toolBar->addAction(quitAction);
    toolBar->addSeparator();
    toolBar->addAction(screenshotAction);
    toolBar->addAction(videoAction);
    toolBar->addSeparator();
    toolBar->addAction(detectFacesAction);
    toolBar->addAction(trackFaceAction);
}

void CameraWindow::createStatusBar() {
    statusLabel = new QLabel(this);
    statusLabel->setIndent(3);
    statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    statusBar()->addPermanentWidget(statusLabel);

    connect(cvWidget, SIGNAL(info(const QString &)), statusLabel, SLOT(setText(const QString &)));

    statusLabel->setText("OpenCV Face Detection");
}

void CameraWindow::createActions() {
    quitAction = new QAction(tr("&Quit"), this);
    quitAction->setIcon(QIcon(":/images/icon_quit.png"));
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setStatusTip(tr("Exit the application"));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    screenshotAction = new QAction(tr("Take Screenshot"), this);
    screenshotAction->setIcon(QIcon(":/images/icon_screenshot.png"));
    screenshotAction->setShortcut(tr("Ctrl+S"));
    screenshotAction->setStatusTip(tr("Take a screenshot from the camera"));
    connect(screenshotAction, SIGNAL(triggered()), this, SLOT(saveScreenshot()));

    videoAction = new QAction(tr("Grab a Video"), this);
    videoAction->setIcon(QIcon(":/images/icon_video.png"));
    videoAction->setShortcut(tr("Ctrl+G"));
    videoAction->setStatusTip(tr("Record a video from the camera"));
    videoAction->setCheckable(true);
    connect(videoAction, SIGNAL(triggered()), this, SLOT(writeVideo()));

    detectFacesAction = new QAction(tr("Detect Faces"), this);
    detectFacesAction->setIcon(QIcon(":/images/icon_detectfaces.png"));
    detectFacesAction->setShortcut(tr("Ctrl+D"));
    detectFacesAction->setStatusTip(tr("Detect the faces for each frame of the camera"));
    detectFacesAction->setCheckable(true);
    connect(detectFacesAction, SIGNAL(triggered()), this, SLOT(detectFaces()));

    trackFaceAction = new QAction(tr("Track a Face(CamShift)"), this);
    trackFaceAction->setIcon(QIcon(":/images/icon_trackface.png"));
    trackFaceAction->setShortcut(tr("Ctrl+T"));
    trackFaceAction->setStatusTip(tr("Track a face by color between frames"));
    trackFaceAction->setCheckable(true);
    connect(trackFaceAction, SIGNAL(triggered()), this, SLOT(trackFace()));

    cascadeFileAction = new QAction(tr("Set a &Cascade File"), this);
    cascadeFileAction->setStatusTip(tr("Set a cascade file for detecting faces"));    
    connect(cascadeFileAction, SIGNAL(triggered()), this, SLOT(setCascadeFile()));

    camshiftDialogAction = new QAction(tr("CamShift Calibration"), this);
    camshiftDialogAction->setStatusTip(tr("Change the vMin and sMin variables for CamShift"));
    connect(camshiftDialogAction, SIGNAL(triggered()), mCamShiftDialog, SLOT(show()));

    flipHorizontallyAction = new QAction(tr("Flip &Horizontally"), this);
    flipHorizontallyAction->setStatusTip(tr("Flip the image horizontally"));
    flipHorizontallyAction->setCheckable(true);
    connect(flipHorizontallyAction, SIGNAL(triggered()), this, SLOT(flipHorizontally()));

    flipVerticallyAction = new QAction(tr("Flip &Vertically"), this);
    flipVerticallyAction->setStatusTip(tr("Flip the image vertically"));
    flipVerticallyAction->setCheckable(true);
    connect(flipVerticallyAction, SIGNAL(triggered()), this, SLOT(flipVertically()));

    // SubMenu Flags
    findBiggestObjectAction = new QAction(tr("Only find the &Biggest Object"), this);
    findBiggestObjectAction->setCheckable(true);
    connect(findBiggestObjectAction, SIGNAL(triggered()), this, SLOT(setFlags()));

    doRoughSearchAction = new QAction(tr("      Activate &Rough Search"), this);
    doRoughSearchAction->setCheckable(true);
    doRoughSearchAction->setEnabled(false);
    connect(doRoughSearchAction, SIGNAL(triggered()), this, SLOT(setFlags()));

    doCannyPruningAction = new QAction(tr("Activate Canny &Pruning"), this);
    doCannyPruningAction->setCheckable(true);
    connect(doCannyPruningAction, SIGNAL(triggered()), this, SLOT(setFlags()));

    scaleImageAction = new QAction(tr("Scales the &Image"), this);
    scaleImageAction->setCheckable(true);
    connect(scaleImageAction, SIGNAL(triggered()), this, SLOT(setFlags()));

    unsetFlagsAction = new QAction(tr("&Unset All Flags"), this);
    connect(unsetFlagsAction, SIGNAL(triggered()), this, SLOT(unsetFlags()));
}
