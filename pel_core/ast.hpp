#pragma once

#include <unordered_map>
#include <functional>
#include <fmt/printf.h>
#include <fmt/color.h>

#define RESTRICT __declspec(restrict)

//#define PEL_MEMORY_POOL_AST

#ifdef PEL_MEMORY_POOL_AST
	#include "memory_pool.hpp"
#endif

//#define PEL_MEMORY_RESOURCE

#ifdef PEL_MEMORY_RESOURCE
	#include <memory_resource>
#endif

namespace pel {

	// very bad for memory (x4 size) if objects will allocate (we uses smart allocate, if names >= 32 then allocate unordered_map.
	// just for this works, need +16 bytes for x64
    //#define FAST_AST_FIND_NAME

	class ast_info_t {
	
	 public:

		void do_break() { is_break = true; }

		bool is_move_parent = false;
		bool is_move_child  = false;
		bool is_move_next   = false;
		bool is_break       = false;

		void reset() {
			is_move_parent = false;
			is_move_child  = false;
			is_move_next   = false;
			is_break       = false;
		}
		 
		std::size_t level   = 0;
	};

	template<typename ast_data_t>
	class element_ast_t {

	  public:

		element_ast_t() = default;

       ~element_ast_t()
		{
		   // ??
			//if ((next() || child()) && !is_process_delete)
			//	cyclic_deletion();

#ifdef PEL_MEMORY_POOL_AST
			if (is_parent_memory_pool) {
				mem_pool->reset();
				delete mem_pool; // ??
			}
#endif
#ifdef PEL_MEMORY_RESOURCE
			if (is_parent_memory_pool)
				upr->release();
#endif

		}

	    [[nodiscard]]
		constexpr element_ast_t* get_this() noexcept  { return this; }
		[[nodiscard]]
		constexpr element_ast_t* get_this() const noexcept { return this; }
		// O (1)
		[[nodiscard]]
		constexpr element_ast_t* next() noexcept { return next_element;   }
		[[nodiscard]]
		constexpr element_ast_t* next() const noexcept  { return next_element;   }
		// O (1)
		[[nodiscard]]
		constexpr element_ast_t* child() noexcept  { return begin_child;    }
		[[nodiscard]]
		constexpr element_ast_t* child() const noexcept { return begin_child;    }
		// O (1)
		[[nodiscard]]
		constexpr element_ast_t* parent() noexcept { return parent_element; }
		[[nodiscard]]
		constexpr element_ast_t* parent() const noexcept { return parent_element; }
		// O (1)
		[[nodiscard]]
		constexpr element_ast_t* end()  noexcept { return end_element;    }
		[[nodiscard]]
		constexpr element_ast_t* end()  const noexcept  { return end_element;    }

		// O (1)
		constexpr RESTRICT element_ast_t* push() noexcept {

#ifdef PEL_MEMORY_POOL_AST

			if (mem_pool)
			{
				memory_block_t* block = nullptr;
				auto element = mem_pool->get(block);

				element->mem_block = block;
				element->mem_pool  = mem_pool;

				return push(element);
			}
			
#endif

#ifdef PEL_MEMORY_RESOURCE

			if (upr)
			{
				auto element = (element_ast_t*) upr->allocate(sizeof(element_ast_t));

				new (element) element_ast_t();

				element->upr = upr;
				return push(element);
			}

#endif
			return push(new element_ast_t);
		}

		// O (1)
		constexpr RESTRICT element_ast_t* push(element_ast_t* element) noexcept  {

			if (!begin_child) {
				begin_child = element;
				begin_child->end_element = begin_child;
			}
			else
			{
				begin_child->end()->next_element = element;
				begin_child->end_element = begin_child->end()->next_element;
			}

			element->parent_element = get_this();

#ifdef FAST_AST_FIND_NAME
			if (parent())
				parent()->size++;
#endif

			return element;
		}

		// O (1)
		[[nodiscard]]
		constexpr RESTRICT ast_data_t *get_data() noexcept { return &data; }

		ast_data_t  data;
		[[nodiscard]]
		const std::string &get_name()  noexcept { return name_ast; };
		[[nodiscard]]
		const std::string &get_value() noexcept  { if (this) return data.get_value(); else  return std::string(); };

		constexpr void set_name(const std::string& new_name)  {

#ifdef FAST_AST_FIND_NAME

			bool is_new = name_ast.empty();

			if (parent() && !new_name.empty()) {

				if (parent()->size == 32)
				{
					// add fast find
					names = new std::unordered_map<std::string, element_ast_t*>;
				}

				if (parent()->size >= 32) {

					if (!is_new)
						parent()->names->erase(name_ast);

					parent()->names->at(new_name) = this;
				}
			}

#endif
			name_ast = new_name;
		}

		// - O (n)
		// - TODO: whould be faster
		[[nodiscard]]
		constexpr element_ast_t& operator[](const std::string& name) {
		
			element_ast_t* result = nullptr;

			if (!this)
				return *result;

			for_next([&](element_ast_t* element, ast_info_t& ast_info) {

				if (element->name_ast == name)
				{
					ast_info.do_break();
					result = element;
				}
				}
			);

			return *result;
		}

		// - with FAST_AST_FIND_NAME O(1) (good case), but x4 need memory
		// - without O(n)
		[[nodiscard]]
		constexpr RESTRICT element_ast_t* find_ast(const std::string& name) {

			element_ast_t* result = nullptr;

#ifdef FAST_AST_FIND_NAME

			auto name_res = names->find(name);

			if (name_res != names->end())
			{
				result = name_res->second;
			}

#else
		for_each([&](element_ast_t* element, ast_info_t& ast_info) {

				if (element->name_ast == name)
				{
					ast_info.do_break();
					result = element;
				}
			}
		 );
#endif // FAST_AST_FIND_NAME

		  return result;
		}

