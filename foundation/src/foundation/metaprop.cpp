#include "metaprop.h"
#include "exception.h"
#include <queue>
#include <fstream>
#include <cctype>
#include <cstdlib>

GF_NAMESPACE_BEGIN

namespace
{
    struct Token
    {
        std::string token;
        size_t line;
    };

    struct Context
    {
        std::string path;
        std::queue<Token> tokens;
    };

    /*
    std::string forward(Context& context)
    {
        check(!context.tokens.empty());

        const auto poped = context.tokens.front();
        context.tokens.pop();

        return poped.token;
    }
    */

    std::string inText(const std::string& str)
    {
        if (str.length() == 0)
        {
            return "\"\"";
        }

        for (size_t p = 0; p < str.length(); ++p)
        {
            if (std::isspace(str[p]))
            {
                return "\"" + str + "\"";
            }
        }
        return str;
    }

    std::vector<std::string> forwardLine(Context& context)
    {
        check(!context.tokens.empty());

        std::vector<std::string> tokens;

        const auto line = context.tokens.front().line;
        while (!context.tokens.empty() && context.tokens.front().line == line)
        {
            const auto poped = context.tokens.front();
            context.tokens.pop();
            tokens.emplace_back(poped.token);
        }

        return tokens;
    }

    size_t stepByNonspace(size_t p, const std::string& str)
    {
        while (p < str.length() && std::isspace(str[p]))
        {
            ++p;
        }
        return p;
    }

    void tokenizeLine(const std::string& line, size_t currentLine, Context& context)
    {
        for (size_t p = stepByNonspace(0, line); p < line.length(); p = stepByNonspace(p, line))
        {
            if (line[p] == '\"')
            {
                const auto q = line.find('\"', p + 1);
                enforce<InvalidMetaPropFile>(q != std::string::npos, "Unexpected systax at line " + std::to_string(currentLine) + " in " + context.path + ".");
                if (p + 1 == q)
                {
                    context.tokens.emplace(Token{ "", currentLine });
                }
                else
                {
                    context.tokens.emplace(Token{ line.substr(p + 1, q - p - 1), currentLine });
                }
                p = q + 1;
            }
            else
            {
                auto q = p;
                while (q < line.length() && !std::isspace(line[q]))
                {
                    ++q;
                }
                context.tokens.emplace(Token{ line.substr(p, q - p), currentLine });
                p = q + 1;
            }
        }
    }

    void tokenize(std::ifstream& stream, Context& context)
    {
        size_t n = 0;
        while (!stream.eof())
        {
            ++n;
            std::string line;
            std::getline(stream, line);
            line = line.substr(0, line.find("#"));
            tokenizeLine(line, n, context);
        }
    }
}

void MetaPropFile::read(const std::string& path)
{
    *this = MetaPropFile();

    std::ifstream stream(path);
    enforce<FileException>(stream.is_open(), "Failed to open file (" + path + ").");

    Context context = {};
    context.path = path;
    tokenize(stream, context);
    stream.close();

    MetaPropGroup currentGroup;
    bool groupNow = false;

    while (!context.tokens.empty())
    {
        const auto tokens = forwardLine(context);

        if (tokens[0].front() == '@')
        {
            // Add current group
            if (groupNow)
            {
                if (currentGroup.name() == "_Header")
                {
                    auther_ = currentGroup.get("Auther")[0];
                    comment_ = currentGroup.get("Comment")[0];
                }
                else
                {
                    add(currentGroup);
                }
            }

            // Create new current group
            const auto groupName = tokens[0];
            enforce<InvalidMetaPropFile>(groupName.length() > 1, "Invalid group name \"" + groupName + "\" in " + context.path + ".");

            currentGroup = MetaPropGroup();
            currentGroup.setName(groupName.substr(1));
            groupNow = true;
        }
        else
        {
            // New property
            enforce<InvalidMetaPropFile>(tokens[0].back() == ':', "Unexpected systax \"" +  tokens[0] + "\" in " + context.path + ".");

            const auto propName = tokens[0];
            enforce<InvalidMetaPropFile>(propName.length() > 1, "Invalid property name \"" + propName + "\" in " + context.path + ".");

            MetaProperty prop;
            prop.setName(propName.substr(0, propName.length() - 1));

            const auto n = tokens.size() - 1;
            for (size_t i = 0; i < n; ++i)
            {
                prop[i] = tokens[i + 1];
            }

            currentGroup.add(prop);
        }
    }

    // Add current group
    if (groupNow)
    {
        if (currentGroup.name() == "_Header")
        {
            auther_ = currentGroup.get("Auther")[0];
            comment_ = currentGroup.get("Comment")[0];
        }
        else
        {
            add(currentGroup);
        }
    }
}

