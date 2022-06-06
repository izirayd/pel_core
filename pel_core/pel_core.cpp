#include <cstdint>
#include <intrin.h>

//#define PEL_MEMORY_POOL
//#define PEL_MEMORY_RESOURCE
//#define PEL_MEMORY_POOL_GRAPH

#ifdef PEL_MEMORY_POOL

	//#define PEL_MEMORY_POOL_GRAPH
	#define PEL_MEMORY_POOL_AST

#endif

#pragma intrinsic(__rdtsc)
#include "base_interpreter.hpp"
#include "windows.h"
#include <Psapi.h>

#include "memory_pool.hpp"

#include <fmt/printf.h>
#include <fmt/color.h>

#include "detail\file.hpp"
#include "detail\path.hpp"

#include <bitset>

#ifdef _WIN64
#define script_file_name L"\\..\\..\\data\\code.txt"
#else
#define script_file_name L"\\..\\data\\code.txt"
#endif // _WIN64

#ifdef _WIN64
#define script_file_data L"\\..\\..\\data\\data.txt"
#else
#define script_file_data L"\\..\\data\\data.txt"
#endif // _WIN64

void read_data_file(std::string& data, const std::wstring& file_name)
{
	file_t file;

	data.clear();

	file.OpenFile(dir_t(file_name.c_str()));

	if (file)
	{
		if (file.GetSizeFile() > 0) {
			data.resize(file.GetSizeFile());
			file.FullReadFile(data.data(), 1);
		}
		else
		{
			fmt::print("Emtpy file {}\n", file.cfilename.c_str());
			return;
		}

		file.CloseFile();
	}
	else
	{
		fmt::print("Can`t open file {}\n", file.cfilename.c_str());
		return;
	}

	if (data.empty())
	{
		fmt::print("File empty {}\n", file.cfilename.c_str());
		return;
	}
}

class output_handle_t {

 public:

	output_handle_t() {
		init();
	}

	void init() {

#ifdef PLATFORM_WINDOWS
		// for support color from FMT
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

		if (hOut == INVALID_HANDLE_VALUE)
		{
			fmt::print("Error init STD_OUTPUT_HANDLE %u\n", GetLastError());
			return;
		}

		DWORD dwMode = 0;

		if (!GetConsoleMode(hOut, &dwMode))
		{
			fmt::print("Error init GetConsoleMode %u\n", GetLastError());
			return;
		}

		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

		if (!SetConsoleMode(hOut, dwMode))
		{
			fmt::print("Error init SetConsoleMode %u\n", GetLastError());
			return;
		}

#endif // PLATFORM_WINDOWS
	}

};

uint64_t get_used_memory() {
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	return pmc.PrivateUsage;
}

//void ast_test() {
//
//	auto test = pel::make_ast();
//
//	test->use_memory_pool();
//
//	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
//
//	pel::element_ast_t* current = test->push();
//
//	for (size_t i = 0; i < 1000000; i++)
//	{
//		current = current->push();
//		current->data.num = i;
//	}
//
//	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
//	std::chrono::duration<double, std::milli> result = end - start;
//
//	print(fg(fmt::color::azure), "Time push in graph: ");
//	print(fg(fmt::color::coral), "{}", result.count());
//	print(fg(fmt::color::azure), "ms\n");
//
//	start = std::chrono::high_resolution_clock::now();
//
//	test->for_each([](pel::element_ast_t *element, pel::ast_info_t &) {
//		element->data.num++;
//	});
//
//	end = std::chrono::high_resolution_clock::now();
//
//	result = end - start;
//
//	print(fg(fmt::color::azure), "Non-recursive graph iteration time: ");
//	print(fg(fmt::color::coral), "{}", result.count());
//	print(fg(fmt::color::azure), "ms\n");
//
//	print(fg(fmt::color::azure), "Used memory: ");
//	print(fg(fmt::color::coral), "{}", get_used_memory() / 1024 / 1024);
//	print(fg(fmt::color::azure), "Mb\n");
//}

