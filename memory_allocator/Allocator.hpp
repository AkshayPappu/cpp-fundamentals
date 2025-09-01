#pragma once
#include <cstddef>
#include <optional>

static const size_t START = 0;

struct Block {
    size_t start;
    size_t size;

    Block* prev;
    Block* next;

    Block(size_t start, size_t size) : start(start), size(size), prev(nullptr), next(nullptr) {};
    ~Block() = default;
};

class FreeList {
    private:
        Block* head;
    public:
        FreeList(size_t size) : head(new Block(START, size)) {};

        Block* find_fit(size_t need) {
            Block* cur = head;
            while (cur != nullptr) {
                if (cur->size >= need) {
                    return cur;
                }
                cur = cur->next;
            }
            return nullptr;

        };

        void split(Block* block, size_t take) {
            if (!block || take == 0 || block->size <= take) return;
            block->start += take;
            block->size -= take;
        }

        void remove(Block* block) {
            if (!block) return;
            if (block == head) {
                head = head->next;
                if (head) head->prev = nullptr;
                block->next = nullptr;
                block->prev = nullptr;
                return; 
            }

            Block* prev = block->prev;
            Block* next = block->next;
            if (prev != nullptr) {
                prev->next = next;
                block->prev = nullptr;
            }
            if (next != nullptr) {
                next->prev = prev;
                block->next = nullptr;
            }

        };


        bool insert(Block* block) {
            if (!block) return false;
            block->prev = block->next = nullptr;

            if (!head) {
                head = block;
                return true;
            }

            Block* prev = nullptr;
            Block* cur = head;

            while (cur && cur->start < block->start) {
                prev = cur;
                cur = cur->next;
            }

            if (prev && prev->start + prev->size > block->start) return false;
            if (cur && block->start + block->size > cur->start) return false;

            block->prev = prev;
            block->next = cur;
            if (prev) prev->next = block; else head = block;
            if (cur) cur->prev = block;

            if (block->prev && block->prev->start + block->prev->size == block->start) {
                Block* p = block->prev;
                p->size += block->size;
                p->next = block->next;
                if (block->next) block->next->prev = p;
                delete block;
                block = p;
            }

            if (block->next && block->start + block->size == block->next->start) {
                Block* n = block->next;
                block->size += n->size;
                block->next = n->next;
                if (n->next) n->next->prev = block;
                delete n;
            } 
            return true;
        };
};
    

class Allocator {
    private:
        FreeList freeList;
    public:
        Allocator(size_t size) : freeList(size) {};

        std::optional<size_t> allocate(size_t size) {
            Block* best_block = freeList.find_fit(size);
            if (best_block == nullptr) return std::nullopt;

            size_t best_start = best_block->start;
            if (best_block->size > size) {
                freeList.split(best_block, size);
            }
            else {
                freeList.remove(best_block);
                best_start = best_block->start;
                delete best_block;
            }
            return best_start;
        };

        bool deallocate(size_t start, size_t size) {
            Block* new_block = new Block(start, size);
            if (!freeList.insert(new_block)) {
                delete new_block;
                return false;
            }
            return true;
        };
};