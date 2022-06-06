#pragma once

#include "signal.hpp"

namespace pel {

	namespace core {

		namespace interpreter {

			constexpr int version_interpreter_major = 3;
			constexpr int version_interpreter_minor = 0;

			class interpreter_t {

				void init() {
					manager_signals.init();

					ast = pel::make_ast<ast_data_t>();
					ast->use_memory_pool();
				}

				manager_signals_t manager_signals;

				bool is_only_ast_stream = false;


				ast_t<ast_data_t> ast;

			public:

				interpreter_t() { init(); }

				using ast_type = ast_t<ast_data_t>;

				scmd_t* get_first_point()     { return manager_signals.get_first_point(); }

				bool only_ast_stream_status() { return is_only_ast_stream; }
				void only_ast_stream_mode()   { is_only_ast_stream = true; }

				ast_t<ast_data_t>	   get_ast() { return ast; }

				void clear_ast() { 
					if (manager_signals.context.get_ast())
						manager_signals.context.get_ast()->cyclic_deletion();
				}

				void clear() {
					manager_signals.clear();
					manager_signals.delete_ast();
					//ast.reset();
				}

				inline void call_ast_stream(element_ast_t<ast_data_t>* const element_ast) {

					if (!element_ast)
						return;

					element_ast->for_each(
						[&](element_ast_t<ast_data_t>* element, ast_info_t& info) {
							if (!ast_streams.empty())
								for (const auto& func : ast_streams)
									if (func)
										func(element, info);
						}
					);
				}

				std::vector<ast_function_t<ast_data_t>> ast_streams;

				void matching_process(pel::groups::array_words_t& array_words) {

					for (size_t position = 0; position < array_words.data.size(); position++)
					{
						auto context = manager_signals.get_context();

						context->current_word = &array_words.data[position];

						manager_signals.run();

						if (context->is_end_state) {

							if (context->is_boolean_state && context->get_ast()) {
								// push in main ast
								if (context->get_ast()->child()) {
									context->get_ast()->cut_child(get_ast()->get_this());
								}
							}

							manager_signals.reset();							
						}

						manager_signals.swap_signal();
					}
				}

				// todo: what about more start scmd? I mean we can use OR instruction
				void push(scmd_t* const start_scmd)
				{
					manager_signals.start_point(start_scmd);
				}
			};
		}
	}
}