//class application_t {
//
//	bool is_debug      = false;
//	bool is_render_ast = false;
//
//public:
//
//	void init() {
//
//		if (is_debug)
//			debug_init();
//
//		root.use_memory_pool();
//
//		auto a = root.push(); a->data.value = "a";
//		auto b = root.push(); b->data.value = "b";
//
//		auto val = a->push(); val->data.value = ";";
//
//		b->push(val);
//
//		a->data.make_paths();
//		auto data_path_a_main = a->data.path_types->push();
//		auto data_path_a_a = a->data.path_types->push();
//
//		data_path_a_main->data.value = "main";
//		data_path_a_a->data.value = "a";
//
//		b->data.make_paths();
//		auto data_path_b_main = b->data.path_types->push();
//		auto data_path_b_b = b->data.path_types->push();
//
//		data_path_b_main->data.value = "main";
//		data_path_b_b->data.value = "b";
//
//		val->data.make_paths();
//		auto data_path_val = val->data.path_types->push();
//		data_path_val->data.value = "main";
//		
//		pel::groups::position_element_t position_element1, position_element2, position_element3;
//		pel::groups::object_t object1;
//
//		object1.data = "a";
//
//		position_element1.words.push_back(object1);
//		array_words.push_group(position_element1);
//
//		object1.data = ";";
//
//		position_element2.words.push_back(object1);
//		array_words.push_group(position_element2);
//
//		for (size_t i = 0; i < 500000; i++)
//		{
//			array_words.push_group(position_element1);
//			array_words.push_group(position_element2);
//		}
//
//	/*	object1.data = "k";
//
//		position_element3.words.push_back(object1);
//		array_words.push_group(position_element3);*/
//
//		//pel::core::interpreter::element_gcmd_t element_gcmd(&root);
//		//element_gcmd.set_current_start_point(root.child());
//
//	//	interpreter.push(element_gcmd);
//	}
//
//	void launch() {
//
//		interpreter.only_ast_stream_mode();
//
//		std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
//
//		uint64_t start_rdtsc, end_rdtsc;
//
//		start_rdtsc = __rdtsc();
//
//		interpreter.matching_process(array_words);
//
//		end_rdtsc = __rdtsc();
//
//		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
//
//		std::chrono::duration<double, std::micro> result = end - start;
//
//		print(fg(fmt::color::azure), "Process parse all steps end: ");
//		print(fg(fmt::color::coral), "{}", result.count());
//		print(fg(fmt::color::azure), "us/");
//		print(fg(fmt::color::coral), "{}", (end_rdtsc - start_rdtsc));
//		print(fg(fmt::color::azure), "cpu cycles\n");
//
//		//interpreter.ast_stream([=](pel::element_ast_t* current_ast, pel::ast_info_t& info) {
//		//		
//		//});
//
//		//if (is_render_ast) 
//		//	interpreter.get_ast()->for_each([=](pel::element_ast_t* current_ast, pel::ast_info_t& info) {
//
//		//	bool is_last = false;
//	
//		//	if (!current_ast->next())
//		//		is_last = true;
//		//				
//		//	if (info.level == 0)
//		//	{
//		//		fmt::print("{} Ast\n", (char)218);
//		//		return;
//		//	}
//		//	else {
//
//		//		for (size_t i = 0; i < info.level - 1; i++)
//		//		{
//		//			fmt::print("{}", (char)179);
//		//		}
//		//	}
//
//		//	if (info.is_move_child)
//		//	{
//		//		if (current_ast->child()) {
//		//			fmt::print("{}", (char)195);
//		//			fmt::print("{}", (char)194);
//		//		}
//		//		else if (is_last)
//		//		{
//		//			fmt::print("{}", (char)192);
//		//		}
//		//	}
//		//	else
//		//		if (info.is_move_next && !is_last)
//		//		{
//		//			fmt::print("{}", (char)195);
//		//		}
//		//		else if (info.is_move_next && is_last) {
//		//			fmt::print("{}", (char)192);
//		//		}
//
//		//	fmt::print(" {} {} : \"{}\"\n", (char) 254, current_ast->get_name(), current_ast->get_data()->gcmd->data.value);
//		//});
//	}
//
//	private:
//		pel::groups::array_words_t            array_words;
//		pel::core::interpreter::gcmd_t		  root;
//		pel::core::interpreter::interpreter_t interpreter;
//
//	void debug_init() {
//
//		interpreter.debug_mode();
//
//		auto debuger = interpreter.get_debuger();
//
//		debuger->get_stream([=](pel::core::interpreter::debug_page_t* debug_page) {
//
//			auto* input_word = &debug_page->input_words()->front();
//
//			fmt::print("{} equal {} ", input_word->data, debug_page->get_value());
//			if (debug_page->get_status())
//				fmt::print(fg(fmt::color::green_yellow), " [true]");
//			else
//				fmt::print(fg(fmt::color::pale_violet_red), " [false]");
//
//			auto ast = debug_page->get_header_ast();
//
//			if (ast) {
//
//				fmt::print("\nast: \n");
//
//				ast->for_each([=](pel::element_ast_t* current_ast) {
//
//					fmt::print("{}\\", current_ast->get_name());
//
//					});
//			}
//
//			fmt::print("\n");
//			});
//
//	}
//};

