/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef DEFINES_H
#define DEFINES_H
#include <QxtLogger>

#define COMPANY "UDP Software"
#define PRODUCT "Dwarf Therapist"

#ifndef VERSION_MAJOR
	#define VERSION_MAJOR 0
#endif

#ifndef VERSION_MINOR
	#define VERSION_MINOR 1
#endif

#ifndef VERSION_PATCH
	#define VERSION_PATCH 6
#endif

#define LOG   qxtLog
#define LOGD  qDebug() << __FILE__ << "(ln" << __LINE__ << "): "
#define LOGI  qxtLog->info()
#define LOGW  qWarning() << __FILE__ << "(ln" << __LINE__ << ") [" << __FUNCTION__ << "] "
#define LOGC  qCritical() << __FILE__ << "(ln" << __LINE__ << ") [" << __FUNCTION__ << "] "
#define FATAL qFatal() << __FILE__ << "(ln" << __LINE__ << ") [" << __FUNCTION__ << "] "
#define TRACE qxtLog->trace()

#define DEFAULT_CELL_SIZE 16

#endif