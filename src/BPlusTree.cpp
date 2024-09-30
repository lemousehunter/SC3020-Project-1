#include "BPlusTree.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <queue>
#include <stack>
#include <set>

bool compareRecordPairs(const std::pair<float, RecordLocation>& a, const std::pair<float, RecordLocation>& b) {
    
    // Compare the record value first
    if (a.first != b.first) {
        return a.first < b.first;
    }
    // If keys are equal, compare datablockId first
    if (a.second.datablockId != b.second.datablockId) {
        return a.second.datablockId < b.second.datablockId;
    }

    // Finally compare the fileOffset
    return a.second.fileOffset < b.second.fileOffset;
}

BPlusTree::BPlusTree(int order, const std::string& indexFilename) 
    : order(order), indexFilename(indexFilename), root(nullptr) {}


std::vector<int> BPlusTree::getNodeCounts() const {
    return {
        internalNodes, leafNodes, totalNodes
    };
}

void BPlusTree::_getTotalNodes() {
    if (!root) {
        std::cout << "Tree is empty. Total nodes: 0" << std::endl;
        return;
    }

    std::queue<std::shared_ptr<BPlusTreeNode>> queue;
    queue.push(root);

    while (!queue.empty()) {
        auto node = queue.front();
        queue.pop();

        totalNodes++;

        if (node->isLeaf) {
            leafNodes++;
        } else {
            internalNodes++;
            for (const auto& child : node->children) {
                queue.push(child);
            }
        }
    }
}

void BPlusTree::buildFromStorage(const Storage& storage) {
    std::cout << "Starting to build B+ tree from storage..." << std::endl;

    // Get all the record in the storage

    auto records = storage.getAllRecords();

    std::cout << "Retrieved " << records.size() << " records from storage." << std::endl;


    int count = 0;

    // TODO: The record is not sorted yet, why we just insert it into the BPlusTree sequentially?

    for (const auto& record : records) {
        try {

            //           key            datablockId         fileOffset
            insert(record.fgPctHome, record.datablockId, record.fileOffset);

            count++;

            if (count % 1000 == 0) {
                std::cout << "Inserted " << count << " records into the B+ tree." << std::endl;
            }

        } catch (const std::exception& e) {
            std::cerr << "Error inserting record: " << e.what() << std::endl;
            std::cerr << "Record details: fgPctHome=" << record.fgPctHome 
                      << ", datablockId=" << record.datablockId 
                      << ", fileOffset=" << record.fileOffset << std::endl;
        }
    }

    std::cout << "Finished building B+ tree. Total records inserted: " << count << std::endl;
    std::cout << "Saving B+ tree to file..." << std::endl;

    saveToFile();
    std::cout << "B+ tree saved to file." << std::endl;
    tree_height = getHeight(root);
    _getTotalNodes();
}

