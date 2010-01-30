#include "opencvwidget.h"

#include <QtDebug>
#include <QFileInfo>

OpenCVWidget::OpenCVWidget(QWidget *parent) : QWidget(parent) {
    mCamera = 0;
    mCamera = cvCaptureFromCAM(CV_CAP_ANY);

    if(mCamera) {
        int w, h;
        mDetectingFaces = false;
        mTrackingFace = false;
        mVideoWriter = 0;
        mCvRect = cvRect(-1, -1, 0, 0);

        // Get a query frame to initialize the capture and to get the frame's dimensions
        IplImage* frame = cvQueryFrame(mCamera);
        if(!frame) {
            w = cvGetCaptureProperty(mCamera, CV_CAP_PROP_FRAME_WIDTH);
            h = cvGetCaptureProperty(mCamera, CV_CAP_PROP_FRAME_HEIGHT);
        } else {
            w = frame->width;
            h = frame->height;
        }

        mFps = cvGetCaptureProperty(mCamera, CV_CAP_PROP_FPS);
        mFps = (mFps > 0) ? mFps : 17;

        setMinimumSize(w,h);

        // QImage to draw on paint event
        mImage = QImage(QSize(w, h), QImage::Format_RGB888);

        // IplImage * to work with OpenCV functions
        mCvImage = cvCreateImageHeader(cvSize(w, h), 8, 3);

        // Share the buffer between QImage and IplImage *
        mCvImage->imageData = (char *)mImage.bits();

        // Init FaceDetect abd CamShift
        mFaceDetect = new FaceDetect();
        mCamShift = new CamShift(cvSize(w, h));

        // Try to load a default cascade file
        QFileInfo cascadeFile("haarcascades/haarcascade_frontalface_alt2.xml");
        if(cascadeFile.exists()) mFaceDetect->setCascadeFile(cascadeFile.absoluteFilePath());

        // We call queryFrame 'mFps' times per second
        mTimer = new QTimer(this);
        connect(mTimer, SIGNAL(timeout()), this, SLOT(queryFrame()));
        mTimer->start(1000/mFps);
    }
}

OpenCVWidget::~OpenCVWidget() {
    delete mFaceDetect;
    delete mCamShift;
    cvReleaseCapture(&mCamera);
}

bool OpenCVWidget::isCaptureActive() const {
    return bool(mCamera);
}

bool OpenCVWidget::isFaceDetectAvalaible() const {
    return !mFaceDetect->cascadeFile().isEmpty();
}

void OpenCVWidget::queryFrame() {
    IplImage* frame = cvQueryFrame(mCamera);
    if(!frame) return;

    // We copy the frame to our buffer(fliping it if necessary) and then we convert it from BGR to RGB
    // (QImage works with RGB and cvQueryFrame returns a BGR IplImage)
    if(frame->origin == IPL_ORIGIN_TL) cvCopy(frame, mCvImage, 0);
        else cvFlip(frame, mCvImage, 0);

    if(mVideoWriter) cvWriteFrame(mVideoWriter, frame);

    if(mDetectingFaces) mListRect = mFaceDetect->detectFaces(mCvImage);

    if(mTrackingFace) {
        // Check if we have a valid rect, if we have a valid one we track the face,
        // if not we get a face rect first
        if(!(mCvRect.width > 0 && mCvRect.height > 0)) {
            // Detect the Face
            mFaceDetect->setFlags(CV_HAAR_FIND_BIGGEST_OBJECT);
            QVector<QRect> listRect = mFaceDetect->detectFaces(mCvImage);

            if(!listRect.isEmpty()) {
                QRect trackRect = listRect.at(0);
                mCvRect = cvRect(trackRect.x(), trackRect.y(), trackRect.width(), trackRect.height());
                mCamShift->startTracking(mCvImage, mCvRect);
            }
        } else {
            // Track the Face
            mCvBox = mCamShift->trackFace(mCvImage);
            cvEllipseBox(mCvImage, mCvBox, CV_RGB(255,0,0), 3, CV_AA, 0);
        }
    }

    // Only necesary if we are going to paint into screen because QImage uses RGB
    cvCvtColor(mCvImage, mCvImage, CV_BGR2RGB);
    this->update();
}

void OpenCVWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    if(!mImage.isNull()) painter.drawPixmap(0, 0, QPixmap::fromImage(mImage));

    if(!mListRect.empty()) {
        QPen pen(palette().dark().color(), 4, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
        painter.setPen(pen);
        foreach(QRect rect, mListRect) painter.drawEllipse(rect);

        // Clean the list when we have painted the rects
        mListRect.clear();
    }
}

void OpenCVWidget::saveScreenshot() {
    int i = 0;
    QString filename = QString("webcamPic%1.jpg").arg(i);
    while(QFileInfo(filename).exists())
        filename = QString("webcamPic%1.jpg").arg(i++);

    if(!mImage.isNull()) mImage.save(filename, "JPG", 80);
}

void OpenCVWidget::videoWrite() {
    int i = 1;
    QString filename = QString("webcamVid%1.avi").arg(i);
    while(QFileInfo(filename).exists())
        filename = QString("webcamVid%1.avi").arg(i++);

    CvSize size = cvGetSize(mCvImage);
    mVideoWriter = cvCreateVideoWriter(filename.toUtf8(), CV_FOURCC('D','I','V','X'), (mFps < 10) ? 5 : mFps/2, size);
}

void OpenCVWidget::videoStop() {
    cvReleaseVideoWriter(&mVideoWriter);
}

void OpenCVWidget::setDetectFaces(bool detect) {
    mDetectingFaces = detect;
}

void OpenCVWidget::setTrackFace(bool track) {
    mTrackingFace = track;
    if(!mTrackingFace) mCvRect = cvRect(-1, -1, 0, 0);
}

void OpenCVWidget::setFaceDetectFlags(int flags) {
    mFaceDetect->setFlags(flags);
}

void OpenCVWidget::setFaceDetectCascadeFile(QString filename) {
    mFaceDetect->setCascadeFile(filename);
}

void OpenCVWidget::setCamShiftVMin(int vMin) {
    mCamShift->setVMin(vMin);
}

void OpenCVWidget::setCamShiftSMin(int sMin) {
    mCamShift->setSMin(sMin);
}

int OpenCVWidget::camshiftVMin() const {
    return mCamShift->vMin();
}

int OpenCVWidget::camshiftSMin() const{
   return mCamShift->sMin();
}
