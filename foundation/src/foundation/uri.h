#ifndef GAMEFRIENDS_URI_H
#define GAMEFRIENDS_URI_H

#include "prerequest.h"
#include <string>

GF_NAMESPACE_BEGIN

struct Uri
{
    static bool isAbsolute(const std::string& path);
    static std::string uniform(const std::string& path);
    static std::string cat(const std::string& a, const std::string& b);
    static std::string absolutePath(const std::string& absBase, const std::string& relPath);
    static std::string relativePath(const std::string& absBase, const std::string& absPath);
};

GF_NAMESPACE_END

#endif
