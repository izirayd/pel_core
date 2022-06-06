#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>

namespace pel {

	namespace until {


		struct position_t {

			position_t() = default;
			position_t(std::size_t num_line, std::size_t start_position, std::size_t end_position) {
				this->number_line = num_line;
				this->start_position = start_position;
				this->end_position = end_position;
			}

			// todo: inline?
			const std::size_t &get_line()  const  { return number_line;    }
			const std::size_t &get_start() const  { return start_position; }
			const std::size_t &get_end()   const  { return end_position;   }

			std::size_t number_line    = 0;
			std::size_t start_position = 0;
			std::size_t end_position   = 0;
		};

	}
}


namespace pel {

	enum class type_error_t
	{
		no_error = 0,
		// its about parse pel code
		parsing_code,
		// types error
		links
	};

	namespace error {

		class element_t {

		public:
			element_t() = default;
			element_t(const std::string& error_text) : text(error_text) { is_only_text = true; }
			element_t(const std::string& error_text, const until::position_t &pos) : text(error_text), position(pos) { is_only_text = true; }
				 
			std::size_t code_error = 0;

			std::string text;

			bool is_only_text = false;

			const std::string &get_text()  const  { return text;                 }
			const std::size_t &get_line()  const  { return position.get_line();  }
			const std::size_t &get_start() const  { return position.get_start(); }
			const std::size_t &get_end()   const  { return position.get_end();   }
			const std::size_t &get_id()    const  { return code_error;           }

			until::position_t position;
		};

	}

	class error_t {

		public:
			bool is_error() { return !errors.empty(); }
				 
			void push(const error::element_t& new_error) {
				errors.push_back(new_error);
			}

			void push(const std::string& error_text) {
				push(error::element_t(error_text));
			}

			// TODO: inline?
			void push(const std::string& error_text, const until::position_t &position_text) {
				errors.push_back(error::element_t(error_text, position_text));
			}

			std::size_t size() { return errors.size(); }

			std::vector<error::element_t> errors;

			void clear() {
				errors.clear();
			}
	};

}