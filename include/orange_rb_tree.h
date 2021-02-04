#ifndef __ORANGE__RB_TREE_H__
#define __ORANGE__RB_TREE_H__

#include <initializer_list>
#include "orange_functional.h"
#include "orange_iterator.h"
#include "orange_memory.h"
#include "orange_type_traits.h"
#include "orange_exceptdef.h"

namespace orange_stl
{

/* rb_tree 节点颜色类型 */
typedef bool rb_tree_color_type;
static constexpr rb_tree_color_type rb_tree_red   = false;
static constexpr rb_tree_color_type rb_tree_black = true;

template <class T> struct rb_tree_node_base;
template <class T> struct rb_tree_node;

template <class T> struct rb_tree_iterator;
template <class T> struct rb_tree_const_iterator;

/* rb_tree value traits */
template <class T, bool>
struct rb_tree_value_traits_imp
{
    typedef T key_type;
    typedef T mapped_type;
    typedef T value_type;

    template <class Ty>
    static const key_type& get_key(const Ty& value)
    {
        return value;
    }

    template <class Ty>
    static const value_type& get_value(const Ty& value)
    {
        return value;
    }
};

template <class T>
struct rb_tree_value_traits_imp<T, true>
{
    typedef typename std::remove_cv<typename T::first_type>::type key_type;
    typedef typename T::second_type                               mapped_type;
    typedef T                                                     value_type;

    template <class Ty>
    static const key_type& get_key(const Ty& value)
    {
        return value.first;
    }

    template <class Ty>
    static const value_type& get_value(const Ty& value)
    {
        return value;
    }
};

template <class T>
struct rb_tree_value_traits
{
    static constexpr bool is_map = orange_stl::is_pair<T>::value;

    typedef rb_tree_value_traits_imp<T, is_map> value_traits_type;

    typedef typename value_traits_type::key_type    key_type;
    typedef typename value_traits_type::mapped_type mapped_type;
    typedef typename value_traits_type::value_type  value_type;

    template <class Ty>
    static const key_type& get_key(const Ty& value)
    {
        return value_traits_type::get_key(value);
    }

    template <class Ty>
    static const value_type& get_value(const Ty& value)
    {
        return value_traits_type::get_value(value);
    }
};

/* rb_tree node traits */
template <class T>
struct rb_tree_node_traits
{
    typedef rb_tree_color_type                 color_type;

    typedef rb_tree_value_traits<T>            value_traits;
    typedef typename value_traits::key_type    key_type;
    typedef typename value_traits::mapped_type mapped_type;
    typedef typename value_traits::value_type  value_type;

    typedef rb_tree_node_base<T>*              base_ptr;
    typedef rb_tree_node<T>*                   node_ptr;
};

/* rb_tree node base */
template <class T>
struct rb_tree_node_base
{
    typedef rb_tree_color_type    color_type;
    typedef rb_tree_node_base<T>* base_ptr;
    typedef rb_tree_node<T>*      node_ptr;

    base_ptr   parent;  // 父节点
    base_ptr   left;    // 左子节点
    base_ptr   right;   // 右子节点
    color_type color;   // 节点颜色

    base_ptr get_base_ptr()
    {
        return &*this;  /* 返回当前对象的地址 */
    }

    node_ptr get_node_ptr()
    {
        return reinterpret_cast<node_ptr>(&*this); /* 强制类型转换，父类转换为子类 */
    }

    node_ptr& get_node_ref()
    {
        return reinterpret_cast<node_ptr&>(*this);
    }
};

/* * 
*  return *this返回的是当前对象的克隆或者本身（若返回类型为A， 则是克隆， 若返回类型为A&， 则是本身 ）
*  return this返回当前对象的地址（指向当前对象的指针）
* */

/* rb_tree node 继承于 rb_tree node base */
template <class T>
struct rb_tree_node : public rb_tree_node_base<T>
{
    typedef rb_tree_node_base<T>* base_ptr;
    typedef rb_tree_node<T>*      node_ptr;

    T value;  // 节点值

    base_ptr get_base_ptr()
    {
        return static_cast<base_ptr>(&*this); /* 子类转换为父类，由下到上（安全）的静态类型转换 */
    }

    node_ptr get_node_ptr()
    {
        return &*this;
    }
};

/* rb_tree traits */
template <class T>
struct rb_tree_traits
{
    typedef rb_tree_value_traits<T>            value_traits;

    typedef typename value_traits::key_type    key_type;
    typedef typename value_traits::mapped_type mapped_type;
    typedef typename value_traits::value_type  value_type;

    typedef value_type*                        pointer;
    typedef value_type&                        reference;
    typedef const value_type*                  const_pointer;
    typedef const value_type&                  const_reference;

    typedef rb_tree_node_base<T>               base_type;
    typedef rb_tree_node<T>                    node_type;

    typedef base_type*                         base_ptr;
    typedef node_type*                         node_ptr;
};

/* rb_tree iterator */
template <class T>
struct rb_tree_iterator_base : public orange_stl::iterator<orange_stl::bidirectional_iterator_tag, T>
{
    typedef typename rb_tree_traits<T>::base_ptr    base_ptr;

    base_ptr node; /* 指向节点本身 */

    rb_tree_iterator_base() : node(nullptr) 
    { }

    /* iterator 前进 */
    void inc()
    {
        if(node->right != nullptr)
        {
            node = rb_tree_min(node->right);
        }
        else
        {
            /* 如果没有右子节点 */
            auto y=node->parent;
            while(y->right == node)
            {
                node=y;
                y=y->parent;
            }
            if(node->right != y) // 寻找根节点的下一节点，而根节点没有右子节点”的特殊情况
                node = y;
        }
        
    }

    /* iterator 后退 */
    void dec()
    {
        if (node->parent->parent == node && rb_tree_is_red(node))
        { // 如果 node 为 header
            node = node->right;  // 指向整棵树的 max 节点
        }
        else if (node->left != nullptr)
        {
            node = rb_tree_max(node->left);
        }
        else
        {  // 非 header 节点，也无左子节点
            auto y = node->parent;
            while (node == y->left)
            {
                node = y;
                y = y->parent;
            }
            node = y;
        }
    }

