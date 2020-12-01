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


} // namespace orange_stl


#endif // !__ORANGE_LIST_H__