#include "filesystem.h"
#include "logging.h"
#include "../windowing/windowsinc.h"
#include "foundation/exception.h"
#include "foundation/uri.h"
#include <Shlwapi.h>

GF_NAMESPACE_BEGIN

bool operator ==(const EnginePath& a, const EnginePath& b)
{
    return a.s == b.s;
}

bool operator !=(const EnginePath& a, const EnginePath& b)
{
    return !(a == b);
}

bool operator <(const EnginePath& a, const EnginePath& b)
{
    return a.s < b.s;
}

bool operator <=(const EnginePath& a, const EnginePath& b)
{
    return a.s <= b.s;
}

bool operator >(const EnginePath& a, const EnginePath& b)
{
    return a.s > b.s;
}

bool operator >=(const EnginePath& a, const EnginePath& b)
{
    return a.s >= b.s;
}

void FileSystem::startup(const std::string& engineRoot)
{
    if (!PathIsDirectoryA(engineRoot.c_str()))
    {
        GF_LOG_ERROR("FileSystem initialization error. {} is invalid relative path or directory not exists.", engineRoot);
        throw FileSystemError("The required root directory (" + engineRoot + ") is not exists.");
    }

    char cur[512];
    GetCurrentDirectoryA(512, cur);
    osCurDir_ = Uri::uniform(cur);

    if (Uri::isAbsolute(engineRoot))
    {
        engineRoot_ = Uri::uniform(engineRoot);
    }
    else
    {
        engineRoot_ = Uri::absolutePath(cur, engineRoot);
    }

    GF_LOG_INFO("FileSystem initialized. Engine root directory is {}.", engineRoot_);
}

void FileSystem::shutdown()
{
    GF_LOG_INFO("FileSystem shutdown.");
}

EnginePath FileSystem::uniform(const EnginePath& path) const
{
    return EnginePath{ Uri::uniform(path.s) };
}

std::string FileSystem::toOSPath(const EnginePath& path) const
{
    return Uri::relativePath(osCurDir_, engineRoot_ + '/' + path.s);
}

FileSystem fileSystem;

GF_NAMESPACE_END