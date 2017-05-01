#include "console.h"
#include <cstdarg>

GF_NAMESPACE_BEGIN

Console console;

Console::Console()
    : inHandle_(NULL)
    , outHandle_(NULL)
    , buffer_()
{
}

void Console::allocate()
{
    if (!inHandle_)
    {
        enforce<WindowsException>(AllocConsole(), "Failed to allocate the console.");
        inHandle_ = GetStdHandle(STD_INPUT_HANDLE);
        outHandle_ = GetStdHandle(STD_OUTPUT_HANDLE);
    }
}

void Console::free()
{
    FreeConsole();
    inHandle_ = NULL;
    outHandle_ = NULL;
}

string_t Console::readLine()
{
    DWORD numReads;
    const auto success = ReadConsole(inHandle_, buffer_, BUFFER_SIZE - 1, &numReads, NULL);
    /// TODO: numReads - 2, failed case
    buffer_[numReads - 2] = GF_T('\0');
    return buffer_;
}

void Console::write(const char_t* str)
{
    WriteConsole(outHandle_, str, static_cast<DWORD>(strlen(str)), NULL, NULL);
}

void Console::writef(const char_t* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer_, fmt, args);
    write(buffer_);
    va_end(args);
}

void Console::writeln(const char_t* str)
{
    write(str);
    write(GF_T("\n"));
}

void Console::writefln(const char_t* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer_, fmt, args);
    writeln(buffer_);
    va_end(args);
}

GF_NAMESPACE_END