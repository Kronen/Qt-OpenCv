#include "opencvwidget.h"

#include <QtDebug>
#include <QFileInfo>
#include <QMessageBox>

OpenCVWidget::OpenCVWidget(QWidget *parent) : QWidget(parent) {
    mCamera = cvCreateCameraCapture(0);
    mVideoWriter = 0;
    fps = 17;

    mDetectFaces = false;
    mFlags = 0;

    // We get a query frame to initialize the capture and to get the frame's dimensions
    IplImage* frame = cvQueryFrame(mCamera);

    // QImage to draw on paint event
    mImage = QImage(QSize(frame->width, frame->height), QImage::Format_RGB888);

    // IplImage * to work with OpenCV functions
    mCvImage = cvCreateImageHeader(cvSize(frame->width, frame->height), frame->depth, frame->nChannels);

    // We share the buffer between QImage and IplImage *
    mCvImage->imageData = (char *)mImage.bits();

    // We copy the frame to our buffer(fliping it if necessary) and then we convert it from BGR to RGB
    if(frame->origin == IPL_ORIGIN_TL) cvCopy(frame, mCvImage, 0);
        else cvFlip(frame, mCvImage, 0);
    cvCvtColor(mCvImage, mCvImage, CV_BGR2RGB);

    // Storage for the rectangles detected
    mStorage = cvCreateMemStorage(0);
    mCascadeFile = "";

    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(queryFrame()));
    mTimer->start(1000/fps);
}

OpenCVWidget::~OpenCVWidget() {
    delete mCascade;
    cvReleaseCapture(&mCamera);
}

void OpenCVWidget::queryFrame() {
    IplImage* frame = cvQueryFrame(mCamera);
    if(!frame) return;    

    if(frame->origin == IPL_ORIGIN_TL) cvCopy(frame, mCvImage, 0);
        else cvFlip(frame, mCvImage, 0);
    cvCvtColor(mCvImage, mCvImage, CV_BGR2RGB);

    if(mVideoWriter && frame) cvWriteFrame(mVideoWriter, frame);

    if(mDetectFaces) detectFace(mCvImage);

    this->update();
}

void OpenCVWidget::videoWrite(QString filename) {
    CvSize size = cvGetSize(mCvImage);
    mVideoWriter = cvCreateVideoWriter(filename.toLatin1(), CV_FOURCC('D','I','V','X'), (fps < 10) ? 5 : fps/2, size);
}

void OpenCVWidget::videoStop() {
    cvReleaseVideoWriter(&mVideoWriter);
}

void OpenCVWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    if(!mImage.isNull()) {
        painter.drawPixmap(0, 0, QPixmap::fromImage(mImage));
    }

    if(!listRect.empty()) {
        QPen pen(palette().dark().color(), 3, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
        painter.setPen(pen);
        foreach(QRect rect, listRect) painter.drawRect(rect);        
    }
}

QString OpenCVWidget::cascadeFile() {
    return mCascadeFile;
}

void OpenCVWidget::setCascadeFile(QString cascadeFile) {
    mCascadeFile = cascadeFile;
    if(mCascade) delete mCascade;
    mCascade = (CvHaarClassifierCascade*)cvLoad(mCascadeFile.toLatin1(), 0, 0, 0);
}

void OpenCVWidget::setFlags(int flags) {
    mFlags = flags;
}

void OpenCVWidget::setDetectFaces(bool detect) {
    mDetectFaces = detect;
}

void OpenCVWidget::detectFace(IplImage *cvImage) {
    CvRect *rect = NULL;
    double scale = 2;

    IplImage *grayImage = cvCreateImage(cvSize(cvImage->width, cvImage->height), cvImage->depth, CV_8UC1);
    IplImage *smallImage = cvCreateImage(cvSize(cvRound(cvImage->width/scale), cvRound(cvImage->height/scale)),
                                         cvImage->depth, CV_8UC1);

    cvCvtColor(cvImage, grayImage, CV_RGB2GRAY);      // Convert to gray scale
    cvResize(grayImage, smallImage, CV_INTER_LINEAR);    // Resize to a small image
    cvEqualizeHist(smallImage, smallImage);         // Grays smoothing (normaliza brillo, incrementa contraste)
    cvClearMemStorage(mStorage);

    if(mCascade) {                                  // It isn't necessary in this context, because mCascade exist if we reach this point
        double timeElapsed = (double)cvGetTickCount();
        CvSeq *faces = cvHaarDetectObjects(smallImage, mCascade, mStorage, 1.1, 2, mFlags, cvSize(40, 40));
        timeElapsed = (double)cvGetTickCount() - timeElapsed;

        qDebug() << QString("detection time = %1").arg(timeElapsed/((double)cvGetTickFrequency()*1000));

        listRect.clear();
        for(int i = 0; i < faces->total; i++) {
            rect = (CvRect*)cvGetSeqElem(faces, i);
            listRect.append(QRect(rect->x * scale, rect->y * scale, rect->width * scale, rect->height * scale));
        }
    }

    cvReleaseImage(&grayImage);
    cvReleaseImage(&smallImage);
}

QImage OpenCVWidget::image() const {
    return mImage;
}

QSize OpenCVWidget::sizeHint() const {
    return mImage.size();
}
