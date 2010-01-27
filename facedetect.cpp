#include "facedetect.h"
#include <QDebug>

FaceDetect::FaceDetect() {
    mCascadeFile = "";
    mCascade = 0;
    mFlags = 0;

    // Storage for the rectangles detected
    mStorage = cvCreateMemStorage(0);
}

FaceDetect::~FaceDetect() {
    if(mCascade) cvReleaseHaarClassifierCascade(&mCascade);
}

void FaceDetect::setCascadeFile(QString cascadeFile) {
    mCascadeFile = cascadeFile;
    if(mCascade) cvReleaseHaarClassifierCascade(&mCascade);
    mCascade = (CvHaarClassifierCascade *) cvLoad(mCascadeFile.toUtf8());
}

QString FaceDetect::cascadeFile() const {
    return mCascadeFile;
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

void FaceDetect::setFlags(int flags) {
    mFlags = flags;
}

QVector<QRect> FaceDetect::detectFaces(IplImage *cvImage) {
    QVector<QRect> listRect;
    CvRect *rect = NULL;
    double scale = 1.3;

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



