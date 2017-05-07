#ifndef GAMEFRIENDS_ANY_H
#define GAMEFRIENDS_ANY_H

#include "exception.h"
#include "prerequest.h"
#include <utility>
#include <type_traits>
#include <memory>
#include <typeinfo>
#include <string>

GF_NAMESPACE_BEGIN

class AnyCastError : public Exception
{
public:
    explicit AnyCastError(const std::string& msg = "")
        : Exception(msg) {}
};

class Any
{
private:
    struct Holder
    {
        virtual ~Holder() noexcept = default;
        virtual Holder* copy() const = 0;
        virtual const std::type_info& type() const noexcept = 0;
    };

    template <class T>
    struct TypedHolder : Holder
    {
        T value;

        template <class U>
        TypedHolder(U&& val) : value(std::forward<U>(val)) {}
        Holder* copy() const { return new TypedHolder(value); }
        const std::type_info& type() const noexcept { return typeid(value); }
    };

    template <class T>
    using TypedHolder_t = TypedHolder<std::decay_t<T>>;

    Holder* payload_;

public:
    Any()
        : payload_(nullptr)
    {
    }

    Any(const Any& that)
        : payload_(that.payload_ ? that.payload_->copy() : nullptr)
    {
    }

    Any(Any&& that)
        : payload_(that.payload_)
    {
        that.payload_ = nullptr;
    }

    template <class T>
    Any(T&& value)
        : payload_(new TypedHolder_t<T>(std::forward<T>(value)))
    {
    }

    ~Any()
    {
        delete payload_;
    }

    void swap(Any& that)
    {
        std::swap(payload_, that.payload_);
    }

    Any& operator =(const Any& that)
    {
        Any(that).swap(*this);
        return *this;
    }

    Any& operator =(Any&& that)
    {
        that.swap(*this);
        return *this;
    }

    template <class T>
    Any& operator =(T&& value)
    {
        Any(std::forward<T>(value)).swap(*this);
        return *this;
    }

    bool hasValue() const
    {
        return !!payload_;
    }

    const type_info& type() const
    {
        if (!payload_)
        {
            return typeid(void);
        }
        return payload_->type();
    }

    void reset()
    {
        Any().swap(*this);
    }

    template <class T>
    T* cast()
    {
        return hasValue() && type() == typeid(std::decay_t<T>) ? /// TODO: remove_cv?
            std::addressof(static_cast<TypedHolder_t<T>*>(payload_)->value) :
            nullptr;
    }

    template <class T>
    const T* cast() const
    {
        return const_cast<Any&>(*this).template cast<T>();
    }
};

template <class T>
T* to(Any* any)
{
    return any->template cast<T>();
}

template <class T>
const T* to(const Any* any)
{
    return any->template cast<T>();
}

template <class T>
T& to(Any& any) noexcept(false)
{
    auto p = enforce<AnyCastError>(to<T>(&any));
    return *p;
}

template <class T>
const T& to(const Any& any) noexcept(false)
{
    return to<T>(const_cast<Any&>(any));
}

GF_NAMESPACE_END

namespace std
{
    template <>
    inline void swap(GF_NAMESPACE::Any& a, GF_NAMESPACE::Any& b)
    {
        a.swap(b);
    }
}

#endif