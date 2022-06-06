#pragma once

#include <memory_resource>

#include "spatial4.hpp"
#include "groups.hpp"

#define PEL_INLINE __forceinline

namespace pel {

    namespace core {

        namespace interpreter {

            template <typename type_t>
            class fast_queue_t {
            public:

                fast_queue_t() {
                    mem_pool = new memory_pool_t;
                    mem_pool->init();
                }

                ~fast_queue_t() {
                    mem_pool->reset();
                    delete mem_pool;
                }

                class data_t;

                using memory_pool_t  = pel::memory_pool::memory_pool_t<data_t>;
                using memory_block_t = pel::memory_pool::memory_block_t<data_t>;

                class data_t {
                public:
                    type_t  data;
                    data_t* next;

                    memory_block_t* mem_block = nullptr;
                };

                void push_back(const type_t& data) {

                    memory_block_t* block = nullptr;
                    auto element = mem_pool->get(block);

                    element->mem_block = block;

                    element->data = data;

                    if (!begin)
                    {
                        begin = element;
                        end = element;
                    }
                    else
                    {
                        end->next = element;
                        end = element;
                    }
                }

                void push(const type_t& data) {

                    memory_block_t* block = nullptr;
                    auto element = mem_pool->get(block);

                    element->mem_block = block;

                    element->data = data;

                    if (!begin)
                    {
                        begin = element;
                        end = element;
                    }
                    else
                    {
                        end->next = element;
                        end = element;
                    }
                }

                type_t& get()   { return begin->data; }
                type_t& front() { return begin->data; }

                void pop() {
                    mem_pool->free(begin->mem_block);
                    begin = begin->next;
                }

                void pop_front() {
                    mem_pool->free(begin->mem_block);
                    begin = begin->next;

                    if (begin == nullptr)
                        end = nullptr;
                }

                bool empty() { return begin == nullptr; }

                data_t* begin = nullptr;
                data_t* end = nullptr;

                memory_pool_t* mem_pool = nullptr;
            };

            //#define TEST_FAST_QUEUE

            class function_signal_t {

            public:

#ifdef TEST_FAST_QUEUE
                fast_queue_t<scmd_t*> scmd;
#else
                //std::deque<scmd_t*> scmd;
                std::pmr::deque<scmd_t*> scmd;
#endif

                class context_t;

                using boolean_function_t = std::function<void(bool&, context_t*, scmd_t*)>;

                class context_t {

                public:
                    //    boolean_function_t boolean_values;
                    //    boolean_function_t boolean_types;

                    function_signal_t* current_signal0 = nullptr;
                    function_signal_t* current_signal1 = nullptr;
                    function_signal_t* current_signal2 = nullptr;
                    function_signal_t* current_signal3 = nullptr;

                    function_signal_t* next_signal0    = nullptr;

                    // last state
                    bool is_have_iteration      = false;
                    bool is_boolean_state       = false;
                    bool is_end_state           = false;

                    bool is_iteration_type_and  = false;
                    bool is_iteration_type_or   = false;
                    bool is_iteration_value_and = false;
                    bool is_iteration_value_or  = false;

                    //scmd_t* current_scmd;
                    pel::groups::position_element_t* current_word = nullptr;

                    pel::element_ast_t<ast_data_t>* get_ast()       { return ast; }
                    pel::element_ast_t<ast_data_t>* get_ast() const { return ast; }

                    void set_ast(pel::element_ast_t<ast_data_t>* new_ast) { ast = new_ast; }

                    uint32_t version = 0;

                private:
                    pel::element_ast_t<ast_data_t>* ast = nullptr;
                };

                bool run_types(context_t* const context) {

                    context->is_have_iteration = false;

                    if (scmd.empty())
                        return false;

                    while (!scmd.empty())
                    {
                        auto data = scmd.front();

                        context->is_have_iteration = true;

                        // context->boolean_types(context->is_boolean_state, context, data);

                        context->is_boolean_state  = true;

                        // bool is_state = boolean_types(data, context);

                        if (context->is_boolean_state) {

                            if (data->spatial(3))
                                context->current_signal2->scmd.push_back(data->spatial(3));

                            if (data->spatial(1))
                                context->current_signal0->scmd.push_back(data->spatial(1));
                        }

                        if (data->spatial(4))
                            context->current_signal3->scmd.push_back(data->spatial(4));

                        if (data->spatial(2))
                            context->current_signal1->scmd.push_back(data->spatial(2));

                        scmd.pop_front();
                    }

                    return true;
                }

