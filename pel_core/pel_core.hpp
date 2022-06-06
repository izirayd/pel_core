#pragma once

#include "base_interpreter.hpp"
#include "cpp_parser.hpp"
#include "error.hpp"

#define debug_log if (false) 

namespace pel {

	class parser_core_t {
		public:

	};

	using parser_t = std::shared_ptr<parser_core_t>;

	class parser_engine_t {

		public:
			using interpreter_t = pel::core::interpreter::interpreter_t;
			using code_parser_t = pel::parser::cpp_pel_parser_t;
			using ast_content_t = pel::parser::cpp_pel_parser_t::ast_content_t;
			using scmd_t        = pel::core::interpreter::scmd_t;
			
			parser_engine_t() = default;
			parser_engine_t(const std::string& code) { set_code(code); }

			interpreter_t::ast_type get_ast() { return interpreter.get_ast(); }

			interpreter_t::ast_type get_ast(const std::string& input_string) { 
				
				parse(input_string);

				return interpreter.get_ast(); 
			}

			void parse(const std::string &input_string) {

				// TODO: 
				if (input_string.empty())
					return;

				pel::groups::array_words_t array_words;
				array_words.push_group({ input_string });

				// TODO: 
				if (!interpreter.get_first_point())
					return;

				interpreter.matching_process(array_words);
			}