    bool operator==(const rb_tree_iterator_base& rhs) { return node == rhs.node; }
    bool operator!=(const rb_tree_iterator_base& rhs) { return node != rhs.node; }
};

template <class T>
struct rb_tree_iterator : public rb_tree_iterator_base<T>
{
    typedef rb_tree_traits<T>                tree_traits;

    typedef typename tree_traits::value_type value_type;
    typedef typename tree_traits::pointer    pointer;
    typedef typename tree_traits::reference  reference;
    typedef typename tree_traits::base_ptr   base_ptr;
    typedef typename tree_traits::node_ptr   node_ptr;

    typedef rb_tree_iterator<T>              iterator;
    typedef rb_tree_const_iterator<T>        const_iterator;
    typedef iterator                         self;

    using rb_tree_iterator_base<T>::node;

    // 构造函数
    rb_tree_iterator() {}
    rb_tree_iterator(base_ptr x) { node = x; }
    rb_tree_iterator(node_ptr x) { node = x; }
    rb_tree_iterator(const iterator& rhs) { node = rhs.node; }
    rb_tree_iterator(const const_iterator& rhs) { node = rhs.node; }

    // 重载操作符
    reference operator*()  const { return node->get_node_ptr()->value; }
    pointer   operator->() const { return &(operator*()); }

    self& operator++()
    {
        this->inc();
        return *this;
    }
    self operator++(int)
    {
        self tmp(*this);
        this->inc();
        return tmp;
    }
    self& operator--()
    {
        this->dec();
        return *this;
    }
    self operator--(int)
    {
        self tmp(*this);
        this->dec();
        return tmp;
    }
};

template <class T>
struct rb_tree_const_iterator :public rb_tree_iterator_base<T>
{
    typedef rb_tree_traits<T>                     tree_traits;

    typedef typename tree_traits::value_type      value_type;
    typedef typename tree_traits::const_pointer   pointer;
    typedef typename tree_traits::const_reference reference;
    typedef typename tree_traits::base_ptr        base_ptr;
    typedef typename tree_traits::node_ptr        node_ptr;

    typedef rb_tree_iterator<T>                   iterator;
    typedef rb_tree_const_iterator<T>             const_iterator;
    typedef const_iterator                        self;

    using rb_tree_iterator_base<T>::node;

    // 构造函数
    rb_tree_const_iterator() {}
    rb_tree_const_iterator(base_ptr x) { node = x; }
    rb_tree_const_iterator(node_ptr x) { node = x; }
    rb_tree_const_iterator(const iterator& rhs) { node = rhs.node; }
    rb_tree_const_iterator(const const_iterator& rhs) { node = rhs.node; }

    // 重载操作符
    reference operator*()  const { return node->get_node_ptr()->value; }
    pointer   operator->() const { return &(operator*()); }

