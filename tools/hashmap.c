#include "hashmap.h"

HashMap_Node *hashnode_init(char *key, size_t item_size, void *value) {
    HashMap_Node *node = (HashMap_Node *) malloc(sizeof(HashMap_Node));
    if (!node) { return NULL; }
    
    if (!value) { node->value = NULL; } 
    else {
        node->value = malloc(item_size);
        if (!node->value) { free(node); return NULL; }
    }

    size_t key_length = strlen(key);
    node->key = (char *)malloc(sizeof(char) * (key_length + 1));
    if (!node->key) { free(node->value); free(node); return NULL; }
    
    if (node->value) { memcpy(node->value, value, item_size); }

    memcpy(node->key, key, key_length);
    node->key[key_length] = 0;

    node->prev = NULL;
    node->next = NULL;

    return node;
}

void hashnode_destroy(HashMap_Node *node) {
    if (!node) return;
    if (node->key) { free(node->key); }
    if (node->value) { free(node->value); }
    free(node);
}


void hashmap_init(HashMap *self, size_t item_size) {
    self->item_size = item_size;
    for (size_t i = 0; i < HASH_MAP_SIZE; ++i) {
        self->nodes[i] = NULL;
    }
}

void hashmap_destroy(HashMap *self) {
    for (size_t i = 0; i < HASH_MAP_SIZE; ++i) {
        HashMap_Node *current = self->nodes[i];
        while (current) {
            HashMap_Node *next = current->next;
            hashnode_destroy(current);
            current = next;
        }
    }
}

size_t hash(char *key) {
    if (!key) { THROW_ERROR("ERROR: `hash` function failed, key == NULL passed to the function\n"); }
    size_t result = 0;
    for (char c = *key; c != '\0'; ++key) {
        c = *key;
        result += c;
    }
    return result % HASH_MAP_SIZE;
}

HashMap_Node *hashmap_get_node(HashMap *self, char *key) {
    size_t index = hash(key);
    HashMap_Node *current = self->nodes[index];
    while (current) {
        if (strcmp(current->key, key) == 0) { return current; }
        current = current->next;
    }
    return NULL;
}

int hashmap_get(HashMap *self, char *key, void *value) {
    HashMap_Node *node = hashmap_get_node(self, key);
    if (!node) { return 0; }
    if (!value) { return 1; } // found it 
    if (!node->value) { return 0; }
    memcpy(value, node->value, self->item_size);
    return 1;
}

int hashmap_find(HashMap *self, char *key) {
    return (hashmap_get_node(self, key) != NULL);
}

int hashmap_add(HashMap *self, char *key, void *value) {
    if (hashmap_find(self, key)) { return 0; }

    HashMap_Node *node = hashnode_init(key, self->item_size, value);
    if (!node) { return 0; }
    
    size_t index = hash(key);
    node->next = self->nodes[index];
    
    if (self->nodes[index]) self->nodes[index]->prev = node;
    
    self->nodes[index] = node;
    return 1;
}

int hashmap_remove(HashMap *self, char *key) {
    HashMap_Node *node = hashmap_get_node(self, key);
    if (!node) { return 0; }
    
    size_t index = hash(key);
    HashMap_Node *prev = node->prev;
    HashMap_Node *next = node->next;

    if (prev) { prev->next = next; } 
    else { self->nodes[index] = next; }

    if (next) { next->prev = prev; }

    hashnode_destroy(node);

    return 1;
}

int hashmap_update(HashMap *self, char *key, void *value) {
    HashMap_Node *node = hashmap_get_node(self, key);
    if (!node) { return 0; }
    if (!value) { node->value = NULL; return 1; }
    memcpy(node->value, value, self->item_size);
    return 1;
}

int hashmap_empty(HashMap *self) {
    for (size_t i = 0; i < HASH_MAP_SIZE; ++i) {
        if (self->nodes[i]) { return 0; }
    } 
    return 1;
}