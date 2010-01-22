#include "camerawindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QtDebug>

CameraWindow::CameraWindow(QWidget *parent) : QMainWindow(parent) {
    mCamShiftDialog = 0;
    cvWidget = new OpenCVWidget(this);

    setWindowIcon(QIcon(":/images/OpenCV.ico"));

    createActions();
    createMenu();
    createToolBar();
    createStatusBar();

    if(!cvWidget->isCaptureActive()) {
        QMessageBox::warning(this, tr("OpenCV + Qt"),
                                        tr("Can't detect a camera connected to the PC.\n"
                                           "This program doesn't provide any option\n"
                                           "to configure the device."),
                                        QMessageBox::Close);

        videoAction->setEnabled(false);
        screenshotAction->setEnabled(false);
        optionsMenu->setEnabled(false);
        flagsMenu->setEnabled(false);
    } else if(!cvWidget->cascadeFile().isEmpty()) {
        detectFacesAction->setEnabled(true);
        trackFaceAction->setEnabled(true);
    }

    setCentralWidget(cvWidget);
}

void CameraWindow::closeEvent(QCloseEvent *event) {
    delete cvWidget;
    if(mCamShiftDialog) delete mCamShiftDialog;
    event->accept();
}

void CameraWindow::takeScreenshot() {
    QImage image;
    quint8 i = 1;

    QString filename = QString("webcamPic%1.jpg").arg(i);
    while(QFileInfo(filename).exists())
        filename = QString("webcamPic%1.jpg").arg(i++);

    image = cvWidget->image();
    if(!image.isNull()) image.save(filename, "JPG", 80);
}

void CameraWindow::writeVideo() {
    if(videoAction->isChecked()) {
        videoAction->setIcon(QIcon(":/images/icon_stopvideo.png"));

        quint8 i = 1;
        QString filename = QString("webcamVid%1.avi").arg(i);
        while(QFileInfo(filename).exists())
            filename = QString("webcamVid%1.avi").arg(i++);

        cvWidget->videoWrite(filename);
    } else {
        videoAction->setIcon(QIcon(":/images/icon_video.png"));
        cvWidget->videoStop();
    }
}

void CameraWindow::detectFaces() {
    // We don't track and detect at the same time
    if(detectFacesAction->isChecked()) {
        camshiftDialogAction->setEnabled(false);
        trackFaceAction->setChecked(false);
        cvWidget->setTrackFace(false);
        cvWidget->setDetectFaces(true);
    } else cvWidget->setDetectFaces(false);
}

void CameraWindow::trackFace() {
    // We don't track and detect at the same time
    if(trackFaceAction->isChecked()) {        
        detectFacesAction->setChecked(false);
        cvWidget->setDetectFaces(false);
        cvWidget->setTrackFace(true);
        camshiftDialogAction->setEnabled(true);
        flagsMenu->setEnabled(false);
    } else {
        cvWidget->setTrackFace(false);
        camshiftDialogAction->setEnabled(false);
        flagsMenu->setEnabled(true);
    }
}

void CameraWindow::showCamShiftDialog() {
    if(!mCamShiftDialog) {
        mCamShiftDialog = new CamShiftDialog(this);

        mCamShiftDialog->vMinSlider->setValue(cvWidget->getCamShiftVMin());
        mCamShiftDialog->sMinSlider->setValue(cvWidget->getCamShiftSMin());

        connect(mCamShiftDialog->vMinSlider, SIGNAL(valueChanged(int)), cvWidget, SLOT(setCamShiftVMin(int)));
        connect(mCamShiftDialog->sMinSlider, SIGNAL(valueChanged(int)), cvWidget, SLOT(setCamShiftSMin(int)));
    }

    mCamShiftDialog->show();
}

void CameraWindow::setCascadeFile() {
    QString cascadeFile = QFileDialog::getOpenFileName(this,
                                                    tr("Choose Cascade File"),
                                                    "./haarcascades",
                                                    tr("Cascade Files (*.xml)"));

    if(!cascadeFile.isNull()) cvWidget->setCascadeFile(cascadeFile);
    detectFacesAction->setEnabled(true);
}

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

    cvWidget->setFlags(flags);
}

