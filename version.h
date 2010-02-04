#ifndef APPVERSION_H
#define APPVERSION_H

#include <QString>

const int MAJOR_VERSION = 0;
const int MINOR_VERSION = 5;
const int REVISION_VERSION = 9;

QString appVersion = QString("%1.%2.%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(REVISION_VERSION);
QString appName = "Qt + OpenCV";

#endif // VERSION_H