//void memory_pool_test() {
//
//	pel::memory_pool::memory_pool_t<pel::element_ast_t> memory_pool;
//
//	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
//
//	memory_pool.init();
//
//	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
//	std::chrono::duration<double, std::milli> result = end - start;
//
//	print(fg(fmt::color::azure), "Time init memory pool: ");
//	print(fg(fmt::color::coral), "{}", result.count());
//	print(fg(fmt::color::azure), "ms\n");
//
//	start = std::chrono::high_resolution_clock::now();
//
//	//memory_pool_t<pel::element_ast_t>::memory_block memory_block;
//
//	//pel::element_ast_t* ast1 = memory_pool.get(memory_block);
//	//memory_pool.free(memory_block);
//	//pel::element_ast_t* ast2 = memory_pool.get(memory_block);
//	//pel::element_ast_t* ast3 = memory_pool.get(memory_block);
//	//pel::element_ast_t* ast4 = memory_pool.get(memory_block);
//
//	for (size_t i = 0; i < 10000000; i++)
//	{
//		pel::memory_pool::memory_pool_t<pel::element_ast_t>::memory_block *memory_block = nullptr;
//
//		pel::element_ast_t *ast = memory_pool.get(memory_block);
//		ast->data.num++;
//		memory_pool.free(memory_block);
//
//		//pel::element_ast_t* ast = new pel::element_ast_t;
//		//ast->data.num++;
//		//delete ast;
//	}
//
//	end = std::chrono::high_resolution_clock::now();
//
//	result = end - start;
//
//	print(fg(fmt::color::azure), "Get/free time: ");
//	print(fg(fmt::color::coral), "{}", result.count());
//	print(fg(fmt::color::azure), "ms\n");
//
//	//memory_pool.reset();
//
//	print(fg(fmt::color::azure), "Used memory: ");
//	print(fg(fmt::color::coral), "{}", get_used_memory() / 1024 / 1024);
//	print(fg(fmt::color::azure), "Mb\n");
//
//}
//
//void malloc_pool_test() {
//
//	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
//
//	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
//	std::chrono::duration<double, std::milli> result = end - start;
//
//	print(fg(fmt::color::azure), "Time init malloc: ");
//	print(fg(fmt::color::coral), "{}", result.count());
//	print(fg(fmt::color::azure), "ms\n");
//
//	start = std::chrono::high_resolution_clock::now();
//
//	for (size_t i = 0; i < 5000000; i++)
//	{
//		pel::element_ast_t* elast = (pel::element_ast_t *) malloc(sizeof(pel::element_ast_t));
//		free(elast);
//	}
//
//	end = std::chrono::high_resolution_clock::now();
//
//	result = end - start;
//
//	print(fg(fmt::color::azure), "Get/free time: ");
//	print(fg(fmt::color::coral), "{}", result.count());
//	print(fg(fmt::color::azure), "ms\n");
//
//	print(fg(fmt::color::azure), "Used memory: ");
//	print(fg(fmt::color::coral), "{}", get_used_memory() / 1024 / 1024);
//	print(fg(fmt::color::azure), "Mb\n");
//}

class test_t {

 public:

	std::string name;
	std::string line;
	std::string file;

	void bad()  { is_result = true; is_good = false;  is_bad  = true; }
	void good() { is_result = true; is_bad  = false;  is_good = true; }

