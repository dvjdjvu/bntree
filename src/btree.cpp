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
    node_t *search_node, *prev_node, **node;

    node = &this->tree->root;
    search_node = this->tree->root;
    prev_node = NULL;
    uint64_t index_node;

    if (!this->tree->root) {
        index_node = 0;
    } else {
        index_node = this->get_child_weight(search_node->left);
    }

    uint64_t index_del = -1;

    for (;;) {

        if (search_node == NULL) {
            // Добавялем узел
            search_node = *node = this->node_new();

            search_node->data.key = key;
            search_node->data.val = val;
            search_node->weight = 1; // новый узел имеет вес 1

            search_node->left = search_node->right = NULL;
            search_node->parent = prev_node;

            break;
        } else if (key > search_node->data.key) {
            // Идем направо
            prev_node = search_node;
            search_node->weight++; // увеличиваем вес узла
            node = &search_node->right;
            search_node = search_node->right;

            if (search_node)
                index_node += (this->get_child_weight(search_node->left) + 1);

        } else if (key < search_node->data.key) {
            // Идем налево
            prev_node = search_node;
            search_node->weight++; // увеличиваем вес узла
            node = &search_node->left;
            search_node = search_node->left;

            if (search_node)
                index_node -= (this->get_child_weight(search_node->left) + 1);
        } else {
            search_node->data.val = val;

            // Идем назад и отменяем изменение веса у верхних узлов
            for (;;) {

                if (search_node->parent) {
                    search_node = search_node->parent;
                } else {
                    return;
                }

                search_node->weight--;
            }
        }
    }

    this->balance(search_node);

    return;
}

bool btree::erase_simple(node_t *search_node) {

    node_t *prev_node = search_node->parent;

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
            this->tree->root->parent = NULL;
        } else if (prev_node->left == search_node) {
            prev_node->left = search_node->left;
            search_node->left->parent = prev_node;
        } else if (prev_node->right == search_node) {
            prev_node->right = search_node->left;
            search_node->left->parent = prev_node;
        }

    } else if (!search_node->left && search_node->right) {
        // Удаляемый узел имеет только правого ребенка.

        // Перекомпануем ссылки родителя на правого внука.
        if (prev_node == NULL) {
            // Удаляемый узел корень.
            this->tree->root = search_node->right;
            this->tree->root->parent = NULL;
        } else if (prev_node->left == search_node) {
            prev_node->left = search_node->right;
            search_node->right->parent = prev_node;
        } else if (prev_node->right == search_node) {
            prev_node->right = search_node->right;
            search_node->right->parent = prev_node;
        }

    } else {
        // Удаляемый узел имеет двух детей. Такой случай здесь не обрабатывается.

        return false;
    }

    return true;
}

void btree::erase(uint64_t index) {

    if (index < 0 || index > this->size() - 1) {
        return;
    }

    node_t *search_node = this->tree->root;
    uint64_t index_node = this->get_child_weight(search_node->left);

    for (;;) {
        if (index == index_node) {
            if (this->erase_simple(search_node)) {
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
                        search_node = search_node->right;
                        index_node += (this->get_child_weight(search_node->left) + 1);
                    } else {
                        search_node->weight--;
                        search_node = search_node->left;
                        index_node -= (this->get_child_weight(search_node->right) + 1);
                    }
                }

                del_node->data = search_node->data;
                this->erase_simple(search_node);
            }

            this->node_free(search_node);

            return;
        } else if (index > index_node) {
            search_node->weight--;
            search_node = search_node->right;
            index_node += (this->get_child_weight(search_node->left) + 1);
        } else {
            search_node->weight--;
            search_node = search_node->left;
            index_node -= (this->get_child_weight(search_node->right) + 1);
        }
    }
}

