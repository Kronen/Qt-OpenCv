#ifndef OPENCVWIDGET_H
#define OPENCVWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QTimer>
#include <cv.h>
#include <highgui.h>

class OpenCVWidget : public QWidget {
    Q_OBJECT

signals:
    void info(const QString &str);

public:
    OpenCVWidget(QWidget *parent = 0);
    ~OpenCVWidget();

    void videoWrite(QString filename);
    void videoStop();
    void setDetectFaces(bool);
    void setCascadeFile(QString cascadeFile);
    void setFlags(int flags);
    QString cascadeFile();

    QImage image() const;
    QSize sizeHint() const;    

protected:
    void paintEvent(QPaintEvent *event);

private:
    void detectFace(IplImage *cvImage);

private slots:
    void queryFrame();

private:
    CvCapture *mCamera;
    double fps;

    IplImage *mCvImage;
    QImage mImage;

    CvHaarClassifierCascade *mCascade;
    CvMemStorage *mStorage;
    QString mCascadeFile;
    QVector<QRect> listRect;
    bool mDetectFaces;
    int mFlags;

    CvVideoWriter *mVideoWriter;

    QTimer *mTimer;
    double totalTime;
    int cont;
};

#endif