	bool is_result = false;
	bool is_good   = false;
	bool is_bad    = false;

	std::function<void()> func;

	void call_test() {

		fmt::print("test | {} -> ", name);

		if (func) 
			func();

		if (is_result) {

			fmt::print("[");

			if (is_bad)
				fmt::print(fmt::fg(fmt::color::red), "bad");

			if (is_good) {
				fmt::print(fmt::fg(fmt::color::green_yellow), "good");
				fmt::print("]\n");
			}

			if (is_bad) {

				fmt::print("]");
				fmt::print(" line: {}\n", line);
			}
		}
		else {
			fmt::print("[");
			fmt::print(fmt::fg(fmt::color::red), "error");
			fmt::print("]");
			fmt::print(" no function or result in test! line: {}\n", line);
		}
	}
};

#define make_test(name_test, code)  {  test_t test; test.line = fmt::format("{}",  __LINE__); test.file = __FILE__; test.name = name_test; test.func = [&]() { code; }; test.call_test(); }
#define test_good  { test.good(); return; }
#define test_bad   { test.line = fmt::format("{}",  __LINE__);  test.bad(); return; }
//
//void launch_tests() {
//
//	using gcmd_t = pel::core::interpreter::gcmd_t;
//
//	make_test("empty", { 
//		test_good; 
//	});
//
//	make_test("gcmd child()", {
//
//		gcmd_t gcmd;
//		gcmd.use_memory_pool();
//
//		auto a = gcmd.push();
//		a->data.set("a");
//
//		auto b = a->push();
//		b->data.set("b");
//
//		auto c = b->push();
//		c->data.set("c");
//
//		gcmd_t* current = &gcmd;
//
//		if (current->child()) {
//			if (current->child()->data.value != "a")
//				test_bad;
//		}
//		else test_bad;
//
//		current = current->child();
//
//		if (current->child()) {
//			if (current->child()->data.value != "b")
//				test_bad;
//		}
//		else test_bad;
//
//		current = current->child();
//
//		if (current->child()) {
//			if (current->child()->data.value != "c")
//				test_bad;
//		}
//		else test_bad;
//
//		test_good;
//
//	 });
//
//	make_test("gcmd child() + next()", {
//	
//		gcmd_t gcmd;
//		gcmd.use_memory_pool();
//
//		gcmd_t * current = &gcmd;
//
//		for (size_t i = 0; i < 50; i++)
//		{
//			auto spec = current->push();
//			spec->data.set("text");
//		}
//
//		if (!current->child())
//			test_bad;
//
//		current = current->child();
//
//		if (!current->next())
//			test_bad;
//
//		do
//		{
//			if (current->data.value != "text")
//				test_bad;
//
//		} while (current = current->next());
//
//		test_good;
//
//	});
//
//	//make_test("ast child() + next()", { 
//	//	
//	//	pel::ast_t ast = pel::make_ast();
//	//	ast->use_memory_pool();
//
//	//	auto current = ast.get();
//
//	//	for (size_t i = 0; i < 50; i++)
//	//	{
//	//		auto spec = current->push();
//	//		spec->data.num = i;
//	//	}
//
//	//	if (!current->child())
//	//		test_bad;
//
//	//	current = current->child();
//
//	//	if (!current->next())
//	//		test_bad;
//
//	//	int counter = 0;
//
//	//	do
//	//	{
//	//		if (current->data.num != counter)
//	//			test_bad;
//
//	//		counter++;
//
//	//	} while (current = current->next());
//
//	//	test_good;
//	//	
//	//});
//
//	make_test("check simple interpreter_t", {
//		
//
//		test_good; 
//
//	});
//
//}
//
//void simple_test() {
//	using cmd_t         = pel::core::interpreter::cmd_t;
//	using gcmd_t        = pel::core::interpreter::gcmd_t;
//	using interpreter_t = pel::core::interpreter::interpreter_t;
//
//	launch_tests();
//
//	test_t test;
//
//	std::shared_ptr<gcmd_t> current    = std::make_shared<gcmd_t>(pel::mp());
//	std::shared_ptr<gcmd_t> type_path1 = std::make_shared<gcmd_t>(pel::mp());
//	std::shared_ptr<gcmd_t> type_path2 = std::make_shared<gcmd_t>(pel::mp());
//
//	type_path1->ncpush({ {"main"}, {"a"} });
//	type_path2->push({ "main" });
//
//	current->push({ {"a", type_path1}, {";", type_path2} });
//
//	//interpreter_t interpreter({ {current.get(), current->child()} });
//
//	//pel::groups::array_words_t  array_words({ {"a"}, { ";"} });
//
//	//interpreter.matching_process(array_words);
//}

