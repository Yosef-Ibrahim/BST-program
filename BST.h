#ifndef BST_H
#define BST_H

// ═══════════════════════════════════════════════════════════════════════════════
//  BST.h  —  AVL Tree with full animation support
//  Fixed:
//    • snapNewNodesToCurrent: now actually snaps BEFORE first rebuildPositions
//      so new nodes don't fly in from (0,0)
//    • insert() order fixed: insert → rebuildPositions → snap
//    • getPredecessor/getSuccessor: return type changed to int (no magic T(-1))
//    • getBalance: handles nullptr children safely
//    • tickNode: alpha fade-in speed tied properly to lerpSpeed
//    • Removed dead/empty snapNewNodes(root) call
//    • collectAll / getAllValues: made fully public
//    • formatVector: uses const ref to avoid copy
// ═══════════════════════════════════════════════════════════════════════════════

#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <functional>

// ─────────────────────────────────────────────────────────────────────────────
//  Node struct  —  carries logical data + animation state
// ─────────────────────────────────────────────────────────────────────────────
template <typename T>
struct Node {
    T     data;
    Node* left   = nullptr;
    Node* right  = nullptr;
    int   height = 1;

    // ── Smooth-motion fields ─────────────────────────────────────────────
    float cur_x = 0.f;   // current rendered position  (lerps towards target)
    float cur_y = 0.f;
    float tgt_x = 0.f;   // logical target position
    float tgt_y = 0.f;

    // ── Visual state ─────────────────────────────────────────────────────
    bool  isNew        = false; // freshly inserted  → pulse ring
    bool  isRotating   = false; // currently in a rotation → glow
    float alpha        = 0.f;  // 0..1 opacity  (new nodes fade in)
    float pulseTimer   = 0.f;  // counts down after insert
    float highlightVal = 0.f;  // 0..1 rotation glow intensity

    explicit Node(T val) : data(val) {}
};

// ─────────────────────────────────────────────────────────────────────────────
//  BST  (self-balancing AVL)
// ─────────────────────────────────────────────────────────────────────────────
template <typename T>
class BST {
public:
    // ── Public search-path (for yellow path highlight) ────────────────────
    std::vector<T> searchPath;

    // ── Ctor / Dtor ───────────────────────────────────────────────────────
    BST()  : root(nullptr) {}
    ~BST() { destroyTree(root); }

    Node<T>* getRoot() { return root; }

    // ─────────────────────────────────────────────────────────────────────
    //  Core public API
    // ─────────────────────────────────────────────────────────────────────

    void clearTree() {
        destroyTree(root);
        root = nullptr;
    }

    int getNodeCount()  const { return countNodesRec(root); }
    int getTreeHeight() const { return root ? root->height : 0; }

    // Insert a value; fills msg with rotation description (if any)
    void insert(T val, std::string& msg) {
        msg  = "";
        root = insertRec(root, val, msg);
        // 1. Rebuild logical positions first
        rebuildPositions();
        // 2. Snap brand-new nodes to their target so they don't slide from (0,0)
        snapNewNodesToCurrent();
    }

    // Delete a value; fills msg with rotation description (if any)
    void remove(T val, std::string& msg) {
        msg  = "";
        root = deleteRec(root, val, msg);
        rebuildPositions();
    }

    // Call once per frame from your game-loop
    void tickAnimations(float dt, float lerpSpeed) {
        tickNode(root, dt, lerpSpeed);
    }

    // Rebuild all tgt_x / tgt_y based on current tree shape
    void rebuildPositions(float startSpacing = 320.f) {
        assignPositions(root, 0.f, 0.f, startSpacing);
    }

    // Snap cur_x/cur_y → tgt_x/tgt_y for any node whose alpha is near 0
    // (i.e. just-created nodes that have never been rendered yet)
    void snapNewNodesToCurrent() {
        snapNew(root);
    }

    // ─────────────────────────────────────────────────────────────────────
    //  Predecessor / Successor
    //  Returns the value, or -1 (cast to T) when not found.
    // ─────────────────────────────────────────────────────────────────────
    T getPredecessor(T val) {
        searchPath.clear();
        Node<T>* current     = root;
        Node<T>* predecessor = nullptr;

        while (current) {
            searchPath.push_back(current->data);
            if (val < current->data) {
                current = current->left;
            } else if (val > current->data) {
                predecessor = current;
                current     = current->right;
            } else {
                // Found exact node — predecessor is the max of the left subtree
                if (current->left) {
                    Node<T>* temp = maxValueNode(current->left);
                    searchPath.push_back(temp->data);
                    return temp->data;
                }
                break;
            }
        }
        return predecessor ? predecessor->data : T(-1);
    }

