#include "filesystem.h"
#include "../windowing/windowsinc.h" // instead of unistd.h
#include "foundation/string.h"
#include "foundation/exception.h"
#include <Shlwapi.h>

GF_NAMESPACE_BEGIN

bool operator ==(const FilePath& a, const FilePath& b)
{
    return a.os == b.os;
}

bool operator !=(const FilePath& a, const FilePath& b)
{
    return !(a == b);
}

bool operator <(const FilePath& a, const FilePath& b)
{
    return a.os < b.os;
}

bool operator <=(const FilePath& a, const FilePath& b)
{
    return a.os <= b.os;
}

bool operator >(const FilePath& a, const FilePath& b)
{
    return a.os > b.os;
}

bool operator >=(const FilePath& a, const FilePath& b)
{
    return a.os >= b.os;
}

void FileSystem::startup(const std::string& rootDirectory)
{
    if (!PathIsDirectoryA(rootDirectory.c_str()))
    {
        /// LOG
        throw FileSystemError("The required root directory (" + rootDirectory + ") is not exists.");
    }

    if (isOSPath(rootDirectory))
    {
        osRootPath_ = standard(rootDirectory);
    }
    else
    {
        char_t buf[256];
        GetCurrentDirectory(256, buf);
        osRootPath_ = standard(narrow(buf) + '/' + rootDirectory);
    }

    /// LOG
}

void FileSystem::shutdown()
{
    /// LOG
}

std::string FileSystem::standard(const std::string& path) const
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

    std::string prefix;
    if (isOSPath(s))
    {
        if (s[0] == '/')
        {
            prefix = s[0];
            s.erase(0, 1);
        }
        else
        {
            prefix = s.substr(0, 3);
            s.erase(0, 3);
        }
    }

    if (s.back() == '/')
    {
        s.pop_back();
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

        if (s[q] == '.' && s[q + 1] == '/')
        {
            s.erase(q, 2);
        }
        else if (s.length() - p >= 3 && s[p] == '.' && s[p + 1] == '.' && s[p + 2] == '/')
        {
            s.erase(q, p - q);
        }
        p = q;
    }

    if (prefix.length() + s.length() == 0)
    {
        s = "./";
    }

    return prefix + s;
}

std::string FileSystem::toOSPath(const std::string& path) const
{
    if (isOSPath(path))
    {
        return standard(path);
    }
    return standard(osRootPath_ + '/' + path);
}

std::string FileSystem::toRelativePath(const std::string& path) const
{
    if (!isOSPath(path))
    {
        return standard(path);
    }
    if (tolower(path[0]) != osRootPath_[0])
    {
        return standard(path);
    }
    if (path == osRootPath_)
    {
        return "./";
    }

    const auto pathSlash = standard(path) + '/';
    const auto rootPathSlash = osRootPath_ + '/';

    int diffAt = 0;
    for (size_t p = 0; p < pathSlash.length() && p < rootPathSlash.length() &&
        pathSlash[p] == rootPathSlash[p]; ++p)
    {
        if (rootPathSlash[p] == '/')
        {
            diffAt = p;
        }
    }

    int back = 0;
    for (size_t p = diffAt + 1; p < rootPathSlash.length(); ++p)
    {
        back += rootPathSlash[p] == '/';
    }

    std::string ret;
    for (int i = 0; i < back; ++i)
    {
        ret += "../";
    }
    ret += pathSlash.substr(diffAt + 1);

    return ret;
}

bool FileSystem::isOSPath(const std::string& path) const
{
    if (!path.empty())
    {
        if (path[0] == '/')
        {
            return true;
        }
        if (path.length() >= 2 && tolower(path[0]) >= 'a' && tolower(path[0]) <= 'z' && path[1] == ':')
        {
            return true;
        }
    }
    return false;
}

FilePath FileSystem::path(const std::string& path) const
{
    FilePath filePath;
    filePath.relative = toRelativePath(path);
    filePath.os = toOSPath(path);
    return filePath;
}

FileSystem fileSystem;

GF_NAMESPACE_END