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
int BST<T>::height(Node<T>* N) const {
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
        Node<T>* newNode = new Node<T>(val);
        // FIX: properly initialize animation state for new nodes
        newNode->alpha      = 0.0f;   // fade in from transparent
        newNode->isNew      = true;
        newNode->pulseTimer = 1.0f;   // full pulse, counts down
        newNode->cur_x      = 0.5f;   // start at center, lerps to target
        newNode->cur_y      = 0.0f;
        return newNode;
    }

    searchPath.push_back(node->data);

    if (val < node->data)
        node->left = insertRec(node->left, val, rotMsg);
    else if (val > node->data)
        node->right = insertRec(node->right, val, rotMsg);
    else
        return node; // duplicate – no-op

    node->height = 1 + std::max(height(node->left), height(node->right));
    return balance(node, rotMsg);
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
        // Found the node to delete
        nodeCount--;

        if (node->left == nullptr || node->right == nullptr) {
            // 0 or 1 child – replace with child (or nullptr)
            Node<T>* child = node->left ? node->left : node->right;
            delete node;
            return child; // early return: no height/balance update needed on deleted node
        } else {
            // 2 children: replace with in-order successor (leftmost of right subtree)
            Node<T>* temp = node->right;
            while (temp->left != nullptr)
                temp = temp->left;
            node->data = temp->data;
            // FIX: restore nodeCount since removeRec will decrement again for the successor
            nodeCount++;
            node->right = removeRec(node->right, temp->data, rotMsg);
        }
    }

    // 'node' is still valid here (only early-returned on 0/1-child case above)
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
    if (!node) return;
    res += std::to_string(node->data) + " ";
    preOrderRec(node->left, res);
    preOrderRec(node->right, res);
}

template <typename T>
std::string BST<T>::getPostOrderString() const {
    std::string res;
    postOrderRec(root, res);
    return res.empty() ? "Tree is empty" : res;
}

template <typename T>
// FIX: was calling preOrderRec on children instead of postOrderRec
void BST<T>::postOrderRec(Node<T>* node, std::string& res) const {
    if (!node) return;
    postOrderRec(node->left, res);
    postOrderRec(node->right, res);
    res += std::to_string(node->data) + " ";
}

template <typename T>
std::string BST<T>::getBreadthFirstString() const {
    if (!root) return "Tree is empty";

    std::string res;
    std::queue<Node<T>*> q;
    q.push(root);

    while (!q.empty()) {
        Node<T>* node = q.front();
        q.pop();

        res += std::to_string(node->data) + " ";

        if (node->left)  q.push(node->left);
        if (node->right) q.push(node->right);
    }

    return res;
}

template <typename T>
void BST<T>::tickAnimations(float dt, float speed) {
    if (!root) return;

    // Normalized layout: root at (0.5, 0), children spread by offset=0.25
    updateTargets(root, 0, 0.5f, 0.25f, dt, speed);
    animateNode(root, dt, speed);
}

template <typename T>
// FIX: lerp speed is now dt*speed-proportional so it's frame-rate independent
void BST<T>::updateTargets(Node<T>* node, int depth, float targetX, float offset, float dt, float speed) {
    if (!node) return;

    float targetY = static_cast<float>(depth);

    // Frame-rate independent lerp: factor scales with dt and speed
    float lerpFactor = std::clamp(dt * speed * 8.0f, 0.0f, 1.0f);
    node->cur_x += (targetX - node->cur_x) * lerpFactor;
    node->cur_y += (targetY - node->cur_y) * lerpFactor;

    if (node->left)
        updateTargets(node->left,  depth + 1, targetX - offset, offset * 0.5f, dt, speed);
    if (node->right)
        updateTargets(node->right, depth + 1, targetX + offset, offset * 0.5f, dt, speed);
}

template <typename T>
void BST<T>::animateNode(Node<T>* node, float dt, float speed) {
    if (!node) return;

    // Fade-in animation for new nodes
    if (node->isNew) {
        // alpha ramps up to 1.0 quickly
        node->alpha = std::clamp(node->alpha + dt * speed * 3.0f, 0.0f, 1.0f);

        // pulseTimer counts down independently to drive a scale/glow pulse in renderer
        node->pulseTimer = std::clamp(node->pulseTimer - dt * speed * 1.5f, 0.0f, 1.0f);
        if (node->pulseTimer == 0.0f) node->isNew = false;
    } else {
        node->alpha = 1.0f; // fully opaque once settled
    }

    // Rotation highlight fades out at half speed
    if (node->isRotating) {
        node->highlightVal = std::clamp(node->highlightVal - dt * speed * 0.5f, 0.0f, 1.0f);
        if (node->highlightVal == 0.0f) node->isRotating = false;
    }

    animateNode(node->left,  dt, speed);
    animateNode(node->right, dt, speed);
}

// == Camera Focus Helpers ==

template <typename T>
Node<T>* BST<T>::findNode(T val) const {
    return findNodeRec(root, val);
}

template <typename T>
Node<T>* BST<T>::findNodeRec(Node<T>* node, T val) const {
    if (!node) return nullptr;
    if (val == node->data) return node;
    if (val < node->data)  return findNodeRec(node->left,  val);
    return                        findNodeRec(node->right, val);
}

template <typename T>
bool BST<T>::computeTargetPos(T val, float& outX, float& outY) const {
    // Mirror the same layout parameters used by updateTargets / tickAnimations
    return computeTargetPosRec(root, val, 0, 0.5f, 0.25f, outX, outY);
}

template <typename T>
bool BST<T>::computeTargetPosRec(Node<T>* node, T val,
                                  int depth, float targetX, float offset,
                                  float& outX, float& outY) const {
    if (!node) return false;

    if (val == node->data) {
        outX = targetX;
        outY = static_cast<float>(depth);
        return true;
    }

    if (val < node->data)
        return computeTargetPosRec(node->left,  val, depth + 1, targetX - offset, offset * 0.5f, outX, outY);
    return     computeTargetPosRec(node->right, val, depth + 1, targetX + offset, offset * 0.5f, outX, outY);
}

// == Explicit Template Instantiation ==
template class BST<int>;