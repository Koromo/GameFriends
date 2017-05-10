#include "filesystem.h"
#include "logging.h"
#include "../windowing/windowsinc.h"
#include "foundation/string.h"
#include "foundation/exception.h"
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

    engineRootPath_ = engineRoot;
    GF_LOG_INFO("FileSystem initialized. Engine root directory is {}.", engineRootPath_);
}

void FileSystem::shutdown()
{
    GF_LOG_INFO("FileSystem shutdown.");
}

namespace
{
    std::string standardRelative(const std::string& path)
    {
        if (path.empty())
        {
            return "";
        }

        auto s = tolowers(path);
        for (auto p = s.find("\\"); p != std::string::npos; p = s.find("\\", p + 1))
        {
            s.replace(p, 1, "/");
        }
        for (auto p = s.find("//"); p != std::string::npos; p = s.find("//", p))
        {
            s.replace(p, 1, "/");
        }

        auto p = s.rfind('/');
        if (p == std::string::npos)
        {
            p = 0;
        }
        else
        {
            p += 1;
        }

        while (p > 0)
        {
            auto q = s.rfind('/', p - 2);
            if (q == std::string::npos)
            {
                q = 0;
            }
            else
            {
                q += 1;
            }

            if (s[q] == '.' && s[q + 1] == '/')
            {
                s.erase(q, 2);
            }
            else if (s.length() - q >= 3 && s[p] == '.' && s[p + 1] == '.' && s[p + 2] == '/')
            {
            }
            else if (s.length() - p >= 3 && s[p] == '.' && s[p + 1] == '.' && s[p + 2] == '/')
            {
                s.erase(q, p - q);
            }

            p = q;
        }

        return s;
    }
}

EnginePath FileSystem::standard(const EnginePath& path) const
{
    return EnginePath{ standardRelative(path.s) };
}

std::string FileSystem::toOSPath(const EnginePath& path) const
{
    return engineRootPath_ + '/' + path.s;
}

FileSystem fileSystem;

GF_NAMESPACE_END