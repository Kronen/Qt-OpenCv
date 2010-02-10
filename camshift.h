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

#ifndef CAMSHIFT_H
#define CAMSHIFT_H

#include "cv.h"

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
        int vMin() const;
        int sMin() const;

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