    self& operator++()
    {
        this->inc();
        return *this;
    }
    self operator++(int)
    {
        self tmp(*this);
        this->inc();
        return tmp;
    }
    self& operator--()
    {
        this->dec();
        return *this;
    }
    self operator--(int)
    {
        self tmp(*this);
        this->dec();
        return tmp;
    }
};

template <class NodePtr>
NodePtr rb_tree_min(NodePtr x) noexcept
{
    while(x->left != nullptr)
        x=x->left;
    return x;
}

template <class NodePtr>
NodePtr rb_tree_max(NodePtr x) noexcept
{
    while(x->right != nullptr)
        x=x->right;
    return x;
}

template <class NodePtr>
bool rb_tree_is_lchild(NodePtr node) noexcept
{
    return node==node->parent->left;
}

template <class NodePtr>
bool rb_tree_is_red(NodePtr node) noexcept
{
    return node->color==rb_tree_red;
}

template <class NodePtr>
void rb_tree_set_black(NodePtr& node) noexcept
{
    node->color=rb_tree_black;
}

template <class NodePtr>
void rb_tree_set_red(NodePtr& node) noexcept
{
    node->color = rb_tree_red;
}

template <class NodePtr>
NodePtr rb_tree_next(NodePtr node) noexcept
{
    if(node->right!=nullptr)
        return rb_tree_min(node->right);
    while(!rb_tree_is_lchild(node))
        node=node->parent;
    return node->parent;
}

/*---------------------------------------*\
|       p                         p       |
|      / \                       / \      |
|     x   d    rotate left      y   d     |
|    / \       ===========>    / \        |
|   a   y                     x   c       |
|      / \                   / \          |
|     b   c                 a   b         |
\*---------------------------------------*/
// 左旋，参数一为左旋点，参数二为根节点
template <class NodePtr>
void rb_tree_rotate_left(NodePtr x, NodePtr& root) noexcept
{
    auto y=x->right;
    x->right=y->left;
    if(y->left!=nullptr)
        y->left->parent=x;
    y->parent=x->parent;

    if(x==root)
        root=y;
    else if(rb_tree_is_lchild(x))
        x->parent->left=y;
    else
        x->parent->right=y;

    y->left=x;
    x->parent=y;
}

/*----------------------------------------*\
|     p                         p          |
|    / \                       / \         |
|   d   x      rotate right   d   y        |
|      / \     ===========>      / \       |
|     y   a                     b   x      |
|    / \                           / \     |
|   b   c                         c   a    |
\*----------------------------------------*/
// 右旋，参数一为右旋点，参数二为根节点
template <class NodePtr>
void rb_tree_rotate_right(NodePtr x, NodePtr& root) noexcept
{
    auto y=x->left;
    x->left=y->right;
    if(y->right != nullptr)
        y->right->parent = x;
    y->parent = x->parent;

    if(x==root)
        root = y;
    else if(rb_tree_is_lchild(x))
        x->parent->left=y;
    else
        x->parent->right=x;
    
    y->right = x;
    x->parent = y;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \   
* 插入节点后使 rb tree 重新平衡，参数一为新增节点，参数二为根节点
*
* case 1: 新增节点位于根节点，令新增节点为黑
* case 2: 新增节点的父节点为黑，没有破坏平衡，直接返回
* case 3: 父节点和叔叔节点都为红，令父节点和叔叔节点为黑，祖父节点为红，
*         然后令祖父节点为当前节点，继续处理
* case 4: 父节点为红，叔叔节点为 NIL 或黑色，父节点为左（右）孩子，当前节点为右（左）孩子，
*         让父节点成为当前节点，再以当前节点为支点左（右）旋
* case 5: 父节点为红，叔叔节点为 NIL 或黑色，父节点为左（右）孩子，当前节点为左（右）孩子，
*         让父节点变为黑色，祖父节点变为红色，以祖父节点为支点右（左）旋
\** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
template <class NodePtr>
void rb_tree_insert_rebalance(NodePtr x, NodePtr& root) noexcept
{
    rb_tree_set_red(x);     /* 新增节点为红色 */
    while(x != root && rb_tree_is_red(x->parent))
    {
        if(rb_tree_is_lchild(x->parent))
        {
            auto uncle=x->parent->parent->right;
            if(uncle != nullptr && rb_tree_is_red(uncle))
            {
                /* case3: 父和叔都为红色节点 */
                rb_tree_set_black(x->parent);
                rb_tree_set_black(uncle);
                x=x->parent->parent;
                rb_tree_set_red(x);
            }
            else
            {
                /* uncle节点为Nil或者uncle节点为黑色 */
                if(!rb_tree_is_lchild(x))
                {
                    /* case4: 当前结点为右子节点 */
                    x=x->parent;
                    rb_tree_rotate_left(x, root);
                }
                /* 转换为case5: 当前结点变为左子节点 */
                rb_tree_set_black(x->parent);
                rb_tree_set_red(x->parent->parent);
                rb_tree_rotate_right(x->parent->parent, root);
                break;
            }
        }
        else    /* 若x的父节点是右子节点，对称处理 */ 
        {
            auto uncle=x->parent->parent->left;
            if(uncle != nullptr && rb_tree_is_red(uncle))
            {
                /* case3: uncle和父节点都是红色 */
                rb_tree_set_black(x->parent);
                rb_tree_set_black(uncle);
                x=x->parent->parent;
                rb_tree_set_red(x);
            }
            else    /* uncle节点为Nil或者uncle节点为黑色  */
            {
                if(rb_tree_is_lchild(x))
                {
                    /* case4: 当前结点为左子节点 */
                    x=x->parent;
                    rb_tree_rotate_right(x, root);
                }
                 /* 转换为case5: 当前结点变为右子节点 */
                rb_tree_set_black(x->parent);
                rb_tree_set_red(x->parent->parent);
                rb_tree_rotate_left(x->parent->parent, root);
                break;
            }
        }
    }
    rb_tree_set_black(root);
}

/*  删除节点后使 rb tree 重新平衡，参数一为要删除的节点，参数二为根节点，参数三为最小节点，参数四为最大节点  */
template <class NodePtr>
NodePtr rb_tree_erase_reblance(NodePtr z, NodePtr& root, NodePtr leftmost, NodePtr rightmost)
{
    /* y是可能的替换节点，指向最终要删除的节点 */
    /* 如果z有双子节点，y就是右子树的最左节点，否则y=z; */
    auto y=(z->left==nullptr || z->right==nullptr) ? z : rb_tree_next(z);
    /* x是y的独子节点或者NIL节点 */
    auto x=y->left!=nullptr?y->left:y->right;
    /* xp为x的父节点 */
    NodePtr xp=nullptr;

    /* y!=z说明有两个非空的子节点，此时y指向z的右子树的最左节点，x指向y的右子节点 
       用y顶替z的位置，用x顶替y的位置，最后用y指向z */
    if(y!=z)
    {
        /*     z
        *       \
        *        o
        *       /
        *      y
        *       \
        *        x
        */
        z->left->parent=y;
        y->left=z->left;
        /* 如果y不是z的右子节点，那么z的右子节点一定有左孩子 */
        if(y!=z->right)
        {
            /* x替换y的位置 */
            xp=y->parent;
            if(x!=nullptr)
                x->parent=y->parent;
            y->parent->left=x;
            y->right=z->right;
            z->right->parent=y;
        }
        else // y==z->right
        {
            xp=y;
        }
        
        /* 连接y与z的父节点 */
        if(root==z)
            root=y;
        else if(rb_tree_is_lchild(z))
            z->parent->left = y;
        else
            z->parent->right = y;

        y->parent = z->parent;
        orange_stl::swap(y->color, z->color);
        y=z;    /* y指向最后要删除的节点 */
    }
    else
    {
        /* z至多有一个孩子节点 */
        xp=y->parent;
        if(x)
            x->parent = y->parent;
        
        /* 连接x和z的父节点 */
        if(root==z)
            root=x;
        else if(rb_tree_is_lchild(z))
            z->parent->left=x;
        else
            z->parent->right=x;
        
        /* 此时z有可能是最左或者最右节点 */
        if(leftmost==z)
            leftmost = x == nullptr ? xp : rb_tree_min(x);
        if(rightmost==z)
            rightmost = x == nullptr ? xp : rb_tree_max(x);
    }
    
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
     * 此时，y 指向要删除的节点，x 为替代节点，从 x 节点开始调整。
     *   如果删除的节点为红色，树的性质没有被破坏，否则按照以下情况调整（x 为左子节点为例）：
     * case 1: 兄弟节点为红色，令父节点为红，兄弟节点为黑，进行左（右）旋，继续处理
     * case 2: 兄弟节点为黑色，且两个子节点都为黑色或 NIL，令兄弟节点为红，父节点成为当前节点，继续处理
     * case 3: 兄弟节点为黑色，左子节点为红色或 NIL，右子节点为黑色或 NIL，
     *         令兄弟节点为红，兄弟节点的左子节点为黑，以兄弟节点为支点右（左）旋，继续处理
     * case 4: 兄弟节点为黑色，右子节点为红色，令兄弟节点为父节点的颜色，父节点为黑色，兄弟节点的右子节点
     *         为黑色，以父节点为支点左（右）旋，树的性质调整完成，算法结束
    \* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    if (!rb_tree_is_red(y)) /* 要删除的节点y是黑色的话，需要进行调整 */
    { 
        // x 为黑色时，调整，否则直接将 x 变为黑色即可
        while (x != root && (x == nullptr || !rb_tree_is_red(x)))
        {
            if (x == xp->left)
            { 
                // 如果 x 为左子节点
                auto brother = xp->right;
                if (rb_tree_is_red(brother))
                { 
                    // case 1
                    rb_tree_set_black(brother);
                    rb_tree_set_red(xp);
                    rb_tree_rotate_left(xp, root);
                    brother = xp->right;
                }
                // case 1 转为为了 case 2、3、4 中的一种
                if ((brother->left == nullptr || !rb_tree_is_red(brother->left)) &&
                    (brother->right == nullptr || !rb_tree_is_red(brother->right)))
                { // case 2
                    rb_tree_set_red(brother);
                    x = xp;
                    xp = xp->parent;
                }
                else
                { 
                    if (brother->right == nullptr || !rb_tree_is_red(brother->right))
                    { // case 3
                        if (brother->left != nullptr)
                            rb_tree_set_black(brother->left);
                        rb_tree_set_red(brother);
                        rb_tree_rotate_right(brother, root);
                        brother = xp->right;
                    }
                    // 转为 case 4
                    brother->color = xp->color;
                    rb_tree_set_black(xp);
                    if (brother->right != nullptr)  
                        rb_tree_set_black(brother->right);
                    rb_tree_rotate_left(xp, root);
                    break;
                }
            }
            else  // x 为右子节点，对称处理
            { 
                auto brother = xp->left;
                if (rb_tree_is_red(brother))
                { // case 1
                    rb_tree_set_black(brother);
                    rb_tree_set_red(xp);
                    rb_tree_rotate_right(xp, root);
                    brother = xp->left;
                }
                if ((brother->left == nullptr || !rb_tree_is_red(brother->left)) &&
                    (brother->right == nullptr || !rb_tree_is_red(brother->right)))
                { // case 2
                    rb_tree_set_red(brother);
                    x = xp;
                    xp = xp->parent;
                }
                else
                {
                    if (brother->left == nullptr || !rb_tree_is_red(brother->left))
                    { // case 3
                        if (brother->right != nullptr)
                        rb_tree_set_black(brother->right);
                        rb_tree_set_red(brother);
                        rb_tree_rotate_left(brother, root);
                        brother = xp->left;
                    }
                    // 转为 case 4
                    brother->color = xp->color;
                    rb_tree_set_black(xp);
                    if (brother->left != nullptr)  
                        rb_tree_set_black(brother->left);
                    rb_tree_rotate_right(xp, root);
                    break;
                }
            }
        }
        if (x != nullptr)
            rb_tree_set_black(x);
    }
    return y;
}

/* 模板类 rb_tree  参数1表示数据类型，参数2表示键值比较类型*/
template <class T, class Compare>
class rb_tree
{
public:
    typedef rb_tree_traits<T>                             tree_traits;
    typedef rb_tree_value_traits<T>                       value_traits;

    typedef typename tree_traits::base_type               base_type;
    typedef typename tree_traits::base_ptr                base_ptr;
    typedef typename tree_traits::node_type               node_type;
    typedef typename tree_traits::node_ptr                node_ptr;
    typedef typename tree_traits::key_type                key_type;
    typedef typename tree_traits::mapped_type             mapped_type;
    typedef typename tree_traits::value_type              value_type;
    typedef Compare                                       key_compare;

    typedef orange_stl::allocator<T>                      allocator_type;
    typedef orange_stl::allocator<T>                      data_allocator;
    typedef orange_stl::allocator<base_type>              base_allocator;
    typedef orange_stl::allocator<node_type>              node_allocator;

    typedef typename allocator_type::pointer              pointer;
    typedef typename allocator_type::const_pointer        const_pointer;
    typedef typename allocator_type::reference            reference;
    typedef typename allocator_type::const_reference      const_reference;
    typedef typename allocator_type::size_type            size_type;
    typedef typename allocator_type::difference_type      difference_type;

    typedef rb_tree_iterator<T>                           iterator;
    typedef rb_tree_const_iterator<T>                     const_iterator;
    typedef orange_stl::reverse_iterator<iterator>        reverse_iterator;
    typedef orange_stl::reverse_iterator<const_iterator>  const_reverse_iterator;

    allocator_type  get_allocator() const { return node_allocator(); }
    key_compare     key_comp()      const { return key_comp_; }

private:
    /* 使用如下三个数字表示红黑树 */
    base_ptr header_;       /* 特殊节点，与根节点互为父节点 */
    size_type node_count_;  /* 节点数 */
    key_compare key_comp_;  /* 节点键值比较准则 */

private:
    base_ptr& root()      const { return header_->parent; }
    base_ptr& leftmost()  const { return header_->left; }
    base_ptr& rightmost() const { return header_->right; }
public:
    // 构造、复制、析构函数
    rb_tree() { rb_tree_init(); }

    rb_tree(const rb_tree& rhs);
    rb_tree(rb_tree&& rhs) noexcept;

    rb_tree& operator=(const rb_tree& rhs);
    rb_tree& operator=(rb_tree&& rhs);

    ~rb_tree() { clear(); }

public:
    /* 迭代器相关的操作 */
    iterator begin() noexcept
    {
        return leftmost();
    }

    const_iterator begin() const noexcept
    {
        return leftmost();
    }

    iterator end() noexcept
    {
        return header_;
    }
    const_iterator end() const noexcept
    {
        return header_;
    }

    reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }
    const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }
    reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }
    const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    /* 容量相关的操作 */
    bool empty() const noexcept { return node_count_ == 0; }
    size_type size() const noexcept { return node_count_; }
    size_type max_size() const noexcept { return static_cast<size_type>(-1); }

    /* 插入相关的操作 */
    template <class ...Args>
    iterator emplace_multi(Args&& ...args);

    template <class ...Args>
    orange_stl::pair<iterator, bool> emplace_unique(Args&& ...args);

    template <class ...Args>
    iterator emplace_multi_use_hint(iterator hint, Args&& ...args);

    template <class ...Args>
    iterator emplace_unique_use_hint(iterator hint, Args&& ...args);

    /* insert */
    iterator insert_multi(const value_type& value);
    iterator insert_multi(value_type&& value)
    {
        return emplace_multi(orange_stl::move(value));
    }

    iterator insert_multi(iterator hint, const value_type& value)
    {
        return emplace_multi_use_hint(hint, value);
    }
    iterator insert_multi(iterator hint, value_type&& value)
    {
        return emplace_multi_use_hint(hint, orange_stl::move(value));
    }

    template <class InputIterator>
    void insert_multi(InputIterator first, InputIterator last)
    {
        size_type n=orange_stl::distance(first, last);
        THROW_LENGTH_ERROR_IF(node_count_>max_size()-n, "rb_tree<T, Comp>'s size too big");
        for(; n>0; --n, ++first)
            insert_multi(end(), *first);
    }

    orange_stl::pair<iterator, bool> insert_unique(const value_type& value);
    orange_stl::pair<iterator, bool> insert_unique(value_type&& value)
    {
        return emplace_unique(orange_stl::move(value));
    }

    iterator insert_unique(iterator hint, const value_type& value)
    {
        return emplace_unique_use_hint(hint, value);
    }
    iterator insert_unique(iterator hint, value_type&& value)
    {
        return emplace_unique_use_hint(hint, orange_stl::move(value));
    }

    template <class InputIterator>
    void insert_unique(InputIterator first, InputIterator last)
    {
        size_type n=orange_stl::distance(first, last);
        THROW_LENGTH_ERROR_IF(node_count_ > max_size()-n, "rb_tree<T, Comp>'s size too big");
        for(; n>0; --n,++first)
            insert_unique(end(), *first);
    }

    /* erase */
    iterator erase(iterator hint);

    size_type erase_multi(const key_type& key);
    size_type erase_unique(const key_type& key);

    void erase(iterator first, iterator last);
    void clear();

    /* 功能性操作 */
    iterator find(const key_type& key);
    const_iterator find(const key_type& key) const;

    size_type count_multi(const key_type& key) const
    {
        auto p=equal_range_multi(key);
        return static_cast<size_type>(orange_stl::distance(p.first, p.second));
    }
    size_type count_unique(const key_type& key) const
    {
        return find(key)!=end()?1:0;
    }

    iterator lower_bound(const key_type& key);
    const_iterator lower_bound(const key_type& key) const;

    iterator upper_bound(const key_type& key);
    const_iterator upper_bound(const key_type& key) const;

    orange_stl::pair<iterator, iterator> equal_range_multi(const key_type& key)
    {
        return orange_stl::pair<iterator, iterator>(lower_bound(key), upper_bound(key));
    }
    orange_stl::pair<const_iterator, const_iterator> equal_range_multi(const key_type& key) const
    {
        return orange_stl::pair<const_iterator, const_iterator>(lower_bound(key), upper_bound(key));
    }

    orange_stl::pair<iterator, iterator> equal_range_unique(const key_type& key)
    {
        iterator it=find(key);
        auto next=it;
        return it==end()?orange_stl::make_pair(it, it):orange_stl::make_pair(it, ++next);
    }
    orange_stl::pair<const_iterator, const_iterator> equal_range_unique(const key_type& key) const
    {
        const_iterator it=find(key);
        auto next=it;
        return it==end()?orange_stl::make_pair(it, it):orange_stl::make_pair(it, ++next);
    }
    void swap(rb_tree& rhs) noexcept;