/*
 Checks cpu cycles
*/
void simple_banch() {

	pel::core::interpreter::interpreter_t interpreter;

	pel::groups::array_words_t array_words;

	auto root = new pel::core::interpreter::scmd_t({ "main" });

	const size_t count_iterations = 3000000;

	pel::core::interpreter::scmd_t* last = root;

	for (size_t i = 0; i < count_iterations; i++)
	{
		last = last->push_type_and({ fmt::format("a{}", i) });
		array_words.push_group({ {"Text"} });
	}

	//root->push_type_and(root);

	interpreter.push(root);

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	uint64_t start_rdtsc, end_rdtsc;

	start_rdtsc = __rdtsc();

	interpreter.matching_process(array_words);

	end_rdtsc = __rdtsc();

	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::micro> result = end - start;

	print(fg(fmt::color::azure), "Process parse all steps end: ");
	print(fg(fmt::color::coral), "{}", result.count());
	print(fg(fmt::color::azure), "us/");
	print(fg(fmt::color::coral), "{}", (end_rdtsc - start_rdtsc));
	print(fg(fmt::color::azure), "cpu cycles for {} iterations\n", count_iterations);

	print(fg(fmt::color::azure), "Process parse all steps end: ");
	print(fg(fmt::color::coral), "{}", result.count() / count_iterations);
	print(fg(fmt::color::azure), "us/");
	print(fg(fmt::color::coral), "{}", (end_rdtsc - start_rdtsc) / count_iterations);
	print(fg(fmt::color::azure), "cpu cycles for 1 iteration\n");
}

void simple_banch2() {

	pel::core::interpreter::interpreter_t interpreter;

	pel::groups::array_words_t array_words;

#ifdef PEL_MEMORY_POOL_SPATIAL4
	spatial4_t<cmd_t>::memory_block_t* block = nullptr;
	auto root = spatial4_t<cmd_t>::instance_mem_pool<cmd_t>::get()->get(block);
	root->mem_block = block;

	root->data.value = "main";
#else
	auto root = new pel::core::interpreter::scmd_t({ "main" });
#endif

	auto bla = root->push_value_and({ "texttexttexttexttexttext" });
	bla->data.push_path(root);

	array_words.push_group({ {"texttexttexttexttexttext"} });

	const size_t count_iterations = 1000000;

	interpreter.push(root);

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	uint64_t start_rdtsc, end_rdtsc;

	start_rdtsc = __rdtsc();

	for (size_t i = 0; i < count_iterations; i++)
	{
		interpreter.matching_process(array_words);
		interpreter.clear_ast();
	}

	end_rdtsc = __rdtsc();

	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::micro> result = end - start;

	print(fg(fmt::color::azure), "Process parse all steps end: ");
	print(fg(fmt::color::coral), "{}", result.count());
	print(fg(fmt::color::azure), "us/");
	print(fg(fmt::color::coral), "{}", (end_rdtsc - start_rdtsc));
	print(fg(fmt::color::azure), "cpu cycles for {} iterations\n", count_iterations);

	print(fg(fmt::color::azure), "Process parse all steps end: ");

	bool is_ns = false;
	double time = result.count() / count_iterations;
	if (time < 1)
	{
		time *= 1000;
		is_ns = true;
	}

	print(fg(fmt::color::coral), "{}", time);
	if (is_ns)
		print(fg(fmt::color::azure), "ns/");
	else
		print(fg(fmt::color::azure), "us/");

	print(fg(fmt::color::coral), "{}", (end_rdtsc - start_rdtsc) / count_iterations);
	print(fg(fmt::color::azure), "cpu cycles for 1 iteration\n");
}

