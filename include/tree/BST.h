#pragma once
#include <string>
#include <vector>
#include <queue>
#include <algorithm>

template <typename T>
struct Node {
    T     data;
    Node* left  = nullptr;
    Node* right = nullptr;

    int   height = 1;

    float cur_x = 0.5f;
    float cur_y = 0.0f;

    float alpha      = 0.0f;
    bool  isNew      = true;
    float pulseTimer = 1.0f;
    bool  isRotating = false;
    float highlightVal = 0.0f;

    explicit Node(T val) : data(val) {}
};

template <typename T>
class BST {
public:
    BST();
    ~BST();

    // Core operations
    void insert(T val, std::string& rotMsg);
    void remove(T val, std::string& rotMsg);
    void clearTree();

    // Getters
    Node<T>* getRoot()      const;
    int      getNodeCount() const;
    int      getTreeHeight() const;

    // Search
    int getPredecessor(T val);
    int getSuccessor(T val);

    // Traversals
    std::string getInOrderString()      const;
    std::string getPreOrderString()     const;
    std::string getPostOrderString()    const;
    std::string getBreadthFirstString() const;

    // Animation tick – call once per frame
    void tickAnimations(float dt, float speed);

    // Returns the node pointer for a given value, or nullptr if not found.
    Node<T>* findNode(T val) const;

    bool computeTargetPos(T val, float& outX, float& outY) const;

    // Search path highlighted in the renderer
    std::vector<T> searchPath;

private:
    Node<T>* root;
    int      nodeCount;

    // AVL helpers
    int      height(Node<T>* N) const;
    int      getBalance(Node<T>* N);
    Node<T>* rotateRight(Node<T>* y);
    Node<T>* rotateLeft(Node<T>* y);
    Node<T>* balance(Node<T>* node, std::string& rotMsg);

    // Recursive operations
    Node<T>* insertRec(Node<T>* node, T val, std::string& rotMsg);
    Node<T>* removeRec(Node<T>* node, T val, std::string& rotMsg);
    void     clearRec(Node<T>* node);

    // Traversal helpers
    void inOrderRec  (Node<T>* node, std::string& res) const;
    void preOrderRec (Node<T>* node, std::string& res) const;
    void postOrderRec(Node<T>* node, std::string& res) const;

    // Animation helpers
    void updateTargets(Node<T>* node, int depth, float targetX, float offset,
                       float dt, float speed);
    void animateNode  (Node<T>* node, float dt, float speed);

    // Camera-focus helpers
    Node<T>* findNodeRec(Node<T>* node, T val) const;
    bool     computeTargetPosRec(Node<T>* node, T val,
                                 int depth, float targetX, float offset,
                                 float& outX, float& outY) const;
};