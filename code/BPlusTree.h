// File: BPlusTree.h
#pragma once
#include <vector>
#include "Record.h"

struct BPlusTreeNode {
    std::vector<float> keys;
    std::vector<BPlusTreeNode*> children;
    std::vector<Record*> records;
    bool is_leaf;
    BPlusTreeNode* next;

    BPlusTreeNode(bool leaf = true) : is_leaf(leaf), next(nullptr) {}
};

class BPlusTree {
private:
    BPlusTreeNode* root;
    int n; // B+ tree order

    void insert_internal(BPlusTreeNode* node, float key, BPlusTreeNode* child);
    BPlusTreeNode* find_leaf(BPlusTreeNode* node, float key);

public:
    BPlusTree(int order);
    void insert(float key, Record* record);
    std::vector<Record*> range_search(float start, float end);
    void print_statistics();
};
