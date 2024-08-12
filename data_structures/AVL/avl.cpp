/*
 * AVL tree implementation using C++ 11
 * Compile with C++ 11 support:
 * g++ -std=c++11 avl.cpp
 */

#include <cstdlib>
#include <memory>
#include <queue>
#include <functional>

template <typename T>
class AVL
{
    template <typename V>
    struct Node
    {
        V value;
        unsigned depth, ref;
        std::shared_ptr< Node<V> > left, right;

        Node(const V &v): value(v), depth(1), ref(1) {}

        /*
         * @brief max_depth Returns maximum children depth
         * @param n [IN] - node
         * @return maximum child node depth
         */
        static unsigned max_depth(Node<V> &n)
        {
            unsigned l = n.left ? n.left->depth : 0;
            unsigned r = n.right ? n.right->depth : 0;

            return l > r ? l : r;
        }

        /*
         * @brief rotate_left Balance rotate branch left
         *  A        B
         *   \      /
         *    B => A
         *   /      \
         *  C        C
         */
        static void rotate_left(std::shared_ptr< Node<V> > &n)
        {
            std::shared_ptr< Node<V> > tmp = n->right;
            n->right = tmp->left;
            tmp->left = n;
            n = tmp;

            n->left->depth = max_depth(*n->left) + 1;
            n->depth = max_depth(*n) + 1;
        }

        /*
         * @brief rotate_right Balance rotate branch right
         *    A    B
         *   /      \
         *  B   =>   A
         *   \      /
         *    C    C
         */
        static void rotate_right(std::shared_ptr< Node<V> > &n)
        {
            std::shared_ptr< Node<V> > tmp = n->left;
            n->left = tmp->right;
            tmp->right = n;
            n = tmp;

            n->right->depth = max_depth(*n->right) + 1;
            n->depth = max_depth(*n) + 1;
        }

        /*
         * @brief balance Returns branch balance factor
         * @return balance between left and right branch
         */
        int balance() const
        {
            unsigned l = left ? left->depth : 0;
            unsigned r = right ? right->depth : 0;

            return depth ? (int)(l - r) : 0;
        }

        /*
         * @brief min_node Returns minimal node of the branch
         * @param n [IN] - node
         * @return minimal node
         */
        static std::shared_ptr< Node<V> > min_node(std::shared_ptr< Node<V> > n)
        {
            while (n->left)
                n = n->left;

            return n;
        }

        /*
         * @brief insert Adds a new node to the tree.
         * If node already exists, it's reference counter is incremented.
         * @param n [IN] - tree node
         * @param v [IN] - value to insert
         * @return True if new unique node added or false if node exists
         */
        static bool insert(std::shared_ptr< Node<V> > &n, const V &v)
        {
            bool res = true;

            if (v < n->value)
            {
                if (!n->left)
                    n->left.reset(new Node<V>(v));
                else
                    res = insert(n->left, v);
            }
            else if (v > n->value)
            {
                if (!n->right)
                    n->right.reset(new Node<V>(v));
                else
                    res = insert(n->right, v);
            }
            else
            {
                n->ref++; // Number already in the tree - increase reference counter;
                return false;
            }

            n->depth = max_depth(*n) + 1; // Advance depth

            auto b = n->balance(); // Balance check

            if (b > 1)
            {
                if (v > n->left->value) // RL else RR
                    rotate_left(n->left);

                rotate_right(n);
            }
            else if (b < -1)
            {
                if (v < n->right->value) // LR else LL
                    rotate_right(n->right);

                rotate_left(n);
            }

            return res;
        }

        /*
         * @brief remove Removes element form the tree.
         * @param n   [IN] - tree node
         * @param v   [IN] - value to remove
         * @param ref [IN] - if true, reference counter is considered during deletion
         * @return True if node removed
         */
        static bool remove(std::shared_ptr< Node<V> > &n, const V &v, bool ref)
        {
            bool ret = true;

            if (!n)
                return false; // Value not found

            if (v < n->value)
                ret = remove(n->left, v, ref);
            else if(v > n->value )
                ret = remove(n->right, v, ref);
            else // Node found
            {
                if (ref && n->ref > 1)
                {
                    n->ref--;
                    return false;
                }

                if(!n->left || !n->right) // One or no children
                {
                    auto tmp = n->left ? n->left : n->right;

                    if (!tmp)
                    {
                        n.reset(); // No children
                        return true;
                    }
                    else
                        n = tmp; // One child
                }
                else // Two children
                {
                    auto tmp = min_node(n->right);

                    remove(n->right, tmp->value, false);
                    tmp->right = n->right;
                    tmp->left = n->left;
                    n = tmp;
                }
            }

            n->depth = max_depth(*n) + 1; // Update depth

            auto b = n->balance(); // Balance check

            if (b > 1)
            {
                if (n->left->balance() < 0) // LR else LL
                    rotate_left(n->left);

                rotate_right(n);
            }
            else if (b < -1)
            {
                if (n->right->balance() > 0) // RL else RR
                    rotate_right(n->right);

                rotate_left(n);
            }

            return ret;
        }
    };

    std::shared_ptr< Node<T> > data_;

public:

    /*
     * @brief traverse_cb Tree traversal callback type definition
     * @param value [IN] - node value
     * @param depth [IN] - node level starting from root (1)
     * @param n     [IN] - number of occurrences, i.e. how many times value was inserted
     * @return True if traversal should stop
     */
    typedef std::function<bool(const T &value, unsigned depth, unsigned n)> traverse_cb;

