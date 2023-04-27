#include "syscall.h"
#include "Utilities.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main() {
    int id1, id2;
    char buffer[256];
    char *Content;

    id1 = Open("hihi.txt", 0);
    id2 = Open("haha.txt", 0);
    Content = Read(buffer,256, id1);
    Write(Content, len(Content), id2);
    Close(id1);
    Close(id2);
    Halt();
}