void BPlusTree::insert(float key, int datablockId, std::streampos fileOffset) {

    // Insert the key into the BPlusTree (I think)
    //std::cout << "Inserting key: " << key << ", datablockId: " << datablockId << ", fileOffset: " << fileOffset << std::endl;

    try {

        // If there not exist any root node we have to create one
        if (!root) {
            //std::cout << "Creating root node" << std::endl;

            /*
            root: is the private variable created inside the BPlusTree when initated

            Create a root node by initiate the 'BPlusTreeNode' and passing 'true'
            as an argument to the constructor to signal that this is the root node.

            This initiation will return the pointer to the initiated object.

            std::make_shared will combine 2 step in 1 line of code:
                - It create the object
                - Create shared_ptr that manages the lifetime of the newly created object
            */ 
            root = std::make_shared<BPlusTreeNode>(true);

            // Push the key to the 'BPlusTreeNode'
            root->recordLocations.push_back({datablockId, fileOffset});
            return;
        }

        int temp;

        
        auto leaf = findLeaf(key, temp);

        if (!leaf) {
            throw std::runtime_error("findLeaf returned nullptr");
        }

        // If the size of the keys vector is smaller the maximum number of keys allowed
        // insert it into the leaf node
        // or else we have to split the leaf node and handle the parrent node correctly
        if (leaf->keys.size() < order - 1) {
            insertIntoLeaf(leaf, key, datablockId, fileOffset);
        } else {
            splitLeafNode(leaf, key, datablockId, fileOffset);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in insert: " << e.what() << std::endl;
        throw;
    }
}

void BPlusTree::insertIntoLeaf(std::shared_ptr<BPlusTreeNode> leaf, float key, int datablockId, std::streampos fileOffset) {
    //std::cout << "Inserting into leaf node" << std::endl;
    try {
        auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
        int index = std::distance(leaf->keys.begin(), it);
        
        leaf->keys.insert(leaf->keys.begin() + index, key);
        leaf->recordLocations.insert(leaf->recordLocations.begin() + index, {datablockId, fileOffset});
    } catch (const std::exception& e) {
        std::cerr << "Error in insertIntoLeaf: " << e.what() << std::endl;
        throw;
    }
}

void BPlusTree::splitLeafNode(std::shared_ptr<BPlusTreeNode> leaf, float newKey, int datablockId, std::streampos fileOffset) {
    //std::cout << "Splitting leaf node" << std::endl;
    try {
        auto newLeaf = std::make_shared<BPlusTreeNode>(true);
        
        std::vector<float> tempKeys = leaf->keys;
        std::vector<RecordLocation> tempLocations = leaf->recordLocations;
        tempKeys.push_back(newKey);
        tempLocations.push_back({datablockId, fileOffset});

        std::vector<std::pair<float, RecordLocation>> pairs;
        for (size_t i = 0; i < tempKeys.size(); ++i) {
            pairs.emplace_back(tempKeys[i], tempLocations[i]);
        }
        std::sort(pairs.begin(), pairs.end(), compareRecordPairs);  // Use the custom comparison function

        int mid = (order + 1) / 2;
        leaf->keys.clear();
        leaf->recordLocations.clear();
        newLeaf->keys.clear();
        newLeaf->recordLocations.clear();

        for (int i = 0; i < mid; ++i) {
            leaf->keys.push_back(pairs[i].first);
            leaf->recordLocations.push_back(pairs[i].second);
        }
        for (size_t i = mid; i < pairs.size(); ++i) {
            newLeaf->keys.push_back(pairs[i].first);
            newLeaf->recordLocations.push_back(pairs[i].second);
        }

        newLeaf->nextLeaf = leaf->nextLeaf;
        leaf->nextLeaf = newLeaf;

        float promotedKey = newLeaf->keys.front();
        insertIntoParent(leaf, promotedKey, newLeaf);
    } catch (const std::exception& e) {
        std::cerr << "Error in splitLeafNode: " << e.what() << std::endl;
        throw;
    }
    
}

void BPlusTree::insertIntoParent(std::shared_ptr<BPlusTreeNode> leftChild, float key, std::shared_ptr<BPlusTreeNode> rightChild) {
    //std::cout << "Inserting into parent node" << std::endl;
    try {
        if (leftChild == root) {
            //std::cout << "Creating new root" << std::endl;
            auto newRoot = std::make_shared<BPlusTreeNode>(false);
            newRoot->keys.push_back(key);
            newRoot->children.push_back(leftChild);
            newRoot->children.push_back(rightChild);
            root = newRoot;
            leftChild->parent = newRoot;
            rightChild->parent = newRoot;
            return;
        }

        auto parent = leftChild->parent.lock();
        if (!parent) {
            throw std::runtime_error("Parent node is null");
        }

        auto it = std::find(parent->children.begin(), parent->children.end(), leftChild);
        if (it == parent->children.end()) {
            throw std::runtime_error("Left child not found in parent's children");
        }
        int index = std::distance(parent->children.begin(), it);

        if (parent->keys.size() < order - 1) {
            parent->keys.insert(parent->keys.begin() + index, key);
            parent->children.insert(parent->children.begin() + index + 1, rightChild);
            rightChild->parent = parent;
        } else {
            splitNonLeafNode(parent, index, key, rightChild);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in insertIntoParent: " << e.what() << std::endl;
        throw;
    }
}

void BPlusTree::splitNonLeafNode(std::shared_ptr<BPlusTreeNode> node, int index, float key, std::shared_ptr<BPlusTreeNode> rightChild) {
    auto newNode = std::make_shared<BPlusTreeNode>(false);
    std::vector<float> tempKeys = node->keys;
    std::vector<std::shared_ptr<BPlusTreeNode>> tempChildren = node->children;

    tempKeys.insert(tempKeys.begin() + index, key);
    tempChildren.insert(tempChildren.begin() + index + 1, rightChild);

    int mid = order / 2;
    float promotedKey = tempKeys[mid];

    node->keys.assign(tempKeys.begin(), tempKeys.begin() + mid);
    node->children.assign(tempChildren.begin(), tempChildren.begin() + mid + 1);

    newNode->keys.assign(tempKeys.begin() + mid + 1, tempKeys.end());
    newNode->children.assign(tempChildren.begin() + mid + 1, tempChildren.end());

    for (auto& child : newNode->children) {
        child->parent = newNode;
    }

    insertIntoParent(node, promotedKey, newNode);
}

std::shared_ptr<BPlusTreeNode> BPlusTree::findLeaf(float key, int &indexNodeCounter) {
    auto current = root;

    // When the current node is not NULL and the current node is not a leaf, 
    // continue to find the leaf node that contain the key value
    while (current && !current->isLeaf) {

        /*
        'std::lower_bound' will return the 'iterator' that have the 'beginning value'
        that is >= the 'key' value given in the 'current->keys' vector

        Note: This method only works on the sorted range
        
        e.g.
        current->keys = {10, 20, 30, 40, 50}

        if key = 30 ------> it = position of 30 in current->keys, i.e. the address of 30 in 
        main memory

        if key = 25 ------> it = position of 30 in current->keys, i.e. the address of 30 in 
        main memory

        That is why when we want to calculate the 'index' of the value 30 in the current->keys
        we have to use 'std::distance(current->keys.begin(), it)'

        */
        auto it = std::lower_bound(current->keys.begin(), current->keys.end(), key);

        // The index of the lower_bound
        int index = std::distance(current->keys.begin(), it);

        current = current->children[index];

        indexNodeCounter++;
    }
    return current;
}

SearchResult BPlusTree::rangeSearch(float lower, float upper, Storage& storage) {
    auto start = std::chrono::high_resolution_clock::now();
    
    SearchResult result = {0, 0, 0.0f, 0};

    if (!root) return result;

    auto leaf = findLeaf(lower, result.indexNodesAccessed);
    std::set<int> accessedDatablocks;


    /*
    This one will take the key as the BlockId
                       the value as the list / vector of the the offsets in the same block

    */
    //               BlockId              fileOffset  
    std::unordered_map<int, std::vector<std::streampos>> block_offset_results; 

    while (leaf && leaf->keys.front() <= upper) {

        for (size_t i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] > upper) break;
            if (leaf->keys[i] >= lower) {

                /*
                If the 'leaf->recordLocation[i].datablockId' not in the 'accesssedDatablocks'
                the '.find' function will return the 'accessDatablocks.end()
                */
                if (accessedDatablocks.find(leaf->recordLocations[i].datablockId) == accessedDatablocks.end()) {
                    accessedDatablocks.insert(leaf->recordLocations[i].datablockId);
                }
                // When accessing record locations:


                //                                             datablockId                        fileOffset
                // Record record = storage.getRecord(leaf->recordLocations[i].datablockId, leaf->recordLocations[i].fileOffset);

                // Instead of reading a block every time we access the record
                // we can append it into the hash map and we just need 1 time to read all the relevant records in the block

                block_offset_results[leaf->recordLocations[i].datablockId].push_back(leaf->recordLocations[i].fileOffset);
                result.numberOfResults++;
            }
        }
        leaf = leaf->nextLeaf;
    }


    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    
    std::vector<Record> resulting_records;
    Record record;




    // I/O shit

    for (std::pair<int, std::vector<std::streampos>> block_offset_result: block_offset_results){
        // 
        // key_of_dict = block_offset_result.first 
        // Data []

        result.dataBlocksAccessed++;        
        storage.getVectorByRecords(block_offset_result.first, block_offset_results[block_offset_result.first], resulting_records);
        
    }
    
    result.found_records = resulting_records;

    return result;
}

