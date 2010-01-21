#ifndef CAMSHIFT_H
#define CAMSHIFT_H

#include <cv.h>

class CamShift {
    public:
        CamShift(CvSize size);
        ~CamShift();

    public:
        // Main Control functions
        void releaseTracker();
        void startTracking(IplImage *cvImage, CvRect cvRect);
        CvBox2D trackFace(IplImage *cvImage);

        // Parameter settings
        void setVMin(int vmin);
        void setSMin(int smin);
        int vMin();
        int sMin();

    private:
        void updateHueImage(const IplImage *cvImage);

    private:
        int mHistBins;          // Number of Histogram Bins
        float mRangesArray[2];       // Histogram Range
        int mVMin, mVMax;
        int mSMin;              // Limits for calculating HUE

        IplImage *mHSVImg;     // Image converted to HSV color mode
        IplImage *mHueImg;     // Hue channel of the HSV image
        IplImage *mMask;       // Image for masking pixels
        IplImage *mProbImg;    // Face Probability Estimates for each pixel
        CvHistogram *mHist;    // Histogram of HUE in the original image

        CvRect mPrevFaceRect;    // Location of Face in Previous Frame
        CvBox2D mFaceBox;        // Current Face-Location Estimate
        int mFrames;
};

#endif // CAMSHIFT_H
