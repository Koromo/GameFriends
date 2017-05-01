#ifndef GAMEFRIENDS_SORTEDVECTOR_H
#define GAMEFRIENDS_SORTEDVECTOR_H

#include "prerequest.h"
#include <vector>
#include <initializer_list>
#include <algorithm>
#include <utility>

GF_NAMESPACE_BEGIN

template <class T, class Comp = std::less<T>, class Alloc = std::allocator<T>>
class SortedVector
{
public:
    using VectorType = std::vector<T, Alloc>;
    using Iterator = typename VectorType::iterator;
    using CIterator = typename VectorType::const_iterator;
    using RIterator = typename VectorType::reverse_iterator;
    using CRIterator = typename VectorType::const_reverse_iterator;

private:
    VectorType vec_;
    Comp comp_;

public:
    explicit SortedVector(const Comp& comp = Comp(), const Alloc& alloc = Alloc())
        : vec_(alloc)
        , comp_(comp)
    {
    }

    explicit SortedVector(const Alloc& alloc)
        : SortedVector(Comp(), alloc)
    {
    }

    SortedVector(size_t n, const T& val = T(), const Comp& comp = Comp(), const Alloc& alloc = Alloc())
        : vec_(n, val, alloc)
        , comp_(comp)
    {
        std::sort(std::cbegin(vec_), std::cend(vec_), comp_);
    }

    SortedVector(size_t n, const T& val = T(), const Alloc& alloc = Alloc())
        : SortedVector(n, val, Comp(), alloc)
    {
    }

    template <class InputIterator>
    SortedVector(InputIterator first, InputIterator last, const Comp& comp = Comp(), const Alloc& alloc = Alloc())
        : vec_(first, last, alloc)
        , comp_(comp)
    {
        std::sort(std::cbegin(vec_), std::cend(vec_), comp_);
    }

    template <class InputIterator>
    SortedVector(InputIterator first, InputIterator last, const Alloc& alloc = Alloc())
        : SortedVector(first, last, Comp(), alloc)
    {
    }

    SortedVector(const SortedVector& x) = default;

    SortedVector(const SortedVector& x, const Alloc& alloc)
        : vec_(x.vec_, alloc)
        , comp_(x.comp_)
    {
    }

    SortedVector(SortedVector&& x) = default;

    SortedVector(SortedVector&& x, const Alloc& alloc)
        : vec_(std::move(x.vec_), alloc)
        , comp_(std::move(x.comp_))
    {
    }

    SortedVector(std::initializer_list<T> il, const Comp& comp = Comp(), const Alloc& alloc = Alloc())
        : vec_(il, alloc)
        , comp_(comp)
    {
        std::sort(std::cbegin(vec_), std::cend(vec_), comp_);
    }

    SortedVector(std::initializer_list<T> il, const Alloc& alloc = Alloc())
        : SortedVector(il, Comp(), alloc)
    {
    }

    ~SortedVector() = default;

    SortedVector& operator= (const SortedVector& x) = default;
    SortedVector& operator= (SortedVector&& x) = default;
    SortedVector& operator= (std::initializer_list<T> il)
    {
        SortedVector(il).swap(*this);
    }

    Iterator begin()    noexcept { return vec_.begin(); }
    Iterator end()      noexcept { return vec_.end(); }
    CIterator begin()   const noexcept { return vec_.begin(); }
    CIterator end()     const noexcept { return vec_.end(); }

    RIterator rbegin()  noexcept { return vec_.rbegin(); }
    RIterator rend()    noexcept { return vec_.rend(); }
    CRIterator rbegin() const noexcept { return vec_.rbegin(); }
    CRIterator rend()   const noexcept { return vec_.rend(); }

    CIterator cbegin()      const noexcept { return vec_.cbegin(); }
    CIterator cend()        const noexcept { return vec_.cend(); }
    CRIterator crbegin()    const noexcept { return vec_.crbegin(); }
    CRIterator crend()      const noexcept { return vec_.crend(); }

    size_t size()       const noexcept { return vec_.size(); }
    size_t maxSize()    const noexcept { return vec_.max_size(); }
    size_t capacity()   const noexcept { return vec_.capacity(); }
    bool empty()        const noexcept { return vec_.empty(); }
    void reserve(size_t n)  { vec_.reserve(n); }
    void shrink()           { vec_.shrink_to_fit(); }

    T& operator[](size_t n)         { return vec_[n]; }
    const T& operator[](size_t n)   const { return vec_[n]; }
    T& at(size_t n)                 { return vec_.at(n); }
    const T& at(size_t n)           const { return vec_.at(n); }
    T& front()          { return vec_.front(); }
    const T& front()    const { return vec_.front(); }
    T& back()           { return vec_.back(); }
    const T& back()     const { return vec_.back(); }
    T* data()           noexcept { return vec_.data(); }
    const T* data()     const noexcept { return vec_.data(); }

    void assign(size_t n, const T& val)
    {
        SortedVector(n, val).swap(*this);
    }

    template <class InputIterator>
    void assign(InputIterator first, InputIterator last)
    {
        SortedVector(first, last).swap(*this);
    }

    void assign(std::initializer_list<T> il)
    {
        SortedVector(il).swap(*this);
    }

    Iterator insert(const T& val)           { return vec_.insert(std::upper_bound(std::cbegin(vec_), std::cend(vec_), val, comp_), val); }
    Iterator insert(T&& val)                { return vec_.insert(std::upper_bound(std::cbegin(vec_), std::cend(vec_), val, comp_), std::move(val)); }
    Iterator insert(size_t n, const T& val) { return vec_.insert(std::upper_bound(std::cbegin(vec_), std::cend(vec_), val, comp_), n, val); }

    template <class InputIterator>
    void insert(InputIterator first, InputIterator last)
    {
        vec_.reserve(vec_.size() + (last - first));
        while (first != last)
        {
            insert(*first);
            ++first;
        }
        std::sort(std::cbegin(vec_), std::cend(vec_), comp_);
    }

    void insert(std::initializer_list<T> il) { insert(std::cbegin(il), std::cend(il)); }

    Iterator erase(CIterator pos)                   { return vec_.erase(pos); }
    Iterator erase(CIterator first, CIterator last) { return vec_.erase(first, last); }

    void swap(SortedVector& x) { vec_.swap(x.vec_); std::swap(comp_, x.comp_); }
    void clear() noexcept { vec_.clear(); }

    Comp comp() const { return comp_; }
    Alloc allocator() const noexcept { return vec_.get_allocator(); }
};

GF_NAMESPACE_END

namespace std
{
    template <class T, class C, class A>
    void swap(GF_NAMESPACE::SortedVector<T, C, A>& a, GF_NAMESPACE::SortedVector<T, C, A>& b)
    {
        a.swap(b);
    }
}

#endif
