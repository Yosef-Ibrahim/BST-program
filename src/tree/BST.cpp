#include <tree/BST.h>

// == Constructor / Destructor ==
template <typename T>
BST<T>::BST()
    : root(nullptr), nodeCount(0) {}

template <typename T>
BST<T>::~BST() {
    clearTree();
}

// == Core Operations ==
template <typename T>
void BST<T>::insert(T val, std::string& rotMsg) {
    root = insertRec(root, val, rotMsg);
}

template <typename T>
void BST<T>::remove(T val, std::string& rotMsg) {
    root = removeRec(root, val, rotMsg);
}

template <typename T>
void BST<T>::clearTree() {
    clearRec(root);
    root = nullptr;
    nodeCount = 0;
}

// == Getters ==
template <typename T>
Node<T>* BST<T>::getRoot() const { return root; }

template <typename T>
int BST<T>::getNodeCount() const { return nodeCount; }

template <typename T>
int BST<T>::getTreeHeight() const { return height(root); }


// == Internal AVL logic ==
template <typename T>
int BST<T>::height(Node<T>* N) {
    if (N == nullptr) return 0;
    return N->height;
}

template <typename T>
int BST<T>::getBalance(Node<T>* N) {
    if (N == nullptr) return 0;
    return height(N->left) - height(N->right);
}

template <typename T>
Node<T>* BST<T>::rotateRight(Node<T>* y) {
    Node<T>* x = y->left;
    y->left = x->right;
    x->right = y;

    y->height = std::max(height(y->left), height(y->right)) + 1;
    x->height = std::max(height(x->left), height(x->right)) + 1;

    y->isRotating = true; y->highlightVal = 1.0f;
    x->isRotating = true; x->highlightVal = 1.0f;

    return x;
}

template <typename T>
Node<T>* BST<T>::rotateLeft(Node<T>* y) {
    Node<T>* x = y->right;
    y->right = x->left;
    x->left = y;

    y->height = std::max(height(y->left), height(y->right)) + 1;
    x->height = std::max(height(x->left), height(x->right)) + 1;

    y->isRotating = true; y->highlightVal = 1.0f;
    x->isRotating = true; x->highlightVal = 1.0f;

    return x;
}

template <typename T>
Node<T>* BST<T>::balance(Node<T>* node, std::string& rotMsg) {
    int balanceFactor = getBalance(node);

    if (balanceFactor > 1) {
        if (getBalance(node->left) < 0) {
            node->left = rotateLeft(node->left);
            rotMsg = "Left-Right Rotation";
        } else {
            rotMsg = "Right Rotation";
        }
        return rotateRight(node);
    }

    if (balanceFactor < -1) {
        if (getBalance(node->right) > 0) {
            node->right = rotateRight(node->right);
            rotMsg = "Right-Left Rotation";
        } else {
            rotMsg = "Left Rotation";
        }
        return rotateLeft(node);
    }

    return node;
}

template <typename T>
Node<T>* BST<T>::insertRec(Node<T>* node, T val, std::string& rotMsg) {
    if (node == nullptr) {
        nodeCount++;
        searchPath.push_back(val);
        return new Node<T>(val);
    }

    searchPath.push_back(node->data);

    if (val < node->data)
        node->left = insertReec(node->left, val, rotMsg);
    else if (val > node->data)
        node->right = insertReec(node->right, val, rotMsg);
    else
        return node;

    node->height = 1 + std::max(height(node->left), height(node->right));
    return balance(node, rotMsg)
}

template <typename T>
Node<T>* BST<T>::removeRec(Node<T>* node, T val, std::string& rotMsg) {
    if (node == nullptr) return node;

    if (val < node->data) {
        searchPath.push_back(node->data);
        node->left = removeRec(node->left, val, rotMsg);
    } else if (val > node->data) {
        searchPath.push_back(node->data);
        node->right = removeRec(node->right, val, rotMsg);
    } else {
        if (node->left == nullptr && node->right == nullptr) {
            delete node;
        } else if ((node->left != nullptr) && (node->right != nullptr)) {
            Node<T>* temp = node->right;
            while (temp->left != nullptr)
                temp = temp->left;
            node->data = temp->data;
            node->right = removeRec(node->right, temp->data, rotMsg);
        } else {
            Node<T>* temp = node->left ? node->left : node->right;
            node = *temp;
            delete temp;
        }

        nodeCount--;
    }

    if (node == nullptr) return node;

    node->height = 1 + std::max(height(node->left), height(node->right));
    return balance(node, rotMsg);
}