private:
    /* 节点相关操作 */
    template <class ...Args>
    node_ptr create_node(Args&&... args);
    node_ptr clone_node(base_ptr x);
    void destroy_node(node_ptr p);

    void rb_tree_init();
    void reset();

    orange_stl::pair<base_ptr, bool> get_insert_multi_pos(const key_type& key);
    orange_stl::pair<orange_stl::pair<base_ptr, bool>, bool> get_insert_unique_pos(const key_type& key);

    iterator insert_value_at(base_ptr x, const value_type& value, bool add_to_left);
    iterator insert_node_at(base_ptr x, node_ptr node, bool add_to_left);

    iterator insert_multi_use_hint(iterator hint, key_type key, node_ptr node);
    iterator insert_unique_use_hint(iterator hint, key_type key, node_ptr node);

    base_ptr copy_from(base_ptr x, base_ptr p);
    void erase_since(base_ptr x);
};

/* 复制构造函数 */
template <class T, class Compare>
rb_tree<T, Compare>::rb_tree(const rb_tree& rhs)
{
    rb_tree_init();
    if(rhs.node_count_!=0)
    {
        root()=copy_from(rhs.root(), header_);
        leftmost()=rb_tree_min(root());
        rightmost()=rb_tree_max(root());
    }
    node_count_=rhs.node_count_;
    key_comp_ = rhs.key_comp_;
}

