#pragma once

#include <iostream>
#include <queue>
#include <memory>

#include "ast.hpp"
#include "memory_pool.hpp"

namespace pel {

    namespace core {

        namespace interpreter {

#define select_spatial number_spatial == 4 ? spatial4 \
    : number_spatial == 3 ? spatial3 \
    : number_spatial == 2 ? spatial2 \
    : number_spatial == 1 ? spatial1 \
    : nullptr

#define select_spatial_no_nullptr number_spatial == 4 ? spatial4 \
    : number_spatial == 3 ? spatial3 \
    : number_spatial == 2 ? spatial2 \
    : spatial1

#define select_spatial_point(p) number_spatial == 4 ? p->spatial4 \
    : number_spatial == 3 ? p->spatial3 \
    : number_spatial == 2 ? p->spatial2 \
    : number_spatial == 1 ? p->spatial1 \
    : nullptr

#define select_spatial_point_no_nullptr(p) number_spatial == 4 ? p->spatial4 \
    : number_spatial == 3 ? p->spatial3 \
    : number_spatial == 2 ? p->spatial2 \
    : p->spatial1

#define debug_log if (true)


            //#define PEL_MEMORY_POOL_SPATIAL4

            template <typename type_t>
            class spatial4_t {

            public:
                type_t data;

                spatial4_t() = default;
                spatial4_t(const type_t& element) { data = element; }

            private:
                spatial4_t<type_t>* spatial1 = nullptr;
                spatial4_t<type_t>* spatial2 = nullptr;
                spatial4_t<type_t>* spatial3 = nullptr;
                spatial4_t<type_t>* spatial4 = nullptr;

                spatial4_t<type_t>* hidden_spatial_point = nullptr;

            public:

#ifdef PEL_MEMORY_POOL_SPATIAL4
                using memory_block_t = pel::memory_pool::memory_block_t<spatial4_t<type_t>>;

                memory_block_t* mem_block = nullptr;

                template<typename type_t>
                class instance_mem_pool {
                public:
                    static pel::memory_pool::memory_pool_t<spatial4_t<type_t>>* get() {
                        static bool is_init = false;
                        static pel::memory_pool::memory_pool_t<spatial4_t<type_t>> mem_pool;

                        if (!is_init) {

                            mem_pool.init();

                            is_init = true;
                        }

                        return &mem_pool;
                    }
                };

#endif

                using type = type_t;

                constexpr spatial4_t<type_t>* push_base(spatial4_t<type_t>* new_spatial, const std::size_t& number_spatial) noexcept {

                    if (number_spatial == 0 || number_spatial > 4)
                        return nullptr;

                    spatial4_t<type_t>* s = spatial(number_spatial);

                    if (s)
                    {
                        if (s != new_spatial) {

                            spatial4_t<type_t>* current = s->spatial(number_spatial);

                            if (current == this) {

                                fmt::print("self-ref detected\n");

                                if (!hidden_spatial()) {
                                    hidden_spatial_point = new spatial4_t<type_t>;
                                }

                                // TODO : can we fix recursion?
                                return hidden_spatial()->push_base(new_spatial, number_spatial);
                            }
                            
                            if (hidden_spatial()) {
                                fmt::print("A strange error, the movement of the exit along the hidden vertex is already busy, it seems that the logic of working with dimensions is not correctly compiled.\n");
                                return new_spatial;
                            }

                            if (!hidden_spatial()) {
                                hidden_spatial_point = new spatial4_t<type_t>;
                            }

                            return hidden_spatial()->push_base(new_spatial, number_spatial);

                          /*  auto& sp = select_spatial_point_no_nullptr(current);
                            sp = new_spatial;
                            return sp;*/
                        }
                        else
                        {
                            fmt::print("An attempt to finish off an entity that has already been added. Maybe it's self-reference.\n");
                            return new_spatial;
                        }
                    }
                    else
                    {
                        auto& sp = select_spatial_no_nullptr;
                        sp = new_spatial;
                        return sp;
                    }
                }

#ifdef PEL_MEMORY_POOL_SPATIAL4

                constexpr spatial4_t<type_t>* push_type_or(const type_t& value) noexcept {
                    memory_block_t* block = nullptr;

                    auto element = instance_mem_pool<type_t>::get()->get(block, value);
                    element->mem_block = block;
                    return push_base(element, 4);
                }