void simple_banch3() {

	const size_t count_iterations = 1000000;

	std::vector<std::string> str1;
	std::vector<std::string> str2;

	for (size_t i = 0; i < count_iterations; i++)
	{
		str1.push_back("texttexttexttexttexttext");
		str2.push_back("texttexttexttexttexttext");
	}

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	uint64_t start_rdtsc, end_rdtsc;

	start_rdtsc = __rdtsc();

	size_t count = 0;

	for (size_t i = 0; i < count_iterations; i++)
	{
		if (str1[i] == str2[i])
			count++;
	}

	end_rdtsc = __rdtsc();

	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::micro> result = end - start;

	print(fg(fmt::color::azure), "Process parse all steps end: ");
	print(fg(fmt::color::coral), "{}", result.count());
	print(fg(fmt::color::azure), "us/");
	print(fg(fmt::color::coral), "{}", (end_rdtsc - start_rdtsc));
	print(fg(fmt::color::azure), "cpu cycles for {} iterations\n", count_iterations);

	print(fg(fmt::color::azure), "Process parse all steps end: ");

	bool is_ns = false;
	double time = result.count() / count_iterations;
	if (time < 1)
	{
		time *= 1000;
		is_ns = true;
	}

	print(fg(fmt::color::coral), "{}", time);
	if (is_ns)
		print(fg(fmt::color::azure), "ns/");
	else
		print(fg(fmt::color::azure), "us/");

	print(fg(fmt::color::coral), "{}", (end_rdtsc - start_rdtsc) / count_iterations);
	print(fg(fmt::color::azure), "cpu cycles for 1 iteration\n");

	fmt::print("{}\n", count);
}

class bla_t {
public:
	int a;
	int b;
	int c;
	int d;
};

void simple_banchQueue() {

	std::queue<bla_t> bla_queue;
	pel::core::interpreter::fast_queue_t<bla_t> fast_queue;

	const size_t count_iterations = 1;

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	uint64_t start_rdtsc, end_rdtsc;

	start_rdtsc = __rdtsc();
	
	 bla_t bla;

	 bla.a = 10;

	for (size_t i = 0; i < count_iterations; i++)
	{
		bla_queue.push(bla);
		bla_queue.push(bla);

		bla_queue.pop();

		bla_queue.push(bla);
		bla_queue.push(bla);
		bla_queue.push(bla);

		bla_queue.pop();
		bla_queue.pop();
		bla_queue.pop();
		bla_queue.pop();
	}

	end_rdtsc = __rdtsc();

	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::micro> result = end - start;

	print(fg(fmt::color::azure), "Process parse all steps end: ");
	print(fg(fmt::color::coral), "{}", result.count());
	print(fg(fmt::color::azure), "us/");
	print(fg(fmt::color::coral), "{}", (end_rdtsc - start_rdtsc));
	print(fg(fmt::color::azure), "cpu cycles for {} iterations\n", count_iterations);

	print(fg(fmt::color::azure), "Process parse all steps end: ");

	bool is_ns = false;
	double time = result.count() / count_iterations;

	if (time < 1)
	{
		time *= 1000;
		is_ns = true;
	}

	print(fg(fmt::color::coral), "{}", time);
	if (is_ns)
		print(fg(fmt::color::azure), "ns/");
	else
		print(fg(fmt::color::azure), "us/");

	print(fg(fmt::color::coral), "{}", (end_rdtsc - start_rdtsc) / count_iterations);
	print(fg(fmt::color::azure), "cpu cycles for 1 iteration\n");
}

#include "cpp_parser.hpp"
#include "pel_core.hpp"

template <typename data_t>
struct spatial_t : public data_t {

	std::bitset<8> bits;

	spatial_t* body = nullptr;
	spatial_t* next = nullptr;

	spatial_t* push(int8_t number_spatial) {

		spatial_t* new_spatial = nullptr;
		body = recreate_alloc(body, number_spatial, new_spatial);

		return new_spatial;	
	}

	spatial_t* spatial(int8_t number_spatial) {

		if (!bits.test(number_spatial))
			return nullptr;

		int8_t position = 0;
		const int8_t count = bits.count();

		for (int8_t i = 0; position < count; i++) {

			if (bits.test(i))
				position++;

			if (number_spatial == position)
				break;
		}
		
		spatial_t* current = this;
		
		for (size_t i = 0; i < position; i++)
		{
			current = current->body;
		}

		return current;
	}