template <typename T>
void BST<T>::clearRec(Node<T>* node) {
    if (node == nullptr) return;
    clearRec(node->left);
    clearRec(node->right);
    delete node;
}

// == Search & Traversals ==
template <typename T>
int BST<T>::getPredecessor(T val) {
    searchPath.clear();

    Node<T>* curr = root;
    Node<T>* pred = nullptr;

    while (curr != nullptr) {
        searchPath.push_back(curr->data);
        if (curr->data < val) {
            pred = curr;
            curr = curr->right;
        } else {
            curr = curr->left;
        }
    }
    return pred ? pred->data : -1;
}
// == Search & Traversals ==
template <typename T>
int BST<T>::getSuccessor(T val) {
    searchPath.clear();

    Node<T>* curr = root;
    Node<T>* succ = nullptr;

    while (curr != nullptr) {
        searchPath.push_back(curr->data);
        if (curr->data > val) {
            succ = curr;
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }
    return succ ? succ->data : -1;
}

template <typename T>
std::string BST<T>::getInOrderString() const {
    std::string res;
    inOrderRec(root, res);
    return res.empty() ? "Tree is empty" : res;
}

template <typename T>
void BST<T>::inOrderRec(Node<T>* node, std::string& res) const {
    if (!node) return;
    inOrderRec(node->left, res);
    res += std::to_string(node->data) + " ";
    inOrderRec(node->right, res);
}

template <typename T>
std::string BST<T>::getPreOrderString() const {
    std::string res;
    preOrderRec(root, res);
    return res.empty() ? "Tree is empty" : res;
}

template <typename T>
void BST<T>::preOrderRec(Node<T>* node, std::string& res) const {
    if(!node) return;
    res += std::to_string(node->data) + " ";
    preOrderRec(node->left, rest);
    preOrderRec(node->right, rest);
}

template <typename T>
std::string BST<T>::getPostOrderString() const {
    std::string res;
    postOrderRec(root, res);
    return res.empty() ? "Tree is empty" : res;
}

template <typename T>
void BST<T>::postOrderRec(Node<T>* node, std::string& res) const {
    if(!node) return;
    preOrderRec(node->left, rest);
    preOrderRec(node->right, rest);
    res += std::to_string(node->data) + " ";
}

template <typename T>
std::string BST<T>::getBreadthFirstString() const {
    std::string res;
    
    std::queue<Node<T>*> q;
    q.push(root);

    while (!q.empty()) {
        Node<T>* node = q.front();
        q.pop();

        res += std::to_string(node->data) + " ";

        if (node->left)
            q.push(node->left);
        if (node->right)
            q.push(node->right);
    }

    return res.empty() ? "Tree is empty" : res;
}

template <typename T>
void BST<T>::tickAnimations(float dt, float speed) {
    if (!root) return;

    // Bottom left = (0, 0)
    // Top right = (1, 1)
    // This is to mimic UV texture cordination of OpenGL (which raylib is actually a wrapper for it BRUH!!)
    updateTargets(root, 0, 0.5f, 0.25f);

    animateNode(root, dt, speed);
}

template <typename T>
void BST<T>::updateTargets(Node<T>* node, int depth, float targetX, float offset) {
    if (!node) return;

    // is the pure level where the renederer handle its value
    float targetY = static_cast<float>(depth);

    node->cur_x += (targetX - node->cur_x) * 0.1f;
    node->cur_y += (targetY - node->cur_y) * 0.1f;

    if (node->left)
        updateTargets(node->left, depth + 1, targetX - offset, offset * 0.5f);
    if (node->right)
        updateTargets(node->right, depth + 1, targetX - offset, offset * 0.5f);
}

template <typename T>
void BST<T>::animateNode(Node<T>* node, float dt, float speed) {
    if (!node) return;

    node->alpha = std::clamp(node->alpha, 0.0f, 1.0f);

    // New node animation
    if (node->isNew) {
        node->pulseTimer = std::clamp(node->pulseTimer - dt * speed, 0.0f, 1.0f);
        if (node->pulseTimer == 0.0f) node->isNew = false;
    }

    // Rotated node animation
    if (node->isRotating) {
        node->highlightVal = std::clamp(node->highlightVal - dt * (speed * 0.5f), 0.0f, 1.0f);
        if (node->highlightVal == 0.0f) node->isRotating = false;
    }

    animateNode(node->left, dt, speed);
    animateNode(node->right, dt, speed);
}
// == Explicit Template Instantiation ==
template class BST<int>;