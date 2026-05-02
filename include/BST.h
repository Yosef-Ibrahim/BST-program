#ifndef BST_H
#define BST_H

// ═══════════════════════════════════════════════════════════════════════════════
//  BST.h  —  AVL Tree with smooth animations & correct Reingold-Tilford layout
//
//  FIX #3  (CRITICAL): Replaced naive halving-spacing with a proper
//           Reingold-Tilford-inspired layout that computes the actual
//           contour of each subtree and separates them by exactly
//           NODE_SEP pixels.  No more crossing or overlapping edges
//           even for 80+ node trees.
// ═══════════════════════════════════════════════════════════════════════════════

#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <limits>
#include <functional>

// ─────────────────────────────────────────────────────────────────────────────
//  Node — data + animation state
// ─────────────────────────────────────────────────────────────────────────────
template <typename T>
struct Node {
    T     data;
    Node* left   = nullptr;
    Node* right  = nullptr;
    int   height = 1;

    // Smooth-motion
    float cur_x = 0.f;
    float cur_y = 0.f;
    float tgt_x = 0.f;
    float tgt_y = 0.f;

    // Visual state
    bool  isNew        = false;
    bool  isRotating   = false;
    float alpha        = 0.f;   // 0->1, new nodes fade in
    float pulseTimer   = 0.f;
    float highlightVal = 0.f;

    // Reingold-Tilford layout scratch fields
    float prelim  = 0.f;
    float mod     = 0.f;

    explicit Node(T val) : data(val) {}
};

// ─────────────────────────────────────────────────────────────────────────────
//  BST (self-balancing AVL)
// ─────────────────────────────────────────────────────────────────────────────
template <typename T>
class BST {
public:
    std::vector<T> searchPath;

    BST()  : root(nullptr) {}
    ~BST() { destroyTree(root); }

    Node<T>* getRoot() { return root; }

    void clearTree()  { destroyTree(root); root = nullptr; }

    int getNodeCount()  const { return countRec(root); }
    int getTreeHeight() const { return root ? root->height : 0; }

    void insert(T val, std::string& msg) {
        msg  = "";
        root = insertRec(root, val, msg);
        rebuildPositions();
        snapNewNodesToCurrent();
    }

    void remove(T val, std::string& msg) {
        msg  = "";
        root = deleteRec(root, val, msg);
        rebuildPositions();
    }

    void tickAnimations(float dt, float lerpSpeed) {
        tickNode(root, dt, lerpSpeed);
    }

    // ─────────────────────────────────────────────────────────────────────
    //  Reingold-Tilford layout — guarantees no overlaps for any tree size
    // ─────────────────────────────────────────────────────────────────────
    void rebuildPositions() {
        if (!root) return;
        firstWalk(root);
        secondWalk(root, 0.f, 0);
    }

    void snapNewNodesToCurrent() { snapNew(root); }

    // ── Predecessor / Successor ───────────────────────────────────────────
    T getPredecessor(T val) {
        searchPath.clear();
        Node<T>* cur  = root;
        Node<T>* pred = nullptr;
        while (cur) {
            searchPath.push_back(cur->data);
            if (val < cur->data) {
                cur = cur->left;
            } else if (val > cur->data) {
                pred = cur;
                cur  = cur->right;
            } else {
                if (cur->left) {
                    Node<T>* t = maxNode(cur->left);
                    searchPath.push_back(t->data);
                    return t->data;
                }
                break;
            }
        }
        return pred ? pred->data : T(-1);
    }

    T getSuccessor(T val) {
        searchPath.clear();
        Node<T>* cur  = root;
        Node<T>* succ = nullptr;
        while (cur) {
            searchPath.push_back(cur->data);
            if (val < cur->data) {
                succ = cur;
                cur  = cur->left;
            } else if (val > cur->data) {
                cur = cur->right;
            } else {
                if (cur->right) {
                    Node<T>* t = minNode(cur->right);
                    searchPath.push_back(t->data);
                    return t->data;
                }
                break;
            }
        }
        return succ ? succ->data : T(-1);
    }

    // ── Traversal strings ─────────────────────────────────────────────────
    std::string getPreOrderString()     const { std::vector<T> v; preV(root,v);   return fmt(v); }
    std::string getInOrderString()      const { std::vector<T> v; inV(root,v);    return fmt(v); }
    std::string getPostOrderString()    const { std::vector<T> v; postV(root,v);  return fmt(v); }
    std::string getBreadthFirstString() const {
        std::vector<T> v;
        if (root) {
            std::queue<Node<T>*> q;
            q.push(root);
            while (!q.empty()) {
                auto* n = q.front(); q.pop();
                v.push_back(n->data);
                if (n->left)  q.push(n->left);
                if (n->right) q.push(n->right);
            }
        }
        return fmt(v);
    }