void BPlusTree::printStatistics() {
    std::cout << "----------------- B+ Tree Statistics -----------------" << std::endl;
    std::cout << "Order (maximum number of keys per node): " << order << std::endl;
    std::cout << "Height of the tree: " << tree_height << std::endl;
    std::cout << "Content of root node (keys): ";
    if (root) {
        for (const auto& key : root->keys) {
            std::cout << key << " ";
        }
    }
    std::cout << std::endl;
    std::cout << "B+ Tree Node Count:" << std::endl;
    std::cout << "Total nodes: " << totalNodes << std::endl;
    std::cout << "Internal nodes: " << internalNodes << std::endl;
    std::cout << "Leaf nodes: " << leafNodes << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
}

int BPlusTree::getHeight(std::shared_ptr<BPlusTreeNode> node) {
    int height = 0;
    while (node) {
        height++;
        if (node->isLeaf) break;
        node = node->children[0];
    }
    return height;
}

void BPlusTree::saveToFile() {
    std::ofstream file(indexFilename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open index file for writing: " + indexFilename);
    }

    // Write the order of the B+ tree
    file.write(reinterpret_cast<const char*>(&order), sizeof(order));

    // Perform a level-order traversal to write nodes
    std::queue<std::shared_ptr<BPlusTreeNode>> queue;
    if (root) queue.push(root);

    while (!queue.empty()) {
        auto node = queue.front();
        queue.pop();

        // Write node type (leaf or internal)
        file.write(reinterpret_cast<const char*>(&node->isLeaf), sizeof(bool));

        // Write number of keys
        size_t keyCount = node->keys.size();
        file.write(reinterpret_cast<const char*>(&keyCount), sizeof(size_t));

        // Write keys
        file.write(reinterpret_cast<const char*>(node->keys.data()), keyCount * sizeof(float));

        if (node->isLeaf) {
            // Write record locations for leaf nodes
            file.write(reinterpret_cast<const char*>(node->recordLocations.data()), keyCount * sizeof(RecordLocation));
            
            // Write next leaf pointer (use nullptr if it's the last leaf)
            bool hasNextLeaf = (node->nextLeaf != nullptr);
            file.write(reinterpret_cast<const char*>(&hasNextLeaf), sizeof(bool));
        } else {
            // Write child count for internal nodes
            size_t childCount = node->children.size();
            file.write(reinterpret_cast<const char*>(&childCount), sizeof(size_t));

            // Add children to the queue
            for (const auto& child : node->children) {
                queue.push(child);
            }
        }
    }

    file.close();
    std::cout << "B+ tree saved to file: " << indexFilename << std::endl;
}


