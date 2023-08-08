#ifndef GET_FILE_NAME_UTILS_H
#define GET_FILE_NAME_UTILS_H

#include "FS.h"

String getBackFileName(fs::FS &SD, String fileName);
String getNextFileName(fs::FS &SD, String fileName);

#endif // GET_FILE_NAME_UTILS_H