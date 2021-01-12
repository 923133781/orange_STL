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
        return &*this;
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


}   /* orange_stl */

#endif // !__ORANGE__RB_TREE_H__