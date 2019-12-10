#ifndef STORAGE_H
#define STORAGE_H

#include <algorithm>
#include <string>
#include <map>
#include <set>
#include <list>
#include <deque>
#include <queue>
#include <vector>
#include <utility>
#include <chrono>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace chrono;

#define EMPTY ""

typedef struct tree_s tree_t;
typedef struct node_s node_t;
typedef struct data_s data_t;
    
struct data_s {
    string key;
    string val;
};

struct node_s {    
    data_t data;
    
    uint64_t weight; // количество узлов у данного узла
    
    node_t *left;
    node_t *right;
};

struct tree_s
{
    node_t *root; // указатель на корень дерева
};

class storage
{    
public:
    
    void insert(string key, string val = "");
    void erase(uint64_t index);
    data_t *get(uint64_t index);
    
    uint64_t search(string key);
        
    uint64_t size();
    void print();
    void print(node_t *p, int indent);
    
    storage();
    ~storage();
    
private:    
    tree_t *tree;
    
    uint64_t get_child_weight(node_t *node);
    node_t *node_new();
    void node_free(node_t *e);
    
    bool erase_simple(node_t *search_node, node_t *prev_node);
    
    void clear(node_t *p);
};

#endif