/* 移动构造函数 */
template <class T, class Compare>
rb_tree<T, Compare>::rb_tree(rb_tree&& rhs) noexcept
    : header_(orange_stl::move(rhs.header_)), 
    node_count_(rhs.key_comp_), 
    key_comp_(rhs.key_comp_)
{
    rhs.reset();
}

/* 复制赋值操作符 */
template <class T, class Compare>
rb_tree<T, Compare>& rb_tree<T, Compare>::operator=(const rb_tree& rhs)
{
    if(this!=&rhs)
    {
        clear();
        if(rhs.node_count_ != 0)
        {
            root() = copy_from(rhs.root(), header_);
            leftmost() = rb_tree_min(root());
            rightmost() = rb_tree_max(root());
        }
        node_count_ = rhs.node_count_;
        key_comp_ = rhs.key_comp_;
    }
    return *this;
}

/* 移动赋值操作符 */
template <class T, class Compare>
rb_tree<T, Compare>& rb_tree<T, Compare>::operator=(rb_tree&& rhs)
{
    clear();
    header_ = orange_stl::move(rhs.header_);
    node_count_=rhs.node_count_;
    key_comp_=rhs.key_comp_;
    rhs.reset();
    return *this;
}

/* 就地插入元素，键值允许重复 */
template <class T, class Compare>
template <class ...Args>
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::emplace_multi(Args&& ...args)
{
    THROW_LENGTH_ERROR_IF(node_count_>max_size()-1, "rb_tree<T, Compare>'s size too big");
    node_ptr np=create_node(orange_stl::forward<Args>(args)...);
    auto res=get_insert_multi_pos(value_traits::get_key(np->value));
    return insert_node_at(res.first, np, res.second);
}