void btree::erase(string key) {

    node_t *search_node = this->tree->root;
    uint64_t index_node = this->get_child_weight(search_node->left);

    for (;;) {
        if (key == search_node->data.key) {
            if (this->erase_simple(search_node)) {
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
                        search_node = search_node->right;
                        index_node += (this->get_child_weight(search_node->left) + 1);
                    } else {
                        search_node->weight--;

                        search_node = search_node->left;
                        index_node -= (this->get_child_weight(search_node->right) + 1);
                    }
                }

                del_node->data = search_node->data;
                this->erase_simple(search_node);
            }

            this->node_free(search_node);

            return;
        } else if (key > search_node->data.key) {
            search_node->weight--;
            search_node = search_node->right;
            index_node += (this->get_child_weight(search_node->left) + 1);
        } else {
            search_node->weight--;
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

    return NULL;
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
            //std::cout << std::setw(indent) << ' ';
        }
        if (p->right) {
            std::cout << " /\n" << std::setw(indent) << ' ';
        }
        std::cout << p->data.key << ":" << p->data.val << "\n ";
        //std::cout << p->data.key << ":" << p << ":" << p->parent << "\n ";
        if (p->left) {
            std::cout << std::setw(indent) << ' ' << " \\\n";
            this->print(p->left, indent + 4);
        }
    }
}

/*
 * Возвращает первое число в степени 2, которое больше или ровно x
 */
uint64_t btree::cpl2(uint64_t x) {
    x = x - 1;
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);
    x = x | (x >> 32);
    return x + 1;
}

/*
 * Балансировка поворотом
 * Балансировка делается на основе весов левого и правого поддерева
 */
void btree::balance(node_t *p) {
    if (!p) {
        return;
    }

    node_t *child = NULL;
    uint64_t ld = 0;
    uint64_t rd = 0;

    for (;;) {

        ld = weight_to_depth(p->left);
        rd = weight_to_depth(p->right);

        if (ld > rd && ld - rd > 1) {
            // Правый поворот. 
            // Глубина левого поддерева больше, чем глубина правого

            child = p->left;

            // Такой поворот не изменит ситуацию
            if (child->right == NULL) {
                return;
            }
            
            this->tmb_data = p->data;
            p->data = child->data;
            child->data = this->tmb_data;

            p->left = child->left;
            if (p->left) {
                p->left->parent = p;
            }

            child->left = child->right;
            if (child->left) {
                child->left->parent = child;
            }
            
            child->right = p->right;
            if (child->right) {
                child->right->parent = child;
            }
            
            p->right = child;

            if (p->right) {
                p->right->weight = 1 + get_child_weight(p->right->left) + get_child_weight(p->right->right);
            }

            p->weight = 1 + get_child_weight(p->left) + get_child_weight(p->right);

            break;
        } else if (rd > ld && rd - ld > 1) {
            // Левый поворот. 
            // Глубина правого поддерева больше, чем глубина левого

            child = p->right;

            // Такой поворот не изменит ситуацию
            if (child->left == NULL) {
                return;
            }
            
            this->tmb_data = p->data;
            p->data = child->data;
            child->data = this->tmb_data;

            p->right = child->right;
            if (p->right) {
                p->right->parent = p;
            }

            child->right = child->left;
            if (child->right) {
                child->right->parent = child;
            }
            
            child->left = p->left;
            if (child->left) {
                child->left->parent = child;
            }
            
            p->left = child;

            if (p->left) {
                p->left->weight = 1 + get_child_weight(p->left->left) + get_child_weight(p->left->right);
            }

            p->weight = 1 + get_child_weight(p->left) + get_child_weight(p->right);
            
            break;
        }

        if (p->parent) {
            p = p->parent;
        } else {
            break;
        }
    }

    return;
}

/*
 * Двоичный логарифм от числа
 */
long btree::ilog2(long d) {
    int result;
    std::frexp(d, &result);
    return result - 1;
}

/*
 * Вес к глубине
 */
uint64_t btree::weight_to_depth(node_t *p) {
    if (p == NULL) {
        return 0;
    }

    return this->ilog2(this->cpl2(p->weight));
}
