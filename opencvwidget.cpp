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

#include "opencvwidget.h"

#include <QDebug>
#include <QFileInfo>

OpenCVWidget::OpenCVWidget(QWidget *parent) : QWidget(parent) {
    mDetectingFaces = false;
    mTrackingFace = false;
    mFlipV = mFlipH = false;
    mVideoWriter = 0;
    mFps = 16;
    mCvRect = cvRect(-1, -1, 0, 0);
    
    // Camera Initialization
    mCamera = cvCaptureFromCAM(CV_CAP_ANY);

    if(mCamera) {
        // Get a query frame to initialize the capture and to get the frame's dimensions
        IplImage* frame = cvQueryFrame(mCamera);
        this->setMinimumSize(frame->width, frame->height);

        // QImage to draw on paint event
        mImage = QImage(QSize(frame->width, frame->height), QImage::Format_RGB888);

        // IplImage * to work with OpenCV functions
        mCvImage = cvCreateImageHeader(cvSize(frame->width, frame->height), 8, 3);

        // Share the buffer between QImage and IplImage *
        mCvImage->imageData = (char *)mImage.bits();

        // Init Face Detection and Face Tracking
        mFaceDetect = new FaceDetect();
        mFaceDetect->setFlags(CV_HAAR_FIND_BIGGEST_OBJECT); // default
        mCamShift = new CamShift(cvSize(frame->width, frame->height));

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
    if(mFaceDetect) delete mFaceDetect;
    if(mCamShift) delete mCamShift;
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

    // We copy the frame to our buffer(fliping it if necessary)
    if(!(mFlipV ^ (frame->origin == IPL_ORIGIN_TL))) cvFlip(frame, mCvImage, 0);
        else cvCopy(frame, mCvImage, 0);        
    if(mFlipH) cvFlip(mCvImage, mCvImage, 1);

    if(mVideoWriter) cvWriteFrame(mVideoWriter, frame);

    if(mDetectingFaces) mListRect = mFaceDetect->detectFaces(mCvImage);

    if(mTrackingFace) {
        // Check if we have a valid rect. If we have a valid one, we track the face,
        // if not we get a face rect first
        if(!(mCvRect.width > 0 && mCvRect.height > 0)) {
            // Detect the Face
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

    // Convert it from BGR to RGB. QImage works with RGB and cvQueryFrame returns a BGR IplImage
    cvCvtColor(mCvImage, mCvImage, CV_BGR2RGB);
    update();
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

    // It seems that my camera don't get more than 8 fps at 640x480
    mVideoWriter = cvCreateVideoWriter(filename.toUtf8(), CV_FOURCC('D','I','V','X'), 8, size);
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

void OpenCVWidget::switchFlipH() {
    mFlipH = !mFlipH;
}

void OpenCVWidget::switchFlipV() {
    mFlipV = !mFlipV;
}

bool OpenCVWidget::flipH() const {
    return mFlipH;
}

bool OpenCVWidget::flipV() const {
    return mFlipV;
}

void OpenCVWidget::setCascadeFile(QString filename) {
    mFaceDetect->setCascadeFile(filename);
}

QString OpenCVWidget::cascadeFile() const {
    if(mFaceDetect) return mFaceDetect->cascadeFile();
        else return "";
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
