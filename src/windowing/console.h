#ifndef GAMEFRIENDS_CONSOLE_H
#define GAMEFRIENDS_CONSOLE_H

#include "windowsinc.h"
#include "../foundation/string.h"
#include "../foundation/prerequest.h"

GF_NAMESPACE_BEGIN

class Console
{
private:
    static const size_t BUFFER_SIZE = 2014;
    HANDLE inHandle_;
    HANDLE outHandle_;
    char_t buffer_[BUFFER_SIZE];

public:
    Console();

    void allocate();
    void free();

    string_t readLine();

    void write(const char_t* str);
    void writef(const char_t* fmt, ...);
    void writeln(const char_t* str);
    void writefln(const char_t* fmt, ...);
};

extern Console console;

GF_NAMESPACE_END

#endif
