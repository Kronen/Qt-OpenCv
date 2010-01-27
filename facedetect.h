#ifndef FACEDETECT_H
#define FACEDETECT_H

#include <QString>
#include <QVector>
#include <QRect>

#include <cv.h>

class FaceDetect {
public:
    FaceDetect();
    ~FaceDetect();

    void setCascadeFile(QString cascadeFile);
    QString cascadeFile() const;
    void setFlags(int flags);
    QVector<QRect> detectFaces(IplImage *cvImage);

private:
    CvHaarClassifierCascade *mCascade;
    CvMemStorage *mStorage;

    QString mCascadeFile;    
    int mFlags;
};

#endif // FACEDETECT_H