                constexpr spatial4_t<type_t>* push_type_and(const type_t& value)noexcept {
                    memory_block_t* block = nullptr;
                    auto element = instance_mem_pool<type_t>::get()->get(block, value);
                    element->mem_block = block;

                    return push_base(element, 3);
                }

                constexpr spatial4_t<type_t>* push_value_or(const type_t& value) noexcept {
                    memory_block_t* block = nullptr;
                    auto element = instance_mem_pool<type_t>::get()->get(block, value);
                    element->mem_block = block;

                    return push_base(element, 2);
                }

                constexpr spatial4_t<type_t>* push_value_and(const type_t& value) noexcept {

                    memory_block_t* block = nullptr;
                    auto element = instance_mem_pool<type_t>::get()->get(block, value);
                    element->mem_block = block;

                    return push_base(element, 1);
                }

                constexpr spatial4_t<type_t>* push_type_or()noexcept {

                    memory_block_t* block = nullptr;
                    auto element = instance_mem_pool<type_t>::get()->get(block);
                    element->mem_block = block;

                    return push_base(element, 4);
                }

                constexpr spatial4_t<type_t>* push_type_and() noexcept {

                    memory_block_t* block = nullptr;
                    auto element = instance_mem_pool<type_t>::get()->get(block);
                    element->mem_block = block;

                    return push_base(element, 3);
                }

                constexpr spatial4_t<type_t>* push_value_or() noexcept {
                    memory_block_t* block = nullptr;
                    auto element = instance_mem_pool<type_t>::get()->get(block);
                    element->mem_block = block;

                    return push_base(element, 2);
                }

                constexpr spatial4_t<type_t>* push_value_and() noexcept {

                    memory_block_t* block = nullptr;
                    auto element = instance_mem_pool<type_t>::get()->get(block);
                    element->mem_block = block;

                    return push_base(element, 1);
                }

#else

                constexpr spatial4_t<type_t>* push_type_or(const type_t& value) noexcept {
                    return push_base(new spatial4_t<type_t>(value), 4);
                }

                constexpr spatial4_t<type_t>* push_type_and(const type_t& value) noexcept {
                    return push_base(new spatial4_t<type_t>(value), 3);
                }

                constexpr spatial4_t<type_t>* push_value_or(const type_t& value) noexcept {
                    return push_base(new spatial4_t<type_t>(value), 2);
                }

                constexpr spatial4_t<type_t>* push_value_and(const type_t& value) noexcept {
                    return push_base(new spatial4_t<type_t>(value), 1);
                }

                constexpr spatial4_t<type_t>* push_type_or() noexcept {
                    return push_base(new spatial4_t<type_t>, 4);
                }

                constexpr spatial4_t<type_t>* push_type_and() noexcept {
                    return push_base(new spatial4_t<type_t>, 3);
                }

                constexpr spatial4_t<type_t>* push_value_or() noexcept {
                    return push_base(new spatial4_t<type_t>, 2);
                }

                constexpr spatial4_t<type_t>* push_value_and() noexcept {
                    return push_base(new spatial4_t<type_t>, 1);
                }
#endif

                constexpr spatial4_t<type_t>* push_type_or(spatial4_t<type_t>* new_spatial) noexcept {
                    return push_base(new_spatial, 4);
                }

                constexpr spatial4_t<type_t>* push_type_and(spatial4_t<type_t>* new_spatial) noexcept {
                    return push_base(new_spatial, 3);
                }

                constexpr spatial4_t<type_t>* push_value_or(spatial4_t<type_t>* new_spatial) noexcept {
                    return push_base(new_spatial, 2);
                }

                constexpr spatial4_t<type_t>* push_value_and(spatial4_t<type_t>* new_spatial) noexcept {
                    return push_base(new_spatial, 1);
                }

                constexpr spatial4_t<type_t>* type_or()   const noexcept { return spatial(4); }
                constexpr spatial4_t<type_t>* type_and()  const noexcept { return spatial(3); }
                constexpr spatial4_t<type_t>* value_or()  const noexcept { return spatial(2); }
                constexpr spatial4_t<type_t>* value_and() const noexcept { return spatial(1); }

                constexpr spatial4_t<type_t>* spatial(const std::size_t& number_spatial) const noexcept {
                    return select_spatial;
                }

                constexpr spatial4_t<type_t>* hidden_spatial() const noexcept {
                    return hidden_spatial_point;
                }

