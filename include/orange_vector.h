#ifndef __ORANGE_VECTOR_H__
#define __ORANGE_VECTOR_H__

#include <initializer_list>
#include "orange_iterator.h"
#include "orange_memory.h"
#include "orange_util.h"
#include "orange_exceptdef.h"

namespace orange_stl
{

#ifdef max
#pragma message("#undefing marco max")
#undef max
#endif

#ifdef min
#pragma message("#undefing marco min")
#undef min
#endif

/* vector模板类 */
template <class T>
class vector
{
    static_assert(!std::is_same<bool, T>::value, "vector<bool> is abandoned in orange_stl");
public:
    /* vector 型别定义 */
    typedef orange_stl::allocator<T> allocator_type;
    typedef orange_stl::allocator<T> data_allocator;

    typedef typename allocator_type::value_type             value_type;
    typedef typename allocator_type::pointer                pointer;
    typedef typename allocator_type::const_pointer          const_pointer;
    typedef typename allocator_type::reference              reference;
    typedef typename allocator_type::const_reference        const_reference;
    typedef typename allocator_type::size_type              size_type;
    typedef typename allocator_type::difference_type        difference_type;

    typedef value_type*                                     iterator;
    typedef const value_type*                               const_iterator;
    typedef orange_stl::reverse_iterator<iterator>          reverse_iterator;
    typedef orange_stl::reverse_iterator<const_iterator>    const_reverse_iterator;

    allocator_type get_allocator()  { return data_allocator(); }

private:
    iterator begin_;
    iterator end_;
    iterator cap_;

public:
    /* 构造，复制，移动，析构函数 */
    vector() noexcept { try_init(); }
    explicit vector(size_type n) { fill_init(n, value_type()); }
    vector(size_type n, const value_type& value) { fill_init(n, value); }
    
    template <class Iter, typename std::enable_if<orange_stl::is_input_iterator<Iter>::value, int>::type = 0>
    vector(Iter first, Iter last)
    {
        ORANGE_STL_DEBUG(!(last<first));
        range_init(first, last);
    }

    vector(const vector& rhs) { range_init(rhs.begin_, rhs.end_); }

    vector(vector&& rhs) noexcept : begin_(rhs.begin_), end_(rhs.end_), cap_(rhs.cap_)
    {
        rhs.begin_=nullptr;
        rhs.end_=nullptr;
        rhs.cap_=nullptr;
    }

    vector(std::initializer_list<value_type> ilist) 
    {
        range_init(ilist.begin(), ilist.end());
    }

    vector& operator=(const vector& rhs);
    vector& operator=(vector&& rhs) noexcept;
    vector& operator=(std::initializer_list<value_type> ilist)
    {
        vector tmp(ilist.begin(), ilist.end());
        swap(tmp);
        return *this;
    }

    ~vector()
    {
        destroy_and_recover(begin_, end_, cap_-begin_);
        begin_=end_=cap_=nullptr;
    }

public:
    /* 迭代器相关操作 */
    iterator begin() noexcept { return begin_; }
    iterator end()  noexcept { return end_; }
    const_iterator begin() const noexcept { return begin_; }
    const_iterator end() const noexcept { return end_; }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    const_reverse_iterator crend() const noexcept { return rend(); }

    /* 容器容量 */
    bool empty() const noexcept { return begin_==end_; }
    size_type size() const noexcept { return static_cast<size_type>(end_-begin_); }
    size_type max_size() const noexcept { return static_cast<size_type>(-1)/sizeof(T); }
    size_type capacity() const noexcept { return static_cast<size_type>(cap_-begin_); }
    void reserve(size_type n);
    void shrink_to_fit();

    /* 访问元素的相关操作 */
    reference operator[](size_type n)
    {
        ORANGE_STL_DEBUG(n < size());
        return *(begin_+n);
    }
    const_reference operator[](size_type n) const
    {
        ORANGE_STL_DEBUG(n < size());
        return *(begin_+n);
    }
    reference at(size_type n)
    {
        THROW_OUT_OF_RANGE_IF(!(n<size()), "vector<T>::at() subscript out of range");
        return (*this)[n];
    }
    const_reference at(size_type n) const
    {
        THROW_OUT_OF_RANGE_IF(!(n<size()), "vector<T>::at() subscript out of range");
        return (*this)[n];
    }
    reference front()
    {
        ORANGE_STL_DEBUG(!empty());
        return *begin_;
    }
    const_reference front() const
    {
        ORANGE_STL_DEBUG(!empty());
        return *begin_;
    }
    reference back()
    {
        ORANGE_STL_DEBUG(!empty());
        return *(end_-1);
    }
    const_reference back() const
    {
        ORANGE_STL_DEBUG(!empty());
        return *(end_-1);
    }