// This function will allow the BPlusTree to load all the information in the 
void BPlusTree::loadFromFile() {

    std::ifstream file(indexFilename, std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open index file for reading: " + indexFilename);
    }

    // Read the order of the B+ tree
    file.read(reinterpret_cast<char*>(&order), sizeof(order));

    std::queue<std::pair<std::shared_ptr<BPlusTreeNode>, size_t>> queue;
    std::shared_ptr<BPlusTreeNode> prevLeaf = nullptr;

    root = nullptr;
    while (file.peek() != EOF) {
        auto node = std::make_shared<BPlusTreeNode>();

        // Read node type
        file.read(reinterpret_cast<char*>(&node->isLeaf), sizeof(bool));

        // Read number of keys
        size_t keyCount;
        file.read(reinterpret_cast<char*>(&keyCount), sizeof(size_t));

        // Read keys
        node->keys.resize(keyCount);
        file.read(reinterpret_cast<char*>(node->keys.data()), keyCount * sizeof(float));

        if (node->isLeaf) {
            // Read record locations for leaf nodes
            node->recordLocations.resize(keyCount);
            file.read(reinterpret_cast<char*>(node->recordLocations.data()), keyCount * sizeof(RecordLocation));

            // Read next leaf pointer
            bool hasNextLeaf;
            file.read(reinterpret_cast<char*>(&hasNextLeaf), sizeof(bool));
            
            if (prevLeaf) {
                prevLeaf->nextLeaf = node;
            }
            prevLeaf = node;

            if (!hasNextLeaf) {
                prevLeaf = nullptr;  // Reset for the next level
            }
        } else {
            // Read child count for internal nodes
            size_t childCount;
            file.read(reinterpret_cast<char*>(&childCount), sizeof(size_t));
            node->children.resize(childCount);
            queue.push({node, childCount});
        }

        if (!root) {
            root = node;
        } else if (!queue.empty()) {
            auto& [parent, remainingChildren] = queue.front();
            parent->children[parent->children.size() - remainingChildren] = node;
            node->parent = parent;
            remainingChildren--;
            if (remainingChildren == 0) {
                queue.pop();
            }
        }
    }

    file.close();
    std::cout << "B+ tree loaded from file: " << indexFilename << std::endl;

    tree_height = getHeight(root);
    _getTotalNodes();
}

