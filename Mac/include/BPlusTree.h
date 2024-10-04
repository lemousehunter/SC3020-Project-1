#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <vector>
#include <memory>
#include <fstream>
#include "Storage.h"

class BPlusTreeNode {
public:
    bool isLeaf;
    std::vector<float> keys;
    std::vector<std::shared_ptr<BPlusTreeNode>> children;
    std::vector<uint32_t> recordIds;
    std::weak_ptr<BPlusTreeNode> parent;
    std::shared_ptr<BPlusTreeNode> nextLeaf;

    BPlusTreeNode(bool leaf = false) : isLeaf(leaf) {}
};

struct SearchResult {
    int indexNodesAccessed;
    int dataBlocksAccessed;
    float avgFG3PctHome;
    int numberOfResults;
    std::vector<Record> found_records;
};

class BPlusTree {
public:
    BPlusTree(int order, const std::string& indexFilename);

    void buildFromStorage(const Storage& storage);
    void insert(float key, uint32_t recordId);
    SearchResult rangeSearch(float lower, float upper, Storage& storage);
    void printStatistics();
    void saveToFile();
    void loadFromFile();
    void verifyTree();
    std::vector<int> getNodeCounts() const;

private:
    std::shared_ptr<BPlusTreeNode> root;
    int order;
    std::string indexFilename;
    int tree_height;
    int totalNodes = 0;
    int internalNodes = 0;
    int leafNodes = 0;

    std::shared_ptr<BPlusTreeNode> findLeaf(float key, int& indexNodeCounter);
    void insertIntoLeaf(std::shared_ptr<BPlusTreeNode> leaf, float key, uint32_t recordId);
    void splitLeafNode(std::shared_ptr<BPlusTreeNode> leaf, float newKey, uint32_t recordId);
    void insertIntoParent(std::shared_ptr<BPlusTreeNode> leftChild, float key, std::shared_ptr<BPlusTreeNode> rightChild);
    void splitNonLeafNode(std::shared_ptr<BPlusTreeNode> node, int index, float key, std::shared_ptr<BPlusTreeNode> rightChild);
    int getHeight(std::shared_ptr<BPlusTreeNode> node);
    void writeNode(std::ofstream& file, std::shared_ptr<BPlusTreeNode> node);
    std::shared_ptr<BPlusTreeNode> readNode(std::ifstream& file);
    void _getTotalNodes();
};

#endif // BPLUSTREE_H