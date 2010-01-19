#include "camerawindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QtDebug>


CameraWindow::CameraWindow(QWidget *parent) : QMainWindow(parent) {
    cvWidget = new OpenCVWidget(this);
    cvWidget->setAttribute(Qt::WA_DeleteOnClose, true);
    setCentralWidget(cvWidget);

    setWindowIcon(QIcon(":/images/OpenCV.ico"));
    createActions();
    createMenu();
    createToolBar();
    createStatusBar();

    writingVideo = false;
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
    if(!isWritingVideo()) {
        qDebug() << "Writing Video";
        writingVideo = true;
        videoAction->setIcon(QIcon(":/images/icon_stopvideo.png"));

        quint8 i = 1;
        QString filename = QString("webcamVideo%1.avi").arg(i);
        while(QFileInfo(filename).exists())
            filename = QString("webcamVideo%1.avi").arg(i++);

        cvWidget->videoWrite(filename);
    } else {
        qDebug() << "Stop Writing Video";
        writingVideo = false;
        videoAction->setIcon(QIcon(":/images/icon_video.png"));

        cvWidget->videoStop();
    }
}

bool CameraWindow::isWritingVideo() {
    return writingVideo;
}

void CameraWindow::detectFaces() {
    if(detectFacesAction->isChecked()) cvWidget->setDetectFaces(true);
        else cvWidget->setDetectFaces(false);
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
    if(findBiggestObjectAction->isChecked()) flags |= CV_HAAR_FIND_BIGGEST_OBJECT;
    if(doRoughSearchAction->isChecked()) flags |= CV_HAAR_DO_ROUGH_SEARCH;
    if(doCannyPruningAction->isChecked()) flags |= CV_HAAR_DO_CANNY_PRUNING;
    if(scaleImageAction->isChecked()) flags |= CV_HAAR_SCALE_IMAGE;
    cvWidget->setFlags(flags);
}

void CameraWindow::createMenu() {
    optionsMenu = menuBar()->addMenu(tr("&Options"));
    optionsMenu->addAction(cascadeFileAction);
    optionsMenu->addSeparator();
    flagsMenu = optionsMenu->addMenu(tr("&Flags"));
    flagsMenu->addAction(findBiggestObjectAction);
    flagsMenu->addAction(doCannyPruningAction);
    flagsMenu->addAction(scaleImageAction);
    flagsMenu->addAction(doRoughSearchAction);
}

void CameraWindow::createToolBar() {
    toolBar = addToolBar(tr("&File"));
    toolBar->addAction(quitAction);
    toolBar->addSeparator();
    toolBar->addAction(screenshotAction);
    toolBar->addAction(videoAction);
    toolBar->addAction(detectFacesAction);
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
    connect(quitAction, SIGNAL(triggered()), this, SLOT(cerrar()));

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

    detectFacesAction = new QAction(tr("Detect &Faces"), this);
    detectFacesAction->setIcon(QIcon(":/images/icon_detectfaces.png"));
    detectFacesAction->setShortcut(tr("F6"));
    detectFacesAction->setStatusTip(tr("Detect the faces on the camera image"));
    detectFacesAction->setCheckable(true);
    detectFacesAction->setEnabled(false);
    connect(detectFacesAction, SIGNAL(triggered()), this, SLOT(detectFaces()));

    cascadeFileAction = new QAction(tr("Set a &Cascade File"), this);
    cascadeFileAction->setStatusTip(tr("Set a cascade file for detecting faces"));    
    connect(cascadeFileAction, SIGNAL(triggered()), this, SLOT(setCascadeFile()));

    // SubMenu Flags
    findBiggestObjectAction = new QAction(tr("Only find the &Biggest Object"), this);
    findBiggestObjectAction->setCheckable(true);
    connect(findBiggestObjectAction, SIGNAL(triggered()), this, SLOT(setFlags()));

    doRoughSearchAction = new QAction(tr("Activate &Rough Search"), this);
    doRoughSearchAction->setCheckable(true);
    connect(doRoughSearchAction, SIGNAL(triggered()), this, SLOT(setFlags()));

    doCannyPruningAction = new QAction(tr("Activate Canny &Pruning"), this);
    doCannyPruningAction->setCheckable(true);
    connect(doCannyPruningAction, SIGNAL(triggered()), this, SLOT(setFlags()));

    scaleImageAction = new QAction(tr("Scales the &Image"), this);
    scaleImageAction->setCheckable(true);
    connect(scaleImageAction, SIGNAL(triggered()), this, SLOT(setFlags()));
}

void CameraWindow::cerrar() {
    delete cvWidget;
    close();
}
