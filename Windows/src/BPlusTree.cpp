#include "BPlusTree.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <stack>
#include <set>
#include <unordered_map>


bool compareRecordPairs(const std::pair<float, uint32_t>& a, const std::pair<float, uint32_t>& b) {
    
    // First we compare the key
    if (a.first != b.first) {
        return a.first < b.first;
    }
    // If keys are equal, compare datablockId first, then fileOffset
    // if (a.second.datablockId != b.second.datablockId) {
    //     return a.second.datablockId < b.second.datablockId;
    // }
    return a.second < b.second;
}



BPlusTree::BPlusTree(int order, const std::string& indexFilename) 
    : order(order), indexFilename(indexFilename), root(nullptr) {}

void BPlusTree::buildFromStorage(const Storage& storage) {
    std::cout << "Starting to build B+ tree from storage..." << std::endl;

    auto records = storage.getAllRecords();
    std::cout << "Retrieved " << records.size() << " records from storage." << std::endl;

    int count = 0;
    for (const auto& record : records) {
        try {

            // START TUAN's CODE

            // int temp;
            // std::shared_ptr<BPlusTreeNode> temp_leaf = findLeaf(record.fgPctHome, temp);
            // if (root != nullptr){
            //     if (std::find(temp_leaf->keys.begin(), temp_leaf->keys.end(), record.fgPctHome) != temp_leaf->keys.end()){
            //         continue;
            //     }
            // }

            // END TUAN's CODE

            insert(record.fgPctHome, record.recordId);
            count++;
            if (count % 1000 == 0) {
                std::cout << "Inserted " << count << " records into the B+ tree." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error inserting record: " << e.what() << std::endl;
            std::cerr << "Record details: fgPctHome=" << record.fgPctHome 
                      << ", recordId=" << record.recordId << std::endl;
        }
    }

    std::cout << "Finished building B+ tree. Total records inserted: " << count << std::endl;
    std::cout << "Saving B+ tree to file..." << std::endl;

    saveToFile();
    std::cout << "B+ tree saved to file." << std::endl;
    tree_height = getHeight(root);
    _getTotalNodes();
}

void BPlusTree::insert(float key, uint32_t recordId) {
    if (!root) {
        root = std::make_shared<BPlusTreeNode>(true);


        root->keys.push_back(key);


        root->recordIds.push_back(recordId);
        return;
    }

    int temp;
    auto leaf = findLeaf(key, temp);

    if (!leaf) {
        throw std::runtime_error("findLeaf returned nullptr");
    }

    if (leaf->keys.size() < order - 1) {
        insertIntoLeaf(leaf, key, recordId);
    } else {
        splitLeafNode(leaf, key, recordId);
    }
}

void BPlusTree::insertIntoLeaf(std::shared_ptr<BPlusTreeNode> leaf, float key, uint32_t recordId) {
    auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    int index = std::distance(leaf->keys.begin(), it);
    
    leaf->keys.insert(leaf->keys.begin() + index, key);
    leaf->recordIds.insert(leaf->recordIds.begin() + index, recordId);
}

void BPlusTree::splitLeafNode(std::shared_ptr<BPlusTreeNode> leaf, float newKey, uint32_t recordId) {
    auto newLeaf = std::make_shared<BPlusTreeNode>(true);
    
    std::vector<float> tempKeys = leaf->keys;
    std::vector<uint32_t> tempRecordIds = leaf->recordIds;

    tempKeys.push_back(newKey);
    tempRecordIds.push_back(recordId);

    //                     key   recordId
    std::vector<std::pair<float, uint32_t>> pairs;


    for (size_t i = 0; i < tempKeys.size(); ++i) {
        pairs.emplace_back(tempKeys[i], tempRecordIds[i]);
    }

    std::sort(pairs.begin(), pairs.end(), compareRecordPairs);

    int mid = (order + 1) / 2;
    leaf->keys.clear();
    leaf->recordIds.clear();
    newLeaf->keys.clear();
    newLeaf->recordIds.clear();

    for (int i = 0; i < mid; ++i) {
        leaf->keys.push_back(pairs[i].first);
        leaf->recordIds.push_back(pairs[i].second);
    }
    for (size_t i = mid; i < pairs.size(); ++i) {
        newLeaf->keys.push_back(pairs[i].first);
        newLeaf->recordIds.push_back(pairs[i].second);
    }

    newLeaf->nextLeaf = leaf->nextLeaf;
    leaf->nextLeaf = newLeaf;

    float promotedKey = newLeaf->keys.front();
    insertIntoParent(leaf, promotedKey, newLeaf);
}

void BPlusTree::insertIntoParent(std::shared_ptr<BPlusTreeNode> leftChild, float key, std::shared_ptr<BPlusTreeNode> rightChild) {
    try{
        if (leftChild == root) {
        auto newRoot = std::make_shared<BPlusTreeNode>(false);

        newRoot->keys.push_back(key);
        newRoot->children.push_back(leftChild);
        newRoot->children.push_back(rightChild);


        root = newRoot;

        leftChild->parent = newRoot;
        // TODO: Debug

        // std::cout << "DEBUG: in 'insertIntoParrent', leftChild's parrent address: " << rightChild->parent.lock() << std::endl;
        
        //TODO: Debugging
        // std::cout << "DEBUG: Left child's parrent address " << leftChild->parent.lock() << std::endl;
        

        rightChild->parent = newRoot;


        // TODO:Debug
        //std::cout << "DEBUG: Right child's parrent address " << leftChild->parent.lock() << std::endl;
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

            // TODO: Debug

            // std::cout << "DEBUG: in 'insertIntoParrent', rightChild's parrent address: " << rightChild->parent.lock() << std::endl;

        } else {
            splitNonLeafNode(parent, index, key, rightChild);
        }
    } catch (const std::exception& e){
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

        // TODO: Debug

        // std::cout << "DEBUG: in 'splitNonLeafNode', child's parent address " << child->parent.lock() << std::endl;
    }

    insertIntoParent(node, promotedKey, newNode);
}

std::shared_ptr<BPlusTreeNode> BPlusTree::findLeaf(float key, int& indexNodeCounter) {
    auto current = root;
    while (current && !current->isLeaf) {
        auto it = std::lower_bound(current->keys.begin(), current->keys.end(), key);
        int index = std::distance(current->keys.begin(), it);
        current = current->children[index];
        indexNodeCounter++;
    }
    return current;
}

SearchResult BPlusTree::rangeSearch(float lower, float upper, Storage& storage) {
    SearchResult result = {0, 0, 0.0f, 0};
    if (!root) return result;

    auto leaf = findLeaf(lower, result.indexNodesAccessed);
    std::unordered_map<uint32_t, std::vector<uint16_t>> datablockRecordIds;

    // START OLD VERSION


    while (leaf && leaf->keys.front() <= upper) {        
        for (size_t i = 0; i < leaf->keys.size(); ++i) {
            
            if (leaf->keys[i] > upper) break;
            if (leaf->keys[i] >= lower) {
                uint16_t recordId = leaf->recordIds[i];
                uint16_t datablockId = storage.getRecordLocations().at(recordId).first;
                datablockRecordIds[datablockId].push_back(recordId);
                result.numberOfResults++;
            }
        }
        leaf = leaf->nextLeaf;
    }

    std::vector<Record> resulting_records;
    for (const auto& pair : datablockRecordIds) {
        result.dataBlocksAccessed++;
        auto records = storage.bulkRead(pair.second);
        resulting_records.insert(resulting_records.end(), records.begin(), records.end());
    }

    result.found_records = resulting_records;

    // END OLD VERSION





    // START TUAN's CODE

    // std::vector<Record> recordsInBlock;

    // while (leaf && leaf->keys.front() <= upper) {

    //     for (size_t i = 0; i < leaf->keys.size(); ++i) {
    //         if (leaf->keys[i] > upper) break;
    //         if (leaf->keys[i] >= lower) {

    //             uint16_t recordId = leaf->recordIds[i];
    //             uint16_t datablockId = storage.getRecordLocations().at(recordId).first;


    //             bool keep_loop = true;
                
    //             while(keep_loop){
    //                 recordsInBlock = storage.getRecordsWithBlockId(datablockId);
    //                 result.dataBlocksAccessed++;

    //                 for (Record &record:recordsInBlock){

    //                     std::cout << "cur fgt: " << record.fgPctHome << std::endl;

    //                     if (record.fgPctHome > upper){
    //                         keep_loop = false;
    //                         std::cout << "we actually break the loop" << std::endl;
    //                         break;
    //                     }

    //                     if (record.fgPctHome >= lower){
    //                         result.found_records.push_back(record);
    //                     }
    //                 }

    //                 recordsInBlock.clear();
    //                 std::cout << "cur_datablockid: " << datablockId << " block count: " << storage.getDatablockCount() << std::endl;
    //                 datablockId++;

    //                 if (datablockId > storage.getDatablockCount()){
    //                     keep_loop = false;
    //                 }
    //             }
                
                
    //             result.numberOfResults++;
    //         }
    //     }

    //     leaf = leaf->nextLeaf;
    // }




    // if (!result.found_records.empty()) {
    //     float totalFG3PctHome = 0.0f;
    //     for (const auto& record : result.found_records) {
    //         totalFG3PctHome += record.fg3PctHome;
    //     }
    //     result.avgFG3PctHome = totalFG3PctHome / result.found_records.size();
    // }

    // END TUAN's CODE

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

        writeNode(file, node);

        if (!node->isLeaf) {
            for (const auto& child : node->children) {
                queue.push(child);
            }
        }
    }

    file.close();
    std::cout << "B+ tree saved to file: " << indexFilename << std::endl;
}

