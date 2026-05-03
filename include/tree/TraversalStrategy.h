#pragma once
#include <string>
#include "BST.h"

class TraversalStrategy {
public:
    virtual ~TraversalStrategy() = default;
    virtual std::string getName() const = 0;
    virtual std::string execute(BST<int>& tree) = 0;
};

class InOrderStrategy : public TraversalStrategy {
public:
    std::string getName() const override { return "In-Order (Sorted)"; }
    std::string execute(BST<int>& tree) override { return tree.getInOrderString(); }
};

class PreOrderStrategy : public TraversalStrategy {
public:
    std::string getName() const override { return "Pre-Order"; }
    std::string execute(BST<int>& tree) override { return tree.getPreOrderString(); }
};

class PostOrderStrategy : public TraversalStrategy {
public:
    std::string getName() const override { return "Post-Order"; }
    std::string execute(BST<int>& tree) override { return tree.getPostOrderString(); }
};

class BFSStrategy : public TraversalStrategy {
public:
    std::string getName() const override { return "BFS"; }
    std::string execute(BST<int>& tree) override { return tree.getBreadthFirstString(); }
};