#ifndef VERSION_H
#define VERSION_H

#include <QString>

const int MAJOR_VERSION = 0;
const int MINOR_VERSION = 4;
const int REVISION_VERSION = 7;

QString version = QString("%1.%2.%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(REVISION_VERSION);

#endif // VERSION_H
