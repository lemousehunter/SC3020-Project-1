#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <vector>
#include <memory>
#include <fstream>
#include "Storage.h"

struct RecordLocation {
    int datablockId;
    std::streampos fileOffset;
};

class BPlusTreeNode {
public:
    bool isLeaf;

    /*
    'Key' is defined by the value of 'FG_PCT_home'
    That is the reason why it is defined as a 'float' value
    A 'node' can contain many 'keys'
    So we will create a 'vector' of 'keys' value of type 'float'
    */
    std::vector<float> keys;

    // A vector of children pointers
    std::vector<std::shared_ptr<BPlusTreeNode>> children;

    // TODO: I think that if it is the leaf node then it will have the value for this RecordLocation?
    // And it can have many record with the same key??? I think?
    std::vector<RecordLocation> recordLocations;  // Add this line

    std::weak_ptr<BPlusTreeNode> parent;

    std::shared_ptr<BPlusTreeNode> nextLeaf;

    BPlusTreeNode(bool leaf = false) : isLeaf(leaf) {}
};

struct SearchResult {
    int     indexNodesAccessed;
    int     dataBlocksAccessed;
    float   avgFG3PctHome;
    int     numberOfResults;
    std::vector<Record> found_records;
};

class BPlusTree {
public:
    BPlusTree(int order, const std::string& indexFilename);

    void buildFromStorage(const Storage& storage);

    void insert(float key, int datablockId, std::streampos fileOffset);

    SearchResult rangeSearch(float lower, float upper, Storage& storage);

    void printStatistics();

    void saveToFile();

    void loadFromFile();

    void verifyTree();

    std::vector<int> getNodeCounts() const;

private:
    std::shared_ptr<BPlusTreeNode> root;

    
    int order; // Maxium number of children that the internal node can have
               // = (Maximum number of key) + 1


    std::string indexFilename;

    int tree_height;
    int totalNodes = 0;
    int internalNodes = 0;
    int leafNodes = 0;

    std::shared_ptr<BPlusTreeNode> findLeaf(float key, int &indexNodeCounter);

    void insertIntoLeaf(std::shared_ptr<BPlusTreeNode> leaf, float key, int datablockId, std::streampos fileOffset);

    void splitLeafNode(std::shared_ptr<BPlusTreeNode> leaf, float newKey, int datablockId, std::streampos fileOffset);

    void insertIntoParent(std::shared_ptr<BPlusTreeNode> leftChild, float key, std::shared_ptr<BPlusTreeNode> rightChild);

    void splitNonLeafNode(std::shared_ptr<BPlusTreeNode> node, int index, float key, std::shared_ptr<BPlusTreeNode> rightChild);

    int getHeight(std::shared_ptr<BPlusTreeNode> node);

    void writeNode(std::ofstream& file, std::shared_ptr<BPlusTreeNode> node);

    std::shared_ptr<BPlusTreeNode> readNode(std::ifstream& file);

    void _getTotalNodes();
    
};

#endif // BPLUSTREE_H