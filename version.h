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

#ifndef APPVERSION_H
#define APPVERSION_H

#include <QString>

const int MAJOR_VERSION = 0;
const int MINOR_VERSION = 6;
const int REVISION_VERSION = 4;

QString appVersion = QString("%1.%2.%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(REVISION_VERSION);
QString appName = "Qt + OpenCV";

#endif // VERSION_H
