#include "camshift.h"

#include <QtDebug>

CamShift::CamShift(CvSize size) {
    float *ranges = mRangesArray;
    mHistBins = 30;
    mRangesArray[0] = 0;
    mRangesArray[1] = 180;
    mVMin = 65;
    mVMax = 256;
    mSMin = 55;
    mFrames = 0;
    mHSVImg  = cvCreateImage(size, 8, 3);
    mHueImg  = cvCreateImage(size, 8, 1);
    mMask    = cvCreateImage(size, 8, 1);
    mProbImg = cvCreateImage(size, 8, 1);

    mHist = cvCreateHist(1, &mHistBins, CV_HIST_ARRAY, &ranges, 1);
}

CamShift::~CamShift() {
    cvReleaseImage(&mHSVImg);
    cvReleaseImage(&mHueImg);
    cvReleaseImage(&mMask);
    cvReleaseImage(&mProbImg);

    cvReleaseHist(&mHist);
}

void CamShift::startTracking(IplImage *cvImage, CvRect cvRect) {
    float maxVal = 0.f;

    updateHueImage(cvImage);        // Get mHSVImg and mHueImg

    // Calc the histogram of the defined rect in mHueImg
    cvSetImageROI(mHueImg, cvRect);
    cvSetImageROI(mMask, cvRect);
    cvCalcHist(&mHueImg, mHist, 0, mMask);                                       // (image, hist, accumulate, mask)

    // We get the MaxValue in the histogram and scale for that value
    cvGetMinMaxHistValue(mHist, 0, &maxVal, 0, 0);                              // (histogram, minVal, maxVal, minIdx, maxIdx)
    cvConvertScale(mHist->bins, mHist->bins, maxVal ? (255.0 / maxVal) : 0, 0); // (src, dst, scale, shift)
    cvResetImageROI(mHueImg);
    cvResetImageROI(mMask);

    // Store the previous face location
    mPrevFaceRect = cvRect;
}

CvBox2D CamShift::trackFace(IplImage *cvImage) {
    CvConnectedComp components;

    // Create a new hue image
    updateHueImage(cvImage);

    // Create a probability image based on the face histogram (precalculated on startTracking())
    cvCalcBackProject(mHueImg, mProbImg, mHist);        // Error: Unable to restore previously selected frame
    cvAnd(mProbImg, mMask, mProbImg, 0);                // (src1, src2, dst, mask)

    CvSize size = cvGetSize(mProbImg);

    // Check for face out of scope
    if(mPrevFaceRect.x < 0) mPrevFaceRect.x = 0;
    if(mPrevFaceRect.x >= size.width) mPrevFaceRect.x = size.width - 1;
    if(mPrevFaceRect.y < 0) mPrevFaceRect.y = 0;
    if(mPrevFaceRect.y >= size.height) mPrevFaceRect.y = size.height - 1;
    if(mPrevFaceRect.x + mPrevFaceRect.width > size.width) mPrevFaceRect.width = size.width - mPrevFaceRect.x;
    if(mPrevFaceRect.y + mPrevFaceRect.height > size.height) mPrevFaceRect.height = size.height - mPrevFaceRect.y;

    // Use CamShift to find the center of the new face probability
    cvCamShift(mProbImg, mPrevFaceRect, cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1),
                &components, &mFaceBox);

    // Update face location and angle
    mPrevFaceRect = components.rect;
    mFaceBox.angle = -mFaceBox.angle;

    return mFaceBox;
}

void CamShift::updateHueImage(const IplImage *cvImage) {
    // Convert to HSV color model
    cvCvtColor(cvImage, mHSVImg, CV_BGR2HSV);

    // Mask out-of-range values
    cvInRangeS(mHSVImg, cvScalar(0, mSMin, MIN(mVMin, mVMax), 0), cvScalar(180, 256, MAX(mVMin, mVMax), 0), mMask);

    // Extract the hue channel
    cvSplit(mHSVImg, mHueImg, 0, 0, 0);
}

void CamShift::setVMin(int vMin) {
    mVMin = vMin;
}

void CamShift::setSMin(int sMin) {
    mSMin = sMin;
}
