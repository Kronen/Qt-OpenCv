#include "opencvwidget.h"

#include <QtDebug>
#include <QFileInfo>
#include <QMessageBox>

OpenCVWidget::OpenCVWidget(QWidget *parent) : QWidget(parent) {
    mCamera = cvCreateCameraCapture(0);
    mVideoWriter = 0;

    fps = cvGetCaptureProperty(mCamera, CV_CAP_PROP_FPS);
    fps = (fps > 0) ? fps : 17;

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

    // We try to load a default cascade file
    //QFileInfo cascadeFile("haarcascades/haarcascade_frontalface_alt2.xml");
    //mCascadeFile = cascadeFile.exists() ? cascadeFile.absoluteFilePath() : "";
    //setCascadeFile(mCascadeFile);
    mCascadeFile ="";

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
        QPen pen(palette().dark().color(), 3, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
        painter.setPen(pen);
        foreach(QRect rect, listRect) painter.drawRect(rect);
        listRect.clear();
    }    
}

QString OpenCVWidget::cascadeFile() {
    return mCascadeFile;
}

void OpenCVWidget::setDetectFaces(bool detect) {
    mDetectFaces = detect;
}

void OpenCVWidget::setCascadeFile(QString cascadeFile) {
    mCascadeFile = cascadeFile;
    mCascade = (CvHaarClassifierCascade*)cvLoad(mCascadeFile.toLatin1(), 0, 0, 0);
}

void OpenCVWidget::setFlags(int flags) {
    mFlags = flags;
}

/*
Possible values for mFlags on cvHaarDetectObjects. It can be a combination of zero or more of the following values:

        * CV_HAAR_SCALE_IMAGE- for each scale factor used the function will downscale the image rather than “zoom”
            the feature coordinates in the classifier cascade. Currently, the option can only be used alone,
            i.e. the flag can not be set together with the others.
        * CV_HAAR_DO_CANNY_PRUNING- If it is set, the function uses Canny edge detector to reject some image regions
            that contain too few or too much edges and thus can not contain the searched object. The particular
            threshold values are tuned for face detection and in this case the pruning speeds up the processing.
        * CV_HAAR_FIND_BIGGEST_OBJECT- If it is set, the function finds the largest object (if any) in the image.
            That is, the output sequence will contain one (or zero) element(s).
        * CV_HAAR_DO_ROUGH_SEARCH- It should be used only when CV_HAAR_FIND_BIGGEST_OBJECT is set and min_neighbors > 0.
            If the flag is set, the function does not look for candidates of a smaller size as soon as it has found the
            object (with enough neighbor candidates) at the current scale. Typically, when min_neighbors is fixed, the
            mode yields less accurate (a bit larger) object rectangle than the regular single-object mode
            (CV_HAAR_FIND_BIGGEST_OBJECT), but it is much faster, up to an order of magnitude. A greater value of
            min_neighbors may be specified to improve the accuracy.

Note, that in single-object mode CV_HAAR_DO_CANNY_PRUNING does not improve performance much and can even slow down the
processing.
*/

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
        CvSeq *faces = cvHaarDetectObjects(smallImage, mCascade, mStorage, 1.2, 3, mFlags, cvSize(40, 40));
        timeElapsed = (double)cvGetTickCount() - timeElapsed;

        qDebug() << QString("detection time = %1").arg(timeElapsed/((double)cvGetTickFrequency()*1000));

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