    T getSuccessor(T val) {
        searchPath.clear();
        Node<T>* current   = root;
        Node<T>* successor = nullptr;

        while (current) {
            searchPath.push_back(current->data);
            if (val < current->data) {
                successor = current;
                current   = current->left;
            } else if (val > current->data) {
                current = current->right;
            } else {
                // Found exact node — successor is the min of the right subtree
                if (current->right) {
                    Node<T>* temp = minValueNode(current->right);
                    searchPath.push_back(temp->data);
                    return temp->data;
                }
                break;
            }
        }
        return successor ? successor->data : T(-1);
    }

    // ─────────────────────────────────────────────────────────────────────
    //  Traversal strings
    // ─────────────────────────────────────────────────────────────────────
    std::string getPreOrderString()     const { std::vector<T> v; preOrderVec(root, v);   return formatVector(v); }
    std::string getInOrderString()      const { std::vector<T> v; inOrderVec(root, v);    return formatVector(v); }
    std::string getPostOrderString()    const { std::vector<T> v; postOrderVec(root, v);  return formatVector(v); }
    std::string getBreadthFirstString() const {
        std::vector<T> v;
        if (root) {
            std::queue<Node<T>*> q;
            q.push(root);
            while (!q.empty()) {
                Node<T>* cur = q.front(); q.pop();
                v.push_back(cur->data);
                if (cur->left)  q.push(cur->left);
                if (cur->right) q.push(cur->right);
            }
        }
        return formatVector(v);
    }

    // Collect all values in the tree (used for deduplication in random gen)
    std::vector<T> getAllValues() const {
        std::vector<T> v;
        collectAll(root, v);
        return v;
    }

// ─────────────────────────────────────────────────────────────────────────────
private:
// ─────────────────────────────────────────────────────────────────────────────

    Node<T>* root;

    // ── Height / balance ──────────────────────────────────────────────────
    int getHeight(Node<T>* n)  const { return n ? n->height : 0; }
    int getBalance(Node<T>* n) const {
        return n ? getHeight(n->left) - getHeight(n->right) : 0;
    }
    void updateHeight(Node<T>* n) {
        if (n) n->height = 1 + std::max(getHeight(n->left), getHeight(n->right));
    }

    // ── Min / Max ─────────────────────────────────────────────────────────
    Node<T>* minValueNode(Node<T>* node) const {
        while (node && node->left) node = node->left;
        return node;
    }
    Node<T>* maxValueNode(Node<T>* node) const {
        while (node && node->right) node = node->right;
        return node;
    }

    // ── Rotations ─────────────────────────────────────────────────────────
    Node<T>* rightRotate(Node<T>* y) {
        Node<T>* x  = y->left;
        Node<T>* T2 = x->right;
        x->right = y;
        y->left  = T2;
        updateHeight(y);
        updateHeight(x);
        y->isRotating = true;  y->highlightVal = 1.f;
        x->isRotating = true;  x->highlightVal = 1.f;
        return x;
    }

    Node<T>* leftRotate(Node<T>* x) {
        Node<T>* y  = x->right;
        Node<T>* T2 = y->left;
        y->left  = x;
        x->right = T2;
        updateHeight(x);
        updateHeight(y);
        x->isRotating = true;  x->highlightVal = 1.f;
        y->isRotating = true;  y->highlightVal = 1.f;
        return y;
    }

