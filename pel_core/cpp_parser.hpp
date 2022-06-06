#pragma once

#include "base_interpreter.hpp"
#include "graph.hpp"
#include "flag.hpp"
#include "error.hpp"
#include <stack>

namespace pel {

	namespace parser {

		enum class parse_flag_t : pel::detail::flag16_t
		{
			unknown = 1 << 0,
			word = 1 << 1,
			space_tab = 1 << 2,
			symbol = 1 << 3,
			group_symbol = 1 << 4,
			new_line = 1 << 5
		};


		const std::string keyword_symbols_cpp = "{}()[]<>;:,./|\"\"\\\n=-+*?~`\'#!@$%^&\t\r";

		bool it_keyword_symbol(const char symbol) {
			for (const auto& keyword_symbol : keyword_symbols_cpp)
				if (keyword_symbol == symbol)
					return true;

			return false;
		}

		inline bool it_keyword_symbol_or_space(const char symbol) { return it_keyword_symbol(symbol) || symbol == ' '; }
		inline bool it_word_symbol(const char symbol) { return !it_keyword_symbol_or_space(symbol); }
		inline bool it_space_or_tab(const char symbol) { return symbol == ' ' || symbol == '\t'; }
		inline bool it_space_or_tab_or_end(const char symbol) { return symbol == ' ' || symbol == '\n' || symbol == '\t'; }
		inline bool it_new_line(const char symbol) { return symbol == '\n'; }

		struct words_base_t
		{
			std::string   data;
			pel::detail::flag16_t type;

			words_base_t() { pel::detail::clear_flag(type); };
			words_base_t(pel::detail::flag16_t t, const std::string& str) { type = t; data = str; }

			until::position_t position;

			inline void clear() {
				pel::detail::clear_flag(type); data.clear();
			}

			inline bool is_space_tab() { return pel::detail::check_flag(type, parse_flag_t::space_tab); }
			inline bool is_word() { return pel::detail::check_flag(type, parse_flag_t::word); }
			inline bool is_group_symbol() { return pel::detail::check_flag(type, parse_flag_t::group_symbol); }
			inline bool is_new_line() { return pel::detail::check_flag(type, parse_flag_t::new_line); }
			inline bool is_symbol() { return pel::detail::check_flag(type, parse_flag_t::symbol); }

			std::intptr_t start_index = -1;
			std::intptr_t end_index = -1;
			std::intptr_t index_pair = -1;

		};

		class words_t
		{
		  public:
			std::vector<words_base_t> words;
			inline void push(const words_base_t& data) { words.push_back(data); }
			inline void clear() { words.clear(); }

		};

