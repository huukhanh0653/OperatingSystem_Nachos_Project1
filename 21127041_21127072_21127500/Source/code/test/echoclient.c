#include "syscall.h"
#include "Utilities.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main()
{
    char *Content;
    int SocketID1, result;
    Write("Hello\n", 7, 1);
    Content = "Hello Socket!\n";
    SocketID1 = SocketTCP();
    result = Connect(SocketID1, "127.0.0.1", 8081);
    Write(Content, len(Content), SocketID1);
    Read(Content, len(Content), SocketID1);
    Write(Content, len(Content), CONSOLE_OUTPUT);
    Close(SocketID1);
    Halt();
}