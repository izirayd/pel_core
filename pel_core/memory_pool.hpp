#pragma once

#include <stdint.h>
#include <vector>
#include <list>

#define RESTRICT __declspec(restrict)

namespace pel {

	namespace memory_pool {

		template <typename type_allocate>
		class storage_t {

		public:
			std::uint8_t* ptr = nullptr;

			std::list<std::uint8_t*> array_memory_blocks;

			constexpr void get(std::uint8_t*& mem) noexcept { mem = ptr; ptr += sizeof(type_allocate); };

			constexpr RESTRICT type_allocate* get() noexcept  {

				type_allocate* mem = (type_allocate*)ptr;
				ptr += sizeof(type_allocate);

				new (mem) type_allocate();

				return mem;
			};

			constexpr void alloc(const std::size_t& count_blocks) noexcept {

				if (count_blocks < 1)
					return;

				const std::size_t size_ast = sizeof(type_allocate);
				std::uint8_t* memory_array = new std::uint8_t[size_ast * count_blocks];

				array_memory_blocks.push_back(memory_array);

				ptr = memory_array;
			}

			constexpr void del() noexcept {

				for (auto& it : array_memory_blocks)
				{
					delete[] it;
				}
			}
		};

		template<typename type_block>
		class element_memory_t {

		public:
			constexpr element_memory_t* next() noexcept { return next_element; }
			constexpr element_memory_t* next() const noexcept  { return next_element; }

			element_memory_t* next_element = nullptr;

			std::uint8_t* data = nullptr;
			type_block* block  = nullptr;

		};

		template <typename type_allocate>
		class memory_block_t {

		public:
			using type = type_allocate;

			element_memory_t<memory_block_t<type_allocate>>* element_memory;

			constexpr RESTRICT memory_block_t* block() noexcept { return element_memory->block; }
			constexpr RESTRICT memory_block_t* block() const noexcept  { return element_memory->block; }

			// call destructor
			constexpr void free() {
				type_allocate* mem = (type_allocate*)element_memory->data;
				mem->~type_allocate();
			}

			// call constructor
			constexpr RESTRICT type_allocate* get() noexcept {
				type_allocate* mem = (type_allocate*)element_memory->data;
				new (mem) type_allocate();
				return mem;
			};

			template <typename constructor_data_t>
			constexpr RESTRICT type_allocate* get(constructor_data_t value_constructor) noexcept {
				type_allocate* mem = (type_allocate*)element_memory->data;
				new (mem) type_allocate(value_constructor);
				return mem;
			};
		};

		template <typename type_allocate>
		class memory_pool_t {

		public:
			using memory_block = memory_block_t<type_allocate>;

			constexpr void init() noexcept {
				new_memory(first);
				last = first;
			}

			const std::size_t count_blocks = 1024;

			constexpr void new_memory(element_memory_t<memory_block>*& start_block) noexcept {

				storage_data.alloc(count_blocks);
				storage_element.alloc(count_blocks);
				storage_blocks.alloc(count_blocks);

				create(start_block);
			}

			void create(element_memory_t<memory_block>*& start_block) noexcept {

				if (count_blocks < 1)
					return;

				// alloc memory for start block
				start_block = storage_element.get();

				element_memory_t<memory_block>* current = start_block;
				current->block = storage_blocks.get();

				storage_data.get(current->data);

				for (size_t i = 0; i < count_blocks - 1; i++)
				{
					current->next_element = storage_element.get();
					current = current->next_element;

					storage_data.get(current->data);
					current->block = storage_blocks.get();
				}

				end_element = current;
			}

			// O(1)
			constexpr RESTRICT type_allocate* get(memory_block*& mem_block) noexcept {

				if (!last) {
					// need new blocks
					new_memory(last);
				}

				mem_block = last->block;
				mem_block->element_memory = last;
				last = last->next_element;
				return mem_block->get();
			}

			// O(1)
			template <typename constructor_data_t>
			constexpr RESTRICT type_allocate* get(memory_block*& mem_block, constructor_data_t constructor_data) noexcept {

				if (!last) {
					// need new blocks
					new_memory(last);
				}

				mem_block = last->block;
				mem_block->element_memory = last;
				last = last->next_element;
				return mem_block->get<constructor_data_t>(constructor_data);
			}

			// O(1)
			void free(memory_block*& mem_block) noexcept  {

				mem_block->free();

				end_element->next_element = mem_block->element_memory;
				end_element = end_element->next_element;

				end_element->next_element = nullptr;
				mem_block->element_memory = nullptr;

				mem_block = nullptr;

				if (!last)
					last = end_element;

			}

			storage_t<element_memory_t<memory_block>> storage_element;
			storage_t<type_allocate>				  storage_data;
			storage_t<memory_block>					  storage_blocks;

			constexpr void reset() noexcept {
				storage_element.del();
				storage_data.del();
				storage_blocks.del();
			}

			element_memory_t<memory_block>* first       = nullptr;
			element_memory_t<memory_block>* last        = nullptr;
			element_memory_t<memory_block>* end_element = nullptr;
		};
	}
}