void MetaPropFile::write(const std::string& path)
{
    std::ofstream stream(path, std::ios_base::out | std::ios_base::trunc);
    enforce<FileException>(stream.is_open(), "Failed open file (" + path + ").");

    stream << "@_Header\nAuther: " + inText(auther_) + "\nComment: " + inText(comment_) + "\n";
    for (const auto& g : groupTable_)
    {
        stream << "\n";
        stream << g.second.asString();
    }

    stream.close();
}

std::string MetaPropGroup::asString() const
{
    std::string asString;
    asString += "@" + inText(name_);
    for (const auto p : propTable_)
    {
        asString += "\n";
        asString += p.second.asString();
    }
    asString += "\n";
    return asString;
}

std::string MetaProperty::asString() const
{
    std::string asString;

    asString += inText(name_) + ":";
    for (const auto& v : values_)
    {
        asString += " ";
        asString += inText(v);
    }
    return asString;
}

MetaProperty::MetaProperty(const std::string& name)
    : name_(name)
    , values_()
{
}

void MetaProperty::setName(const std::string& name)
{
    name_ = name;
}

std::string MetaProperty::name() const
{
    return name_;
}

size_t MetaProperty::size() const
{
    return values_.size();
}

std::string MetaProperty::get(size_t i) const
{
    check(i < values_.size());
    return values_[i];
}

int MetaProperty::stoi(size_t i) const
{
    return std::stoi(get(i));
}

long MetaProperty::stol(size_t i) const
{
    return std::stol(get(i));
}

unsigned long MetaProperty::stoul(size_t i) const
{
    return std::stoul(get(i));
}

long long MetaProperty::stoll(size_t i) const
{
    return std::stoll(get(i));
}

unsigned long long MetaProperty::stoull(size_t i) const
{
    return std::stoull(get(i));
}

float MetaProperty::stof(size_t i) const
{
    return std::stof(get(i));
}

double MetaProperty::stod(size_t i) const
{
    return std::stod(get(i));
}

long double MetaProperty::stold(size_t i) const
{
    return std::stoi(get(i));
}

const std::string& MetaProperty::operator [](size_t i) const
{
    if (i >= values_.size())
    {
        values_.resize(i + 1);
    }
    return values_[i];
}

std::string& MetaProperty::operator [](size_t i)
{
    return const_cast<std::string&>(static_cast<const MetaProperty*>(this)->operator[](i));
}

MetaPropGroup::MetaPropGroup(const std::string& name)
    : name_(name)
    , propTable_()
{
}

void MetaPropGroup::setName(const std::string& name)
{
    name_ = name;
}

std::string MetaPropGroup::name() const
{
    return name_;
}

bool MetaPropGroup::has(const std::string& name) const
{
    return propTable_.find(name) != std::cend(propTable_);
}

void MetaPropGroup::add(const MetaProperty& prop)
{
    propTable_.emplace(prop.name(), prop);
}

const MetaProperty& MetaPropGroup::get(const std::string& name) const
{
    check(has(name));
    return propTable_.find(name)->second;
}

std::string MetaPropFile::auther() const
{
    return auther_;
}

std::string MetaPropFile::comment() const
{
    return comment_;
}

void MetaPropFile::setAuther(const std::string& auther)
{
    auther_ = auther;
}

void MetaPropFile::setComment(const std::string& comment)
{
    comment_ = comment;
}

bool MetaPropFile::has(const std::string& name) const
{
    return groupTable_.find(name) != std::cend(groupTable_);
}

void MetaPropFile::add(const MetaPropGroup& group)
{
    groupTable_[group.name()] = group;
}

const MetaPropGroup& MetaPropFile::get(const std::string& name) const
{
    check(has(name));
    return groupTable_.find(name)->second;
}

GF_NAMESPACE_END