/* 就地插入元素， 键值不允许重复 */
template <class T, class Compare>
template <class ...Args>
orange_stl::pair<typename rb_tree<T, Compare>::iterator, bool>
rb_tree<T, Compare>::emplace_unique(Args&& ...args)
{
    THROW_LENGTH_ERROR_IF(node_count_>max_size()-1, "rb_tree<T, Compare>'s size too big");
    node_ptr np=create_node(orange_stl::forward<Args>(args)...);
    auto res=get_insert_unique_pos(value_traits::get_key(np->value));
    if(res.second)
    {
        return orange_stl::make_pair(insert_node_at(res.first.first, np, res.first.second), true);
    }
    destroy_node(np);
    return orange_stl::make_pair(iterator(res.first.first), false);
}

/* 就地插入元素，键值允许重复， 当hint位置与插入位置接近时，插入操作的时间复杂度可以降低 */
template <class T, class Compare>
template <class ...Args>
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::emplace_multi_use_hint(iterator hint, Args&& ...args)
{
    THROW_LENGTH_ERROR_IF(node_count_>max_size()-1, "rb_tree<T, Compare>'s size too big");
    node_ptr np=create_node(orange_stl::forward<Args>(args)...);
    if(node_count_ == 0)
    {
        return insert_node_at(header_, np, true);
    }
    key_type key = value_traits::get_key(np->value);
    if(hint == begin())
    {
        if(key_comp_(key, value_traits::get_key(&hint)))
        {
            return insert_node_at(hint.node, np, true);
        }
        else
        {
            auto pos=get_insert_multi_pos(key);
            return insert_node_at(pos.first, np, pos.second);
        }
    }
    else if(hint == end() )
    {
        if(!key_comp_(key, value_traits::get_key(rightmost()->get_node_ptr()->value)))
        {
            return insert_node_at(rightmost(), np, false);
        }
        else
        {
            auto pos=get_insert_multi_pos(key);
            return insert_node_at(pos.first, np, pos.second);
        }
    }
    return insert_multi_use_hint(hint, key, np);
}

/* 就地插入元素，键值允许重复， 当hint位置与插入位置接近时，插入操作的时间复杂度可以降低 */
template <class T, class Compare>
template <class ...Args>
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::emplace_unique_use_hint(iterator hint, Args&& ...args)
{
    THROW_LENGTH_ERROR_IF(node_count_>max_size()-1, "rb_tree<T, Compare>'s size too big");
    node_ptr np=create_node(orange_stl::forward<Args>(args)...);
    if(node_count_==0)
    {
        return insert_node_at(header_, np, true);
    }
    key_type key=value_traits::get_key(np->value);
    if(hint == begin())
    {
        /* 位于begin()处 */
        if(key_comp_(key, value_traits::get_key(*hint)))
        {
            return insert_node_at(hint.node, np, true);
        }
        else
        {
            auto pos=get_insert_unique_pos(key);
            if(!pos.second)
            {
                destroy_node(np);
                return pos.first.first;
            }
            return insert_node_at(pos.first.first, np, pos.first.second);
        }
    }
    else if(hint == end())
    {
        if(key_comp_(value_traits::get_key(rightmost()->get_node_ptr()->value), key))
        {
            return insert_node_at(rightmost(), np, false);
        }
        else
        {
            auto pos=get_insert_unique_pos(key);
            if(!pos.second)
            {
                destroy(np);
                return pos.first.first;
            }
            return insert_node_at(pos.first.first, np, pos.first.second);
        }
    }
    return insert_unique_use_hint(hint, key, np);
}

/* 插入元素，节点键值允许重复 */
template <class T, class Compare>
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::insert_multi(const value_type& value)
{
    THROW_LENGTH_ERROR_IF(node_count_>max_size()-1, "rb_tree<T, Compare>'s size too big");
    auto res=get_insert_multi_pos(value_traits::get_key(value));
    return insert_value_at(res.first, value, res.second);
}

// 插入新值，节点键值不允许重复，返回一个 pair
// 若插入成功，pair 的第二参数为 true，否则为 false
template <class T, class Compare>
orange_stl::pair<typename rb_tree<T, Compare>::iterator, bool>
rb_tree<T, Compare>::insert_unique(const value_type& value)
{
    THROW_LENGTH_ERROR_IF(node_count_>max_size()-1, "rb_tree<T, Compare>'s size too big");
    auto res=get_insert_unique_pos(value_traits::get_key(value));
    if(res.second)
    {
        return orange_stl::make_pair(res.first.first, false);
    }
    return orange_stl::make_pair(res.first.fitst, false);
}

/* 删除hint位置的节点 */
template <class T, class Compare>
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::erase(iterator hint)
{
    auto node = hint.node->get_node_ptr();
    iterator next(node);
    ++next;

    rb_tree_erase_reblance(hint.node, root(), leftmost(), rightmost());
    destroy_node(node);
    --node_count_;
    return next;
}

/* 删除键值等于key的元素，返回删除的个数 */
template <class T, class Compare>
typename rb_tree<T, Compare>::size_type
rb_tree<T, Compare>::erase_multi(const key_type& key)
{
    auto p=equal_range_multi(key);
    size_type n=orange_stl::distance(p.first, p.second);
    erase(p.first, p.second);
    return n;
}

/* 删除键值等于key的元素，返回删除的个数 */
template <class T, class Compare>
typename rb_tree<T, Compare>::size_type
rb_tree<T, Compare>::erase_unique(const key_type& key)
{
    auto it=find(key);
    if(it!=end())
    {
        erase(it);
        return 1;
    }
    return 0;
}

/* 删除[first, last)区间内的元素 */
template <class T, class Compare>  
void rb_tree<T, Compare>::erase(iterator first, iterator last)
{
    if(first == begin() && last==end())
    {
        clear();
    }
    else
    {
        while(first!=last)
            erase(first++);
    }
}

/* 清空rb_tree */
template <class T, class Compare>
void rb_tree<T, Compare>::clear()
{
    if(node_count_!=0)
    {
        erase_since(root());
        leftmost() = header_;
        root=nullptr;
        rightmost() = header_;
        node_count_=0;
    }
}

/* 查找键值为key的节点，返回指向它的迭代器 */
template <class T, class Compare>   
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::find(const key_type& key)
{
    auto y = header_;
    auto x = root();
    while(x!=nullptr)
    {
        if(!key_comp_(value_traits::get_key(x->get_node_ptr()->value), key))
        {
            /* key小于等于x的键值，向左走 */
            y = x;
            x=x->left;
        }
        else
        {
            /* key大于x键值，向右走 */
            x=x->right;
        }
    }
    iterator j=iterator(y);
    return (j==end() || key_comp_(key, value_traits::get_key(*j)))?end():j;
}