	spatial_t* recreate_alloc(spatial_t* old_body, int8_t number_spatial, spatial_t* &new_spatial) {

		if (!bits.test(number_spatial)) {

			old_body->body = new spatial_t<data_t>;
			old_body->bits.set(number_spatial, true);

		}
		else
		{
			// error?
		}


		return old_body;
	}

};

int main()
{
	std::system("title PEL DEV: Parser Engine Lang Dev console");

	output_handle_t output_handle;

	std::bitset<64> bitset;

	bitset.set(0, 0);
	bitset.set(1, 1);
	bitset.set(2, 0);
	bitset.set(3, 1);
	bitset.set(4, 1);
	bitset.set(5, 1);
	bitset.set(6, 0);
	bitset.set(7, 0);

	for (size_t i = 7; i < 64; i++)
	{
		bitset.set(i, 0);
	}

	uint64_t flag = 0;
	flag |= 1 << 1;
	flag |= 1 << 3;
	flag |= 1 << 4;
	flag |= 1 << 5;

	size_t count = 0;

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	uint64_t start_rdtsc, end_rdtsc;

	start_rdtsc = __rdtsc();

	for (size_t i = 0; i < 10000000; i++)
	{
	/*	for (size_t w = 0; w < 64; w++)
		{
			if (flag & 1 << w)
				count++;
		}*/

		count += bitset.count();

	}

	end_rdtsc = __rdtsc();
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::micro> result = end - start;

	print(fg(fmt::color::azure), "Process end ");
	print(fg(fmt::color::coral), "{}", result.count());
	print(fg(fmt::color::azure), "us/");
	print(fg(fmt::color::coral), "{} cpucylces\n", (end_rdtsc - start_rdtsc));

	print(fg(fmt::color::azure), "Result {}\n", count);




	/*std::string code, data;

	read_data_file(code, script_file_name);
	read_data_file(data, script_file_data);

	for (size_t i = 0; i < 1; i++)
	{
		pel::parser_engine_t pe;
		pe.set_code(code);

		std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

		uint64_t start_rdtsc, end_rdtsc;

		start_rdtsc = __rdtsc();

		auto error_context = pe.compile_code();

		end_rdtsc = __rdtsc();
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double, std::micro> result = end - start;

		print(fg(fmt::color::azure), "Process parse all steps end: ");
		print(fg(fmt::color::coral), "{}", result.count());
		print(fg(fmt::color::azure), "us/");
		print(fg(fmt::color::coral), "{} cpucylces\n", (end_rdtsc - start_rdtsc));

		if (error_context.is_error()) {

			fmt::print("Pel compile error:\n");

			for (const auto& error : error_context.errors)
				fmt::print("{}|{}:{}	{}\n",
					error.get_line(), 
					error.get_start(), 
					error.get_end(), 
					error.get_text());
		}
		else
		{
			fmt::print("Success compile\n");

			auto ast = pe.get_ast("text");

			if (ast) {

			 ast->for_each([=](pel::element_ast_t<pel::core::interpreter::ast_data_t>* current_ast, pel::ast_info_t& info) {

			bool is_last = false;
	
			if (!current_ast->next())
				is_last = true;
						
			if (info.level == 0)
			{
				fmt::print("{} Ast\n", (char)218);
				return;
			}
			else {

				for (size_t i = 0; i < info.level - 1; i++)
				{
					fmt::print("{}", (char)179);
				}
			}

			if (info.is_move_child)
			{
				if (current_ast->child()) {
					fmt::print("{}", (char)195);
					fmt::print("{}", (char)194);
				}
				else if (is_last)
				{
					fmt::print("{}", (char)192);
				}
			}
			else
				if (info.is_move_next && !is_last)
				{
					fmt::print("{}", (char)195);
				}
				else if (info.is_move_next && is_last) {
					fmt::print("{}", (char)192);
				}

			  fmt::print(" {} {} : \"{}\"\n", (char) 254, current_ast->get_name(), current_ast->get_data()->scmd->data.value);
		});

			}

		}


		pe.clear();
	}
*/

	fmt::print("End\n");
	/*
	
		value_block2 : ("string");
		type_block1  : "string" and "string";
		type_block2  : { "string" and "string" };

		v : "string";
		a1 : b;
		a2 : "string" and "string";
		a3 : b and "string";
		a4 : "string" and b;
		
	
	*/

	//type name2 : "text";
	//value value1 : "string";
	//type multi_blocks : { aand b };
	//type multi_blocks : { aand b, { c or d } };
	//type name1 : {} = property;
	//type name1 : {} = property1 and property2 and property3;
	//type name1 : {} = { property1, property2, property3 };


    ///simple_banch2();

	//{
	//	pel::core::interpreter::interpreter_t interpreter;

	//	pel::groups::array_words_t array_words;

	//	auto root = new pel::parser_engine_t::scmd_t({ "main" });

	//	auto a1 = root->push_type_and({ "type_a1", root });
	//	auto a2 = a1->push_type_or({ "type_a2", root });
	//	auto a3 = a2->push_type_or({ "type_a3", root });

	//	auto value_a1 = a1->push_value_and({ "a1" });

	//	value_a1->data.push_path(root);
	//	value_a1->data.push_path(a1);

	//	auto value_a2 = a2->push_value_and({ "a2" });

	//	value_a2->data.push_path(root);
	//	value_a2->data.push_path(a2);

	//	auto value_a3 = value_a2->push_value_and({ "a3" });

	//	value_a3->data.push_path(root);
	//	value_a3->data.push_path(a2);

	//	array_words.push_group({ {"a2"} });
	//	array_words.push_group({ {"a3"} });
	//	//array_words.push_group({ {"a4"} });

	//	interpreter.push(root);

	//	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	//	uint64_t start_rdtsc, end_rdtsc;

	//	start_rdtsc = __rdtsc();

	//	auto first_point = interpreter.get_first_point();

	//	if (first_point)
	//	{
	//		pel::core::interpreter::smcd_print(first_point);
	//	}

	//	interpreter.matching_process(array_words);

	//	end_rdtsc = __rdtsc();

	//	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

	//	std::chrono::duration<double, std::micro> result = end - start;

	//	print(fg(fmt::color::azure), "Process parse all steps end: ");
	//	print(fg(fmt::color::coral), "{}", result.count());
	//	print(fg(fmt::color::azure), "us/");
	//	print(fg(fmt::color::coral), "{}", (end_rdtsc - start_rdtsc));
	//	print(fg(fmt::color::azure), "cpu cycles for\n");

	//}


	//interpreter.manager_signals

	//fmt::print("result: {}\n", ast["main"]["a"].get_value());
	
		
//	malloc_pool_test();
//	memory_pool_test();

	//print(fg(fmt::color::azure), "	Ast test\n");

	//ast_test();

	//application_t application;

	//application.init();
	//application.launch();


				//	debug_log fmt::print("End of job\n");

					/*manager_signals.context.ast->for_each([=](pel::element_ast_t<ast_data_t>* current_ast, pel::ast_info_t& info)
					{
						bool is_last = false;

						if (!current_ast->next())
						is_last = true;

						if (info.level == 0)
						{
							fmt::print("{} Ast\n", (char)218);
							return;
						}
						else {

							for (size_t i = 0; i < info.level - 1; i++)
							{
								fmt::print("{}", (char)179);
							}
						}

					if (info.is_move_child)
					{
						if (current_ast->child()) {
							fmt::print("{}", (char)195);
							fmt::print("{}", (char)194);
						}
						else if (is_last)
						{
							fmt::print("{}", (char)195);
							fmt::print("{}", (char)192);
						}
						else
						{
							fmt::print("{}", (char)195);
							fmt::print("{}", (char)194);
						}
					}
						else
					if (info.is_move_next && !is_last)
					{
						fmt::print("{}", (char)195);
					}
					else if (info.is_move_next && is_last) {
						fmt::print("{}", (char)179);
						fmt::print("{}", (char)192);
					}

						fmt::print(" {} \"{}\"\n", (char) 254, current_ast->get_data()->scmd->data.value);

				  });*/


	for (;;)
	{

	}
}
