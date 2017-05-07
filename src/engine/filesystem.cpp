#include "filesystem.h"
#include "../windowing/windowsinc.h" // instead of unistd.h
#include "foundation/string.h"
#include "foundation/exception.h"

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
}

void FileSystem::shutdown()
{
}

std::string FileSystem::standard(const std::string& path) const
{
    auto s = tolowers(path);
    for (auto p = s.find("\\"); p != std::string::npos; p = s.find("\\", p + 1))
    {
        s.replace(p, 1, "/");
    }

    const auto isOS = isOSPath(s);
    std::string stdPath;
    int p = 0;

    if (isOS)
    {
        p = 3;
        stdPath = s.substr(0, 3);
    }

    const auto beginOffset = p;
    while (p < static_cast<int>(s.length()))
    {
        if (s[p] == '/')
        {
            p += 1;
        }
        else if (s[p] == '.' && s[p + 1] == '/')
        {
            p += 2;
        }
        else if (s[p] == '.' && s[p + 1] == '.')
        {
            check(s[p + 2] == '/');
            const auto q = stdPath.rfind('/');
            if (q != std::string::npos)
            {
                stdPath.erase(q + 1);
            }
            p += 3;
        }
        else
        {
            auto q = s.find('/', p + 1);
            if (p != beginOffset)
            {
                stdPath += '/';
            }
            if (q == std::string::npos)
            {
                q = s.length();
            }
            stdPath += s.substr(p, q - p);
            p = q + 1;
        }
    }

    if (stdPath.empty())
    {
        stdPath = "./";
    }
    return stdPath;
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

    const auto stdPathSlash = standard(path) + '/';
    const auto osRootPathSlash = osRootPath_ + '/';
    int diffAt = 0;
    for (size_t p = 0; p < stdPathSlash.length() && p < osRootPathSlash.length() &&
        stdPathSlash[p] == osRootPathSlash[p]; ++p)
    {
        if (osRootPathSlash[p] == '/')
        {
            diffAt = p;
        }
    }

    int back = 0;
    for (size_t p = diffAt + 1; p < osRootPathSlash.length(); ++p)
    {
        back += osRootPathSlash[p] == '/';
    }

    std::string ret;
    for (int i = 0; i < back; ++i)
    {
        ret += "../";
    }
    ret += stdPathSlash.substr(diffAt + 1);

    if (ret.empty())
    {
        return "./";
    }
    return ret;
}

bool FileSystem::isOSPath(const std::string& path) const
{
    if (path.length() >= 2)
    {
        if (((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) && path[1] == ':')
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