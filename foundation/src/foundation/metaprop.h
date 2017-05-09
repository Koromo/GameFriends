#ifndef GAMEFRIENDS_METAPROP_H
#define GAMEFRIENDS_METAPROP_H

#include "prerequest.h"
#include "exception.h"
#include <string>
#include <unordered_map>
#include <vector>

GF_NAMESPACE_BEGIN

class InvalidMetaPropFile : public FileException
{
public:
    explicit InvalidMetaPropFile(const std::string& msg)
        : FileException(msg) {}
};

class MetaProperty
{
private:
    std::string name_;
    mutable std::vector<std::string> values_;

public:
    MetaProperty() = default;
    explicit MetaProperty(const std::string& name);

    void setName(const std::string& name);
    std::string name() const;

    size_t size() const;

    std::string get(size_t i) const;
    int stoi(size_t i) const;
    long stol(size_t i) const;
    unsigned long stoul(size_t i) const;
    long long stoll(size_t i) const;
    unsigned long long stoull(size_t i) const;
    float stof(size_t i) const;
    double stod(size_t i) const;
    long double stold(size_t i) const;

    const std::string& operator [](size_t i) const;
    std::string& operator [](size_t i);

    template <class T>
    void set(size_t i, T val)
    {
        this->operator[](i) = std::to_string(val);
    }

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

    bool has(const std::string& name) const;
    void add(const MetaProperty& prop);
    const MetaProperty& get(const std::string& name) const;

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

    bool has(const std::string& name) const;
    void add(const MetaPropGroup& group);
    const MetaPropGroup& get(const std::string& name) const;

    void read(const std::string& path) noexcept(false);
    void write(const std::string& path) noexcept(false);
};

GF_NAMESPACE_END

#endif