    // ── Recursive insert ──────────────────────────────────────────────────
    Node<T>* insertRec(Node<T>* node, T val, std::string& msg) {
        if (!node) {
            Node<T>* n   = new Node<T>(val);
            n->isNew     = true;
            n->pulseTimer = 1.4f;
            n->alpha     = 0.f;   // will fade in via tickNode
            return n;
        }
        if      (val < node->data) node->left  = insertRec(node->left,  val, msg);
        else if (val > node->data) node->right = insertRec(node->right, val, msg);
        else                       return node; // duplicate — silently ignored

        updateHeight(node);
        int balance = getBalance(node);

        // ── AVL rebalance cases ──────────────────────────────────────────
        if (balance > 1  && val < node->left->data) {
            msg = "Right Rotation";
            return rightRotate(node);
        }
        if (balance < -1 && val > node->right->data) {
            msg = "Left Rotation";
            return leftRotate(node);
        }
        if (balance > 1  && val > node->left->data) {
            msg = "Left-Right Rotation";
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if (balance < -1 && val < node->right->data) {
            msg = "Right-Left Rotation";
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
        return node;
    }

    // ── Recursive delete ──────────────────────────────────────────────────
    Node<T>* deleteRec(Node<T>* node, T val, std::string& msg) {
        if (!node) return nullptr;

        if      (val < node->data) node->left  = deleteRec(node->left,  val, msg);
        else if (val > node->data) node->right = deleteRec(node->right, val, msg);
        else {
            // Node to delete found
            if (!node->left || !node->right) {
                Node<T>* child = node->left ? node->left : node->right;
                if (!child) {
                    // Leaf node
                    delete node;
                    return nullptr;
                } else {
                    // One child — copy and delete
                    *node = *child;
                    delete child;
                }
            } else {
                // Two children — replace with in-order successor
                Node<T>* successor = minValueNode(node->right);
                node->data         = successor->data;
                node->right        = deleteRec(node->right, successor->data, msg);
            }
        }

        if (!node) return nullptr;

        updateHeight(node);
        int balance = getBalance(node);

        // ── AVL rebalance cases after delete ────────────────────────────
        if (balance > 1  && getBalance(node->left) >= 0) {
            msg = "Right Rotation (rebalance)";
            return rightRotate(node);
        }
        if (balance > 1  && getBalance(node->left) < 0) {
            msg = "Left-Right Rotation (rebalance)";
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if (balance < -1 && getBalance(node->right) <= 0) {
            msg = "Left Rotation (rebalance)";
            return leftRotate(node);
        }
        if (balance < -1 && getBalance(node->right) > 0) {
            msg = "Right-Left Rotation (rebalance)";
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
        return node;
    }

    // ── Destroy ───────────────────────────────────────────────────────────
    void destroyTree(Node<T>* node) {
        if (!node) return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }

    // ── Count ─────────────────────────────────────────────────────────────
    int countNodesRec(Node<T>* node) const {
        if (!node) return 0;
        return 1 + countNodesRec(node->left) + countNodesRec(node->right);
    }

    // ── Position assignment ───────────────────────────────────────────────
    void assignPositions(Node<T>* node, float x, float y, float spacing) {
        if (!node) return;
        node->tgt_x = x;
        node->tgt_y = y;
        float half = std::max(40.f, spacing * 0.5f);
        assignPositions(node->left,  x - spacing, y + 90.f, half);
        assignPositions(node->right, x + spacing, y + 90.f, half);
    }

    // ── Snap new (alpha≈0) nodes to their target immediately ─────────────
    void snapNew(Node<T>* n) {
        if (!n) return;
        if (n->isNew && n->alpha < 0.05f) {
            n->cur_x = n->tgt_x;
            n->cur_y = n->tgt_y;
        }
        snapNew(n->left);
        snapNew(n->right);
    }

    // ── Per-frame tick: lerp position + animate timers ────────────────────
    void tickNode(Node<T>* n, float dt, float lerpSpeed) {
        if (!n) return;

        // Exponential smoothing (frame-rate independent)
        float t  = 1.f - std::exp(-lerpSpeed * dt);
        n->cur_x = n->cur_x + (n->tgt_x - n->cur_x) * t;
        n->cur_y = n->cur_y + (n->tgt_y - n->cur_y) * t;

        // Fade-in for newly inserted nodes
        if (n->alpha < 1.f) {
            n->alpha = std::min(1.f, n->alpha + dt * lerpSpeed * 0.35f);
        }

        // Pulse timer countdown
        if (n->pulseTimer > 0.f) {
            n->pulseTimer -= dt;
            if (n->pulseTimer <= 0.f) {
                n->pulseTimer = 0.f;
                n->isNew      = false;
            }
        }

        // Rotation glow fade
        if (n->highlightVal > 0.f) {
            n->highlightVal -= dt * 0.9f;  // slow fade so user can see it
            if (n->highlightVal <= 0.f) {
                n->highlightVal = 0.f;
                n->isRotating   = false;
            }
        }

        tickNode(n->left,  dt, lerpSpeed);
        tickNode(n->right, dt, lerpSpeed);
    }

    // ── Traversal helpers ─────────────────────────────────────────────────
    void preOrderVec (Node<T>* n, std::vector<T>& v) const {
        if (!n) return;
        v.push_back(n->data);
        preOrderVec(n->left, v);
        preOrderVec(n->right, v);
    }
    void inOrderVec  (Node<T>* n, std::vector<T>& v) const {
        if (!n) return;
        inOrderVec(n->left, v);
        v.push_back(n->data);
        inOrderVec(n->right, v);
    }
    void postOrderVec(Node<T>* n, std::vector<T>& v) const {
        if (!n) return;
        postOrderVec(n->left, v);
        postOrderVec(n->right, v);
        v.push_back(n->data);
    }

    void collectAll(Node<T>* n, std::vector<T>& out) const {
        if (!n) return;
        out.push_back(n->data);
        collectAll(n->left,  out);
        collectAll(n->right, out);
    }

    std::string formatVector(const std::vector<T>& vec) const {
        if (vec.empty()) return "[ Empty Tree ]";
        std::string res = "[ ";
        for (size_t i = 0; i < vec.size(); ++i) {
            res += std::to_string(vec[i]);
            if (i + 1 < vec.size()) res += ",  ";
        }
        return res + " ]";
    }
};

#endif // BST_H