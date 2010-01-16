#include "camerawindow.h"

#include <QFileInfo>
#include <QtDebug>

CameraWindow::CameraWindow(QWidget *parent) : QMainWindow(parent) {
    cvWidget = new OpenCVWidget(this);
    cvWidget->setAttribute(Qt::WA_DeleteOnClose, true);
    setCentralWidget(cvWidget);

    setWindowIcon(QIcon(":/images/OpenCV.ico"));
    createActions();
    createToolBar();
    createStatusBar();

    writingVideo = false;
    detectingFaces = false;
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
    if(!writingVideo) {
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

void CameraWindow::detectFaces() {
    if(!detectingFaces) {
        detectingFaces = true;
        cvWidget->setDetectFaces(detectingFaces);
    } else {
        detectingFaces = false;
        cvWidget->setDetectFaces(detectingFaces);
    }
}


void CameraWindow::createToolBar() {
    toolBar = addToolBar(tr("&File"));
    toolBar->addAction(quitAction);
    toolBar->addSeparator();
    toolBar->addAction(screenshotAction);
    toolBar->addAction(videoAction);
    toolBar->addAction(detectfacesAction);
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
    screenshotAction->setShortcut(tr("F5"));
    screenshotAction->setStatusTip(tr("Take a screenshot from the camera"));
    connect(screenshotAction, SIGNAL(triggered()), this, SLOT(takeScreenshot()));

    videoAction = new QAction(tr("Grab a &Video"), this);
    videoAction->setIcon(QIcon(":/images/icon_video.png"));
    videoAction->setShortcut(tr("F6"));
    videoAction->setStatusTip(tr("Record a video from the camera"));
    connect(videoAction, SIGNAL(triggered()), this, SLOT(writeVideo()));

    detectfacesAction = new QAction(tr("Detect &Faces"), this);
    detectfacesAction->setIcon(QIcon(":/images/icon_detectfaces.png"));
    detectfacesAction->setShortcut(tr("F6"));
    detectfacesAction->setStatusTip(tr("Detect the faces on the camera image"));
    connect(detectfacesAction, SIGNAL(triggered()), this, SLOT(detectFaces()));
}

void CameraWindow::cerrar() {
    delete cvWidget;
    close();
}