template <class T, class Compare>   
typename rb_tree<T, Compare>::const_iterator
rb_tree<T, Compare>::find(const key_type& key) const
{
    auto y=header_; /* 最后一个不小于key的节点 */
    auto x=root();
    while(x!=nullptr)
    {
        if(!key_comp_(value_traits::get_key(x->get_node_ptr()->value), key))
        {
            /* key小于等于x键值， 向左走 */
            y=x;
            x=x->left;
        }
        else
        {
            /* key大于x键值，向右走 */
            x=x->right;
        }
    }
    const_iterator j = const_iterator(y);
    return (j==end() || key_comp_(key, value_traits::get_key(*j)))?end():j;
}

/* 键值不小于key的第一个位置 */
template <class T, class Compare>
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::lower_bound(const key_type& key)
{
    auto y=header_;
    auto x=root();
    while(x!=nullptr)
    {
        if(!key_comp_(value_traits::get_key(x->get_node_ptr()->value), key))
        {
            /* key <= x */
            y=x;
            x=x->left;
        }
        else
        {
            x=x->right;
        }
    }
    return iterator(y);
}

template <class T, class Compare>
typename rb_tree<T, Compare>::const_iterator
rb_tree<T, Compare>::lower_bound(const key_type& key) const
{
    auto y=header_;
    auto x=root();
    while(x!=nullptr)
    {
        if(!key_comp_(value_traits::get_key(x->get_node_ptr()->value), key))
        {
            /* key <= x */
            y=x;
            x=x->left;
        }
        else
        {
            x=x->right;
        }
    }
    return const_iterator(y);
}

/* 键值不小于key的最后一个位置 */
template <class T, class Compare>
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::upper_bound(const key_type& key)
{
    auto y=header_;
    auto x=root();
    while(x!=nullptr)
    {
        if(key_comp_(key, value_traits::get_key(x->get_node_ptr()->value)))
        {
            /* key < x */
            y=x;
            x=x->left;
        }
        else
        {
            x=x->right;
        }
    }
    return iterator(y);
}

template <class T, class Compare>
typename rb_tree<T, Compare>::const_iterator
rb_tree<T, Compare>::upper_bound(const key_type& key) const
{
    auto y=header_;
    auto x=root();
    while(x!=nullptr)
    {
        if(!key_comp_(key, value_traits::get_key(x->get_node_ptr()->value)))
        {
            /* key <= x */
            y=x;
            x=x->left;
        }
        else
        {
            x=x->right;
        }
    }
    return const_iterator(y);
}

/* 交换rb_tree */
template <class T, class Compare>
void rb_tree<T, Compare>::swap(rb_tree& rhs) noexcept
{
    if(this != &rhs)
    {
        orange_stl::swap(header_, rhs.header_);
        orange_stl::swap(node_count_, rhs.node_count_);
        orange_stl::swap(key_comp_, rhs.key_comp_);
    }
}

/* 辅助函数 */

/* 创建一个节点 */
template <class T, class Compare>
template <class ...Args>
typename rb_tree<T, Compare>::node_ptr
rb_tree<T, Compare>::create_node (Args&&... args)
{
    auto tmp=node_allocator::allocate(1);
    try
    {
        data_allocator::construct(orange_stl::address_of(tmp->value), orange_stl::forward<Args>(args)...);
        tmp->left = nullptr;
        tmp->right = nullptr;
        tmp->parent = nullptr;
    }
    catch(...)
    {
        node_allocator::deallocate(tmp);
        throw;
    }
    return tmp;
}

/* 复制一个结点 */
template <class T, class Compare>
typename rb_tree<T, Compare>::node_ptr
rb_tree<T, Compare>::clone_node(base_ptr x)
{
    node_ptr tmp=create_node(x->get_node_ptr()->value);
    tmp->color=x->color;
    tmp->left=nullptr;
    tmp->right=nullptr;
    return tmp;
}

/* 销毁一个结点 */
template <class T, class Compare>
void rb_tree<T, Compare>::destroy_node (node_ptr p)
{
    data_allocator::destroy(&p->value);
    node_allocator::deallocate(p);
}

/* 初始化容器 */
template <class T, class Compare>
void rb_tree<T, Compare>::rb_tree_init()
{
    header_ = base_allocator::allocate(1);
    header_->color = rb_tree_red;
    root() = nullptr;
    leftmost() = header_;
    rightmost() = header_;
    node_count_ = 0;
}

/* reset函数 */
template <class T, class Compare>
void rb_tree<T, Compare>::reset()
{
    header = nullptr;
    node_count_ = 0;
}

/* get_insert_multi_pos函数 */
template <class T, class Compare>
orange_stl::pair<typename rb_tree<T, Compare>::base_ptr, bool>
rb_tree<T, Compare>::get_insert_multi_pos (const key_type& key)
{
    auto x = root();
    auto y = header_;
    bool add_to_left = true;
    while(x!=nullptr)
    {
        y=x;
        add_to_left = key_comp_(key, value_traits::get_key(x->get_node_ptr()->value));
        x=add_to_left?x->left:x->right;
    }
    return orange_stl::make_pair(y, add_to_left);
}

/* get_insert_unique_pos函数 */
template <class T, class Compare>
orange_stl::pair<orange_stl::pair<typename rb_tree<T, Compare>::base_ptr, bool>, bool>
rb_tree<T, Compare>::get_insert_unique_pos (const key_type& key)
{
    // 返回一个 pair，第一个值为一个 pair，包含插入点的父节点和一个 bool 表示是否在左边插入，
    // 第二个值为一个 bool，表示是否插入成功
    auto x = root();
    auto y = header_;
    bool add_to_left = true; /* 树为空的时候，也在左边插入 */
    while(x!=nullptr)
    {
        y=x;
        add_to_left = key_comp_(key, value_traits::get_key(x->get_node_ptr()->value));
        x = add_to_left?x->left:x->right;
    }
    iterator j=iterator(y); /* 此时y为插入点的父节点 */
    if(add_to_left)
    {
        if(y==header_ || j==begin())
        {
            /* 如果树为空树或者插入点在最左节点处，肯定可以插入新的结点 */
            return orange_stl::make_pair(orange_stl::make_pair(y, true), true);
        }
        else
        {
            /* 否则，如果存在重复结点，那么--j就是重复的值 */
            --j;
        }
    }
    if(key_comp_(value_traits::get_key(*j), key))
    {
        /* 表明新节点没有重复 */
        return orange_stl::make_pair(orange_stl::make_pair(y, add_to_left), true);
    }
    /* 表示新节点与现有结点值重复 */
    return orange_stl::make_pair(orange_stl::make_pair(y, add_to_left), false);
}

