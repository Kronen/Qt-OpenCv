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

#ifndef OPENCVWIDGET_H
#define OPENCVWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QTimer>

#include "cv.h"
#include "highgui.h"

#include "facedetect.h"
#include "camshift.h"

class OpenCVWidget : public QWidget {
    Q_OBJECT

signals:
    void info(const QString &str);

public:
    OpenCVWidget(QWidget *parent = 0);
    ~OpenCVWidget();

    bool isCaptureActive() const;
    bool isFaceDetectAvalaible() const;

    void saveScreenshot();
    void videoWrite();
    void videoStop();

    void flipH();
    void flipV();

    void setDetectFaces(bool);
    void setTrackFace(bool);

    void setFaceDetectFlags(int flags);
    void setCascadeFile(QString filename);

    int camshiftSMin() const;
    int camshiftVMin() const;    

protected:
    void paintEvent(QPaintEvent *event);

private:
    QVector<QRect> detectFaces(IplImage *cvImage);
    void trackFace(CvRect cvRect);

private slots:
    void queryFrame();
    void setCamShiftVMin(int vMin);
    void setCamShiftSMin(int sMin);    

private:
    CvCapture *mCamera;
    IplImage *mCvImage;
    QImage mImage;

    CvVideoWriter *mVideoWriter;
    FaceDetect *mFaceDetect;
    CamShift *mCamShift;

    QVector<QRect> mListRect;
    CvBox2D mCvBox;
    CvRect mCvRect;

    bool mDetectingFaces;
    bool mTrackingFace;
    bool mFlipV, mFlipH;
    double mFps;

    QTimer *mTimer;
};

#endif
