#ifndef GAMEFRIENDS_METAPROP_H
#define GAMEFRIENDS_METAPROP_H

#include "prerequest.h"
#include "exception.h"
#include <string>
#include <unordered_map>
#include <vector>

GF_NAMESPACE_BEGIN

class InvalidMetaPropFile : FileException
{
public:
    explicit InvalidMetaPropFile(const std::string& msg)
        : FileException(msg) {}
};

class MetaProperty
{
private:
    std::string name_;
    mutable std::vector<std::string> tokens_;

public:
    MetaProperty() = default;
    explicit MetaProperty(const std::string& name);

    void setName(const std::string& name);
    std::string name() const;

    const std::string& operator [](size_t i) const;
    std::string& operator [](size_t i);

    template <class T>
    void set(size_t i, T val)
    {
        this->operator[](i) = std::to_string(val);
    }

    int getInt(size_t i) const;
    long getLong(size_t i) const;
    unsigned long getULong(size_t i) const;
    long long getLLong(size_t i) const;
    unsigned long long getULLong(size_t i) const;
    float getFloat(size_t i) const;
    double getDouble(size_t i) const;
    long double getLDouble(size_t i) const;

    size_t size() const;

    std::string asString() const;
};

class MetaPropGroup
{
private:
    std::string name_;
    std::unordered_map<std::string, MetaProperty> propTable_;

public:
    MetaPropGroup() = default;
    explicit MetaPropGroup(const std::string& name);

    void setName(const std::string& name);
    std::string name() const;

    bool hasProp(const std::string& name) const;
    const MetaProperty& prop(const std::string& name) const;
    void addProp(const MetaProperty& prop);

    std::string asString() const;
};

class MetaPropFile
{
private:
    std::string auther_;
    std::string comment_;
    std::unordered_map<std::string, MetaPropGroup> groupTable_;

public:
    std::string auther() const;
    std::string comment() const;

    void setAuther(const std::string& auther);
    void setComment(const std::string& comment);

    bool hasGroup(const std::string& name) const;
    const MetaPropGroup& group(const std::string& name) const;
    void addGroup(const MetaPropGroup& group);

    void read(const std::string& path) noexcept(false);
    void write(const std::string& path) noexcept(false);
};

GF_NAMESPACE_END

#endif