		// - O (1)
		constexpr void cut_child(element_ast_t* anower_ast) noexcept {

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
		constexpr void for_each(std::function<void(element_ast_t*, ast_info_t &)> func)  {

			if (!func)
				return;

			element_ast_t* current = get_this();
			ast_info_t ast_info;

			bool is_exit = false;

			for (;;) {

				func(current, ast_info);

				if (ast_info.is_break)
					break;

				if (current->child())
				{
					current = current->child();

					ast_info.reset();
					ast_info.is_move_child = true;
					ast_info.level++;

					continue;
				}

				if (current->next())
				{
					current = current->next();

					ast_info.reset();
					ast_info.is_move_next = true;
				
					continue;
				}

				current = current->parent();

				if (current) {

					ast_info.reset();

					ast_info.is_move_parent = true;
					ast_info.level--;

					while (current) {

						if (current->next())
						{
							current = current->next();

							ast_info.is_move_next = true;

							break;
						}
						else
						{
							current = current->parent();

							ast_info.level--;
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
		constexpr void for_each(std::function<void(element_ast_t*)> func) {

			if (!func)
				return;

			for_each([&](element_ast_t* element_ast, ast_info_t&) {
				func(element_ast);
			});
		
		}

		// - o(n) 
		// - non-recursion
		// - brute over without referring to child elements, child will uses only first times
		constexpr void for_next(std::function<void(element_ast_t*, ast_info_t&)> func)
		{
			if (!func)
				return;

			element_ast_t* current = get_this()->child();

			if (!current)
				return;

			ast_info_t ast_info;

			do {

				ast_info.is_move_next = true;

				func(current, ast_info);

				if (ast_info.is_break)
					break;

			} while (current = current->next());			 
		}

		// - o(n) 
		// - non-recursion
		// - brute over without referring to child elements, without first child
		constexpr void for_this_next(std::function<void(element_ast_t*, ast_info_t&)> func)
		{
			if (!func)
				return;

			element_ast_t* current = get_this();

			ast_info_t ast_info;

			do {

				ast_info.is_move_next = true;

				func(current, ast_info);

				if (ast_info.is_break)
					break;

			} while (current = current->next());

		}
		
		bool is_read = false;

		// O(n * 2), no idea how do it like o(n)
		void cyclic_deletion() {

			std::deque<element_ast_t*>  graph_queue;
			std::vector<element_ast_t*> graph_vector;

			graph_queue.push_back(this);

			bool is_first_element = true;

			for (;;) {

				auto current = graph_queue.back();
				graph_queue.pop_back();

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


					is_first_element = false;

				} while (current = current->child());

				if (graph_queue.empty())
					break;
			}


			for (auto& it : graph_vector)
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

		// - o(n) 
		// - non-recursion
		// - variant DFS
		constexpr void delete_graph() noexcept {

			element_ast_t* current = get_this();

			if (this->begin_child)
				current = this->begin_child;
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

				element_ast_t* tmp_parent = current->parent();

				while (current)
				{
					element_ast_t* next_element_ast = current->next();
					current->is_process_delete = true;
					
#ifdef FAST_AST_FIND_NAME
					if (current->names)
						delete current->names;
#endif

#ifdef PEL_MEMORY_POOL_AST
					if (current->mem_block && current->mem_pool) {
						current->mem_pool->free(current->mem_block);
					}
					else
					{
						delete current;
					}
#elif defined(PEL_MEMORY_RESOURCE)
					if (upr)
					{
						current->~element_ast_t();
						upr->deallocate(current, sizeof(element_ast_t));
					}
					else
					{
						delete current;
					}
#else
					delete current;
#endif
				
					current = next_element_ast;
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

		constexpr void use_memory_pool() noexcept
		{
#ifdef PEL_MEMORY_POOL_AST
			mem_pool = new memory_pool_t;
			mem_pool->init();
			is_parent_memory_pool = true;
#endif
#ifdef PEL_MEMORY_RESOURCE
			upr = new std::pmr::unsynchronized_pool_resource;
			is_parent_memory_pool = true;
#endif
		}

	private:
		element_ast_t* end_element    = nullptr;
		element_ast_t* parent_element = nullptr;
		element_ast_t* next_element   = nullptr;
		element_ast_t* begin_child    = nullptr;

		std::string	   name_ast;

#ifdef FAST_AST_FIND_NAME
		std::unordered_map<std::string, element_ast_t*> *names = nullptr;
#endif

#ifdef FAST_AST_FIND_NAME
		std::size_t size		  = 0;
#endif

#ifdef PEL_MEMORY_POOL_AST
		using memory_pool_t  = memory_pool::memory_pool_t<element_ast_t<ast_data_t>>;
		using memory_block_t = memory_pool::memory_block_t<element_ast_t<ast_data_t>>;

		memory_pool_t  *mem_pool  = nullptr;
		memory_block_t *mem_block = nullptr;
#endif

#ifdef PEL_MEMORY_RESOURCE
		std::pmr::unsynchronized_pool_resource* upr = nullptr;
#endif

		bool is_process_delete     = false;
		bool is_parent_memory_pool = false;
	};

	template<typename type_ast_t>
	using ast_t = std::shared_ptr<pel::element_ast_t<type_ast_t>>;

	// shared ast
	template<typename type_ast_t>
	ast_t<type_ast_t> make_ast() { return std::make_shared<pel::element_ast_t<type_ast_t>>(); }

	template<typename type_ast_t>
	using ast_function_t = std::function<void(element_ast_t<type_ast_t>*, ast_info_t&)>;
}