void BPlusTree::writeNode(std::ofstream& file, std::shared_ptr<BPlusTreeNode> node) {
    file.write(reinterpret_cast<const char*>(&node->isLeaf), sizeof(node->isLeaf));
    
    size_t keyCount = node->keys.size();
    file.write(reinterpret_cast<const char*>(&keyCount), sizeof(keyCount));
    file.write(reinterpret_cast<const char*>(node->keys.data()), keyCount * sizeof(float));

    if (node->isLeaf) {
        file.write(reinterpret_cast<const char*>(node->recordLocations.data()), keyCount * sizeof(RecordLocation));
    } else {
        size_t childCount = node->children.size();
        file.write(reinterpret_cast<const char*>(&childCount), sizeof(childCount));
    }
}

std::shared_ptr<BPlusTreeNode> BPlusTree::readNode(std::ifstream& file) {
    auto node = std::make_shared<BPlusTreeNode>();
    
    file.read(reinterpret_cast<char*>(&node->isLeaf), sizeof(node->isLeaf));
    
    size_t keyCount;
    file.read(reinterpret_cast<char*>(&keyCount), sizeof(keyCount));
    node->keys.resize(keyCount);
    file.read(reinterpret_cast<char*>(node->keys.data()), keyCount * sizeof(float));

    if (node->isLeaf) {
        node->recordLocations.resize(keyCount);
        file.read(reinterpret_cast<char*>(node->recordLocations.data()), keyCount * sizeof(RecordLocation));
    } else {
        size_t childCount;
        file.read(reinterpret_cast<char*>(&childCount), sizeof(childCount));
        node->children.resize(childCount);
    }

    return node;
}

void BPlusTree::verifyTree() {
    if (!root) {
        std::cout << "Tree is empty" << std::endl;
        return;
    }

    std::cout << "Verifying B+ tree structure..." << std::endl;

    if (root->parent.lock()) {
        std::cout << "Error: Root node has a parent" << std::endl;
    }

    std::queue<std::shared_ptr<BPlusTreeNode>> queue;
    queue.push(root);

    while (!queue.empty()) {
        auto node = queue.front();
        queue.pop();

        if (node->isLeaf) {
            if (!node->children.empty()) {
                std::cout << "Error: Leaf node has children" << std::endl;
            }
            if (node->keys.size() != node->recordLocations.size()) {
                std::cout << "Error: Leaf node keys and recordLocations size mismatch" << std::endl;
            }
        } else {
            if (node->children.size() != node->keys.size() + 1) {
                std::cout << "Error: Internal node keys and children size mismatch" << std::endl;
            }
            for (const auto& child : node->children) {
                if (child->parent.lock() != node) {
                    std::cout << "Error: Child-parent link mismatch" << std::endl;
                }
                queue.push(child);
            }
        }

        for (size_t i = 1; i < node->keys.size(); ++i) {

            /*
            Since computer is very stupid in comparing between 2 numbers 
            */
            float epsilon = 0.000001; 

            if (node->keys[i] <= node->keys[i-1] - epsilon) {
                std::cout << "Error: Keys not in ascending order, key[i] = " << node->keys[i] << ", key[i - 1] = " << node->keys[i - 1] << std::endl;
            }
        }

        if (node != root && (node->keys.size() < (order - 1) / 2 || node->keys.size() > order - 1)) {
            std::cout << "Error: Node does not meet occupancy requirements" << std::endl;
        }
    }

    std::cout << "B+ tree verification complete" << std::endl;
}