			[[nodiscard]]
			pel::error_t compile_code() {

				debug_log fmt::print("Process compile code:\n {}\n", code_parser.get_code());

				code_parser.process_parse(error_context);

				auto pel_ast = code_parser.get_ast();

				if (!pel_ast) {
					error_context.push("Can`t get pel-ast.");
					return error_context;
				}

				auto current = pel_ast->child();

				if (!current) {
					error_context.push("Child ast is empty");
					return error_context;
				}

				do
				{
					// if (current->data.is_exist)
					if (current->data.value == "a")
						exist_names[current->data.value] = current;

					global_names[current->data.value] = current;

				} while (current = current->next());

				for (auto& it : global_names) {
					it.second->do_previous_null();
					it.second->do_next_null();
					it.second->do_end_null();
					it.second->do_parent_null();
				}

				current = pel_ast->child();

				if (exist_names.empty()) {
					error_context.push("No declarare types with exist property.");
					return error_context;
				}

				debug_log fmt::print("Start process linker\n\n");

				for (auto& element_ast : exist_names) {

					type_links(element_ast.second);

					debug_log fmt::print("\nStart process create execute scmd\n");

					const auto ast_root      = element_ast.second;
					const auto ast_root_data = ast_root->get_data();

					if (ast_root_data->is_declaration_type) {

						if (ast_root_data->value.empty()) {
							// no name error
							continue;
						}

						auto scmd_root    = new scmd_t(ast_root_data->value);
						auto scmd_current = scmd_root;

						scmd_compile(ast_root, scmd_current);

						debug_log fmt::print("Compile \"{}\" was success, result path:\n\n", scmd_root->data.value);
						debug_log pel::core::interpreter::smcd_print(scmd_root);

						interpreter.push(scmd_root);
					}
				}

				code_parser.clear();

				if (false)
					for (auto& element_ast : exist_names) {

						auto ast_root = element_ast.second;

						ast_root->for_each([=](ast_content_t* element, pel::core::graph_info_t& info) {

							auto data = element->get_data();

							std::string tabs;
							for (size_t i = 0; i < info.level; i++)
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

				return error_context;
			}

			void set_code(const std::string& code_pel) {
				code_parser.set_code(code_pel);
			}

			void clear() {
				code_parser.clear();
				global_names.clear();
				exist_names.clear();
				interpreter.clear();
				error_context.clear();
			}

	private:
			interpreter_t interpreter;
			code_parser_t code_parser;
			pel::error_t  error_context;

			std::unordered_map<std::string,	ast_content_t*>	global_names, exist_names;

			void type_links(ast_content_t* ast_content) {
					
				auto current = ast_content;

				bool is_exit = false;

				std::string select_type = current->get_data()->value;
				auto parent_adress      = fmt::ptr(current);

				debug_log fmt::print("Linker: {}({})\n", select_type, parent_adress);

				if (current->get_data()->is_read) {
					debug_log fmt::print("\tType: {}({}) was read\n", current->get_data()->value, parent_adress);
					return;
				}

				for (;;) {

					current->get_data()->is_read = true;

					if (current->get_data()->is_declaration_unknown || current->get_data()->is_declaration_body_unknown) {

						debug_log fmt::print("\tLinker: {}({}) parent type {}({})\n", current->get_data()->value, fmt::ptr(current), select_type, parent_adress);

						auto result_name = global_names.find(current->get_data()->value);

						if (result_name != global_names.end()) {
				
							type_links(result_name->second);

							debug_log fmt::print("\t\tLink \"{}({})\" was success\n", current->get_data()->value, fmt::ptr(current));

							if (current == result_name->second) 
								debug_log fmt::print("\t\t\"{}\" some content\n", current->get_data()->value);
							
							debug_log fmt::print("\t\ \"{}({})\" copy data from \"{}({})\"\n", current->get_data()->value, fmt::ptr(current), result_name->second->get_data()->value, fmt::ptr(result_name->second));
						
							current->push(result_name->second, true);

							if (current->get_data()->is_declaration_unknown)
								current->get_data()->is_having_child_like_real_element = true;
						}
						else
						{
							debug_log fmt::print("\t\tError, no found type name {}\n", current->get_data()->value);
							return;
						}				
					}

					if (current->child())
					{
						if (!current->child()->get_data()->is_read) {
							current = current->child();
							continue;
						}
					}

					if (current->next()) {
						current = current->next();
						continue;
					}

					current = current->parent();

					if (current) {

						while (current) {

							if (current->next())
							{
								if (!current->next()->get_data()->is_read) {
									current = current->next();
									break;
								}
								else
								{
									// TODO: its should check read state to?
									current = current->parent();
								}
							}
							else
							{
								current = current->parent();
							}

							if (!current || current == ast_content) {
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

			pel::error_t weave_scmd(ast_content_t* ast_content, scmd_t* &scmd_current, ast_content_t::type_t *current_data)
			{
				if (!ast_content || !scmd_current) {
					error_context.push("Can`t get pel-ast or allocate scmd buffer.");
					return error_context;
				}

				auto element = ast_content;

				bool is_use_scmd_link = (ast_content->current_parent() == nullptr);

				if (current_data->is_indivisible) {

					if (current_data->is_property_or) {

						if (current_data->scmd_link && is_use_scmd_link)
							scmd_current = scmd_current->push_value_or(current_data->scmd_link);
						else
							scmd_current = scmd_current->push_value_or();
					}
					else {

						if (current_data->scmd_link && is_use_scmd_link)
							scmd_current = scmd_current->push_value_and(current_data->scmd_link);
						else
							scmd_current = scmd_current->push_value_and();
					}
				}
				else {

					if ((!current_data->is_indivisible && current_data->is_declaration_body_graph) || current_data->is_declaration_type || current_data->is_declaration_value) {

						if (current_data->is_property_or) {
							if (current_data->scmd_link && is_use_scmd_link)
								scmd_current = scmd_current->push_type_or(current_data->scmd_link);
							else
								scmd_current = scmd_current->push_type_or();
						}
						else {
							if (current_data->scmd_link && is_use_scmd_link)
								scmd_current = scmd_current->push_type_and(current_data->scmd_link);
							else
								scmd_current = scmd_current->push_type_and();
						}
					}
					else {
						error_context.push(fmt::format("Entity \"{}\" does not have the ability to compile because it is unknown.", element->get_data()->value));
					}
				}

				if (element->parent()) {

					if (element->parent()->data.scmd_link)
					{
						scmd_current->data.copy_path(element->parent()->data.scmd_link);
						scmd_current->data.push_path(element->parent()->data.scmd_link);
					}

				}
				else if (element->current_parent()) {
					if (element->current_parent()->data.scmd_link)
					{
						scmd_current->data.copy_path(element->current_parent()->data.scmd_link);
						scmd_current->data.push_path(element->current_parent()->data.scmd_link);
					}
				}

				return error_context;
			}

			pel::error_t scmd_compile(ast_content_t* ast_content, scmd_t*& scmd_current) {

				if (!ast_content || !scmd_current) {
					error_context.push("Can`t get pel-ast or allocate scmd buffer.");
					return error_context;
				}

				std::deque<ast_content_t*> ast_queue;
				ast_queue.push_back(ast_content);

				pel::core::graph_info_t graph_info;

				bool is_first_element = true;

				for (;;) {

					auto current = ast_queue.back();
					ast_queue.pop_back();

					graph_info.reset();
					graph_info.is_move_next = true;

					if (current->child()) {
						if (current->get_data()->is_having_child_like_real_element && current->child())
							current->child()->set_current_parent(current);
					}

					do
					{
						if (current->next())
							ast_queue.push_back(current->next());

						auto current_data = current->get_data();

						if (current_data->is_declaration_unknown || current_data->is_declaration_body_unknown) {

							if ((!current_data->is_having_child_like_real_element) && (!current->child())) {

								auto result_name = global_names.find(current->get_data()->value);

								if (result_name != global_names.end()) {
									current_data = result_name->second->get_data();
								}
								else
								{
									error_context.push(fmt::format("No declarare {} name.", current->get_data()->value));

									// TODO: ?
									//return error_context;
								}
							}
							else
							{
								current_data->is_declaration_type = true;
							}
						}

						/* function */
						auto adress = fmt::ptr(current);
						debug_log fmt::print("For \"{}\" ({})\n", current_data->value, adress);

						if ((graph_info.is_move_next || graph_info.is_move_child) && !is_first_element)
							weave_scmd(current, scmd_current, current_data);

						scmd_current->data.value    = current_data->value;
						scmd_current->data.is_type  = current_data->is_declaration_type;
						scmd_current->data.is_value = current_data->is_indivisible;

						if (current_data->scmd_link) {

							debug_log fmt::print("Strange error, current_data->scmd_link was try rewrite!\n");

							if (current_data->scmd_link == scmd_current) {
								debug_log fmt::print("Self-red detected for {} ({}), child node should be ignore.\n", current_data->value, adress);
							}

						}
						else
						{
							current_data->scmd_link = scmd_current;
						}

						/* function */
						if (current->get_data()->version_read != 0)
							break;

						current->get_data()->version_read++;

						if (current->child()) {

							if (current->get_data()->is_having_child_like_real_element && current->child())
								current->child()->set_current_parent(current);

							graph_info.reset();
							graph_info.is_move_child = true;
						}

						is_first_element = false;

					} while (current = current->child());

					if (ast_queue.empty())
						break;
				}

				return error_context;
			}
	};
}