void CameraWindow::unsetFlags() {
    findBiggestObjectAction->setChecked(false);
    doRoughSearchAction->setChecked(false);
    doCannyPruningAction->setChecked(false);
    scaleImageAction->setChecked(false);
    setFlags();
}

void CameraWindow::createMenu() {
    optionsMenu = menuBar()->addMenu(tr("&Settings"));
    optionsMenu->addAction(cascadeFileAction);
    optionsMenu->addSeparator();
    optionsMenu->addAction(camshiftDialogAction);

    flagsMenu = menuBar()->addMenu(tr("&Flags"));
    flagsMenu->addAction(findBiggestObjectAction);
    flagsMenu->addAction(doRoughSearchAction);
    flagsMenu->addSeparator();
    flagsMenu->addAction(doCannyPruningAction);
    flagsMenu->addSeparator();
    flagsMenu->addAction(scaleImageAction);
    flagsMenu->addSeparator();
    flagsMenu->addAction(unsetFlagsAction);
}

void CameraWindow::createToolBar() {
    toolBar = addToolBar(tr("&File"));
    toolBar->setIconSize(QSize(32,32));
    toolBar->addAction(quitAction);
    toolBar->addSeparator();
    toolBar->addAction(screenshotAction);
    toolBar->addAction(videoAction);
    toolBar->addAction(detectFacesAction);
    toolBar->addAction(trackFaceAction);
}

void CameraWindow::createStatusBar() {
    statusLabel = new QLabel(this);
    statusLabel->setIndent(3);
    statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    statusBar()->addWidget(statusLabel);

    connect(cvWidget, SIGNAL(info(const QString &)), statusLabel, SLOT(setText(const QString &)));

    statusLabel->setText("OpenCV Face Detection");
}

void CameraWindow::createActions() {
    quitAction = new QAction(tr("&Quit"), this);
    quitAction->setIcon(QIcon(":/images/icon_quit.png"));
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setStatusTip(tr("Exit the application"));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    screenshotAction = new QAction(tr("Take &Screenshot"), this);
    screenshotAction->setIcon(QIcon(":/images/icon_screenshot.png"));
    screenshotAction->setShortcut(tr("F4"));
    screenshotAction->setStatusTip(tr("Take a screenshot from the camera"));
    connect(screenshotAction, SIGNAL(triggered()), this, SLOT(takeScreenshot()));

    videoAction = new QAction(tr("Grab a &Video"), this);
    videoAction->setIcon(QIcon(":/images/icon_video.png"));
    videoAction->setShortcut(tr("F5"));
    videoAction->setStatusTip(tr("Record a video from the camera"));
    videoAction->setCheckable(true);
    connect(videoAction, SIGNAL(triggered()), this, SLOT(writeVideo()));

    detectFacesAction = new QAction(tr("&Detect Faces"), this);
    detectFacesAction->setIcon(QIcon(":/images/icon_detectfaces.png"));
    detectFacesAction->setShortcut(tr("F6"));
    detectFacesAction->setStatusTip(tr("Detect the faces for each frame of the camera"));
    detectFacesAction->setCheckable(true);
    detectFacesAction->setEnabled(false);
    connect(detectFacesAction, SIGNAL(triggered()), this, SLOT(detectFaces()));

    trackFaceAction = new QAction(tr("CamShift. &Track a Face"), this);
    trackFaceAction->setIcon(QIcon(":/images/icon_detectfaces.png"));
    trackFaceAction->setShortcut(tr("F7"));
    trackFaceAction->setStatusTip(tr("Track a face between frames"));
    trackFaceAction->setCheckable(true);
    trackFaceAction->setEnabled(false);
    connect(trackFaceAction, SIGNAL(triggered()), this, SLOT(trackFace()));

    cascadeFileAction = new QAction(tr("Set a &Cascade File"), this);
    cascadeFileAction->setStatusTip(tr("Set a cascade file for detecting faces"));    
    connect(cascadeFileAction, SIGNAL(triggered()), this, SLOT(setCascadeFile()));

    camshiftDialogAction = new QAction(tr("CamShift"), this);
    camshiftDialogAction->setStatusTip(tr("Change the vMin and sMin variables for CamShift"));
    camshiftDialogAction->setEnabled(false);
    connect(camshiftDialogAction, SIGNAL(triggered()), this, SLOT(showCamShiftDialog()));

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
