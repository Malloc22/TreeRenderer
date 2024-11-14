//---------------------------

#ifndef MAP_HPP_INCLUDED
#define MAP_HPP_INCLUDED

//---------------------------

#include <utility>
#include <iomanip>

#include <map>
#include <queue>

//---------------------------

template <class Key, class Data>
struct Node {

    Key key;
    Data data;

    unsigned char height;

    Node* left = nullptr;
    Node* right = nullptr;

    void node(Key key, Data data) { this->key = key; this->data = data; left=right=0; height= 1; }
};

//---------------------------

template <class TKey, class TData>
struct DataS {
    Node<TKey, TData>* node;
    int level = 0;
    int state;  //0 - root, 1 - left, 2 - right
};

//---------------------------

template <class Key, class Data>
class Map {
public:

    Map() { pRoot = nullptr; }
    ~Map() { this->clear(); }

    //---------------------------

    bool add(const Key& key, const Data& data) {
        return this->add({ key, data });
    }

    //---------------------------

    bool add(const std::pair<Key, Data>& pair) {

        bool contains = false;
        pRoot = addNode(pair, pRoot, contains);

        return !contains;
    }

    //---------------------------

    Data* get(const Key& key) {

        Node<Key, Data>* node = getNode(key);
        if(!node)
            return nullptr;

        return &node->data;
    }

    //---------------------------

    bool remove(const Key& key) {

        bool contains = false;
        pRoot = remove(pRoot, key, contains);

        return contains;
    }

    //---------------------------

    void clear() {

        if(!pRoot)
            return;

        removeAll(pRoot);
        pRoot = nullptr;
    }

    //---------------------------

    std::string inorder() {

        std::string data;
        inorder(pRoot, data);

        data += " end";

        return data;
    }

    //---------------------------

    std::string preorder() {

        std::string data;
        preorder(pRoot, data);

        data += " end";

        return data;
    }

    //---------------------------

    std::string postorder() {

        std::string data;
        postorder(pRoot, data);

        data += " end";

        return data;
    }

    //---------------------------

    std::string getPrintHorizontal() {

        if(!pRoot)
            return "";

        std::string data;
        printHorizontal(pRoot, data);

        return data;
    }

    //---------------------------

    std::string getPrintVertical() {

        if(!pRoot)
            return "";

        std::string data;
        printVertical(pRoot, data);

        return data;
    }

    //---------------------------

    std::vector<DataS<Key, Data>> getTree() {

        std::vector<DataS<Key, Data>> buff;

        pDCount = 0;

        if (pRoot != nullptr) {
            DataS<Key, Data> data;

            data.node = pRoot;
            data.level = pDCount;
            data.state = 0;

            buff.push_back(data);
            tree(pRoot, buff);
        }

        return buff;
    }

    //---------------------------

    size_t getCountElement(Data data) {

        size_t counter = 0;
        this->preorderFind(pRoot, data, counter);

        return counter;
    }

    //---------------------------

    void debugPrint() {
        std::cout << "tree print:\n";

        pDCount = 0;

        if (pRoot != nullptr) {

            std::cout << "[+]={" + std::to_string(pRoot->key) + ":" + pRoot->data + "}\n";
            debug(pRoot);
        }
    }

    //---------------------------

private:

    Node<Key, Data>* pRoot;
    int pDCount;

    //---------------------------

    void debug(Node<Key, Data>* node) {

        pDCount += 2;

        if (node != nullptr) {

            std::string str = "";
            for (int i = 0; i < pDCount; ++i)
                str += " ";

            if (node->left != nullptr)
                std::cout << str + "[L]={" + std::to_string(node->left->key) + ":" + node->left->data + "}\n";

            debug(node->left);

            if (node->right != nullptr)
                std::cout << str + "[R]={" + std::to_string(node->right->key) + ":" + node->right->data + "}\n";

            debug(node->right);
        }

        pDCount -= 2;
    }

    //---------------------------

    void preorderFind(Node<Key, Data>* node, const Data& data, size_t& count) {

        if(!node) return;

        if(node->data == data)
            ++count;

        preorderFind(node->left, data, count);
        preorderFind(node->right, data, count);
    }

    //---------------------------

    void tree(Node<Key, Data>* node, std::vector<DataS<Key, Data>> &data) {
        pDCount++;

        if (node != nullptr) {

            std::string str = "";
            for (int i = 0; i < pDCount; ++i)
                str += "-";

            if (node->left != nullptr) {

                DataS<Key, Data> buff;
                buff.level = pDCount;
                buff.state = 1;
                buff.node = node->left;

                data.push_back(buff);
            }

            tree(node->left, data);

            if (node->right != nullptr) {

                DataS<Key, Data> buff;
                buff.level = pDCount;
                buff.state = 2;
                buff.node = node->right;

                data.push_back(buff);
            }

            tree(node->right, data);
        }

        pDCount--;
    }

    //---------------------------

    void printHorizontal(Node<Key, Data>* node, std::string& data) {

        if(!node)
            return;

        std::queue<Node<Key, Data>*> q;
        q.push(node);

        while (!q.empty()) {
            Node<Key, Data>* current = q.front();
            q.pop();

            data += std::to_string(current->key) + " ";

            if (current->left) q.push(current->left);
            if (current->right) q.push(current->right);
        }
    }

    //---------------------------

