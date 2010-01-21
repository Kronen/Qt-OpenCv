#include "opencvwidget.h"

#include <QtDebug>
#include <QFileInfo>

OpenCVWidget::OpenCVWidget(QWidget *parent) : QWidget(parent) {
    mCamera = cvCaptureFromCAM(CV_CAP_ANY);

    if(mCamera) {
        int w, h;
        mDetectingFaces = false;
        mTrackingFace = false;
        mCascade = 0;
        mFlags = 0;
        mVideoWriter = 0;
        mCvRect = cvRect(-1, -1, 0, 0);

        mFps = cvGetCaptureProperty(mCamera, CV_CAP_PROP_FPS);
        mFps = (mFps > 0) ? mFps : 17;

        // Storage for the rectangles detected
        mStorage = cvCreateMemStorage(0);

        // Try to load a default cascade file
        QFileInfo cascadeFile("haarcascades/haarcascade_frontalface_alt2.xml");
        if(cascadeFile.exists()) setCascadeFile(cascadeFile.absoluteFilePath());

        // Get a query frame to initialize the capture and to get the frame's dimensions
        IplImage* frame = cvQueryFrame(mCamera);
        if(!frame) {
            w = cvGetCaptureProperty(mCamera, CV_CAP_PROP_FRAME_WIDTH);
            h = cvGetCaptureProperty(mCamera, CV_CAP_PROP_FRAME_HEIGHT);
        } else {
            w = frame->width;
            h = frame->height;
        }

        // QImage to draw on paint event
        mImage = QImage(QSize(w, h), QImage::Format_RGB888);

        // IplImage * to work with OpenCV functions
        mCvImage = cvCreateImageHeader(cvSize(w, h), 8, 3);

        // Share the buffer between QImage and IplImage *
        mCvImage->imageData = (char *)mImage.bits();

        // Init CamShift
        mCamShift = new CamShift(cvSize(w, h));

        // We call queryFrame 'mFps' times per second
        mTimer = new QTimer(this);
        connect(mTimer, SIGNAL(timeout()), this, SLOT(queryFrame()));
        mTimer->start(1000/mFps);
    }
}

OpenCVWidget::~OpenCVWidget() {
    delete mCamShift;
    if(mCascade) cvReleaseHaarClassifierCascade(&mCascade);
    cvReleaseCapture(&mCamera);
}

bool OpenCVWidget::isCaptureActive() {
    if(!mCamera) return false;
    return true;
}

void OpenCVWidget::queryFrame() {
    IplImage* frame = cvQueryFrame(mCamera);
    if(!frame) return;

    // We copy the frame to our buffer(fliping it if necessary) and then we convert it from BGR to RGB
    // (QImage works with RGB and cvQueryFrame returns a BGR IplImage)
    if(frame->origin == IPL_ORIGIN_TL) cvCopy(frame, mCvImage, 0);
        else cvFlip(frame, mCvImage, 0);

    if(mVideoWriter && frame) cvWriteFrame(mVideoWriter, frame);

    if(mDetectingFaces) mListRect = detectFaces(mCvImage);

    if(mTrackingFace) {
        cvCopy(frame, mCvImage, 0);

        if(mCvRect.x > 0 && mCvRect.y > 0) {
            mCvBox = mCamShift->trackFace(mCvImage);
        } else {
            setFlags(CV_HAAR_FIND_BIGGEST_OBJECT);
            QVector<QRect> listRect = detectFaces(mCvImage);
            if(!listRect.isEmpty()) {
                QRect trackRect = listRect.at(0);
                mCvRect = cvRect(trackRect.x(), trackRect.y(), trackRect.width(), trackRect.height());
                mCamShift->startTracking(mCvImage, mCvRect);
            }
        }
    }

    // Only necesary if we are going to paint into screen because QImage uses RGB
    cvCvtColor(mCvImage, mCvImage, CV_BGR2RGB);
    this->update();
}

void OpenCVWidget::videoWrite(QString filename) {
    CvSize size = cvGetSize(mCvImage);
    mVideoWriter = cvCreateVideoWriter(filename.toUtf8(), CV_FOURCC('D','I','V','X'), (mFps < 10) ? 5 : mFps/2, size);
}

void OpenCVWidget::videoStop() {
    cvReleaseVideoWriter(&mVideoWriter);
}

void OpenCVWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    if(!mImage.isNull()) painter.drawPixmap(0, 0, QPixmap::fromImage(mImage));

    if(!mListRect.empty()) {
        QPen pen(palette().dark().color(), 3, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
        painter.setPen(pen);
        foreach(QRect rect, mListRect) painter.drawRect(rect);
        // Clean the list when we have painted the rects
        mListRect.clear();
    }
}

/*
Possible values for mFlags on cvHaarDetectObjects. It can be a combination of zero or more of the following values:

        * CV_HAAR_SCALE_IMAGE- for each scale factor used the function will downscale the image rather than "zoom"
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

QVector<QRect> OpenCVWidget::detectFaces(IplImage *cvImage) {
    QVector<QRect> listRect;
    CvRect *rect = NULL;
    double scale = 1;

    // Create a gray scale image (1 channel) to turn it into a small image that we send to cvHaarDetectObjects to process
    IplImage *grayImage = cvCreateImage(cvSize(cvImage->width, cvImage->height), cvImage->depth, CV_8UC1);
    IplImage *smallImage = cvCreateImage(cvSize(cvRound(cvImage->width/scale), cvRound(cvImage->height/scale)),
                                         cvImage->depth, CV_8UC1);
    
    cvCvtColor(cvImage, grayImage, CV_BGR2GRAY);


    cvResize(grayImage, smallImage);

    cvEqualizeHist(smallImage, smallImage);         // Grays smoothing (normaliza brillo, incrementa contraste)
    cvClearMemStorage(mStorage);

    if(mCascade) {                                  // It isn't necessary in this context, because mCascade exist if we reach this point
        double timeElapsed = (double)cvGetTickCount();
        CvSeq *faces = cvHaarDetectObjects(smallImage, mCascade, mStorage, 1.2, 3, mFlags, cvSize(64, 64));
        timeElapsed = (double)cvGetTickCount() - timeElapsed;

        qDebug() << QString("detection time = %1").arg(timeElapsed/((double)cvGetTickFrequency()*1000));

        for(int i = 0; i < faces->total; i++) {
            rect = (CvRect*)cvGetSeqElem(faces, i);
            listRect.append(QRect(rect->x * scale, rect->y * scale, rect->width * scale, rect->height * scale));
        }
    }

    cvReleaseImage(&grayImage);
    cvReleaseImage(&smallImage);

    return listRect;
}

void OpenCVWidget::setDetectFaces(bool detect) {
    mDetectingFaces = detect;
}

void OpenCVWidget::setTrackFace(bool track) {
    if(!track) mCvRect = cvRect(-1, -1, 0, 0);
    mTrackingFace = track;
}

QString OpenCVWidget::cascadeFile() {
    return mCascadeFile;
}

void OpenCVWidget::setCascadeFile(QString cascadeFile) {
    mCascadeFile = cascadeFile;
    if(mCascade) cvReleaseHaarClassifierCascade(&mCascade);
    mCascade = (CvHaarClassifierCascade*)cvLoad(mCascadeFile.toUtf8());
}

void OpenCVWidget::setFlags(int flags) {
    mFlags = flags;
}

QImage OpenCVWidget::image() const {
    return mImage;
}

QSize OpenCVWidget::sizeHint() const {
    return mImage.size();
}