    std::vector<T> getAllValues() const {
        std::vector<T> v; collectAll(root, v); return v;
    }

// ─────────────────────────────────────────────────────────────────────────────
private:
// ─────────────────────────────────────────────────────────────────────────────

    Node<T>* root;

    // Minimum horizontal gap between any two adjacent nodes at the same level
    static constexpr float NODE_SEP  = 68.f;
    static constexpr float LEVEL_SEP = 92.f;

    // ══════════════════════════════════════════════════════════════════════
    //  Reingold-Tilford  (contour-based, Walker 1990 binary variant)
    //
    //  Phase 1 (firstWalk): compute a preliminary x for every node relative
    //  to its parent, ensuring siblings don't overlap by measuring the
    //  right contour of the left subtree vs the left contour of the right.
    //
    //  Phase 2 (secondWalk): propagate accumulated modifiers top-down to
    //  get the absolute tgt_x for every node.
    // ══════════════════════════════════════════════════════════════════════

    // Collect the rightmost x values at each depth under `n`
    void rightContour(Node<T>* n, float modAcc, int depth,
                      std::vector<float>& cont) {
        if (!n) return;
        float absX = n->prelim + modAcc;
        if ((int)cont.size() <= depth) cont.push_back(absX);
        else                           cont[depth] = std::max(cont[depth], absX);
        rightContour(n->left,  modAcc + n->mod, depth + 1, cont);
        rightContour(n->right, modAcc + n->mod, depth + 1, cont);
    }

    // Collect the leftmost x values at each depth under `n`
    void leftContour(Node<T>* n, float modAcc, int depth,
                     std::vector<float>& cont) {
        if (!n) return;
        float absX = n->prelim + modAcc;
        if ((int)cont.size() <= depth) cont.push_back(absX);
        else                           cont[depth] = std::min(cont[depth], absX);
        leftContour(n->left,  modAcc + n->mod, depth + 1, cont);
        leftContour(n->right, modAcc + n->mod, depth + 1, cont);
    }

    void firstWalk(Node<T>* n) {
        n->prelim = 0.f;
        n->mod    = 0.f;

        if (!n->left && !n->right) return;  // leaf

        if (n->left)  firstWalk(n->left);
        if (n->right) firstWalk(n->right);

        if (n->left && n->right) {
            // Measure how far apart we need the two subtrees
            std::vector<float> rc, lc;
            rightContour(n->left,  0.f, 0, rc);
            leftContour (n->right, 0.f, 0, lc);

            float shift = 0.f;
            int   levels = (int)std::min(rc.size(), lc.size());
            for (int i = 0; i < levels; ++i)
                shift = std::max(shift, (rc[i] - lc[i]) + NODE_SEP);

            if (shift < NODE_SEP) shift = NODE_SEP;

            // Place left child at -shift/2, right at +shift/2
            n->left->prelim  = -shift * 0.5f;
            n->right->prelim =  shift * 0.5f;
            // mod carries the offset down into the subtree
            n->left->mod     = n->left->prelim;
            n->right->mod    = n->right->prelim;
            n->prelim        = 0.f;  // parent is centred

        } else if (n->left) {
            // Only left child — parent sits above it
            n->prelim = n->left->prelim;
        } else {
            // Only right child
            n->prelim = n->right->prelim;
        }
    }

    // Phase 2: walk top-down, adding accumulated modifiers
    void secondWalk(Node<T>* n, float modAcc, int depth) {
        if (!n) return;
        n->tgt_x = n->prelim + modAcc;
        n->tgt_y = depth * LEVEL_SEP;
        secondWalk(n->left,  modAcc + n->mod, depth + 1);
        secondWalk(n->right, modAcc + n->mod, depth + 1);
    }

    // ── AVL helpers ───────────────────────────────────────────────────────
    int  h (Node<T>* n) const { return n ? n->height : 0; }
    int  bf(Node<T>* n) const { return n ? h(n->left) - h(n->right) : 0; }
    void uh(Node<T>* n)       { if (n) n->height = 1 + std::max(h(n->left), h(n->right)); }

    Node<T>* minNode(Node<T>* n) const { while (n && n->left)  n = n->left;  return n; }
    Node<T>* maxNode(Node<T>* n) const { while (n && n->right) n = n->right; return n; }

    Node<T>* rotR(Node<T>* y) {
        Node<T>* x = y->left; Node<T>* t = x->right;
        x->right = y; y->left = t;
        uh(y); uh(x);
        y->isRotating = x->isRotating = true;
        y->highlightVal = x->highlightVal = 1.f;
        return x;
    }
    Node<T>* rotL(Node<T>* x) {
        Node<T>* y = x->right; Node<T>* t = y->left;
        y->left = x; x->right = t;
        uh(x); uh(y);
        x->isRotating = y->isRotating = true;
        x->highlightVal = y->highlightVal = 1.f;
        return y;
    }