		template <typename words_type>
		void process_parse_word(const std::string& code, words_type& words)
		{
			pel::detail::flag16_t    parse_flag;
			pel::detail::flag16_t    parse_event_change_state;  // ¬ случае изменени€, получи флаг, который был изменен, что бы обработчик мог его обработать

			pel::detail::clear_flag(parse_flag);
			pel::detail::clear_flag(parse_event_change_state);

			words_base_t word;
			words_base_t word_for_one_symbol;

			std::size_t number_line     = 1;
			std::size_t number_position = 1;
			std::size_t start_position  = 1;

			std::size_t it = 0;

			char next_symbol     = 0x00;
			char previous_symbol = 0x00;

			for (const auto& symbol : code)
			{
				next_symbol     = (it + 1) < code.size() ? code[it + 1] : 0x00;
				previous_symbol = it > 0 ? code[it - 1] : 0x00;

				it++;

				if (symbol == '\r')
					continue;

				parse_event_change_state = parse_flag;

				if (it_space_or_tab(symbol)) {
					pel::detail::clear_flag(parse_flag);
					pel::detail::add_flag(parse_flag, parse_flag_t::space_tab);
				}
				else if (it_new_line(symbol)) {

					// ≈сли нова€ лини€, то запретит стакнутьс€ с самим собой
					if (pel::detail::check_flag(parse_flag, parse_flag_t::new_line)) {

						word.position.number_line    = number_line - 1;
						word.position.start_position = start_position;
						word.position.end_position   = number_position;

						words.push(word);
						word.clear();

						start_position = number_position;
					}

					pel::detail::clear_flag(parse_flag);
					pel::detail::add_flag(parse_flag, parse_flag_t::new_line);

				}
				else if (it_word_symbol(symbol)) {
					pel::detail::clear_flag(parse_flag);
					pel::detail::add_flag(parse_flag, parse_flag_t::word);
				}
				else if (it_keyword_symbol(symbol)) {
					pel::detail::clear_flag(parse_flag);
					pel::detail::add_flag(parse_flag, parse_flag_t::symbol);
				}
				else
				{
					pel::detail::clear_flag(parse_flag);
					pel::detail::add_flag(parse_flag, parse_flag_t::unknown);
				}

				// ѕроизошло изменени€ буквы, теперь это что-то новое, но мы обработаем старое
				if (((parse_event_change_state != parse_flag) && ((pel::detail::flag16_t)parse_event_change_state > 0)) && !pel::detail::check_flag(parse_event_change_state, parse_flag_t::symbol)) {

					word.position.number_line = pel::detail::check_flag(parse_event_change_state, parse_flag_t::new_line) ? number_line - 1 : number_line;

					word.position.start_position = start_position;
					word.position.end_position   = number_position;

					words.push(word);
					word.clear();

					start_position = number_position;
				}

				if ((pel::detail::flag16_t)parse_flag > 0) {

					if (pel::detail::check_flag(parse_flag, parse_flag_t::symbol))
					{
						word_for_one_symbol.type = parse_flag;
						word_for_one_symbol.data += symbol;

						word_for_one_symbol.position.number_line    = number_line;
						word_for_one_symbol.position.start_position = start_position;
						word_for_one_symbol.position.end_position   = number_position;

						words.push(word_for_one_symbol);

						word_for_one_symbol.clear();

						start_position = number_position;

					}
					else
					{
						if (previous_symbol == '\r')
							word.data += '\r';

						word.type = parse_flag;
						word.data += symbol;
					}
				}

				if (it_new_line(symbol)) {

					number_line++;
					start_position = 1;
					number_position = 1;
				}

				if (!pel::detail::check_flag(parse_flag, parse_flag_t::new_line))
					number_position++;
			}

			if (!word.data.empty())
			{
				// ≈сли нова€ лини€, то запретит стакнутьс€ с самим собой
				if (pel::detail::check_flag(parse_flag, parse_flag_t::new_line)) {

					word.position.number_line    = number_line - 1;
					word.position.start_position = start_position;
					word.position.end_position   = number_position;

					words.push(word);
					word.clear();

					start_position = number_position;
				}
				else
				{
					word.position.number_line    = number_line;
					word.position.start_position = start_position;
					word.position.end_position   = number_position;

					words.push(word);
					word.clear();

					start_position = number_position;
				}
			}
		}

		class cpp_pel_parser_t {

			std::string code;

		public:
		
			void set_code(const std::string& code_str) { code = code_str; }
			const std::string& get_code()              { return code;     }
				 
			class cmd_t {

			public:
				parser::words_base_t word;

				bool is_block = false; // { }
				bool is_block_value = false; // ( )
				bool is_block_meta = false; // [ ]
				bool is_string = false; // ""
			};

			class content_t {
			
			public:
				std::string value;

				std::size_t version_read   = 0;

				bool is_read  = false;

				bool is_declaration_type  = false;
				bool is_declaration_value = false;
				bool is_declaration_value_in_superposition = false;
				bool is_declaration_unknown      = false;
				bool is_declaration_body_value   = false;
				bool is_declaration_body_unknown = false;
				bool is_declaration_body_type    = false;
				bool is_declaration_body_graph   = false;
				bool is_declaration_not          = false;
				bool is_having_child_like_real_element = false;

				bool is_indivisible = false;

				bool is_property_and = false;
				bool is_property_or  = false;

				bool is_exist = false;

				pel::core::interpreter::scmd_t *scmd_link;				
			};

			class pel_words_t
			{
			public:
				std::vector<parser::words_base_t> words;

				inline void push(const parser::words_base_t& words_base) {
					words.push_back(words_base);
				}

				inline void clear() { words.clear(); }
			};

			using ast_t         = pel::core::graph_t<cmd_t>;
			using ast_content_t = pel::core::graph_t<content_t>;

