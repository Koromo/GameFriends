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
                check(q != std::string::npos);
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
    enforce<FileException>(stream.is_open(), "File not found (" + path + ").");

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
                    auther_ = currentGroup.prop("Auther")[0];
                    comment_ = currentGroup.prop("Comment")[0];
                }
                else
                {
                    addGroup(currentGroup);
                }
            }

            // Create new current group
            const auto groupName = tokens[0];
            currentGroup = MetaPropGroup();
            currentGroup.setName(groupName.substr(1));
            groupNow = true;
        }
        else
        {
            // New property
            check(tokens[0].back() == ':');

            const auto propName = tokens[0];
            MetaProperty prop;
            prop.setName(propName.substr(0, propName.length() - 1));

            const auto n = tokens.size() - 1;
            if (n == 0)
            {
                prop[0] == "";
            }
            else
            {
                for (size_t i = 0; i < n; ++i)
                {
                    prop[i] = tokens[i + 1];
                }
            }

            currentGroup.addProp(prop);
        }
    }

    // Add current group
    if (groupNow)
    {
        if (currentGroup.name() == "_Header")
        {
            auther_ = currentGroup.prop("Auther")[0];
            comment_ = currentGroup.prop("Comment")[0];
        }
        else
        {
            addGroup(currentGroup);
        }
    }
}

void MetaPropFile::write(const std::string& path)
{
    std::ofstream stream(path, std::ios_base::out | std::ios_base::trunc);
    enforce<FileException>(stream.is_open(), "File open error (" + path + ").");

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
    for (const auto& t : tokens_)
    {
        asString += " ";
        asString += inText(t);
    }
    return asString;
}

MetaProperty::MetaProperty(const std::string& name)
    : name_(name)
    , tokens_()
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

const std::string& MetaProperty::operator [](size_t i) const
{
    if (i >= tokens_.size())
    {
        tokens_.resize(i + 1);
    }
    return tokens_[i];
}

std::string& MetaProperty::operator [](size_t i)
{
    return const_cast<std::string&>(static_cast<const MetaProperty*>(this)->operator[](i));
}

int MetaProperty::getInt(size_t i) const
{
    return std::stoi(this->operator[](i));
}

long MetaProperty::getLong(size_t i) const
{
    return std::stol(this->operator[](i));
}

unsigned long MetaProperty::getULong(size_t i) const
{
    return std::stoul(this->operator[](i));
}

long long MetaProperty::getLLong(size_t i) const
{
    return std::stoll(this->operator[](i));
}

unsigned long long MetaProperty::getULLong(size_t i) const
{
    return std::stoull(this->operator[](i));
}

float MetaProperty::getFloat(size_t i) const
{
    return std::stof(this->operator[](i));
}

double MetaProperty::getDouble(size_t i) const
{
    return std::stod(this->operator[](i));
}

long double MetaProperty::getLDouble(size_t i) const
{
    return std::stoi(this->operator[](i));
}


size_t MetaProperty::size() const
{
    return tokens_.size();
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

bool MetaPropGroup::hasProp(const std::string& name) const
{
    return propTable_.find(name) != std::cend(propTable_);
}

const MetaProperty& MetaPropGroup::prop(const std::string& name) const
{
    check(hasProp(name));
    return propTable_.find(name)->second;
}

void MetaPropGroup::addProp(const MetaProperty& prop)
{
    check(!prop.name().empty());
    propTable_.emplace(prop.name(), prop);
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

bool MetaPropFile::hasGroup(const std::string& name) const
{
    return groupTable_.find(name) != std::cend(groupTable_);
}

const MetaPropGroup& MetaPropFile::group(const std::string& name) const
{
    check(hasGroup(name));
    return groupTable_.find(name)->second;
}

void MetaPropFile::addGroup(const MetaPropGroup& group)
{
    check(!group.name().empty());
    groupTable_[group.name()] = group;
}

GF_NAMESPACE_END