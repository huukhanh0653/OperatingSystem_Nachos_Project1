/* add.c
 *	Simple program to test whether the systemcall interface works.
 *
 *	Just do a add syscall that adds two values and returns the result.
 *
 */
#include "syscall.h"
#include "Utilities.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main()
{
  int result, id, SocketID1, SocketID2, id1, id2;
  char buffer[256];
  char *Content;

  // //! Create, Write and Read => Successfully!
  // Create("hihi.txt");
  // id = Open("hihi.txt", 0);
  // result = Write("Hello\n",6,id);
  // if (result < 0) Write("Cannot Write\n",14,1);
  // result = Read(buffer,5,id);
  // if (result < 0)
  // Write("Failed!\n",9,1);
  // Close(id);

  /*//! Echo program here: => Successfully!
  Content = "Please input content from keyboard (Press Ctrl+D to end):";
  Write(Content,len(Content),CONSOLE_OUTPUT);
  Read(buffer,256,CONSOLE_INPUT);
  Content = "Content you wrote: ";
  Write(Content,len(Content),CONSOLE_OUTPUT);
  Write(buffer,len(buffer),CONSOLE_OUTPUT);*/

  /*//! Seek file program here: => Successfully
  id = Open("hihi.txt", 0);
  result = Seek(7,id);
  result = Read(buffer,6,id);
  _concat(buffer,"\n");
  Write(buffer,len(buffer),CONSOLE_OUTPUT);
  Close(id);*/

  /*//! Remove program here: => Successfully
  result = Remove("hihi.txt");
  if (result == 0) Write("Success\n",8,CONSOLE_OUTPUT);*/

  //! Socket here
  // Write("Hello\n", 7, 1);
  // Content = "Hello Socket!\n";
  // SocketID1 = SocketTCP();
  // result = Connect(SocketID1, "127.0.0.1", 8081);
  // Write(Content, len(Content), SocketID1);
  // Read(Content, len(Content), SocketID1);
  // Close(SocketID1);
  // Halt();
  //! File transfering
  // Create("hihi.txt");
  // Create("haha.txt");
  // id1 = Open("hihi.txt", 0);
  // Content = Read(buffer,256, id1);
  // Write(Content, len(Content), SocketID1);
  // Read(Content, len(Content), SocketID1);
  // id2 = Open("haha.txt", 0);
  // Write(Content, len(Content), id2);
  // Close(SocketID1);
  // Close(id1);
  // Close(id2);

  //!
  result = Open("hihi.txt",0);
  Read(Content,256,result);
  SocketID1 = SocketTCP();
  Connect(SocketID1, "127.0.0.1", 8081);
  Write(Content, len(Content), SocketID1);
  Read(Content, len(Content), SocketID1);
  id2 = Open("haha.txt", 0);
  Write(Content, len(Content), id2);
  Close(SocketID1);
  // Close(id1);
  Close(id2);
  // Halt();
  Halt();

  /* not reached */
}