			ast_content_t* get_ast() { return ast_content; };

			ast_content_t* ast_content = nullptr;
			pel_words_t    pel_words;

			void clear() {
				if (ast_content) {
					ast_content->cyclic_deletion();
					ast_content = nullptr;
				}
				pel_words.clear();
				code.clear();
			}

			void process_parse(pel::error_t& error_context) {
				parser::process_parse_word(code, pel_words);
				process_cast_to_ast(error_context);
			}

			void process_cast_to_ast(pel::error_t &error_context) {
	
				ast_t* current = new ast_t;
				std::stack<ast_t*> block;

				bool is_string_read = false;

				std::string string_buffer;

				// TODO: add support \n \" \' \\ and etc
				// tODO: add support comments
				for (auto& it : pel_words.words)
				{
					if (it.data == "\"") {

						if (is_string_read) {
							auto new_element = current->push();
							new_element->data.is_string = true;

							// TODO: we should add meta data about position words
							new_element->data.word.data = string_buffer;
						}

						string_buffer.clear();
						is_string_read = !is_string_read;
						continue;
					}

					if (is_string_read)
					{
						string_buffer += it.data;
						continue;
					}

					// todo: add ignore for strings " ' `
					if (it.is_space_tab() || it.is_new_line()) {
						continue;
					}

					if (it.data == "}" || it.data == ")" || it.data == "]") {

						if (block.empty()) {
							error_context.push(error::element_t(fmt::format("No close block \"{}\"", it.data), it.position));
							continue;
						}

						current = block.top();
						block.pop();
						continue;
					}

					auto new_element = current->push();

					new_element->data.word = it;

					if (it.data == "{") {
						new_element->data.is_block = true;
						new_element->data.word.data = "Block {}";

						block.push(current);
						current = new_element;
					}

					if (it.data == "(") {
						new_element->data.is_block_value = true;
						new_element->data.word.data = "Block ()";

						block.push(current);
						current = new_element;
					}

					if (it.data == "[") {
						new_element->data.is_block_meta = true;
						new_element->data.word.data = "Block []";

						block.push(current);
						current = new_element;
					}
				}

				while (!block.empty()) {

					auto ast_block = block.top();
					block.pop();

					const pel::parser::words_base_t &it = ast_block->child() ? ast_block->child()->get_data()->word : ast_block->get_data()->word;

					error_context.push(error::element_t(fmt::format("Not close block \"{}\".", it.data), it.position));
				}

				ast_content = new ast_content_t;

				ast_content->get_data()->value = "PelRoot";

				bool is_empty_block = false;
				block_parse_ast(error_context, current->child(), ast_content, is_empty_block);

				current->cyclic_deletion();
				current = nullptr;

				if (false) 
					ast_content->for_each([=](ast_content_t* element, pel::core::graph_info_t& info) {

					// skip root
					if (info.level == 0)
						return;

					auto data = element->get_data();

					std::string tabs;

					for (size_t i = 0; i < info.level - 1; i++)
					{
						tabs += " ";
					}

					//data->name
					fmt::print("{}{}", tabs, data->value);

					std::string str = " = ";

					bool is_have_first = false;

			#define push_meta_info(state, name_state) if (state) { if (state) { if (is_have_first) { str += ", ";  } else is_have_first = true;  str += name_state; } }

					push_meta_info(data->is_property_and, "is_property_and");
					push_meta_info(data->is_property_or, "is_property_or");
					push_meta_info(data->is_declaration_body_type, "is_declaration_body_type");
					push_meta_info(data->is_declaration_body_unknown, "is_declaration_body_unknown");
					push_meta_info(data->is_declaration_type, "is_declaration_type");
					push_meta_info(data->is_declaration_unknown, "is_declaration_unknown");
					push_meta_info(data->is_declaration_value, "is_declaration_value");
					push_meta_info(data->is_declaration_value_in_superposition, "is_declaration_value_in_superposition");
					push_meta_info(data->is_declaration_not, "is_declaration_not");
					push_meta_info(data->is_declaration_body_graph, "is_declaration_body_graph");
					push_meta_info(data->is_indivisible, "is_indivisible");

					fmt::print("{}\n", str);
				});
			}

