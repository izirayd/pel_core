#pragma once

#include <cstddef>
#include <iostream>
#include <string.h>
#include <typeinfo>
#include <vector>
#include <functional>
#include <cstring>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <future>

#ifdef PEL_MEMORY_POOL_GRAPH
    #include "memory_pool.hpp"
#endif

namespace pel {

    // use memory_pool
    struct mp {};

	namespace core {

        class graph_info_t {

         public:

            void do_break()        { is_break = true; }
            void do_ignore_child() { is_ignore_child = true; }

            bool is_move_parent = false;
            bool is_move_child = false;
            bool is_move_next = false;

            bool is_break = false;

            bool is_ignore_child = false;

            void reset() {
                is_move_parent = false;
                is_move_child  = false;
                is_move_next   = false;

                is_break = false;

                is_ignore_child = false;
            }

            std::size_t level = 0;
        };

        template <typename vertex_data_t>
        class graph_t
        {
          public:
            graph_t() = default;

            graph_t(const mp &m) { this->use_memory_pool(); }

            graph_t(const vertex_data_t & element) { data = element; }

           ~graph_t()
            {
                if ((next() || child()) && !is_process_delete)
                    delete_graph();

#ifdef PEL_MEMORY_POOL_GRAPH
                if (is_parent_memory_pool && mem_pool)
                    mem_pool->reset();
#endif
            }

            using graph_type_t = graph_t<vertex_data_t>;
            using type_t = vertex_data_t;

            vertex_data_t data;

            vertex_data_t* get_data() { return &data; }

            //std::size_t level = 0;
  
            inline graph_type_t* next()     { return next_element;           }
            inline graph_type_t* previous() { return previous_element;       }
            inline graph_type_t* child()    { return begin_child;            }
            inline graph_type_t* parent()   { return parent_element;         }
            inline graph_type_t* current_parent()   { return current_parent_element; }
            inline graph_type_t* end()      { return end_element;            }

            void set_current_parent(graph_type_t* cur_parent) { current_parent_element = cur_parent; }

            // return last vertex
            inline graph_type_t* push(std::initializer_list<vertex_data_t> list) {

                graph_type_t *current = this;

                for (const auto &l: list)
                {
                    current = current->push(l);
                }

                return current;
            }

            // no child push
            inline graph_type_t* ncpush(std::initializer_list<vertex_data_t> list) {

                graph_type_t* current = this;

                for (const auto& l : list)
                {
                    current->push(l);
                }

                return current->end();
            }

            inline graph_type_t* push(const vertex_data_t &value) {
            
#ifdef PEL_MEMORY_POOL_GRAPH

                if (mem_pool)
                {
                    memory_block_t* block = nullptr;
                    auto element = mem_pool->get<const vertex_data_t&>(block, value);

                    element->mem_block = block;
                    element->mem_pool  = mem_pool;

                    return push(element);
                }

#endif
                return push(new graph_type_t(value));
            }

            inline graph_type_t* push()     { 

#ifdef PEL_MEMORY_POOL_GRAPH

                if (mem_pool)
                {
                    memory_block_t* block = nullptr;
                    auto element = mem_pool->get(block);

                    element->mem_block = block;
                    element->mem_pool = mem_pool;

                    return push(element);
                }

#endif
                return push(new graph_type_t);         
            }

            graph_type_t* push(graph_type_t* vertex, bool is_ignore_parent_link = false) {

                if (!vertex)
                    return nullptr;

                if (!is_ignore_parent_link)
                    vertex->parent_element = this;

                if (begin_child == nullptr)
                {
                    begin_child = vertex;
                }
                else
                {
                    if (begin_child->end_element == nullptr) {

                        vertex->previous_element = begin_child;
                        begin_child->next_element = vertex;

                        if (vertex->end_element)
                        {
                            begin_child->end_element = vertex->end_element;
                        }
                        else
                        {
                            begin_child->end_element = vertex;
                        }
                    }
                    else
                    {
                        begin_child->end_element->next_element = vertex;
                        vertex->previous_element = begin_child->end_element;

                        if (vertex->end_element)
                        {
                            begin_child->end_element = vertex->end_element;
                        }
                        else
                        {
                            begin_child->end_element = vertex;
                        }
                    }
                }

                return vertex;
            }

