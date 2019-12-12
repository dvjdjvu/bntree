#include "btree.hpp"

btree::btree() {
    this->tree = new tree_t();
}

btree::~btree() {
    this->clear(this->tree->root);
    delete this->tree->root;
}

node_t *btree::node_new() {
    return new node_t();
}

void btree::node_free(node_t *n) {
    delete n;
}

void btree::clear(node_t *p) {
    if (p != NULL) {
        if (p->right) {
            this->clear(p->right);
            this->node_free(p->right);
        }

        if (p->left) {
            this->clear(p->left);
            this->node_free(p->left);
        }
    }
}

uint64_t btree::search(string key) {
    if (!this->tree->root) {
        return -1;
    }

    node_t *search_node = this->tree->root;
    uint64_t index_node = this->get_child_weight(search_node->left);

    for (;;) {
        if (key == search_node->data.key) {
            return index_node; // найден узел с таким ключем, вернем его индекс
        } else if (key > search_node->data.key) {
            search_node = search_node->right;

            if (search_node == NULL)
                return -1;

            index_node += (this->get_child_weight(search_node->left) + 1);
        } else {
            search_node = search_node->left;

            if (search_node == NULL)
                return -1;

            index_node -= (this->get_child_weight(search_node->left) + 1);
        }
    }
}

void btree::insert(string key, string val) {
    // Защита от вставки элемента который уже существует
    if (this->search(key) < 0) {
        return;
    }

    node_t *search_node, **node;

    node = &this->tree->root;
    search_node = this->tree->root;

    for (;;) {
        if (search_node == NULL) {
            search_node = *node = this->node_new();

            search_node->data.key = key;
            search_node->data.val = val;
            search_node->weight = 1; // новый узел имеет вес 1

            search_node->left = search_node->right = NULL;
            return;
        } else if (key > search_node->data.key) {
            search_node->weight++; // увеличиваем вес узла
            node = &search_node->right;
            search_node = search_node->right;
        } else {
            search_node->weight++; // увеличиваем вес узла
            node = &search_node->left;
            search_node = search_node->left;
        }
    }
}

bool btree::erase_simple(node_t *search_node, node_t *prev_node) {
    if (!search_node->left && !search_node->right) {
        // Удаляемый узел является листом.

        // Обнуляем соответствующую ссылку родителя.
        if (prev_node == NULL) {
            // Удаляемый узел корень.
            this->tree->root = NULL;
        } else if (prev_node->left == search_node) {
            prev_node->left = NULL;
        } else if (prev_node->right == search_node) {
            prev_node->right = NULL;
        }

    } else if (search_node->left && !search_node->right) {
        // Удаляемый узел имеет только левого ребенка.

        // Перекомпануем ссылки родителя на левого внука.
        if (prev_node == NULL) {
            // Удаляемый узел корень.
            this->tree->root = search_node->left;
        } else if (prev_node->left == search_node) {
            prev_node->left = search_node->left;
        } else if (prev_node->right == search_node) {
            prev_node->right = search_node->left;
        }

    } else if (!search_node->left && search_node->right) {
        // Удаляемый узел имеет только правого ребенка.

        // Перекомпануем ссылки родителя на правого внука.
        if (prev_node == NULL) {
            // Удаляемый узел корень.
            this->tree->root = search_node->right;
        } else if (prev_node->left == search_node) {
            prev_node->left = search_node->right;
        } else if (prev_node->right == search_node) {
            prev_node->right = search_node->right;
        }

    } else {
        return false;
    }

    return true;
}

/*
void btree::balance(node_t *p, uint64_t index) {
    uint64_t lw = get_child_weight(p->left);
    uint64_t rw = get_child_weight(p->right);

    uint64_t _index;

    if (abs(lw - rw) > 1) {
        if (lw - rw > 1) {
            // Левое поддерево весит больше правого
            _index = index - 1;
        } else if (rw - lw > 1) {
            // Правое поддерево весит больше левого
            _index = index + 1;
        }

        node_t *rep_node = p;
        node_t *prev_node = NULL;
        node_t *search_node = p;
        uint64_t index_node = index;
        
        data_t _d = p->data;
        
        // Ищем узел со следующим индексом.
        for (;;) {
            if (_index == index_node) {
                // Узел найден
                break;
            } else if (_index > index_node) {
                search_node->weight--;
                prev_node = search_node;
                search_node = search_node->right;
                index_node += (this->get_child_weight(search_node->left) + 1);
            } else {
                search_node->weight--;
                prev_node = search_node;
                search_node = search_node->left;
                index_node -= (this->get_child_weight(search_node->right) + 1);
            }

            rep_node->data = search_node->data;
            this->erase_simple(search_node, prev_node);
        }

        this->node_free(search_node);

    }

    return;
}
*/
void btree::erase(uint64_t index) {

    if (index < 0 || index > this->size() - 1) {
        return;
    }

    node_t *search_node = this->tree->root;
    uint64_t index_node = this->get_child_weight(search_node->left);

    node_t *prev_node = NULL;

    for (;;) {
        if (index == index_node) {
            if (this->erase_simple(search_node, prev_node)) {
                // pass
            } else if (search_node->left && search_node->right) {
                // Самый сложный случай, удаляемый узел имеет 2-х детей.

                // Обходим через право.
                node_t *del_node = search_node;
                uint64_t _index;
                // Балансировка, если правый вес больше левого то удаляем через право
                // иначе через лево
                if (search_node->right->weight > search_node->left->weight) {
                    _index = index_node + 1;
                } else {
                    _index = index_node - 1;
                }

                // Ищем узел со следующим индексом.
                for (;;) {
                    if (_index == index_node) {
                        // Узел найден
                        break;
                    } else if (_index > index_node) {
                        search_node->weight--;
                        prev_node = search_node;
                        search_node = search_node->right;
                        index_node += (this->get_child_weight(search_node->left) + 1);
                    } else {
                        search_node->weight--;
                        prev_node = search_node;
                        search_node = search_node->left;
                        index_node -= (this->get_child_weight(search_node->right) + 1);
                    }
                }

                del_node->data = search_node->data;
                this->erase_simple(search_node, prev_node);
            }

            this->node_free(search_node);

            return;
        } else if (index > index_node) {
            search_node->weight--;
            prev_node = search_node;
            search_node = search_node->right;
            index_node += (this->get_child_weight(search_node->left) + 1);
        } else {
            search_node->weight--;
            prev_node = search_node;
            search_node = search_node->left;
            index_node -= (this->get_child_weight(search_node->right) + 1);
        }

    }
}