    unsigned nodes, unique_nodes;

    AVL() : nodes(0), unique_nodes(0) {}
    AVL(T &value) : nodes(1), unique_nodes(1)
    {
        data_.reset(new Node<T>(value));
    }
    virtual ~AVL() {}

    /*
     * @brief find Searching for specified value and returns it's number of occurrences (0 if no such value)
     * @param value [IN] - value to search for
     * @return Number of occurrences
     */
    unsigned find(const T &value) const
    {
        if (!data_)
            return false;

        std::shared_ptr< Node<T> > n = data_;

        while(true)
        {
            if (value < n->value)
            {
                if (!n->left)
                    break;

                n = n->left;
            }
            else if (n->value < value)
            {
                if (!n->right)
                    break;

                n = n->right;
            }
            else
                return n->ref;
        }

        return false;
    }

    /*
     * @brief insert Adds new value to the tree
     * @param value [IN] - value to insert
     * @return reference to the current object for method call chaining
     */
    AVL<T>& insert(const T &value)
    {
        nodes++;

        if (!data_)
        {
            data_.reset(new Node<T>(value));
            unique_nodes++;
            return *this;
        }
        else if (Node<T>::insert(data_, value))
            unique_nodes ++;

        return *this;
    }

    /*
     * @brief remove Decreases value reference counter. If reference counter reached 0, node is deleted.
     * @param value [IN] - value to remove
     * @return reference to the current object for method call chaining
     */
    AVL<T>& remove(const T &value)
    {
        nodes -= find(value) ? 1 : 0;

        if (Node<T>::remove(data_, value, true))
            unique_nodes--;

        return *this;
    }

    /*
     * @brief drop Removes value from tree ignoring reference counter.
     * @param value [IN] - value to remove
     * @return reference to the current object for method call chaining
     */
    AVL<T>& drop(const T &value)
    {
        nodes -= find(value);

        if (Node<T>::remove(data_, value, false))
            unique_nodes--;

        return *this;
    }

    void clear()
    {
        data_.reset();
    }

    /*
     * @brief traverse Performs tree traversal from the root using breadth-first search algorithm.
     * @param cb [IN] - callback that receive node data.
     *
     * Note: If callback returns true, traversal is stopped.
     */
    void traverse(traverse_cb cb) const
    {
        std::queue<std::shared_ptr<Node<T> > > q;

        if (!data_ || !cb)
            return;

        q.push(data_);

        bool stop = false;
        unsigned d = 0;

        do
        {
            auto s = q.size();

            stop = s == 0 ? true : false;
            d++;

            for(decltype(s) i = 0; i < s && !stop; i++)
            {
                auto n = q.front();
                stop = cb(n->value, d, n->ref);

                if (n->left)
                    q.push(n->left);
                if (n->right)
                    q.push(n->right);

                q.pop();
            }
        } while(!stop);
    }

    unsigned depth() const
    {
        return data_ ? data_->depth : 0;
    }
};

int main()
{
    AVL<int> tree;
    constexpr const unsigned unique_nodes = 19;
    unsigned nodes = unique_nodes;

    for(int i = 1; i < (int)(nodes + 1); i++)
    {
        tree.insert(i);
    }
    tree.insert(1)
        .insert(2)
        .insert(12)
        .insert(12);
    nodes += 4;
    /*
     *              8
     *             / \
     *            4   12
     *          / |    |  \
     *         2  6   10  16
     *       /|  /|   |\   |  \
     *      1 3 5 7   9 11 14   18
     *                    / \   / \
     *                   13 15 17 19
     */

    if (tree.depth() != 5)
        printf("Tree depth doesn't match\n");

    if (tree.nodes != nodes)
        printf("Node count doesn't match\n");

    if (tree.unique_nodes != unique_nodes)
        printf("Unique node count doesn't match\n");

    if (tree.find(1) != 2 ||
            tree.find(2) != 2 ||
            tree.find(7) != 1 ||
            tree.find(8) != 1 ||
            tree.find(12) != 3 ||
            tree.find(20) != 0 ||
            tree.find(0) != 0)
    {
        printf("Unexpected node count\n");
    }

    tree.remove(2).remove(7).remove(8).drop(12);
    /*
     *              9
     *             / \
     *            4   13
     *          / |    |  \
     *         2  6   10  16
     *       /|  /     \   |  \
     *      1 3 5      11 14   18
     *                     \   / \
     *                     15 17 19
     */

    if (tree.depth() != 5)
        printf("Tree depth doesn't match\n");

    if (tree.nodes != 17)
        printf("Node count doesn't match\n");

    if (tree.unique_nodes != 16)
        printf("Unique node count doesn't match\n");

    if (tree.find(1) != 2 ||
            tree.find(2) != 1 ||
            tree.find(7) != 0 ||
            tree.find(8) != 0 ||
            tree.find(12) != 0 ||
            tree.find(20) != 0 ||
            tree.find(0) != 0)
    {
        printf("Unexpected node count\n");
    }

    tree.traverse([](const int &v, unsigned d, unsigned ref) {
            printf("Level %u: %d (%u occurrences)\n", d, v, ref);
            return false; // Return true to stop
        });

    return 0;
}
