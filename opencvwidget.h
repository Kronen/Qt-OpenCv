#ifndef OPENCVWIDGET_H
#define OPENCVWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QTimer>

#include <cv.h>
#include <highgui.h>

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
    double mFps;        

    QTimer *mTimer;
};

#endif
