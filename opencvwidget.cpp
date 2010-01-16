#include "opencvwidget.h"
#include <QtDebug>
#include <QResizeEvent>

OpenCVWidget::OpenCVWidget(QWidget *parent) : QWidget(parent) {
    mCamera = cvCreateCameraCapture(0);
    mVideoWriter = 0;
    fps = 10;

    mDetectFaces = false;
    mFaceRect = QRect(-1,-1,0,0);

    IplImage* frame = cvQueryFrame(mCamera);
    w = frame->width;
    h = frame->height;

    mImage = QImage(QSize(w, h), QImage::Format_RGB888);

    mCvImage = cvCreateImageHeader(cvSize(w, h), frame->depth, frame->nChannels);
    mCvImage->imageData = (char *)mImage.bits();        // Buffer sharing between QImage and IplImage

    // Convert from BGR to RGB
    if(frame->origin == IPL_ORIGIN_TL) cvCopy(frame, mCvImage, 0);
        else cvFlip(frame, mCvImage, 0);
    cvCvtColor(mCvImage, mCvImage, CV_BGR2RGB);

    mStorage = cvCreateMemStorage(0);
    QString cascadeName = "haarcascade_frontalface_alt2.xml";
    mCascade = (CvHaarClassifierCascade*)cvLoad(cascadeName.toLatin1(), 0, 0, 0);    

    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(queryFrame()));
    mTimer->start(1000/fps);
}

OpenCVWidget::~OpenCVWidget() {
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
        QPen pen(palette().dark().color(), 5, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
        painter.setPen(pen);
        foreach(QRect rect, listRect) painter.drawRect(mFaceRect);
        listRect.clear();
    }

/*  if(mFaceRect.x() > 0 && mFaceRect.y() > 0) {
        QPen pen(palette().dark().color(), 5, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
        painter.setPen(pen);
        painter.drawRect(mFaceRect);
    }*/
}

void OpenCVWidget::setDetectFaces(bool detect) {
    mDetectFaces = detect;
    if(!detect) mFaceRect = QRect(-1,-1,0,0);
}

void OpenCVWidget::detectFace(IplImage *cvImage) {
    CvRect *rect = NULL;
    double scale = 1;
    int i;

    IplImage *grayImage = cvCreateImage(cvSize(cvImage->width, cvImage->height), cvImage->depth, CV_8UC1);
    IplImage *smallImage = cvCreateImage(cvSize(cvRound(cvImage->width/scale), cvRound(cvImage->height/scale)),
                                         cvImage->depth, CV_8UC1);

    //cvCvtColor(cvImage, grayImage, CV_RGB2GRAY);      // Convert to gray scale (Segmentation Fault)
    cvConvertImage(cvImage, grayImage, 0);

    //cvResize(grayImage, smallImage, CV_INTER_LINEAR);    // Resize to a small image (Segmentation Fault)
    cvCopy(grayImage, smallImage, 0);

    cvEqualizeHist(smallImage, smallImage);         // Grays smoothing (normaliza brillo, incrementa contraste)
    cvClearMemStorage(mStorage);

    if(mCascade) {
        double timeElapsed = (double)cvGetTickCount();
        CvSeq *faces = cvHaarDetectObjects(smallImage, mCascade, mStorage, 1.1, 2, 0

                                           | CV_HAAR_FIND_BIGGEST_OBJECT
                                           | CV_HAAR_DO_ROUGH_SEARCH
                                           //| CV_HAAR_DO_CANNY_PRUNING
                                           | CV_HAAR_SCALE_IMAGE
                                           , cvSize(30, 30));
        timeElapsed = (double)cvGetTickCount() - timeElapsed;

        qDebug() << QString("detection time = %1").arg(timeElapsed/((double)cvGetTickFrequency()*1000));

        for(i = 0; i < faces->total; i++) {
            rect = (CvRect*)cvGetSeqElem(faces, i);
            listRect.append(QRect(rect->x * scale, rect->y * scale, rect->width * scale, rect->height * scale));
        }
    }

    cvReleaseImage(&grayImage);
    cvReleaseImage(&smallImage);

    if(rect) mFaceRect =  QRect(rect->x * scale, rect->y * scale, rect->width * scale, rect->height * scale);
        else mFaceRect =  QRect(-1,-1,0,0);
}

QImage OpenCVWidget::image() const {
    return mImage;
}

QSize OpenCVWidget::sizeHint() const {
    return mImage.size();
}
