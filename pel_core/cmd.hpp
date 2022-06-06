#pragma once

#include "flag.hpp"
#include "graph.hpp"

namespace pel {

	namespace core {

		namespace interpreter {

			struct cmd_t;

			using gcmd_t = pel::core::graph_t<cmd_t>;

			enum quantum_flag_t : pel::detail::flag32_t
			{
				quantum_value = 1 << 0,
				quantum_type = 1 << 1,
				quantum_exists = 1 << 2,
				quantum_and = 1 << 3,
				quantum_or = 1 << 4,
				quantum_not = 1 << 5,
				empty_operation = 1 << 6,
				quantum_group = 1 << 7,
				quantum_repeat = 1 << 8,
				quantum_maybe = 1 << 9,
				quantum_return = 1 << 10,
				quantum_exit = 1 << 11,
				quantum_recursion = 1 << 12,
				quantum_autogen = 1 << 13,
				quantum_breakpoint = 1 << 14, // for debug from c++
				quantum_repeat_end = 1 << 15,
				quantum_true = 1 << 16,
				quantum_false = 1 << 17,
				quantum_global = 1 << 18,
				quantum_local = 1 << 19,
				quantum_break = 1 << 20,
				quantum_break_now = 1 << 21,
				quantum_break_all = 1 << 22,
			};

			struct cmd_t
			{
				std::string   value;

				void set(const std::string& str) { value = str; }

				cmd_t() = default;

				cmd_t(pel::detail::flag32_t f) { flag = f; }

				cmd_t(const std::string& v)
				{
					value = v;

					if (!pel::detail::check_flag(flag, quantum_value))
						pel::detail::add_flag(flag, quantum_value);
				}

				cmd_t(const std::string& v, std::shared_ptr<gcmd_t> paths)
				{
					value = v;

					if (!pel::detail::check_flag(flag, quantum_value))
						pel::detail::add_flag(flag, quantum_value);

					path_types = paths;
				}

				cmd_t(const std::string& v, pel::detail::flag32_t f) {

					value = v;
					flag = f;

					if (!pel::detail::check_flag(flag, quantum_value) && !pel::detail::check_flag(flag, quantum_type) && !value.empty())
						pel::detail::add_flag(flag, quantum_value);
				}

				inline bool is_or() { return pel::detail::check_flag(flag, quantum_or); }
				inline bool is_and() { return pel::detail::check_flag(flag, quantum_and); }
				inline bool is_not() { return pel::detail::check_flag(flag, quantum_not); }
				inline bool is_value() { return pel::detail::check_flag(flag, quantum_value); }
				inline bool is_type() { return pel::detail::check_flag(flag, quantum_type); }
				inline bool is_exists() { return pel::detail::check_flag(flag, quantum_exists); }
				inline bool is_group() { return pel::detail::check_flag(flag, quantum_group); }
				inline bool is_repeat() { return pel::detail::check_flag(flag, quantum_repeat); }
				inline bool is_maybe() { return pel::detail::check_flag(flag, quantum_maybe); }
				inline bool is_return() { return pel::detail::check_flag(flag, quantum_return); }
				inline bool is_exit() { return pel::detail::check_flag(flag, quantum_exit); }
				inline bool is_recursion() { return pel::detail::check_flag(flag, quantum_recursion); }
				inline bool is_autogen() { return pel::detail::check_flag(flag, quantum_autogen); }
				inline bool is_breakpoint() { return pel::detail::check_flag(flag, quantum_breakpoint); }
				inline bool is_repeat_end() { return pel::detail::check_flag(flag, quantum_repeat_end); }
				inline bool is_true() { return pel::detail::check_flag(flag, quantum_true); }
				inline bool is_false() { return pel::detail::check_flag(flag, quantum_false); }
				inline bool is_global() { return pel::detail::check_flag(flag, quantum_global); }
				inline bool is_local() { return pel::detail::check_flag(flag, quantum_local); }
				inline bool is_break() { return pel::detail::check_flag(flag, quantum_break); }
				inline bool is_break_now() { return pel::detail::check_flag(flag, quantum_break_now); }
				inline bool is_break_all() { return pel::detail::check_flag(flag, quantum_break_all); }

				inline bool is_empty_operation() { return pel::detail::check_flag(flag, empty_operation); }

				std::shared_ptr<gcmd_t> path_types;

				void make_paths() { path_types = std::make_shared<gcmd_t>(); }

				pel::detail::flag32_t flag = 0;
			};

			std::shared_ptr<gcmd_t> make_paths() { return std::make_shared<gcmd_t>(); }
		}
	}
}