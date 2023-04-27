#include "syscall.h"
#include "Utilities.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main()
{
    int result, id1, id2, SocketID1, SocketID2;
    char Content[256];
    id1 = Open("Text1.txt", 0);
    Read(Content, 256, id1);
    SocketID1 = SocketTCP();
    Connect(SocketID1, "127.0.0.1", 8081);
    Write(Content, len(Content), SocketID1);
    Read(Content, len(Content), SocketID1);
    id2 = Open("Text2.txt", 0);
    Write(Content, len(Content), id2);
    Close(SocketID1);
    Close(id1);
    Close(id2);
    Halt();
}