/* insert_value_at 函数 */
/* x为插入点的父节点，value为要插入的值，add_to_left表示是否在左边插入 */
template <class T, class Compare>
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::insert_value_at (base_ptr x, const value_type& value, bool add_to_left)
{
    node_ptr node = create_node(value);
    node->parent = x;
    auto base_node = node->get_base_ptr();
    if(x == header_)
    {
        root() = base_node;
        leftmost() = base_node;
        rihgtmost() = base_node;
    }
    else if(add_to_left)
    {
        x->left=base_node;
        if(leftmost()==x)
            leftmost()=base_node;
    }
    else
    {
        x->right = base_node;
        if(rightmost()==x)
            rightmost()=base_node;
    }
    rb_tree_insert_rebalance(base_node, root());
    ++node_count_;
    return iterator(node);
}

/* 在x结点处插入新的结点
    x为插入点的父节点，node为要插入的结点，add_to_left表示是否在左边插入 */
template <class T, class Compare>
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::insert_node_at(base_ptr x, node_ptr node, bool add_to_left)
{
    node->parent=x;
    auto base_node = node->get_base_ptr();
    if(x==header_)
    [
        root()=base_node;
        leftmost()=base_node;
        rightmost()=base_node;
    ]
    else if(add_to_left)
    {
        x->left = base_node;
        if(left_most()==x)
            leftmost()=base_node;
    }
    else
    {
        x->right=base_node;
        if(rightmost()==x)
            rightmost()=base_node;
    }
    rb_tree_insert_rebalance(base_node, root());
    ++node_count_;
    return iterator(node);
}

/* 插入元素，键值允许重复，使用hint */
template <class T, class Compare>
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::insert_multi_use_hint(iterator hint, key_type key, node_ptr node)
{
    /* 在hint附近寻找可插入的位置 */
    auto np=hint.node;
    auto before = hint;
    --before;
    auto bnp=before.node;
    if(!key_comp_(key, value_traits::get_key(*before)) &&
       !key_comp_(value_traits::get_key(*hint), key))
    {
        /* before<=key<=hint */
        if(bnp->right == nullptr)
        {
            return insert_node_at(bnp, node, false);
        }
        else if(np->left==nullptr)
        {
            return insert_node_at(np, node, true);
        }
    }
    auto pos = get_insert_multi_pos(key);
    return insert_node_at(pos.first, node, pos.second);
}

/* 插入元素，键值不允许重复，使用hint */
template <class T, class Compare>
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::insert_unique_use_hint(iterator hint, key_type key, node_ptr node)
{
    /* 在hint附近寻找可以插入的位置 */
    auto np = hint.node;
    auto before = hint;
    --before;
    auto bnp = before.node;
    if(key_comp_(value_traits::get_key(*before), key) &&
       key_comp_(key, value_traits::get_key(*hint)))
    {
        /* before<node<hint */
        if(bnp->right == nullptr)
        {
            return insert_node_at(bnp, node, false);
        }
        else if(np->left == nullptr)
        {
            return insert_node_at(np, node, true);
        }
    }
    auto pos = get_insert_unique_pos(key);
    if(!pos.second)
    {
        destroy_node(node);
        return pos.first.first;
    }
    return insert_node_at(pos.first.first, node, pos.first.second);
}

/* copy_from 函数 */
/* 递归的复制一棵树，节点冲x开始，p为x的父节点 */
template <class T, class Compare>
typename rb_tree<T, Compare>::base_ptr
rb_tree<T, Compare>::copy_from (base_ptr x, base_ptr p)
{
    auto top = clone_node(x);
    top->parent = p;
    try
    {
        if(x->right)
            top->right = copy_from(x->right, top);
        p=top;
        x=x->left;
        while(x!=nullptr)
        {
            auto y=clone_node(x);
            p->left=y;
            y->parent = p;
            if(x->right)
                y->right=copy_from(x->right, y);
            p=y; 
            x=x->left;
        }
    }
    catch(...)
    {
        erase_since(top);
        throw;
    }
    return top;
}

/* erase_since 函数 */
/* 从x节点开始删除该节点及其子树 */
template <class T, class Compare>
void rb_tree<T, Compare>::erase_since (base_ptr x)
{
    while(x!=nullptr)
    {
        erase_since(x->right);
        auto y=x->left;
        destroy_node(x->get_node_ptr());
        x=y;
    }
}

/* 重载比较操作符 */
template <class T, class Compare>
bool operator==(const rb_tree<T, Compare>& lhs, const rb_tree<T, Compare>& rhs)
{
    return lhs.size() == rhs.size() && orange_stl::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <class T, class Compare>
bool operator<(const rb_tree<T, Compare>& lhs, const rb_tree<T, Compare>& rhs)
{
    return orange_stl::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class T, class Compare>
bool operator!=(const rb_tree<T, Compare>& lhs, const rb_tree<T, Compare>& rhs)
{
    return !(lhs==rhs);
}

template <class T, class Compare>
bool operator>(const rb_tree<T, Compare>& lhs, const rb_tree<T, Compare>& rhs)
{
    return rhs<lhs;
}

template <class T, class Compare>
bool operator<=(const rb_tree<T, Compare>& lhs, const rb_tree<T, Compare>& rhs)
{
    return !(rhs < lhs);
}

template <class T, class Compare>
bool operator>=(const rb_tree<T, Compare>& lhs, const rb_tree<T, Compare>& rhs)
{
    return !(lhs < rhs);
}

/* 重载mystl的swap */
template <class T, class Compare>
void swap(rb_tree<T, Compare>& lhs, rb_tree<T, Compare>& rhs) noexcept
{
    lhs.swap(rhs);
}


}   /* orange_stl */

#endif // !__ORANGE__RB_TREE_H__