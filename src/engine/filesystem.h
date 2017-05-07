#ifndef GAMEFRIENDS_FILESYSTEM_H
#define GAMEFRIENDS_FILESYSTEM_H

#include "foundation/exception.h"
#include "foundation/prerequest.h"
#include <string>

GF_NAMESPACE_BEGIN

class FileSystemError : public Error
{
public:
    explicit FileSystemError(const std::string& msg)
        : Error(msg) {}
};

struct FilePath
{
    std::string relative;
    std::string os;
};

bool operator ==(const FilePath& a, const FilePath& b);
bool operator !=(const FilePath& a, const FilePath& b);
bool operator <(const FilePath& a, const FilePath& b);
bool operator <=(const FilePath& a, const FilePath& b);
bool operator >(const FilePath& a, const FilePath& b);
bool operator >=(const FilePath& a, const FilePath& b);

class FileSystem
{
private:
    std::string osRootPath_;

public:
    void startup(const std::string& rootDirectory) noexcept(false);
    void shutdown();

    bool isOSPath(const std::string& path) const;
    std::string standard(const std::string& path) const;
    std::string toOSPath(const std::string& path) const;
    std::string toRelativePath(const std::string& path) const;
    FilePath path(const std::string& path) const;
};

extern FileSystem fileSystem;

GF_NAMESPACE_END

namespace std
{
    template <>
    struct hash<GF_NAMESPACE::FilePath>
    {
        size_t operator ()(const GF_NAMESPACE::FilePath& p) const
        {
            return std::hash<std::string>()(p.os);
        }
    };
}

#endif
