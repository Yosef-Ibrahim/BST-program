#pragma once
#include <string>
#include <vector>
#include <queue>
#include <algorithm>

#include <tree/Node.h>

/**
 * @brief An AVL Tree designed for visualization.
 */
template <typename T>
class BST {
public:
    BST();
    ~BST();

    // == Core Tree Operations ==

    /**
     * @brief Inserts a new value into the tree and balances it.
     * @param val The value to insert.
     * @param rotMsg Output string capturing any rotations that occurred (e.g., "LL Rotation").
     */
    void insert(T val, std::string& rotMsg);

    /**
     * @brief Removes a value from the tree and re-balances it.
     * @param val The value to remove.
     * @param rotMsg Output string capturing any rotations that occurred.
     */
    void remove(T val, std::string& rotMsg);

    /**
     * @brief Completely clears the tree and deallocates all nodes.
     */
    void clearTree();

    // == Data Retrieval ==

    Node<T>* getRoot() const;
    int getNodeCount() const;
    int getTreeHeight() const;
    
    // == Search & Traversals ==

    /**
     * @brief Finds the largest value smaller than the given value.
     * @return The predecessor value, or -1 (or a sentinel) if none exists.
     */
    int getPredecessor(T val);

    /**
     * @brief Finds the smallest value larger than the given value.
     * @return The successor value, or -1 (or a sentinel) if none exists.
     */
    int getSuccessor(T val);

    // Formatted string outputs for the UI traversal panel
    std::string getPreOrderString() const;
    std::string getInOrderString() const;
    std::string getPostOrderString() const;
    std::string getBreadthFirstString() const;

    // == Animation & Rendering ==
    /**
     * @brief Updates node positions, layout calculations, and visual timers.
     * @param dt Delta time since the last frame.
     * @param speed The current animation speed multiplier.
     */
    void tickAnimations(float dt, float speed);

    // Public state read by the UI to highlight nodes during a search/insert
    std::vector<T> searchPath;

private:
    Node<T>* root;
    int nodeCount;

    // == Internal Recursive Helpers ==

    Node<T>* insertRec(Node<T>* node, T val, std::string& rotMsg);
    Node<T>* removeRec(Node<T>* node, T val, std::string& rotMsg);
    void clearRec(Node<T>* node);
    
    // AVL Balancing routines
    Node<T>* balance(Node<T>* node, std::string& rotMsg);
    Node<T>* rotateRight(Node<T>* y);
    Node<T>* rotateLeft(Node<T>* y);
    int height(Node<T>* N);
    int getBalance(Node<T>* N);

    // Helpers for string generation and animation
    void inOrderRec(Node<T>* node, std::string& res) const;
    void preOrderRec(Node<T>* node, std::string& res) const;
    void postOrderRec(Node<T>* node, std::string& res) const;
    void updateTargets(Node<T>* node, int depth, float xPos, float offset);
    void animateNode(Node<T>* node, float dt, float speed);
};