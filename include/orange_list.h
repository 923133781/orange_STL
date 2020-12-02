#ifndef __ORANGE_LIST_H__
#define __ORANGE_LIST_H__

/* 模板类list ----> 双向链表 */

#include <initializer_list>
#include "orange_iterator.h"
#include "orange_util.h"
#include "orange_memory.h"
#include "orange_exceptdef.h"
#include "orange_functional.h"

namespace orange_stl
{

template <class T>
struct list_node_base;

template <class T>
struct list_node;

template <class T>
struct node_traits{
    typedef list_node_base<T>*  base_ptr;
    typedef list_node<T>*       node_ptr; 
};

/* list结点的结构 */
template <class T>
struct list_node_base{
    typedef typename node_traits<T>::base_ptr base_ptr;
    typedef typename node_traits<T>::node_ptr node_ptr;

    base_ptr prev; /* 前一个节点 */
    base_ptr next; /* 后一个节点 */

    list_node_base() = default;

    node_ptr as_node()
    {
        return static_cast<node_ptr>(self());
    }

    void unlink()
    {
        prev = next = self();
    }

    base_ptr self()
    {
        return static_cast<base_ptr>(&*this);
    }
};

template <class T>
struct list_node : public list_node_base<T>
{
    typedef typename node_traits<T>::base_ptr base_ptr;
    typedef typename node_traits<T>::node_ptr node_ptr;

    T value;    // 数据域

    list_node() = default;
    list_node(const T& v) : value(v) {}
    list_node(const T&& v) : value(orange_stl::move(v)){}

    base_ptr as_base()
    {
        return static_cast<base_ptr>(&*this);
    }
    node_ptr self()
    {
        return static_cast<node_ptr>(&*this);
    }
};

/* list迭代器的设计 */
template <class T>
struct list_iterator : public orange_stl::iterator<orange_stl::bidirectional_iterator_tag, T>
{
    typedef T                                   value_type;
    typedef T*                                  pointer;
    typedef T&                                  reference;
    typedef typename node_traits<T>::base_ptr   base_ptr;
    typedef typename node_traits<T>::node_ptr   node_ptr;
    typedef list_iterator<T>                    self;

    base_ptr node_; /* 指向当前节点 */

    /* 构造函数 */
    list_iterator() = default;
    list_iterator(base_ptr x) : node_(x){}
    list_iterator(node_ptr x) : node_(x->as_base()){}
    list_iterator(const list_iterator &rhs) : node_(rhs.node_){}

    /* 重载操作符 */
    reference   operator*()  const { return node_->as_node()->value; }
    pointer     operator->() const { return &(operator*()); } 

    self& operator++()
    {
        ORANGE_STL_DEBUG(node_!=nullptr);
        node_ = node_->next;
        return *this;
    }

    self operator++(int)
    {
        self tmp = *this;
        ++*this;
        return tmp;
    }

    self& operator--()
    {
        ORANGE_STL_DEBUG(node_!=nullptr);
        node_=node_->prev;
        return *this;
    }

    self operator--(int)
    {
        self tmp=*this;
        --*this;
        return tmp;
    }

    /* 重载比较操作符  */
    bool operator==(const self &rhs) const { return node_==rhs.node_; }
    bool operator!=(const self &rhs) const { return node_!=rhs.node_; } 
};

template <class T> 
struct list_const_iterator : public iterator<bidirectional_iterator_tag, T>
{
    typedef T                                   value_type;
    typedef const T*                            pointer;
    typedef const T&                            reference;
    typedef typename node_traits<T>::base_ptr   base_ptr;
    typedef typename node_traits<T>::node_ptr   node_ptr;
    typedef list_const_iterator<T>              self;

    base_ptr node_;

    list_const_iterator() = default;
    list_const_iterator(base_ptr x) : node_(x) {}
    list_const_iterator(node_ptr x) : node_(x->as_base()) {}
    list_const_iterator(const list_iterator<T> &rhs) : node_(rhs.node_){}
    list_const_iterator(const list_const_iterator &rhs) : node_(rhs.node_) {}

    reference   operator*()  const { return node_->as_node()->value; }
    pointer     operator->() const { return &(operator*()); }

