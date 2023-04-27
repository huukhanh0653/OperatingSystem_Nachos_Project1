/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__
#define __USERPROG_KSYSCALL_H__

// #include "kernel.h"
#include "FileManager.h"
#include "synchconsole.h"

#define _ConsoleInput 0
#define _ConsoleOutput 1

// File management-------------------------
Table table;

//! For File processing

int SysOpen(char *fileName, int type)
{

  if (type != 0 && type != 1)
  {
    cout << "Invalid file type!\n";
    return -1;
  }
  int OpenID = table.open(fileName, type);
  if (OpenID != -1)
  {
    cout << "Opened file!\n";
    return OpenID;
  }
  else
  {
    cout << "Cannot open file!\n";
    return -1;
  }
}

int SysRead(char *buffer, int charCount, OpenFileId fileId)
{
  if (fileId < 0)
    return -1;
  if (fileId == _ConsoleInput)
    return INPUT->GetString(buffer, charCount);
  if (!table.getFile(fileId))
    return -1;
  return table.getFile(fileId)->ReadFile(buffer, charCount);
  // return -1;
}

int SysWrite(char *buffer, int charCount, OpenFileId fileId)
{
  if (fileId == _ConsoleOutput)
    return OUTPUT->PutString(buffer, charCount);

  if (!table.getFile(fileId))
    return -1;

  if (fileId / 100 != READONLY_MODE)
  {
    int result = (table.getFile(fileId)!=NULL?table.getFile(fileId)->WriteFile(buffer, charCount):-1);
    if (result < 0)
      cout << "Cannot Write\n";
    return result;
  }
  return -1;
}

int SysClose(OpenFileId id)
{
  if (table.isEmpty())
    return -1;
  return table.closeFile(id);
}

int SysSeek(int position, OpenFileId id)
{
  if (id == 0 || id == 1)
    return -1;
  if (!table.getFile(id))
    return -1;

  int result = table.getFile(id)->seekFile(position);
  cout << "Seeking to: " << result << endl;
  return table.getFile(id)->Offset();
}

int SysCreate(char *filename)
{
  if (strlen(filename) == 0)
  {
    // cout << "\n Invalid file name \n";
    DEBUG(dbgAddr, "\n Invalid file name \n");
    // callMachine->WriteRegister(2,-1);
    return -1;
  }
  else if (filename == NULL)
  {
    // cout << "\n Problem with memory \n";
    DEBUG(dbgAddr, "\n Problem with memory \n");
    // Return(-1);
    return -1;
  }
  else if (!FILESYSTEM->Create(filename))
  {
    cout << "\n File creating failed! \n";
    return -1;
  }
  cout << " \n File " << filename << " has been created! \n";
  return 1;
}
// ----------------------------------

int SysRemove(char *name)
{
  OpenFileId _Exception = table.isOpened(name, 0);
  if (_Exception > -1)
    table.closeFile(_Exception);

  _Exception = table.isOpened(name, 1);
  if (_Exception > -1)
    table.closeFile(_Exception);

  return (FILESYSTEM->Remove(name) ? 0 : -1);
}

void SysHalt()
{
  kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

int SysSocketTCP()
{
  return table.open(NULL, SOCKET_MODE);
}

int SysConnect(int socketid, char *ip, int port)
{
  if (table.getFile(socketid)->connectSocket(ip, port) < 0)
  {
    cout << "Connection failed!\n";
    return -1;
  }
  return socketid; // Return socket file descriptor on success
}

int SysGetIdSocket(int id)
{
  return table.getFile(id)->getSocketID();
}

void copyStringFromMachine(int from, char *to, unsigned size)
{
  ASSERT(size >= 0);

  unsigned i = 0;

  // Loop through the bytes until we find the null terminator or reach the end of the buffer.
  while (i < size && kernel->machine->ReadMem(from + i, 1, (int *)&to[i]))
  {
    if (to[i] == '\0')
      return;
    ++i;
  }

  // If we didn't find the null terminator, make sure the buffer is null-terminated.
  if (i == size)
    --i;
  to[i] = '\0';
}
#endif /* ! __USERPROG_KSYSCALL_H__ */