    // ── Insert ────────────────────────────────────────────────────────────
    Node<T>* insertRec(Node<T>* n, T val, std::string& msg) {
        if (!n) {
            auto* nd = new Node<T>(val);
            nd->isNew = true; nd->pulseTimer = 1.4f; nd->alpha = 0.f;
            return nd;
        }
        if      (val < n->data) n->left  = insertRec(n->left,  val, msg);
        else if (val > n->data) n->right = insertRec(n->right, val, msg);
        else                    return n;

        uh(n);
        int b = bf(n);
        if (b > 1  && val < n->left->data)  { msg = "Right Rotation";      return rotR(n); }
        if (b < -1 && val > n->right->data) { msg = "Left Rotation";       return rotL(n); }
        if (b > 1  && val > n->left->data)  { msg = "Left-Right Rotation"; n->left  = rotL(n->left);  return rotR(n); }
        if (b < -1 && val < n->right->data) { msg = "Right-Left Rotation"; n->right = rotR(n->right); return rotL(n); }
        return n;
    }

    // ── Delete ────────────────────────────────────────────────────────────
    Node<T>* deleteRec(Node<T>* n, T val, std::string& msg) {
        if (!n) return nullptr;
        if      (val < n->data) n->left  = deleteRec(n->left,  val, msg);
        else if (val > n->data) n->right = deleteRec(n->right, val, msg);
        else {
            if (!n->left || !n->right) {
                Node<T>* child = n->left ? n->left : n->right;
                if (!child) { delete n; return nullptr; }
                *n = *child; delete child;
            } else {
                Node<T>* s = minNode(n->right);
                n->data    = s->data;
                n->right   = deleteRec(n->right, s->data, msg);
            }
        }
        if (!n) return nullptr;
        uh(n);
        int b = bf(n);
        if (b > 1  && bf(n->left)  >= 0) { msg = "Right Rotation (rebalance)";      return rotR(n); }
        if (b > 1  && bf(n->left)  <  0) { msg = "Left-Right Rotation (rebalance)"; n->left  = rotL(n->left);  return rotR(n); }
        if (b < -1 && bf(n->right) <= 0) { msg = "Left Rotation (rebalance)";       return rotL(n); }
        if (b < -1 && bf(n->right) >  0) { msg = "Right-Left Rotation (rebalance)"; n->right = rotR(n->right); return rotL(n); }
        return n;
    }

    void destroyTree(Node<T>* n) {
        if (!n) return;
        destroyTree(n->left); destroyTree(n->right); delete n;
    }

    int countRec(Node<T>* n) const {
        return n ? 1 + countRec(n->left) + countRec(n->right) : 0;
    }

    void snapNew(Node<T>* n) {
        if (!n) return;
        if (n->isNew && n->alpha < 0.05f) { n->cur_x = n->tgt_x; n->cur_y = n->tgt_y; }
        snapNew(n->left); snapNew(n->right);
    }

    void tickNode(Node<T>* n, float dt, float lerpSpeed) {
        if (!n) return;
        float t  = 1.f - std::exp(-lerpSpeed * dt);
        n->cur_x += (n->tgt_x - n->cur_x) * t;
        n->cur_y += (n->tgt_y - n->cur_y) * t;

        if (n->alpha < 1.f)
            n->alpha = std::min(1.f, n->alpha + dt * lerpSpeed * 0.35f);

        if (n->pulseTimer > 0.f) {
            n->pulseTimer -= dt;
            if (n->pulseTimer <= 0.f) { n->pulseTimer = 0.f; n->isNew = false; }
        }
        if (n->highlightVal > 0.f) {
            n->highlightVal -= dt * 0.9f;
            if (n->highlightVal <= 0.f) { n->highlightVal = 0.f; n->isRotating = false; }
        }
        tickNode(n->left, dt, lerpSpeed);
        tickNode(n->right, dt, lerpSpeed);
    }

    void preV (Node<T>* n, std::vector<T>& v) const { if (!n) return; v.push_back(n->data); preV(n->left,v);  preV(n->right,v);  }
    void inV  (Node<T>* n, std::vector<T>& v) const { if (!n) return; inV(n->left,v);  v.push_back(n->data); inV(n->right,v);   }
    void postV(Node<T>* n, std::vector<T>& v) const { if (!n) return; postV(n->left,v); postV(n->right,v); v.push_back(n->data); }
    void collectAll(Node<T>* n, std::vector<T>& v) const {
        if (!n) return;
        v.push_back(n->data);
        collectAll(n->left,  v);
        collectAll(n->right, v);
    }

    std::string fmt(const std::vector<T>& v) const {
        if (v.empty()) return "[ Empty Tree ]";
        std::string s = "[ ";
        for (size_t i = 0; i < v.size(); ++i) {
            s += std::to_string(v[i]);
            if (i + 1 < v.size()) s += ",  ";
        }
        return s + " ]";
    }
};

#endif // BST_H