void BPlusTree::loadFromFile() {
    std::ifstream file(indexFilename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open index file for reading: " + indexFilename);
    }

    // Read the order of the B+ tree
    file.read(reinterpret_cast<char*>(&order), sizeof(order));

    std::queue<std::pair<std::shared_ptr<BPlusTreeNode>, size_t>> queue;
    std::shared_ptr<BPlusTreeNode> prevLeaf = nullptr;

    root = readNode(file);
    if (root) queue.push({root, root->keys.size() + 1});

    while (!queue.empty()) {
        auto [parent, childCount] = queue.front();
        queue.pop();

        for (size_t i = 0; i < childCount; ++i) {
            auto child = readNode(file);
            if (!child) break;

            parent->children.push_back(child);
            child->parent = parent;

            // TODO: Debug
            // std::cout << "DEBUG: child's Parrent address: " << child->parent.lock() << std::endl;

            if (!child->isLeaf) {
                queue.push({child, child->keys.size() + 1});
            } else {
                if (prevLeaf) {
                    prevLeaf->nextLeaf = child;
                }
                prevLeaf = child;
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
        file.write(reinterpret_cast<const char*>(node->recordIds.data()), keyCount * sizeof(uint32_t));
        
        bool hasNextLeaf = (node->nextLeaf != nullptr);
        file.write(reinterpret_cast<const char*>(&hasNextLeaf), sizeof(bool));
    }
}

std::shared_ptr<BPlusTreeNode> BPlusTree::readNode(std::ifstream& file) {
    if (file.peek() == EOF) return nullptr;

    auto node = std::make_shared<BPlusTreeNode>();
    
    file.read(reinterpret_cast<char*>(&node->isLeaf), sizeof(node->isLeaf));
    
    size_t keyCount;
    file.read(reinterpret_cast<char*>(&keyCount), sizeof(keyCount));
    node->keys.resize(keyCount);
    file.read(reinterpret_cast<char*>(node->keys.data()), keyCount * sizeof(float));

    if (node->isLeaf) {
        node->recordIds.resize(keyCount);
        file.read(reinterpret_cast<char*>(node->recordIds.data()), keyCount * sizeof(uint32_t));

        bool hasNextLeaf;
        file.read(reinterpret_cast<char*>(&hasNextLeaf), sizeof(bool));
        // Note: We'll set the nextLeaf pointer when we read the next leaf node
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
            if (node->keys.size() != node->recordIds.size()) {
                std::cout << "Error: Leaf node keys and recordIds size mismatch" << std::endl;
            }
        } else {
            if (node->children.size() != node->keys.size() + 1) {
                std::cout << "Error: Internal node keys and children size mismatch" << std::endl;
            }
            for (const auto& child : node->children) {
                if (child->parent.lock() != node) {

                    std::cout << "child's parrent address: " << child->parent.lock() << std::endl;
                    std::cout << "node's address: " << node << std::endl;
                    std::cout << "children node's keys: ";
                    for (float key: child->keys){
                        std::cout << key << " ";
                    }
                    std::cout << std::endl;


                    std::cout << "Error: Child-parent link mismatch" << std::endl;
                }
                queue.push(child);
            }
        }

        for (size_t i = 1; i < node->keys.size(); ++i) {
            float epsilon = 0.000001; 
            if (node->keys[i] <= node->keys[i-1] - epsilon) {
                std::cout << "Error: Keys not in ascending order, key[" << i << "] = " << node->keys[i] 
                          << ", key[" << (i-1) << "] = " << node->keys[i-1] << std::endl;
            }
        }

        if (node != root && (node->keys.size() < (order - 1) / 2 || node->keys.size() > order - 1)) {
            std::cout << "Error: Node does not meet occupancy requirements" << std::endl;
        }
    }

    std::cout << "B+ tree verification complete" << std::endl;
}

void BPlusTree::_getTotalNodes() {
    if (!root) {
        std::cout << "Tree is empty. Total nodes: 0" << std::endl;
        return;
    }

    std::queue<std::shared_ptr<BPlusTreeNode>> queue;
    queue.push(root);

    totalNodes = 0;
    internalNodes = 0;
    leafNodes = 0;

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

std::vector<int> BPlusTree::getNodeCounts() const {
    return {internalNodes, leafNodes, totalNodes};
}