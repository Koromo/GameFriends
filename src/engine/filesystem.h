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

struct EnginePath
{
    std::string s;

    EnginePath() = default;
    explicit EnginePath(const std::string& p) : s(p) {};
};

bool operator ==(const EnginePath& a, const EnginePath& b);
bool operator !=(const EnginePath& a, const EnginePath& b);
bool operator <(const EnginePath& a, const EnginePath& b);
bool operator <=(const EnginePath& a, const EnginePath& b);
bool operator >(const EnginePath& a, const EnginePath& b);
bool operator >=(const EnginePath& a, const EnginePath& b);

class FileSystem
{
private:
    std::string engineRootPath_;

public:
    void startup(const std::string& engineRoot) noexcept(false);
    void shutdown();

    EnginePath standard(const EnginePath& path) const;
    std::string toOSPath(const EnginePath& path) const;
};

extern FileSystem fileSystem;

GF_NAMESPACE_END

namespace std
{
    template <>
    struct hash<GF_NAMESPACE::EnginePath>
    {
        size_t operator ()(const GF_NAMESPACE::EnginePath& p) const
        {
            return std::hash<std::string>()(p.s);
        }
    };
}

#endif
