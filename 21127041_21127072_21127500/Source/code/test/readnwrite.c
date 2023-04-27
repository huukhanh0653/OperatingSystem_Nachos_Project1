#include "syscall.h"

int main() {
  OpenFileID fileID;
  char buffer[512];
  int bytesRead, bytesWritten;

  // open a file to read and write
  fileID = Open("example.txt");
  
  if (fileID == -1) {
    // fail if file couldn't be opened
    Write("Failed to open file.\n", 22, ConsoleOutput);
    return -1;
  }

  // read data from file
  bytesRead = Read(buffer, 512, fileID);

  if (bytesRead == -1) {
    // fail if unable to read from file
    Write("Failed to read file.\n", 21, ConsoleOutput);
    return -1;
  }

  // write data to console
  bytesWritten = Write(buffer, bytesRead, ConsoleOutput);

  if (bytesWritten == -1) {
    // fail if unable to write to console
    Write("Failed to write to console.\n", 29, ConsoleOutput);
    return -1;
  }

  // write data to file
  bytesWritten = Write(buffer, bytesRead, fileID);

  if (bytesWritten == -1) {
    // fail if unable to write to file
    Write("Failed to write to file.\n", 26, ConsoleOutput);
    return -1;
  }

  Close(fileID);

  return 0;
}
