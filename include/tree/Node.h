#pragma once

/**
 * @brief Represents a single node within the AVL Tree.
 */
template <typename T>
struct Node {
    // == Core Tree Data ==
    T data;         /// The value stored in the node
    Node* left;     /// Pointer to the left child node
    Node* right;    /// Pointer to the right child node
    int height;     /// Height of the node

    // == Position & Rendering State ==
    float cur_x;    // Current X-coordinate on the screen
    float cur_y;    // Current Y-coordinate on the screen
    float alpha;    // Current opacity level (0.0 to 1.0)
    
    // == Insertion Animation State ==
    bool isNew;         // Flag indicating if this node was just inserted
    float pulseTimer;   // Countdown timer controlling the pulse ring effect
    
    // == Rotation Animation State ==
    bool isRotating;    // Flag indicating if this node is currently part of a tree rotation
    float highlightVal; // Intensity of the glowing highlight during a rotation

    /**
     * @brief Constructs a new Node with default animation states.
     * @param val The initial value to store in the node.
     */
    Node(T val) : 
        data(val), left(nullptr), right(nullptr), height(1),
        cur_x(0.f), cur_y(0.f), alpha(0.f), 
        isNew(true), pulseTimer(1.0f), 
        isRotating(false), highlightVal(0.f) {}
};