			void block_parse_ast(pel::error_t& error_context, ast_t* current_ast, ast_content_t* content_root, bool &is_empty) {

				if (!current_ast) {
					is_empty = true;
					return;
				}

				if (!content_root) {
					error_context.push(fmt::format("No content for compile."));
					return;
				}

				ast_content_t* current_content = content_root->push();

				bool is_keyword_type            = false;
				bool is_keyword_value           = false;
				bool is_keyword_read_data       = false;
				bool is_wait_symbol_read_data   = false;
				bool is_wait_after_logical_word = false;
				bool is_wait_after_not_word     = false;
				bool is_have_next               = false;
				bool is_not                     = false;
				bool is_not_empty               = false;

				until::position_t position_code;

				do
				{
					if (is_have_next) {
						current_content = content_root->push();
						is_have_next    = false;
					}

					const std::string& keyword = current_ast->get_data()->word.data;

					position_code = current_ast->get_data()->word.position;

					if (!is_keyword_read_data && !is_wait_symbol_read_data && !current_ast->get_data()->is_string && !current_ast->get_data()->is_block) {

						if (keyword == ";") {

							if (is_keyword_type || is_keyword_value || is_keyword_read_data || is_wait_symbol_read_data || is_wait_after_not_word) {
								error_context.push(fmt::format("After \"{}\" can`t be \";\" here.", current_content->data.value), position_code);
								return;
							}

							if (is_wait_after_logical_word) {
								error_context.push(fmt::format("No data after of AND(,)/OR/NOT(!) words."), position_code);
								return;
							}

							is_have_next = true;

							continue;
						}

						if (keyword == "and" || keyword == ",") {

							if (current_content->parent()) {

								if (current_content->parent()->get_data()->is_property_or) {
									error_context.push(fmt::format("Parent can`t be OR. After \"{}\" was declared AND.", current_content->data.value), position_code);
									return;
								}

								current_content = current_content->parent()->push();
								current_content->parent()->get_data()->is_property_and = true;
								is_wait_after_logical_word = true;
							}
							else {
								error_context.push(fmt::format("No parent for \"{}\", looks like block used solo and property.", current_content->data.value), position_code);
								return;
							}
				
							continue;
						}

						if (keyword == "or") {

							if (current_content->parent()) {

								if (current_content->parent()->get_data()->is_property_and) {
									error_context.push(fmt::format("Parent can`t be AND. After \"{}\" was declared OR.", current_content->data.value), position_code);
									return;
								}

								current_content = current_content->parent()->push();
								current_content->parent()->get_data()->is_property_or = true;
								is_wait_after_logical_word = true;
							}
							else {
								error_context.push(fmt::format("No parent for \"{}\", looks like block used solo and property.", current_content->data.value), position_code);
								return;
							}

							continue;
						}

						if (keyword == "not" || keyword == "!") {
							is_not = true;
							is_wait_after_not_word = true;
							continue;
						}

						if (keyword == "type") {
							is_keyword_type          = true;
							is_wait_symbol_read_data = false;
							continue;
						}
						else if (keyword == "value") {
							is_keyword_value         = true;
							is_wait_symbol_read_data = false;
							continue;
						}
						else if (is_keyword_type) {

							is_keyword_type          = false;
							is_wait_symbol_read_data = true;

							current_content->data.value = keyword;
							current_content->data.is_declaration_type = true;
							current_content->data.is_declaration_unknown = false;

							position_code = current_ast->get_data()->word.position;


							continue;
						}
						else if (is_keyword_value) {

							is_keyword_value = false;
							is_wait_symbol_read_data = true;

							current_content->data.value = keyword;
							current_content->data.is_declaration_value  = true;
							current_content->data.is_declaration_unknown = false;

							position_code = current_ast->get_data()->word.position;
							continue;
						}
						else
						{
							is_wait_symbol_read_data                     = true;
							current_content->data.value                  = keyword;
							current_content->data.is_declaration_unknown = true;
							is_wait_after_logical_word                   = false;
							continue;
						}
					}

					if (is_wait_symbol_read_data && keyword == ":") {
						is_keyword_read_data     = true;
						is_wait_symbol_read_data = false;
						continue;
					}
					else if (is_wait_symbol_read_data) {

						if (keyword == "and" || keyword == ",") {

							if (current_content->parent()) {

								if (current_content->parent()->get_data()->is_property_or) {
									error_context.push(fmt::format("Parent can`t be OR. After \"{}\" was declared AND.", current_content->data.value), position_code);
									return;
								}

								current_content = current_content->parent()->push();
								current_content->parent()->get_data()->is_property_and = true;
								is_wait_after_logical_word = true;
							}
							else {
								error_context.push(fmt::format("No parent for \"{}\", looks like block used solo and property.", current_content->data.value), position_code);
								return;
							}

							is_wait_symbol_read_data = false;
							continue;
						}

						if (keyword == "or") {

							if (current_content->parent()) {

								if (current_content->parent()->get_data()->is_property_and) {
									error_context.push(fmt::format("Parent can`t be AND. After \"{}\" was declared OR.", current_content->data.value), position_code);
									return;
								}

								current_content = current_content->parent()->push();
								current_content->parent()->get_data()->is_property_or = true;
								is_wait_after_logical_word = true;
							}
							else {
								error_context.push(fmt::format("No parent for \"{}\", looks like block used solo and property.", current_content->data.value), position_code);
								return;
							}

							is_wait_symbol_read_data = false;
							continue;
						}

						if (keyword == "not" || keyword == "!") { 
							is_not = true;
							is_wait_after_not_word = true;
							continue;
						}

						if (keyword == ";") {
							is_keyword_read_data     = false;
							is_wait_symbol_read_data = false;
							is_have_next             = true;
							continue;
						}

						is_keyword_read_data     = false;
						is_wait_symbol_read_data = false;
	
						std::string abstraction_name;

						if (current_content->get_data()->is_declaration_value)						
							abstraction_name = "value";
						
						if (current_content->get_data()->is_declaration_type)
							abstraction_name = "type";
						
						if (abstraction_name.empty())
							error_context.push(fmt::format("Symbol expected \":\" or \";\" after declare \"{}\", but here unknown word \"{}\".", current_content->get_data()->value, keyword), position_code);
								else
							error_context.push(fmt::format("Symbol expected \":\" or \";\" after declare {} \"{}\", but here unknown word \"{}\".", abstraction_name, current_content->get_data()->value, keyword), position_code);

						continue;
					}

					if (is_keyword_read_data) {

						if (keyword == "and" || keyword == ",") {

							//current_content->get_data()->is_declaration_body_graph = true;
							//current_content->get_data()->is_property_and = true;

							//if (current_content->get_data()->is_declaration_value) {
							//	current_content->get_data()->is_declaration_value = false;
							//	current_content->get_data()->is_declaration_type  = true;
							//}

							error_context.push(fmt::format("After \":\", can`t be uses \"and/,\" keyword/symbol.", current_content->data.value), position_code);
							continue;
						}

						if (keyword == "or") {

							//current_content->get_data()->is_declaration_body_graph = true;
							//current_content->get_data()->is_property_or = true;


							//if (current_content->get_data()->is_declaration_value) {
							//	current_content->get_data()->is_declaration_value = false;
							//	current_content->get_data()->is_declaration_type = true;
							//}
							error_context.push(fmt::format("After \":\", can`t be uses \"or\" keyword.", current_content->data.value), position_code);
							continue;
						}

						// end
						if (keyword == ";") {
							
							error_context.push(fmt::format("No data after \"{}\", but was used end-close symbol \";\".", current_content->data.value), position_code);
							
							is_keyword_read_data     = false;
							is_wait_symbol_read_data = false;

							is_have_next = true;

							continue;
						}

						if (current_ast->get_data()->is_block) {

							if (current_content->get_data()->is_declaration_unknown) {
								current_content->get_data()->is_declaration_type   = true;
								current_content->get_data()->is_declaration_unknown = false;
							}

							current_content->get_data()->is_declaration_body_graph = true;
							current_content->get_data()->is_declaration_not        = is_not;

							is_wait_after_logical_word = false;
							is_wait_after_not_word     = false;
							is_keyword_read_data       = false;

							bool is_empty_block = false;
							block_parse_ast(error_context, current_ast->child(), current_content, is_empty_block);

							if (!is_not_empty && is_empty_block) {

								is_empty = true;
								current_content->delete_graph();

							}

							continue;
						}
						else if (current_ast->get_data()->is_block_value) {

							if (current_content->get_data()->is_declaration_unknown) {
								current_content->get_data()->is_declaration_value_in_superposition = true;
								current_content->get_data()->is_declaration_unknown                = false;
							}

							current_content->get_data()->is_indivisible            = true;
							current_content->get_data()->is_declaration_body_graph = true;
							current_content->get_data()->is_declaration_not        = is_not;

							is_wait_after_logical_word = false;
							is_wait_after_not_word     = false;

							bool is_empty_block = false;
							block_parse_ast(error_context, current_ast->child(), current_content, is_empty_block);

							if (!is_not_empty && is_empty_block) {

								is_empty = true;
								current_content->delete_graph();

							}

							continue;

						}
						else if (current_ast->get_data()->is_block_meta) {}
						else if (current_ast->get_data()->is_string) {

							/* like strings */

							if (current_content->get_data()->is_declaration_unknown && !current_content->get_data()->is_declaration_body_graph) {
								current_content->get_data()->is_declaration_value  = true;
								current_content->get_data()->is_declaration_unknown = false;
							}

							auto new_element = current_content->push();

							new_element->get_data()->value = keyword;
							new_element->get_data()->is_declaration_not = is_not;
							new_element->get_data()->is_indivisible = true;

							is_not_empty = true;
							is_not       = false;

							is_wait_after_logical_word = false;
							is_wait_after_not_word     = false;
							is_keyword_read_data       = false;

						}
						else { 
							/* like type/group words */

							auto new_element = current_content->push();

							if (current_content->get_data()->is_declaration_unknown && !current_content->get_data()->is_declaration_body_graph) {
								current_content->get_data()->is_declaration_body_graph = true;
								current_content->get_data()->is_declaration_type       = true;
								current_content->get_data()->is_declaration_unknown     = false;
							} 



							new_element->get_data()->value = keyword;
							new_element->get_data()->is_declaration_not = is_not;
							new_element->get_data()->is_declaration_body_unknown = true;

							is_not_empty = true;
							is_not       = false;

							is_wait_after_logical_word = false;
							is_wait_after_not_word     = false;
							is_keyword_read_data       = false;
						}
					}
					else
					{
						if (current_ast->get_data()->is_string) {

							current_content->get_data()->is_declaration_value = true;
							current_content->get_data()->is_declaration_unknown = false;
							current_content->get_data()->is_declaration_body_graph = false;
							current_content->get_data()->is_indivisible = true;

							current_content->get_data()->is_declaration_not = is_not;

							current_content->data.value = keyword;

							is_not_empty = true;

							is_not = false;
							is_wait_after_not_word = false;
							is_wait_after_logical_word = false;
						}

						if (current_ast->get_data()->is_block) {

							if (current_content->get_data()->is_declaration_unknown) {
								current_content->get_data()->is_declaration_type = true;
								current_content->get_data()->is_declaration_unknown = false;
							}

							if (current_content->get_data()->value.empty())
								current_content->get_data()->value = keyword;

							current_content->get_data()->is_declaration_body_graph = true;

							current_content->get_data()->is_declaration_not = is_not;

							is_wait_after_logical_word = false;
							is_wait_after_not_word     = false;

							is_not = false;

							bool is_empty_block = false;

							block_parse_ast(error_context, current_ast->child(), current_content, is_empty_block);

							if (!is_not_empty && is_empty_block) {

								is_empty = true;
								current_content->delete_graph();

							}
						}
					}

				} while (current_ast = current_ast->next());

				if (is_keyword_type)
					error_context.push(fmt::format("No name after of type keyword."), position_code);
				
				if (is_keyword_type)
					error_context.push(fmt::format("No value after of value keyword."), position_code);

				if (is_keyword_read_data)
					error_context.push(fmt::format("No data after of : symbol."), position_code);

			//	if (is_wait_symbol_read_data)
			//		error_context.push(fmt::format("No symbol \":\"."), line_code, start_position, end_position);

				if (is_wait_after_logical_word)
					error_context.push(fmt::format("No data after of AND(,)/OR/NOT(!) words."), position_code);

			}

		};
	}
}