                void weave_ast(const function_signal_t::context_t* const ctx, scmd_t* const current_scmd) {

                    pel::element_ast_t<ast_data_t>* current_ast = ctx->get_ast();
               
                    if (current_scmd->data.true_path)
                    {
                        const auto& last = current_scmd->data.true_path->path.back();

                        // O(1) find :D
                        if (last && last->data.ast_element && last->data.ast_element->data.version == ctx->version) {
                            current_ast = last->data.ast_element;
                        }
                        else {

                            for (const auto& it : current_scmd->data.true_path->path)
                            {
                                if (!it->data.ast_element || it->data.ast_element->data.version != ctx->version)
                                {
                                    current_ast = current_ast->push();
                                    current_ast->data.scmd = it;
                                    it->data.ast_element = current_ast;
                                }
                                else
                                {
                                    current_ast = it->data.ast_element;
                                }
                            }
                        }

                        current_ast = current_ast->push();
                        current_ast->data.scmd = current_scmd;
                        current_scmd->data.ast_element = current_ast;
                    }
                }

                bool run_values(
                    context_t* const context
                ) {
                    context->is_have_iteration = false;

                    if (scmd.empty())
                        return false;

                    auto data = scmd.front();

                    context->is_have_iteration = true;
                    context->is_boolean_state  = false;

                    for (const auto& word : context->current_word->words)
                    {
                        if (word.data == data->data.value) {
                            weave_ast(context, data);
                            context->is_boolean_state = true;
                            break;
                        }
                    }

                    if (context->is_boolean_state) {

                        if (data->spatial(3)) {
                            context->current_signal2->scmd.push_back(data->spatial(3));
                        }

                        if (data->spatial(1)) {
                            context->current_signal0->scmd.push_back(data->spatial(1));
                        }
                    }

                    if (data->spatial(4)) {
                        context->current_signal3->scmd.push_back(data->spatial(4));
                    }

                    if (data->spatial(2)) {
                        context->current_signal1->scmd.push_back(data->spatial(2));
                    }

                    scmd.pop_front();

                    context->is_end_state = scmd.empty() && context->next_signal0->scmd.empty();

                    return true;
                }
            };

            class manager_signals_t {

            public:
                PEL_INLINE void start_point(scmd_t* const start_scmd) {
                    first_point = start_scmd;
                    reset();
                }

                void clear() {

                    if (first_point)
                    {
                        first_point->cyclic_deletion();
                        first_point = nullptr;
                    }

                }

                PEL_INLINE void reset() {
                    context.current_signal3->scmd.push_back(first_point);
                    // its smart hack for not clear ast with realloc him
                    context.version++;
                }

                PEL_INLINE void init() {

                    context.current_signal0 = new function_signal_t;
                    context.current_signal1 = new function_signal_t;
                    context.current_signal2 = new function_signal_t;
                    context.current_signal3 = new function_signal_t;
                    context.next_signal0    = new function_signal_t;

                    context.set_ast(new pel::element_ast_t<ast_data_t>);
                }

                PEL_INLINE void delete_ast() {

                    context.get_ast()->cyclic_deletion();

                    // todo: check
                    context.set_ast(nullptr);

                    delete context.current_signal0; context.current_signal0 = nullptr;
                    delete context.current_signal1; context.current_signal1 = nullptr;
                    delete context.current_signal2; context.current_signal2 = nullptr;
                    delete context.current_signal3; context.current_signal3 = nullptr;
                    delete context.next_signal0;    context.next_signal0    = nullptr;
                }

                PEL_INLINE bool run_values_base() {

                    if (!context.current_signal0->run_values(&context))
                        return false;

                    return true;
                }

                PEL_INLINE bool run_values_or_base() {

                    if (!context.current_signal1->run_types(&context))
                        return false;

                    return true;
                }

                PEL_INLINE bool run_types_base() {

                    if (!context.current_signal3->run_types(&context))
                        if (!context.current_signal2->run_types(&context))
                            return false;

                    return true;
                }

                PEL_INLINE void  run_values() {
                    while (run_values_base());
                }

                PEL_INLINE void  run_values_or() {
                    while (run_values_or_base());
                }

                PEL_INLINE void  run_types() {
                    while (run_types_base());
                }

                function_signal_t::context_t* run() {

                    get_context()->is_have_iteration = false;

                    run_types();

                    if (get_context()->is_have_iteration) {
                        get_context()->is_iteration_type_and = true;
                        get_context()->is_iteration_type_or  = true;
                    }

                    get_context()->is_have_iteration = false;

                    run_values_or();

                    if (get_context()->is_have_iteration) {
                        get_context()->is_iteration_value_or = true;
                    }

                    get_context()->is_have_iteration = false;

                    if (!get_context()->is_iteration_type_or)
                    {
                        run_values();
                    }

                    // or iteration
                    if (get_context()->is_have_iteration) {
                        get_context()->is_iteration_value_and = true;
                    }

                    return get_context();
                }

                constexpr function_signal_t::context_t* get_context() { return &context; };

                function_signal_t::context_t context;

                // start point
                scmd_t* first_point = nullptr;

                scmd_t* get_first_point() { return first_point; };

                PEL_INLINE void swap_signal() {
                    std::swap(context.current_signal0, context.next_signal0);
                }
            };
        }
    }
}