    void printVertical(Node<Key, Data>* node, std::string& data) {

        if(!node)
            return;

        std::map<int, std::vector<Node<Key, Data>*>> nodes;
        std::queue<std::pair<Node<Key, Data>*, int>> q; // Пара - узел и его горизонтальный уровень
        q.push({node, 0});

        while (!q.empty()) {
            auto current = q.front();
            q.pop();
            Node<Key, Data>* node = current.first;
            int level = current.second;

            nodes[level].push_back(node);

            if (node->left) q.push({node->left, level - 1});
            if (node->right) q.push({node->right, level + 1});
        }

        // Вывод значений в порядке вертикального обхода
        for (const auto& pair : nodes) {
            for (const auto& val : pair.second) {
                data += std::to_string(val->key) + " ";
            }
            data += "\n"; // Перевод строки для каждого уровня
        }

    }

    void inorder(Node<Key, Data>* node, std::string& data) {
        if(!node) return;

        inorder(node->left, data);
        data += "{" + std::to_string(node->key) + ":" + node->data + "} --> ";
        inorder(node->right, data);
    }

    //---------------------------

    void preorder(Node<Key, Data>* node, std::string& data) {

        if(!node) return;

        data += "{" + std::to_string(node->key) + ":" + node->data + "} --> ";
        preorder(node->left, data);
        preorder(node->right, data);
    }

    //---------------------------

    void postorder(Node<Key, Data>* node, std::string& data) {
        if(!node) return;

        preorder(node->left, data);
        preorder(node->right, data);

        data += "{" + std::to_string(node->key) + ":" + node->data + "} --> ";
    }

    void removeAll(Node<Key, Data>* node) {
        if(node != nullptr) {
            removeAll(node->left);
            removeAll(node->right);

            delete node;
            node = nullptr;
        }
    }

    Node<Key, Data>* addNode(const std::pair<Key, Data>& item, Node<Key, Data>* node, bool& contains) {

        if (!node) {
            node = new Node<Key, Data>();

            node->key = item.first;
            node->data = item.second;
        }

        else if ( item.first < node->key)
            node->left = addNode(item, node->left, contains);

        else if ( item.first > node->key )
            node->right = addNode(item, node->right, contains);

        else
            contains = true;

        return balance(node);
    }

    //---------------------------

    unsigned char height(Node<Key, Data>* p) {
        return p ? p->height : 0;
    }

    //---------------------------

    int balanceFactor(Node<Key, Data>* p) {
        return height(p->right) - height(p->left);
    }

    //---------------------------

    void fixHeight(Node<Key, Data>* p) {
        unsigned char   hl = height(p->left),
                        hr = height(p->right);

        p->height = (hl > hr ? hl : hr) +1;
    }

    //---------------------------

    Node<Key, Data>* rotateRight(Node<Key, Data>* p) { // правый поворот вокруг p
        Node<Key, Data>* q = p->left;

        p->left = q->right;
        q->right = p;

        this->fixHeight(p);
        this->fixHeight(q);

        return q;
    }

    //---------------------------

    Node<Key, Data>* rotateLeft(Node<Key, Data>* q) { // левый поворот вокруг q

        Node<Key, Data>* p = q->right;

        q->right = p->left;
        p->left = q;

        this->fixHeight(q);
        this->fixHeight(p);

        return p;
    }

    //---------------------------

    Node<Key, Data>* balance(Node<Key, Data>* p) {// балансировка узла p

        this->fixHeight(p);

        if( this->balanceFactor(p) == 2 ) {
            if( this->balanceFactor(p->right) < 0 )
                p->right = rotateRight(p->right);
            return rotateLeft(p);
        }

        if( balanceFactor(p) == -2 ) {
            if( balanceFactor(p->left) > 0  )
                p->left = rotateLeft(p->left);
            return rotateRight(p);
        }
        return p; // балансировка не нужна
    }

    //---------------------------

    Node<Key, Data>* findMin(Node<Key, Data>* node) {
        return node->left != nullptr ? findMin(node->left) : node;
    }

    //---------------------------

    Node<Key, Data>* removeMin(Node<Key, Data>* node) {
        if (node->left == nullptr)
            return node->right;

        node->left = removeMin(node->left);
        return balance(node);
    }

    //---------------------------

    Node<Key, Data>* remove(Node<Key, Data>* node, const Key& key, bool& contains) {
        if (!node)
            return nullptr;

        if ( key < node->key )
            node->left = remove(node->left, key, contains);

        else if ( key > node->key )
            node->right = remove(node->right, key, contains);

        else {
            contains = true;
            Node<Key, Data> *left = node->left,
                            *right = node->right;

            delete node;
            node = nullptr;

            if (!right) return left;

            Node<Key, Data>* min = findMin(right);
            min->right = removeMin(right);
            min->left = left;

            return balance(min);
        }

        return balance(node);
    }

    //---------------------------

    Node<Key, Data>* getNode(const Key& key) {

        Node<Key, Data>* node = findNode(pRoot, key);
        return node;
    }

    //---------------------------

    Node<Key, Data>* findNode(Node<Key, Data>* node, const Key& key) {
        if(!node || key == node->key)
            return node;

        if( key < node->key )
            return findNode(node->left, key);
        else
            return findNode(node->right, key);
    }
};

//---------------------------

#endif // MAP_HPP_INCLUDED

//---------------------------