    self& operator++()
    {
        ORANGE_STL_DEBUG(node_!=nullptr);
        node_=node_->next;
        return *this;
    }
    self operator++(int)
    {
        self tmp=*this;
        ++*this;
        return tmp;
    }
    self& operator--()
    {
        ORANGE_STL_DEBUG(node_!=nullptr);
        node_=node_->prev;
        return *this;
    }
    self operator--(int)
    {
        self tmp=*this;
        --*this;
        return tmp;
    }

    //重载比较操作符
    bool operator==(const self &rhs) const { return node==rhs.node_; }
    bool operator!=(const self &rhs) const { return node!=rhs.node_; }
};

/*  模板类 list
    模板参数T表示数据类型 */
template <class T>
class list
{
public:
    // 嵌套类型定义
    typedef orange_stl::allocator<T>                        allocator_type;
    typedef orange_stl::allocator<T>                        data_allocator;
    typedef orange_stl::allocator<list_node_base<T>>        base_allocator;
    typedef orange_stl::allocator<list_node<T>>             node_allocator;

    typedef typename allocator_type::value_type             value_type;
    typedef typename allocator_type::pointer                pointer;
    typedef typename allocator_type::const_pointer          const_pointer;
    typedef typename allocator_type::reference              reference;
    typedef typename allocator_type::const_reference        const_reference;
    typedef typename allocator_type::size_type              size_type;
    typedef typename allocator_type::difference_type        difference_type;

    typedef list_iterator<T>                                iterator;
    typedef list_const_iterator<T>                          const_iterator;
    typedef orange_stl::reverse_iterator<iterator>          reverse_iterator;
    typedef orange_stl::reverse_iterator<const_iterator>    const_reverse_iterator;

    typedef typename node_traits<T>::base_ptr               base_ptr;
    typedef typename node_traits<T>::node_ptr               node_ptr;

    allocator_type get_allocator()  { return node_allocator(); }

private:
    base_ptr    node_;  /* 指向末尾的节点 */
    size_type   size_;  /* 大小 */ 

public:
    /* 构造、复制、移动、析构函数 */
    list() { fill_init(0, value_type()); }
    
    explicit list(size_type n) { fill_init(n, value_type()); }
    
    list(size_type n, const T& value) { fill_init(n, value); }

    template <class Iter, typename std::enable_if<orange_stl::is_input_iterator<Iter>::value, int>::type = 0>
    list(Iter first, Iter last) { copy_init(first, last); } 

    list(std::initializer_list<T> ilist)
    { copy_init(ilist.begin(), ilist.end()); }

    list(const list& rhs)
    { copy_init(rhs.cbegin(), rhs.cend()); }

    list(list &&rhs) noexcept : node_(rhs.node_), size_(rhs.size_)
    { 
        rhs.node_ = nullptr;
        rhs.size_ = 0;
    }

    list& operator=(const list& rhs)
    {
        if(this != &rhs) assign(rhs.begin(), rhs.end());
        return *this;
    }

    list& operator=(list&& rhs) noexcept
    {
        clear();
        splice(end(), rhs);
        return *this;
    }

    list& operator=(std::initializer_list<T> ilist)
    {
        list tmp(ilist.begin(), ilist.end());
        swap(tmp);
        return *this;
    }

    ~list()
    {
        if(node_)
        {
            clae();
            base_allocator::deallocate(node_);
            node_=nulllptr;
            size_=0;
        }
    }

public:
    /* 迭代器的相关操作 */
    iterator begin() noexcept { return node_->next; };
    const_iterator begin() const noexcept { return node_->next; }
    iterator end() noexcept {return node_; }
    const_iterator end() const noexcept { return node_; }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }

    const_iterator cbegin() const noexcept { return begin(); };
    const_iterator cend() const noexcept { return end(); };
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    const_reverse_iterator crend() const noexcept { return rend(); }

    /* list容器容量相关函数  */
    bool empty() const noexcept { return node_->next == node_; }

    size_type size() const noexcept { return size_; }

    size_type max_size() const noexcept { retrun static_cast<size_type>(-1); }

    /* 访问相关的元素 */
    reference front()
    {
        ORANGE_STL_DEBUG(!empty());
        return *bengin();
    }
    const_reference front() const
    {
        ORANGE_STL_DEBUG(!empty());
        return *begin();
    }
    reference back()
    {
        ORANGE_STL_DEBUG(!empty());
        return *(--end());
    }
    const_reference back() const
    {
        ORANGE_STL_DEBUG(!empty());
        return *(--end());
    }

};

} // namespace orange_stl


#endif // !__ORANGE_LIST_H__