void btree::erase(string key) {

    node_t *search_node = this->tree->root;
    uint64_t index_node = this->get_child_weight(search_node->left);

    node_t *prev_node = NULL;

    for (;;) {
        if (key == search_node->data.key) {
            if (this->erase_simple(search_node, prev_node)) {
                // pass
            } else if (search_node->left && search_node->right) {
                // Самый сложный случай, удаляемый узел имеет 2-х детей.

                // Обходим через право.
                node_t *del_node = search_node;
                uint64_t _index;
                // Балансировка, если правый вес больше левого то удаляем через право
                // иначе через лево
                if (search_node->right->weight > search_node->left->weight) {
                    _index = index_node + 1;
                } else {
                    _index = index_node - 1;
                }

                // Ищем узел со следующим индексом.
                for (;;) {
                    if (_index == index_node) {
                        // Узел найден
                        break;
                    } else if (_index > index_node) {
                        search_node->weight--;
                        prev_node = search_node;
                        search_node = search_node->right;
                        index_node += (this->get_child_weight(search_node->left) + 1);
                    } else {
                        search_node->weight--;
                        prev_node = search_node;
                        search_node = search_node->left;
                        index_node -= (this->get_child_weight(search_node->right) + 1);
                    }
                }

                del_node->data = search_node->data;
                this->erase_simple(search_node, prev_node);
            }

            this->node_free(search_node);

            return;
        } else if (key > search_node->data.key) {
            search_node->weight--;
            prev_node = search_node;
            search_node = search_node->right;
            index_node += (this->get_child_weight(search_node->left) + 1);
        } else {
            search_node->weight--;
            prev_node = search_node;
            search_node = search_node->left;
            index_node -= (this->get_child_weight(search_node->right) + 1);
        }

    }
}

uint64_t btree::get_child_weight(node_t *node) {
    if (node) {
        return node->weight;
    }

    return 0;
}

data_t *btree::get(uint64_t index) {
    if (index < 0 || index > this->size() - 1) {
        return NULL;
    }

    node_t *search_node = this->tree->root;
    uint64_t index_node = this->get_child_weight(search_node->left);

    for (;;) {
        if (index == index_node) {
            return &search_node->data;
        } else if (index > index_node) {
            search_node = search_node->right;
            index_node += (this->get_child_weight(search_node->left) + 1);
        } else {
            search_node = search_node->left;
            index_node -= (this->get_child_weight(search_node->right) + 1);
        }
    }
}

data_t *btree::get(string key) {
    node_t *search_node = this->tree->root;
    uint64_t index_node = this->get_child_weight(search_node->left);

    for (;;) {
        if (key == search_node->data.key) {
            return &search_node->data;
        } else if (key > search_node->data.key) {
            search_node = search_node->right;
            index_node += (this->get_child_weight(search_node->left) + 1);
        } else {
            search_node = search_node->left;
            index_node -= (this->get_child_weight(search_node->right) + 1);
        }
    }
}

uint64_t btree::size() {
    if (this->tree->root) {
        return this->tree->root->weight;
    }

    return 0;
}

void btree::print() {
    this->print(this->tree->root, 5);
}

void btree::print(node_t *p, int indent) {
    if (p != NULL) {
        if (p->right) {
            this->print(p->right, indent + 4);
        }
        if (indent) {
            std::cout << std::setw(indent) << ' ' << p->weight << ' ';
        }
        if (p->right) {
            std::cout << " /\n" << std::setw(indent) << ' ';
        }
        std::cout << p->data.key << "\n ";
        if (p->left) {
            std::cout << std::setw(indent) << ' ' << " \\\n";
            this->print(p->left, indent + 4);
        }
    }
}