				constexpr spatial4_t<type_t>* operator[](const std::size_t& number_spatial) { return select_spatial; }

                // O(n * 2)
                void cyclic_deletion() {
                   
#ifdef PEL_MEMORY_POOL_SPATIAL4
#error "Not support cuclic deletion, TODO: create this"
#else 

                    std::deque<spatial4_t<type_t>* >  spatial4_queue;
                    std::vector<spatial4_t<type_t>* > spatial4_vector;

                    spatial4_queue.push_back(this);

                    while (!spatial4_queue.empty()) {

                        auto current = spatial4_queue.back();
                        spatial4_queue.pop_back();

                        if (current->is_read)
                            continue;

                        current->is_read = true;

                        if (current->spatial(1))
                            spatial4_queue.push_back(current->spatial(1));

                        if (current->spatial(2))
                            spatial4_queue.push_back(current->spatial(2));

                        if (current->spatial(3))
                            spatial4_queue.push_back(current->spatial(3));

                        if (current->spatial(4))
                            spatial4_queue.push_back(current->spatial(4));

                        if (current->hidden_spatial())
                            spatial4_queue.push_back(current->hidden_spatial());

                        spatial4_vector.push_back(current);
                    }

                    for (auto& it : spatial4_vector)
                    {
                        auto delete_element = it;

                        if (delete_element)
                        {
                           delete delete_element;
                        }
                    }

                    spatial4_vector.clear();
#endif
                }

                public:
                    bool is_read = false;

            };

            class cmd_t;
            using scmd_t = spatial4_t<cmd_t>;

            class true_path_t {
            
             public:

                true_path_t() = default;
                true_path_t(const std::vector<scmd_t*>& new_path) { path = new_path; }

                std::vector<scmd_t*> path;
                cmd_t* current_cmd;

                inline void push(scmd_t* data) { path.push_back(data); }
            };

            class ast_data_t;

            class cmd_t {
              public:
                using path_t = std::shared_ptr<true_path_t>;

                cmd_t() {}
                cmd_t(const std::string& data) { value = data; }
                cmd_t(const std::string& data, spatial4_t<cmd_t>* vertex) { value = data; push_path(vertex); }

                bool is_type  = false;
                bool is_value = false;

                std::string  value;
                std::shared_ptr<true_path_t> true_path;

                void copy_path(const cmd_t * const from) {

                    if (!from || !from->true_path)
                        return;

                    create_path();

                    true_path->path = from->true_path->path;
                }

                void copy_path(const spatial4_t<cmd_t>* const vertex_from) {

                    if (!vertex_from)
                        return;

                    copy_path(&vertex_from->data);
                }
                
                void create_path() {
                    if (!true_path) {
                        true_path = std::make_shared<true_path_t>();
                        true_path->current_cmd = this;
                    }
                }

                void push_path(spatial4_t<cmd_t>* vertex) {

                    if (!vertex)
                        return;

                    create_path();

                    true_path->push(vertex);
                }

                void print_path() {

                    if (true_path) {

                        for (const auto& it : true_path->path)
                            fmt::print("{}/", it->data.value);
    
                        fmt::print(fmt::fg(fmt::color::fuchsia), "{}\n", value);
                    }
                    else {
                        fmt::print(fmt::fg(fmt::color::fuchsia), "{}\n", value);
                    }

                }

                pel::element_ast_t<ast_data_t>* ast_element = nullptr;
            };

            class ast_data_t {

            public:
                std::size_t version = 0; // spec hack for not clear ast
                scmd_t* scmd;
            };

            void smcd_print(scmd_t* scmd_root, int depth = 0) {

                if (!scmd_root)
                    return;

                scmd_root->data.print_path();

                if (scmd_root->type_or())   { if (scmd_root->type_or() != scmd_root)   smcd_print(scmd_root->type_or(), ++depth);   }
                if (scmd_root->type_and())  { if (scmd_root->type_and() != scmd_root)  smcd_print(scmd_root->type_and(), ++depth);  }
                if (scmd_root->value_or())  { if (scmd_root->value_or() != scmd_root)  smcd_print(scmd_root->value_or(), ++depth);  }
                if (scmd_root->value_and()) { if (scmd_root->value_and() != scmd_root) smcd_print(scmd_root->value_and(), ++depth); }


            }
        }
    }
}