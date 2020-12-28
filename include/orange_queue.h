#ifndef __ORANGE_QUEUE_H__
#define __ORANGE_QUEUE_H__

#include "orange_deque.h"
#include "orange_vector.h"
#include "orange_functional.h"
#include "orange_heap_algo.h"

namespace orange_stl
{

/* 模板类queue */
template <class T, class Container = orange_stl::deque<T>>
class queue
{
public:
    typedef Container                           container_type;
    // 使用底层容器的型别
    typedef typename Container::value_type      value_type;
    typedef typename Container::size_type       size_type;
    typedef typename Container::reference       reference;
    typedef typename Container::const_reference const_reference;

    static_assert(std::is_same<T, value_type>::value, "the value_type of Container should be same with T");

private:
    Container c_;

public:
    queue() = default;

    explicit queue(size_type n) : c_(n)
    { }

    queue(size_type n, const value_type& value) : c_(n, value)
    { }

    template <class IIter>
    queue(IIter first, IIter last) : c_(first, last)
    { }

    queue(std::initializer_list<T> ilist) : c_(ilist.begin(), ilist.end())
    { }

    queue(const Container& c) : c_(c)
    { }

    queue(Container&& c) noexcept(std::is_nothrow_move_constructible<Container>::value)
        :c_(orange_stl::move(c)) 
    { }

    queue(const queue& rhs) : c_(rhs.c_) 
    { }

    queue(queue&& rhs) noexcept(std::is_nothrow_move_constructible<Container>::value)
        :c_(orange_stl::move(rhs.c_)) 
    { }

    queue& operator=(const queue& rhs)
    {
        c_=rhs.c_;
        return *this;
    }

    queue& operator=(queue&& rhs) noexcept(std::is_nothrow_move_assignable<Container>::value)
    { 
        c_ = orange_stl::move(rhs.c_);
        return *this;
    }

    queue& operator=(std::initializer_list<T> ilist)
    { 
        c_ = ilist; 
        return *this; 
    }

    ~queue() = default;

    
    /* 访问元素的相关操作 */
    reference       front()       { return c_.front(); }
    const_reference front() const { return c_.front(); }
    reference       back()        { return c_.back(); }
    const_reference back()  const { return c_.back(); }

    /* size */
    bool empty() const noexcept { return c_.empty(); }
    size_type size() const noexcept { return c_.size(); }

    template <class ...Args>
    void emplace(Args&& ...args)
    { 
        c_.emplace_back(orange_stl::forward<Args>(args)...); 
    }

    void  push(const value_type& value)
    {
        c_.push_back(value);
    }

    void push(value_type&& value)
    {
        c_.emplace_back(orange_stl::move(value));
    }

    void pop()
    {
        c_.pop_front();
    }

    void clear()
    {
        while(!empty())
            pop();
    }

    void swap(queue &rhs) noexcept(noexcept(orange_stl::swap(c_, rhs.c_)))
    {
        orange_stl::swap(c_, rhs.c_);
    }

public:
    friend bool operator==(const queue& lhs, const queue& rhs)
    {
        return lhs.c_ == rhs.c_;
    }
    friend bool operator<(const queue& lhs, const queue& rhs)
    {
        return lhs.c_<rhs.c_;
    }
};

// 重载比较操作符
template <class T, class Container>
bool operator==(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
    return lhs == rhs;
}

template <class T, class Container>
bool operator!=(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
    return !(lhs == rhs);
}

template <class T, class Container>
bool operator<(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
    return lhs < rhs;
}

template <class T, class Container>
bool operator>(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
    return rhs < lhs;
}

template <class T, class Container>
bool operator<=(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
    return !(rhs < lhs);
}

template <class T, class Container>
bool operator>=(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
    return !(lhs < rhs);
}

// 重载 orange_stl 的 swap
template <class T, class Container>
void swap(queue<T, Container>& lhs, queue<T, Container>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}



/* ****************************************************************************************** */
/* priority_queue */
template <class T, class Container = orange_stl::vector<T>, 
          class Compare=orange_stl::less<typename Container::value_type>>
class priority_queue
{
public:
    typedef Container                           container_type;
    typedef Compare                             value_compare;
    typedef typename Container::value_type      value_type;
    typedef typename Container::size_type       size_type;
    typedef typename Container::reference       reference;
    typedef typename Container::const_reference const_reference;

    static_assert(std::is_same<T, value_type>::value, "the value_type of Container should be same with T");

private:
    container_type  c_;
    value_compare   comp_;

public:
    priority_queue() = default;