    pointer data() noexcept { return begin_; }
    const_pointer data() const noexcept { return begin_; }

    /* assign */
    void assign(size_type n, const value_type& value)
    {
        fill_assign(n, value);
    }
    template <class Iter, typename std::enable_if<orange_stl::is_input_iterator<Iter>::value, int>::type=0>
    void assign(Iter first, Iter last)
    {
        ORANGE_STL_DEBUG(!(last<first));
        copy_assign(first, last, iterator_category(first));
    }
    void assign(std::initializer_list<value_type> il)
    {
        copy_assign(il.begin(), il.end(), orange_stl::forward_iterator_tag{});
    }

    /* emplace */
    template <class... Args>
    iterator emplace(const_iterator pos, Args&& ...args);

    template <class... Args>
    void emplace_back(Args&& ...args);

    /* push_back   pop_back    */
    void push_back(const value_type& value);
    void push_back(value_type&& value)
    {
        emplace_back(orange_stl::move(value));
    }
    void pop_back();

    /* insert */
    iterator insert(const_iterator pos, const value_type& value);
    iterator insert(const_iterator pos, value_type&& value)
    {
        return emplace(pos, orange_stl::move(value));
    }
    iterator insert(const_iterator pos, size_type n, const value_type& value){
        ORANGE_STL_DEBUG(pos>=begin() && pos<=end());
        return fill_insert(const_cast<iterator>(pos), n, value);
    }
    template<class Iter, typename std::enable_if<orange_stl::is_input_iterator<Iter>::value, int>::type=0>
    void insert(const_iterator pos, Iter first, Iter last)
    {
        ORANGE_STL_DEBUG(pos>=begin() && pos<=end() && !(last<first));
        copy_insert(const_cast<iterator>(pos), first, last);
    }

    /* erase */
    iterator erase(const_iterator pos);
    iterator erase(const_iterator first, const_iterator last);
    void clear() { erase(begin(), end()); }

    /* resize */
    void resize(size_type new_size) { return resize(new_size, value_type()); }
    void resize(size_type new_size, const value_type& value);

    void reverse() { orange_stl::reverse(begin(), end()); }

    /* swap */
    void swap(vector& rhs) noexcept;
private:
    /* initialize */
    void try_init() noexcept;
    void init_space(size_type size, size_type cap);
    void fill_init(size_type n, const value_type& value);
    template <class Iter>
    void range_init(Iter first, Iter last);
    void destroy_and_recover(iterator first, iterator last, size_type n);

    size_type get_new_cap(size_type add_size);

    /* assign */
    void fill_assign(size_type n, const value_type& value );
    
    template<class IIter> 
    void copy_assign(IIter first, IIter last, input_iterator_tag);
    
    template <class FIter>
    void copy_assign(FIter first, FIter last, forward_iterator_tag);

    /* reallocate */
    template <class ...Args>
    void reallocate_emplace(iterator pos, Args&& ...args);
    void reallocate_insert(iterator pos, const value_type& value);

    /* insert */
    iterator fill_insert(iterator pos, size_type n, const value_type& value);

    template <class IIter>
    void copy_insert(iterator pos, IIter first, IIter last);

