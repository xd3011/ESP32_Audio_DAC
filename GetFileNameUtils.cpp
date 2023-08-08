#include "GetFileNameUtils.h"
#include "FS.h"

String getBackFileName(fs::FS &SD, String fileName) {
  File dir = SD.open("/");
  File dirFirst = SD.open("/");

  // Check open folder
  if (!dir || !dirFirst) {
    return "Unable to open folder";
  }

  dirFirst.openNextFile();

  // If fileName is first of SD Card
  if (String(dirFirst.openNextFile().name()) == fileName) {
    // LastFile is last of SD Card
    File lastFile;
    while (true) {
      File entry = dirFirst.openNextFile();
      if (!entry) {
        break;
      }
      // Search and find is lastFile of SD Card
      if (!entry.isDirectory()) {
        lastFile = entry;
      }
    }
    if (!lastFile) {
      return "File can't found 1";
    }
    return String(lastFile.name());
  }

  File prevFile;
  // Get PrevFileName and lastFileName
  while (true) {
    File currentFile = dir.openNextFile();
    if (!currentFile) {
      break;
    }
    // Search and file is lastFile
    if (!currentFile.isDirectory() && String(currentFile.name()) == fileName) {
      break;
    }
    // If currentFile is not fileName and currentFile go to fileName => currentFile is prev of fileName in SD
    if (String(currentFile.name()) != fileName) {
      prevFile = currentFile;
    }
  }

  if (!prevFile) {
    return "File can't found 2";
  }

  return String(prevFile.name());
  prevFile.close();
}

String getNextFileName(fs::FS &SD, String fileName) {
  // Search and find fileName
  File dir = SD.open("/");

  // Check open folder
  if (!dir) {
    return "Unable to open folder";
  }

  // Get lastFileName
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    // Search and file is lastFile
    if (!entry.isDirectory() && String(entry.name()) == fileName) {
      break;
    }
    entry.close();
  }

  // Get nextFileName of fileName
  File nextFile = dir.openNextFile();

  // if nextFile is not found (dir pointer is null "lastFile of SD")
  if (!nextFile) {
    File dirNew = SD.open("/");
    File firstFile = dirNew.openNextFile();
    firstFile = dirNew.openNextFile();
    if (!firstFile) {
      return "Unable to open folder";
    } else {
      return String(firstFile.name());
    }
  } else {
    return String(nextFile.name());
  }
  nextFile.close();
  return "NULL";
}