            void remove() {

                if (this->previous_element)
                    this->previous_element->next_element = this->next_element;

                if (this->next_element)
                    this->next_element->previous_element = this->previous_element;

                if (end_element == this)
                    end_element = this->previous_element;

                if (this->parent_element)
                {
                    if (this->parent_element->begin_child)
                    {
                        if (this->parent_element->begin_child == this)
                        {
                            this->parent_element->begin_child = this->next_element;
                        }
                    }
                }

                graph_type_t* current = this->begin_child;

                if (current) {

                    if (this->previous_element)
                    {
                        this->previous_element->push(current);
                    }
                    else
                        if (this->next_element)
                        {
                            this->next_element->push(current);
                        }
                        else
                            if (this->parent_element)
                            {
                                this->parent_element->push(current);
                            }
                }
            }

            // need test
            void copy(graph_type_t* anower_ast) {

                if (!anower_ast)
                    return;

                //  prev
                if (this->previous_element)
                    this->previous_element->next_element = anower_ast;
                
                //  next
                if (this->next_element) 
                    this->next_element->previous_element = anower_ast;
                
                anower_ast->parent_element   = this->parent_element;
              //  anower_ast->next_element     = this->next_element;
              //  anower_ast->previous_element = this->previous_element;

                if (this->parent())
                {
                    if (this->parent()->end() == this)
                        this->parent()->end_element = anower_ast;
                    

                    if (this->parent()->child() == this)
                        this->parent()->begin_child = anower_ast;

                }

                *this = *anower_ast;
            }

       /*     void operator=(graph_type_t* anower_ast) {

                this->parent_element = anower_ast->parent_element;
                this->begin_child    = anower_ast->begin_child;

            }*/

            // O (1)
            void cut_child(graph_type_t* anower_ast) {

                if (!anower_ast)
                    return;

                if (anower_ast->child())
                {
                    anower_ast->begin_child->end_element->next_element = this->begin_child;
                    anower_ast->begin_child->end_element = this->begin_child->end_element;
                    this->begin_child = nullptr;
                }
                else
                {
                    anower_ast->begin_child = this->begin_child;
                    this->begin_child = nullptr;
                }
            }

            // - o(n) 
            // - non-recursion
            // - depth-first search
            void for_each(std::function<void(graph_type_t*, graph_info_t&)> func) {

                if (!func)
                    return;

                graph_type_t* current = this;
                graph_info_t  graph_info;

                bool is_exit = false;

                for (;;) {

                    func(current, graph_info);

                    if (graph_info.is_break)
                        break;

                    if (!graph_info.is_ignore_child) {

                        if (current->child())
                        {
                            current = current->child();

                            graph_info.reset();
                            graph_info.is_move_child = true;
                            graph_info.level++;

                            continue;
                        }

                    }
                    else
                    {
                        graph_info.reset();
                    }

                    if (current->next())
                    {
                        current = current->next();

                        graph_info.reset();
                        graph_info.is_move_next = true;

                        continue;
                    }

                    current = current->parent();

                    if (current) {

                        graph_info.reset();

                        graph_info.is_move_parent = true;
                        graph_info.level--;

                        while (current) {

                            if (current->next())
                            {
                                current = current->next();

                                graph_info.is_move_next = true;

                                break;
                            }
                            else
                            {
                                current = current->parent();

                                graph_info.level--;
                            }

                            if (!current || current == this) {
                                is_exit = true;
                                break;
                            }
                        }
                    }
                    else {
                        // no parent
                        break;
                    }

                    if (is_exit)
                        break;
                }
            }

            // - o(n) 
            // - non-recursion
            // - depth-first search
            void for_each(std::function<void(graph_type_t*)> func) {

                if (!func)
                    return;

                for_each([&](graph_type_t* graph, graph_info_t&) {
                    func(graph);
                });

            }

            // - o(n) 
            // - non-recursion
            // - brute over without referring to child elements, child will uses only first times
            void for_next(std::function<void(graph_type_t*, graph_info_t&)> func)
            {
                if (!func)
                    return;

                graph_type_t* current = this->child();

                if (!current)
                    return;

                graph_info_t graph_info;

                do {

                    graph_info.is_move_next = true;

                    func(current, graph_info);

                    if (graph_info.is_break)
                        break;

                } while (current = current->next());
            }

            // - o(n) 
            // - non-recursion
             // - brute over without referring to child elements, without first child
            void for_this_next(std::function<void(graph_type_t*, graph_info_t&)> func)
            {
                if (!func)
                    return;

                graph_type_t* current = this;

                graph_info_t graph_info;

                do {

                    graph_info.is_move_next = true;

                    func(current, graph_info);

                    if (graph_info.is_break)
                        break;

                } while (current = current->next());

            }

