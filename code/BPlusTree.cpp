// File: BPlusTree.cpp
#include "BPlusTree.h"
#include <algorithm>
#include <iostream>
#include <queue>

BPlusTree::BPlusTree(int order) : n(order), root(new BPlusTreeNode()) {}

void BPlusTree::insert(float key, Record* record) {
    if (root->keys.empty()) {
        root->keys.push_back(key);
        root->records.push_back(record);
        return;
    }

    BPlusTreeNode* leaf = find_leaf(root, key);

    if (leaf->keys.size() < n - 1) {
        auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
        int index = it - leaf->keys.begin();
        leaf->keys.insert(it, key);
        leaf->records.insert(leaf->records.begin() + index, record);
    } else {
        BPlusTreeNode* new_leaf = new BPlusTreeNode();
        std::vector<float> temp_keys = leaf->keys;
        std::vector<Record*> temp_records = leaf->records;

        temp_keys.push_back(key);
        temp_records.push_back(record);

        leaf->keys.clear();
        leaf->records.clear();
        new_leaf->keys.clear();
        new_leaf->records.clear();

        int mid = (n + 1) / 2;
        for (int i = 0; i < mid; i++) {
            leaf->keys.push_back(temp_keys[i]);
            leaf->records.push_back(temp_records[i]);
        }
        for (int i = mid; i < n; i++) {
            new_leaf->keys.push_back(temp_keys[i]);
            new_leaf->records.push_back(temp_records[i]);
        }

        new_leaf->next = leaf->next;
        leaf->next = new_leaf;

        if (leaf == root) {
            BPlusTreeNode* new_root = new BPlusTreeNode(false);
            new_root->keys.push_back(new_leaf->keys[0]);
            new_root->children.push_back(leaf);
            new_root->children.push_back(new_leaf);
            root = new_root;
        } else {
            insert_internal(root, new_leaf->keys[0], new_leaf);
        }
    }
}

void BPlusTree::insert_internal(BPlusTreeNode* node, float key, BPlusTreeNode* child) {
    if (node->keys.size() < n - 1) {
        auto it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
        int index = it - node->keys.begin();
        node->keys.insert(it, key);
        node->children.insert(node->children.begin() + index + 1, child);
    } else {
        BPlusTreeNode* new_internal = new BPlusTreeNode(false);
        std::vector<float> temp_keys = node->keys;
        std::vector<BPlusTreeNode*> temp_children = node->children;

        temp_keys.push_back(key);
        temp_children.push_back(child);

        node->keys.clear();
        node->children.clear();
        new_internal->keys.clear();
        new_internal->children.clear();

        int mid = n / 2;
        for (int i = 0; i < mid; i++) {
            node->keys.push_back(temp_keys[i]);
            node->children.push_back(temp_children[i]);
        }
        node->children.push_back(temp_children[mid]);

        float median = temp_keys[mid];
        for (int i = mid + 1; i < n; i++) {
            new_internal->keys.push_back(temp_keys[i]);
            new_internal->children.push_back(temp_children[i]);
        }
        new_internal->children.push_back(temp_children[n]);

        if (node == root) {
            BPlusTreeNode* new_root = new BPlusTreeNode(false);
            new_root->keys.push_back(median);
            new_root->children.push_back(node);
            new_root->children.push_back(new_internal);
            root = new_root;
        } else {
            insert_internal(root, median, new_internal);
        }
    }
}

BPlusTreeNode* BPlusTree::find_leaf(BPlusTreeNode* node, float key) {
    if (node->is_leaf) {
        return node;
    }

    auto it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
    int index = it - node->keys.begin();

    if (index == node->keys.size() || key < node->keys[index]) {
        return find_leaf(node->children[index], key);
    } else {
        return find_leaf(node->children[index + 1], key);
    }
}

std::vector<Record*> BPlusTree::range_search(float start, float end) {
    std::vector<Record*> result;
    BPlusTreeNode* leaf = find_leaf(root, start);

    while (leaf != nullptr) {
        for (size_t i = 0; i < leaf->keys.size(); i++) {
            if (leaf->keys[i] >= start && leaf->keys[i] <= end) {
                result.push_back(leaf->records[i]);
            }
            if (leaf->keys[i] > end) {
                return result;
            }
        }
        leaf = leaf->next;
    }

    return result;
}

void BPlusTree::print_statistics() {
    std::cout << "\nB+ Tree Statistics:" << std::endl;
    std::cout << "B+ tree order (n): " << n << std::endl;

    int num_nodes = 0;
    int num_levels = 0;
    std::queue<BPlusTreeNode*> q;
    q.push(root);

    while (!q.empty()) {
        int level_size = q.size();
        num_levels++;

        for (int i = 0; i < level_size; i++) {
            BPlusTreeNode* node = q.front();
            q.pop();
            num_nodes++;

            if (!node->is_leaf) {
                for (BPlusTreeNode* child : node->children) {
                    q.push(child);
                }
            }
        }
    }

    std::cout << "Number of nodes: " << num_nodes << std::endl;
    std::cout << "Number of levels: " << num_levels << std::endl;

    std::cout << "Root node keys: ";
    for (float key : root->keys) {
        std::cout << key << " ";
    }
    std::cout << std::endl;
}