    void reinsert(size_type size);

};

/* 赋值复制操作符 */
template <class T>
vector<T>& vector<T>::operator=(const vector& rhs)
{
    if(this!=&rhs)
    {
        const auto len=rhs.size();
        if(len>capacity())/* rhs的大小比当前容器数量多 */
        {
            vector tmp(rhs.begin(), rhs.end());
            swap(tmp);
        }
        else if(size()>=len)    /* 当前容器数据数量比比rhs多 */
        {
            auto i=orange_stl::copy(rhs.begin(), rhs.end(), begin());
            data_allocator::destroy(i, end_);
            end_=begin_+len;
        }
        else    /* len 比size()大且比 容量reserve()小 */
        {
            orange_stl::copy(rhs.begin(), rhs.begin()+size(), begin_); /* 将原有size()大小进行赋值 */
            orange_stl::uninitialized_copy(rhs.begin()+size(), rhs.end(), end_);/* 目标区间未初始化，依次调用拷贝构造函数 */
            cap_=end_=begin_+len;
        }
    }
    return *this;
}

/* 移动赋值操作符 */
template<class T>
vector<T>& vector<T>::operator=(vector&& rhs) noexcept
{
    destroy_and_recover(begin_, end_, cap_-begin_);
    begin_=rhs.begin_;
    end_=rhs.end_;
    cap_=rhs.cap_;
    rhs.begin_=nullptr;
    rhs.end_=nullptr;
    rhs.cap_=nullptr;
    return *this;
}

/* 预留空间大小，原容量小于要求的时候，才会重新进行分配 */
template <class T>
void vector<T>::reserve(size_type n)
{
    if(capacity() < n)
    {
        THROW_LENGTH_ERROR_IF(n > max_size(), "n can not larger than  max_size() int vector<T>::reserve(n)");
        const auto old_size=size();
        auto tmp=data_allocator::allocate(n);
        orange_stl::uninitialized_move(begin_, end_, tmp);
        begin_=tmp;
        end_=tmp+old_size;
        cap_=begin_+n;
    }
}

/* 缩小当前容器容量 */
template <class T>
void vector<T>::shrink_to_fit()
{
    if(end_<cap_)
    {
        reinsert(size());
    }
}

/* 在pos位置构造元素，避免额外的复制或移动的开销 */
template <class T>
template <class ...Args>
typename vector<T>::iterator
vector<T>::emplace(const_iterator pos, Args&& ...args)
{
    ORANGE_STL_DEBUG(pos>=begin() && pos<=end());
    iterator xpos=const_cast<iterator>(pos);
    const size_type n=xpos-begin_;
    if(end_!=cap && xpos==end)
    {
        data_allocator::construct(orange_stl:::address_of(*end_), orange_stl::forward<Args>(args)...);
        ++end_;
    }
    else if(end_!=cap_)
    {
        auto new_end=end_;
        data_allocator::construct(orange_stl::address_of(*end_), *(end_-1));
        ++new_end;
        orange_stl::copy_backward(xpos, end_-1, end_);
        *xpos=value_type(orange_stl::forward<Args>>(args)...);
    }
    else
    {
        reallocate_emplace(xpos, orange_stl::forward<Args>(args)...);
    }
    return begin()+n;
}

// 在尾部就地构造元素，避免额外的复制或移动开销
template <class T>
template <class ...Args>
void vector<T>::emplace_back(Args&& ...args)
{
    if(end_<cap_)
    {
        data_allocator::construct(orange_stl::address_of(*end_), orange_stl::forward<Args>(args)...);
        ++end_;
    }
    else
    {
        reallocate_emplace(end_, orange_stl::forward<Args>(args)...);
    }
}


/* 重新分配空间，并且在pis处就地构造元素 */
template <class T>
template <class ...Args>
void vector<T>::reallocate_emplace(iterator pos, Args&& ...args)
{
    const auto new_size = get_new_cap(1);
    auto new_begin = data_allocator::allocate(new_size);
    auto new_end=new_begin;
    try
    {
        new_end=orange_stl::uninitialized_move(begin_, pos, new_begin);
        data_allocator::construct(orange_stl::address_of(*new_end), orange_stl::forward<Args>(args)...);
        ++new_end;
        new_end=orange_stl::uninitialized_move(pos, end_, new_end);
    }
    catch(...)
    {
        data_allocator::deallocate(new_begin, new_size);
        throw;
    }
    destroy_and_recover(begin_, end_, cap_-begin_);
    begin_=new_begin;
    end_=new_end;
    cap_=new_begin+new_size;
}


/* resinert */
template<class T>
void vector<T>::reinsert(size_type size)
{
    auto new_begin = data_allocator::allocate(size);
    try{
        orange_stl::uninitialized_move(begin_, end_, new_begin);
    }
    catch(...)
    {
        data_allocator::deallocate(new_begin, size);
        throw;
    }
    data_allocator::deallocate(begin_, cap_-begin_);
    begin_=new_begin;
    end_=begin_+size;
    cap_=begin_+size;
}

/* destroy_and_recover */
template<class T>
void vector<T>::destroy_and_recover(iterator first, iterator last, size_type n)
{
    data_allocator::destroy(first, last);
    data_allocator::deallocate(first, n);
}

/* get_new_cap函数 */
template <class T>
typename vector<T>::size_type
vector<T>::get_new_cap(size_type add_size)
{
    const auto old_size = capacity();
    THROW_LENGTH_ERROR_IF(old_size>max_size()-add_size, "vector<T>'s size too big");
    if(old_size > max_size() - old_size/2)
    {
        return old_size+add_size>max_size-16?old_size+add_size:old_size+add_size+16;
    }
    const size_type new_size= old_size==0
        ?orange_stl::max(add_size, static_cast<size_type>(16))
        :orange_stl::max(old_size+old_size/2, old_size+add_size);
    return new_size;
}

}


#endif