    priority_queue(const Compare& c) : c_(), comp(c)
    { }

    explicit priority_queue(size_type n) : c_(n)
    {
        orange_stl::make_heap(c_.begin(), c_.end(), comp_);
    }

    priority_queue(size_type n, const value_type& value) : c_(n, value)
    {
        orange_stl::make_heap(c_.begin(), c_.end(), comp_);
    }

    template <class IIter>
    priority_queue(IIter first, IIter last) : c_(first, last)
    {
        orange_stl::make_heap(c_.begin(), c_.end(), comp_);
    }

    priority_queue(std::initializer_list<T> ilist) : c_(ilist)
    {
        orange_stl::make_heap(c_.begin(), c_.end(), comp_);
    }

    priority_queue(const Container& s) : c_(s)
    {
        orange_stl::make_heap(c_.begin(), c_.end(), comp_);
    }

    priority_queue(Container&& s) : c_(orange_stl::move(s))
    {
        orange_stl::make_heap(c_.begin(), c_.end(), comp_);
    }

    priority_queue(const priority_queue& rhs) : c_(rhs.c_), comp_(rhs.comp_)
    {
        orange_stl::make_heap(c_.begin(), c_.end(), comp_);
    }

    priority_queue(priority_queue&& rhs) : c_(orange_stl::move(rhs.c_)), comp_(rhs.comp_)
    {
        orange_stl::make_heap(c_.begin(), c_.end(), comp_);
    }

    priority_queue& operator=(const priority_queue& rhs)
    {
        c_ = rhs.c_;
        comp_ = rhs.comp_;
        orange_stl::make_heap(c_.begin(), c_.end(), comp_);
        return *this;
    }
    priority_queue& operator=(priority_queue&& rhs)
    {
        c_ = mystl::move(rhs.c_);
        comp_ = rhs.comp_;
        orange_stl::make_heap(c_.begin(), c_.end(), comp_);
        return *this;
    }
    priority_queue& operator=(std::initializer_list<T> ilist)
    {
        c_ = ilist;
        comp_ = value_compare();
        orange_stl::make_heap(c_.begin(), c_.end(), comp_);
        return *this;
    }

    ~priority_queue() = default;
public:
    // 访问元素相关操作
    const_reference top() const 
    { 
        return c_.front(); 
    }

    // 容量相关操作
    bool      empty() const noexcept { return c_.empty(); }
    size_type size()  const noexcept { return c_.size(); }

    // 修改容器相关操作
    template <class... Args>
    void emplace(Args&& ...args)
    {
        c_.emplace_back(orange_stl::forward<Args>(args)...);
        orange_stl::push_heap(c_.begin(), c_.end(), comp_);
    }

    void push(const value_type& value)
    {
        c_.push_back(value);
        orange_stl::push_heap(c_.begin(), c_.end(), comp_);
    }
    
    void push(value_type&& value)
    {
        c_.push_back(orange_stl::move(value));
        orange_stl::push_heap(c_.begin(), c_.end(), comp_);
    }

    void pop()
    {
        orange_stl::pop_heap(c_.begin(), c_.end(), comp_);/* 将元素放到了末尾 */
        c_.pop_back();/* 弹出末尾元素 */
    }

    void clear()
    {
        while (!empty())
            pop();
    }

    void swap(priority_queue& rhs) noexcept(noexcept(orange_stl::swap(c_, rhs.c_)) && 
                                            noexcept(orange_stl::swap(comp_, rhs.comp_)))
    {
        orange_stl::swap(c_, rhs.c_);
        orange_stl::swap(comp_, rhs.comp_);
    }

public:
    friend bool operator==(const priority_queue& lhs, const priority_queue& rhs)
    {
        return lhs.c_ == rhs.c_;
    }
    friend bool operator!=(const priority_queue& lhs, const priority_queue& rhs)
    {
        return lhs.c_ != rhs.c_;
    }
};
// 重载比较操作符
template <class T, class Container, class Compare>
bool operator==(priority_queue<T, Container, Compare>& lhs,
                priority_queue<T, Container, Compare>& rhs)
{
    return lhs == rhs;
}

template <class T, class Container, class Compare>
bool operator!=(priority_queue<T, Container, Compare>& lhs,
                priority_queue<T, Container, Compare>& rhs)
{
    return lhs != rhs;
}

// 重载 orange_stl 的 swap
template <class T, class Container, class Compare>
void swap(priority_queue<T, Container, Compare>& lhs, 
          priority_queue<T, Container, Compare>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}

} // namespace orange_stl

#endif // !__ORANGE_QUEUE_H__