#include "uri.h"
#include "string.h"

GF_NAMESPACE_BEGIN

bool Uri::isAbsolute(const std::string& path)
{
    if (!path.empty())
    {
        if (path[0] == '/' || path[0] == '\\')
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

std::string Uri::uniform(const std::string& path)
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
    if (isAbsolute(path))
    {
        if (path[0] == '/')
        {
            prefix = "/";
            s.erase(0, 1);
        }
        else
        {
            prefix = s.substr(0, 3);
            s.erase(0, 3);
        }
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

    size_t back = 0;
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
        else if (s.length() - q >= 3 && s[q] == '.' && s[q + 1] == '.' && s[q + 2] == '/')
        {
            ++back;
        }
        else if (back > 0)
        {
            s.erase(q, p - q + 2);
            --back;
        }

        p = q;
    }

    if (s.length() != back * 3 && !s.empty() && s.back() == '/')
    {
        s.pop_back();
    }

    if (!prefix.empty())
    {
        s.erase(0, back * 3);
    }

    return prefix + s;
}

std::string Uri::cat(const std::string& a, const std::string& b)
{
    return uniform(a + '/' + b);
}

std::string Uri::absolutePath(const std::string& absBase, const std::string& relPath)
{
    return uniform(absBase + '/' + relPath);
}

std::string Uri::relativePath(const std::string& absBase, const std::string& absPath)
{
    const auto baseSlash = uniform(absBase) + '/';
    const auto pathSlash = uniform(absPath) + '/';

    int diffAt = 0;
    for (size_t p = 0; p < baseSlash.length() && p < pathSlash.length() &&
        baseSlash[p] == pathSlash[p]; ++p)
    {
        if (baseSlash[p] == '/')
        {
            diffAt = p;
        }
    }

    int back = 0;
    for (size_t p = diffAt + 1; p < baseSlash.length(); ++p)
    {
        back += baseSlash[p] == '/';
    }

    std::string ret;
    for (int i = 0; i < back; ++i)
    {
        ret += "../";
    }
    ret += pathSlash.substr(diffAt + 1);

    return uniform(ret);
}

GF_NAMESPACE_END