            // - o(n) 
            // - non-recursion
            // - variant DFS
            // TODO: have bug, Doesn't delete some elements, should it be replaced with a safe cyclic_deletion?
            void delete_graph() {

                graph_type_t* current = this;

                if (this->child())
                    current = this->child();
                else
                    if (current->next())
                        current = current->next();
                    else
                        return;

                for (;;) {

                    if (current == this)
                        return;

                    while (true)
                    {
                        if (!current->child())
                            break;

                        current = current->child();
                    }

                    graph_type_t* tmp_parent = current->parent();

                    while (current)
                    {
                        graph_type_t* next_element_ = current->next();
                        current->is_process_delete = true;

#ifdef PEL_MEMORY_POOL_GRAPH
                        if (current->mem_block && current->mem_pool) {
                            current->mem_pool->free(current->mem_block);
                        }
                        else
                        {
                            delete current;
                        }
#else 
                        delete current;
#endif

                        current = next_element_;
                    }

                    if (tmp_parent) {
                        tmp_parent->begin_child = nullptr;
                        current = tmp_parent;
                    }
                    else
                    {
                        break;
                    }
                }
            }

          
            // O(n * 2), no idea how do it like o(n)
            void cyclic_deletion() {

                std::deque<graph_type_t *>  graph_queue;
                std::vector<graph_type_t *> graph_vector;

                graph_queue.push_back(this);

                pel::core::graph_info_t graph_info;

                bool is_first_element        = true;

                for (;;) {

                    auto current = graph_queue.back();
                    graph_queue.pop_back();

                    graph_info.reset();
                    graph_info.is_move_next = true;

                    do
                    {
                        if (current->next())
                            graph_queue.push_back(current->next());

                        /* function */

                        if (current->is_read)
                            break;

                        graph_vector.push_back(current);

                        current->is_read = true;

                        /* function */

                        if (current->child()) {
                            graph_info.reset();
                            graph_info.is_move_child = true;
                        }

                        is_first_element = false;

                    } while (current = current->child());

                    if (graph_queue.empty())
                        break;
                }


                for (auto &it : graph_vector)
                {
                    auto delete_element = it;

                    if (delete_element)
                    {
                        delete_element->is_process_delete = true;

#ifdef PEL_MEMORY_POOL_GRAPH
                        if (delete_element->mem_block && delete_element->mem_pool) {
                            delete_element->mem_pool->free(delete_element->mem_block);
                        }
                        else
                        {
                            delete delete_element;
                        }
#else 
                        delete delete_element;
#endif

                    }
                }

                graph_vector.clear();
            }

            // not tested
            void delete_only_child() {

                graph_type_t* current       = this;
                graph_type_t* current_child = nullptr;

                do {
                    current_child = current->child();

                    current->is_process_delete = true;

#ifdef PEL_MEMORY_POOL_GRAPH
                    if (current->mem_block && current->mem_pool) {
                        current->mem_pool->free(current->mem_block);
                    }
                    else
                    {
                        delete current;
                    }
#else 
                    delete current;
#endif

                } while (current = current_child);
            }

            void use_memory_pool()
            {
#ifdef PEL_MEMORY_POOL_GRAPH
                mem_pool = new memory_pool_t;
                mem_pool->init();
                is_parent_memory_pool = true;
#endif
            }

            void do_next_null() { next_element = nullptr; }
            void do_previous_null() { previous_element = nullptr; }
            void do_child_null() { begin_child = nullptr; }
            void do_parent_null() { parent_element = nullptr; }
            void do_end_null() { end_element = nullptr; }

          protected:
              graph_type_t* next_element     = nullptr;
              graph_type_t* previous_element = nullptr;
              graph_type_t* begin_child      = nullptr;
              graph_type_t* parent_element   = nullptr; // only as debug!!! because no multiparent support, remove() have bug
              graph_type_t* current_parent_element   = nullptr; // only as debug!!! because no multiparent support, remove() have bug
              graph_type_t* end_element      = nullptr;
              graph_type_t* current_point    = nullptr;

#ifdef PEL_MEMORY_POOL_GRAPH
              using memory_pool_t  = memory_pool::memory_pool_t<graph_t<vertex_data_t>>;
              using memory_block_t = memory_pool::memory_block_t<graph_t<vertex_data_t>>;

              memory_pool_t*  mem_pool  = nullptr;
              memory_block_t* mem_block = nullptr;;
#endif

              bool is_process_delete         = false;
              bool is_parent_memory_pool     = false;
              bool is_read                   = false;
        };	
	}
}