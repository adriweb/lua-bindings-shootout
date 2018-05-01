// lua bindings shootout
// The MIT License (MIT)

// Copyright © 2018 ThePhD

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <lua.hpp>

#include "lbs_lua.hpp"
#include "lbs_lib.hpp"
#include "benchmark.hpp"

void plain_c_global_string_get_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(false);
	lua_State* L = lua.get();

	lua_pushnumber(L, 3);
	lua_setglobal(L, "value");
	double x = 0;
	for (auto _ : benchmark_state) {
		lua_getglobal(L, "value");
		double v = static_cast<double>(lua_tonumber(L, -1));
		x += v;
		lua_pop(L, 1);
	}
	lbs::expect(benchmark_state, x, benchmark_state.iterations() * 3);
}

void plain_c_global_string_set_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(false);
	lua_State* L = lua.get();

	double v = 0;
	for (auto _ : benchmark_state) {
		v += 3;
		lua_pushnumber(L, v);
		lua_setglobal(L, "value");
	}
	lua_getglobal(L, "value");
	double x = lua_tonumber(L, -1);
	lua_pop(L, 1);
	lbs::expect(benchmark_state, x, v);
	lbs::expect(benchmark_state, x, benchmark_state.iterations() * 3);
	lbs::expect(benchmark_state, v, benchmark_state.iterations() * 3);
}

void plain_c_table_get_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(false);
	lua_State* L = lua.get();

	lua_createtable(L, 0, 0);
	lua_pushnumber(L, 3);
	lua_setfield(L, -2, "value");
	lua_setglobal(L, "warble");

	lua_getglobal(L, "warble");
	double x = 0;
	for (auto _ : benchmark_state) {
		lua_getfield(L, -1, "value");
		double v = static_cast<double>(lua_tonumber(L, -1));
		x += v;
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
	lbs::expect(benchmark_state, x, benchmark_state.iterations() * 3);
}

void plain_c_table_set_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(false);
	lua_State* L = lua.get();

	lua_createtable(L, 0, 0);
	lua_pushnumber(L, 3);
	lua_setfield(L, -2, "value");
	lua_setglobal(L, "warble");

	lua_getglobal(L, "warble");
	double v = 0;
	for (auto _ : benchmark_state) {
		v += 3;
		lua_pushnumber(L, v);
		lua_setfield(L, -2, "value");
	}
	lua_getfield(L, -1, "value");
	double x = lua_tonumber(L, -1);
	lua_pop(L, 2);
	lbs::expect(benchmark_state, x, v);
	lbs::expect(benchmark_state, x, benchmark_state.iterations() * 3);
	lbs::expect(benchmark_state, v, benchmark_state.iterations() * 3);
}

void plain_c_table_chained_get_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(false);
	lua_State* L = lua.get();

	lua_createtable(L, 0, 0);
	lua_createtable(L, 0, 0);
	lua_pushnumber(L, 3);
	lua_setfield(L, -2, "value");
	lua_setfield(L, -2, "warble");
	lua_setglobal(L, "ulahibe");
	double x = 0;
	for (auto _ : benchmark_state) {
		lua_getglobal(L, "ulahibe");
		lua_getfield(L, -1, "warble");
		lua_getfield(L, -1, "value");
		double v = static_cast<double>(lua_tonumber(L, -1));
		x += v;
		lua_pop(L, 3);
	}
	lbs::expect(benchmark_state, x, benchmark_state.iterations() * 3);
}

void plain_c_table_chained_set_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(false);
	lua_State* L = lua.get();

	lua_createtable(L, 0, 0);
	lua_createtable(L, 0, 0);
	lua_pushnumber(L, 3);
	lua_setfield(L, -2, "value");
	lua_setfield(L, -2, "warble");
	lua_setglobal(L, "ulahibe");
	double v = 0;
	for (auto _ : benchmark_state) {
		v += 3;
		lua_getglobal(L, "ulahibe");
		lua_getfield(L, -1, "warble");
		lua_pushnumber(L, v);
		lua_setfield(L, -2, "value");
		lua_pop(L, 2);
	}
	lua_getglobal(L, "ulahibe");
	lua_getfield(L, -1, "warble");
	lua_getfield(L, -1, "value");
	double x = lua_tonumber(L, -1);
	lua_pop(L, 3);
	lbs::expect(benchmark_state, x, v);
	lbs::expect(benchmark_state, x, benchmark_state.iterations() * 3);
	lbs::expect(benchmark_state, v, benchmark_state.iterations() * 3);
}

void plain_c_c_function_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(true);
	lua_State* L = lua.get();

	lua_pushcfunction(L, &lbs::basic_call_wrap);
	lua_setglobal(L, "f");

	lbs::lua_bench_do_or_die(L, lbs::c_function_check);

	std::string code = lbs::repeated_code(lbs::c_function_code);
	int code_index = lbs::lua_bench_load_up(L, code.c_str(), code.size());
	for (auto _ : benchmark_state) {
		lbs::lua_bench_preload_do_or_die(L, code_index);
	}
	lbs::lua_bench_unload(L, code_index);
}

void plain_c_lua_function_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(false);
	lua_State* L = lua.get();

	lbs::lua_bench_do_or_die(L, "function f (i) return i end");

	double x = 0;
	for (auto _ : benchmark_state) {
		lua_getglobal(L, "f");
		lua_pushnumber(L, 3);
		lua_pcallk(L, 1, 1, LUA_NOREF, 0, nullptr);
		double v = static_cast<double>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		x += v;
	}
}

void plain_c_c_through_lua_function_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(true);
	lua_State* L = lua.get();

	lua_pushcfunction(L, &lbs::basic_call_wrap);
	lua_setglobal(L, "f");

	double x = 0;
	for (auto _ : benchmark_state) {
		lua_getglobal(L, "f");
		lua_pushnumber(L, 3);
		lua_pcallk(L, 1, 1, LUA_NOREF, 0, nullptr);
		double v = static_cast<double>(lua_tonumber(L, -1));
		x += v;
		lua_pop(L, 1);
	}
}

void plain_c_member_function_call_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(true);
	lua_State* L = lua.get();

	lbs::basic** s = static_cast<lbs::basic**>(lua_newuserdata(L, sizeof(lbs::basic*)));
	lbs::basic b;
	*s = &b;
	luaL_Reg funcs[] = {
		//{ "__index", &lbs::basic_index_wrap },
		//{ "__newindex", &lbs::basic_newindex_wrap },
		{ "set", &lbs::basic_set_wrap },
		{ "get", &lbs::basic_get_wrap },
		{ nullptr, nullptr }
	};
	luaL_newmetatable(L, "struct_basic");
	lua_createtable(L, 0, 0);
	luaL_setfuncs(L, funcs, 0);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);
	lua_setglobal(L, "b");

	lbs::lua_bench_do_or_die(L, lbs::member_function_call_check);

	std::string code = lbs::repeated_code(lbs::member_function_call_code);
	int code_index = lbs::lua_bench_load_up(L, code.c_str(), code.size());
	for (auto _ : benchmark_state) {
		lbs::lua_bench_preload_do_or_die(L, code_index);
	}
	lbs::lua_bench_unload(L, code_index);
}

void plain_c_userdata_variable_access_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(true);
	lua_State* L = lua.get();

	lbs::basic** s = static_cast<lbs::basic**>(lua_newuserdata(L, sizeof(lbs::basic*)));
	lbs::basic b;
	*s = &b;
	luaL_Reg funcs[] = {
		{ "__index", &lbs::basic_index_wrap },
		{ "__newindex", &lbs::basic_newindex_wrap },
		{ nullptr, nullptr }
	};
	luaL_newmetatable(L, "struct_basic");
	luaL_setfuncs(L, funcs, 0);
	lua_setmetatable(L, -2);
	lua_setglobal(L, "b");

	lbs::lua_bench_do_or_die(L, lbs::userdata_variable_access_check);

	std::string code = lbs::repeated_code(lbs::userdata_variable_access_code);
	int code_index = lbs::lua_bench_load_up(L, code.c_str(), code.size());
	for (auto _ : benchmark_state) {
		lbs::lua_bench_preload_do_or_die(L, code_index);
	}
	lbs::lua_bench_unload(L, code_index);
}

void plain_c_userdata_variable_access_large_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(true);
	lua_State* L = lua.get();

	lbs::basic_large** s = static_cast<lbs::basic_large**>(lua_newuserdata(L, sizeof(lbs::basic_large*)));
	lbs::basic_large b;
	*s = &b;
	luaL_Reg funcs[] = {
		{ "__index", &lbs::basic_large_index_wrap },
		{ "__newindex", &lbs::basic_large_newindex_wrap },
		{ nullptr, nullptr }
	};
	luaL_newmetatable(L, "struct_lbs::basic_large");
	luaL_setfuncs(L, funcs, 0);
	lua_setmetatable(L, -2);
	lua_setglobal(L, "b");

	lbs::lua_bench_do_or_die(L, lbs::userdata_variable_access_large_check);

	std::string code = lbs::repeated_code(lbs::userdata_variable_access_large_code);
	int code_index = lbs::lua_bench_load_up(L, code.c_str(), code.size());
	for (auto _ : benchmark_state) {
		lbs::lua_bench_preload_do_or_die(L, code_index);
	}
	lbs::lua_bench_unload(L, code_index);
}

void plain_c_userdata_variable_access_last_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(true);
	lua_State* L = lua.get();

	lbs::basic_large** s = static_cast<lbs::basic_large**>(lua_newuserdata(L, sizeof(lbs::basic_large*)));
	lbs::basic_large b;
	*s = &b;
	luaL_Reg funcs[] = {
		{ "__index", &lbs::basic_large_index_wrap },
		{ "__newindex", &lbs::basic_large_newindex_wrap },
		{ nullptr, nullptr }
	};
	luaL_newmetatable(L, "struct_lbs::basic_large");
	luaL_setfuncs(L, funcs, 0);
	lua_setmetatable(L, -2);
	lua_setglobal(L, "b");

	lbs::lua_bench_do_or_die(L, lbs::userdata_variable_access_large_last_check);

	std::string code = lbs::repeated_code(lbs::userdata_variable_access_large_last_code);
	int code_index = lbs::lua_bench_load_up(L, code.c_str(), code.size());
	for (auto _ : benchmark_state) {
		lbs::lua_bench_preload_do_or_die(L, code_index);
	}
	lbs::lua_bench_unload(L, code_index);
}

void plain_c_stateful_function_object_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(false);
	lua_State* L = lua.get();

	new (lua_newuserdata(L, sizeof(lbs::basic_stateful))) lbs::basic_stateful();
	lua_createtable(L, 0, 1);
	lua_pushcclosure(L, &lbs::gc_wrap<lbs::basic_stateful>, 0);
	lua_setfield(L, -2, "__gc");
	lua_setmetatable(L, -2);
	lua_pushcclosure(L, &lbs::basic_stateful_wrap, 1);
	lua_setglobal(L, "f");

	double x = 0;
	for (auto _ : benchmark_state) {
		lua_getglobal(L, "f");
		lua_pushnumber(L, 3);
		lua_pcallk(L, 1, 1, LUA_NOREF, 0, nullptr);
		double v = static_cast<double>(lua_tonumber(L, -1));
		x += v;
		lua_pop(L, 1);
	}
	lbs::expect(benchmark_state, x, benchmark_state.iterations() * 3);
}

void plain_c_multi_return_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(false);
	lua_State* L = lua.get();

	lua_pushcclosure(L, &lbs::basic_multi_return_wrap, 0);
	lua_setglobal(L, "f");

	double x = 0;
	for (auto _ : benchmark_state) {
		lua_getglobal(L, "f");
		lua_pushnumber(L, 3);
		lua_pcallk(L, 1, 2, LUA_NOREF, 0, nullptr);
		double v = static_cast<double>(lua_tonumber(L, -1));
		double w = static_cast<double>(lua_tonumber(L, -2));
		x += v;
		x += w;
		lua_pop(L, 2);
	}
	lbs::expect(benchmark_state, x, benchmark_state.iterations() * 9);
}

void plain_c_base_derived_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(false);
	lua_State* L = lua.get();

	lbs::complex_ab* data = static_cast<lbs::complex_ab*>(lua_newuserdata(L, sizeof(lbs::complex_ab)));
	new (data) lbs::complex_ab();
	lua_createtable(L, 0, 1);
	lua_pushcclosure(L, &lbs::gc_wrap<lbs::complex_ab>, 0);
	lua_setfield(L, -2, "__gc");
	lua_setmetatable(L, -2);
	lua_setglobal(L, "b");

	double x = 0;
	for (auto _ : benchmark_state) {
		lua_getglobal(L, "b");
		lbs::complex_base_a& va = *static_cast<lbs::complex_base_a*>(static_cast<lbs::complex_ab*>(lua_touserdata(L, -1)));
		lbs::complex_base_b& vb = *static_cast<lbs::complex_base_b*>(static_cast<lbs::complex_ab*>(lua_touserdata(L, -1)));

		x += va.a_func();
		x += vb.b_func();

		lua_pop(L, 1);
	}
	data->~complex_ab();
	lbs::expect(benchmark_state, x, benchmark_state.iterations() * 6);
}

void plain_c_return_userdata_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(true);
	lua_State* L = lua.get();

	lua_pushcclosure(L, &lbs::basic_return_wrap, 0);
	lua_setglobal(L, "f");
	lua_pushcclosure(L, &lbs::basic_get_wrap, 0);
	lua_setglobal(L, "h");

	lbs::lua_bench_do_or_die(L, lbs::return_userdata_check);

	std::string code = lbs::repeated_code(lbs::return_userdata_code);
	int code_index = lbs::lua_bench_load_up(L, code.c_str(), code.size());
	for (auto _ : benchmark_state) {
		lbs::lua_bench_preload_do_or_die(L, code_index);
	}
	lbs::lua_bench_unload(L, code_index);
}

void plain_c_optional_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(false);
	lua_State* L = lua.get();

	double x = 0;
	for (auto _ : benchmark_state) {
		lua_getglobal(L, "warble");
		if (lua_type(L, -1) != LUA_TNIL) {
			lua_getfield(L, -1, "value");
			if (lua_type(L, -1) != LUA_TNIL) {
				double v = static_cast<double>(lua_tonumber(L, -1));
				x += v;
			}
			lua_pop(L, 2);
			continue;
		}
		lua_pop(L, 1);
		x += 1;
	}
	lbs::expect(benchmark_state, x, benchmark_state.iterations() * 1);
}

void plain_c_implicit_inheritance_measure(benchmark::State& benchmark_state) {
	auto lua = lbs::create_state(true);
	lua_State* L = lua.get();

	lbs::complex_ab** s = static_cast<lbs::complex_ab**>(lua_newuserdata(L, sizeof(lbs::complex_ab*)));
	lbs::complex_ab b;
	*s = &b;
	luaL_Reg funcs[] = {
		{ "__index", &lbs::complex_ab_index_wrap },
		{ nullptr, nullptr }
	};
	luaL_newmetatable(L, "struct_complex_ab");
	luaL_setfuncs(L, funcs, 0);
	lua_setmetatable(L, -2);
	lua_setglobal(L, "b");

	lbs::lua_bench_do_or_die(L, lbs::implicit_inheritance_check);

	std::string code = lbs::repeated_code(lbs::implicit_inheritance_code);
	int code_index = lbs::lua_bench_load_up(L, code.c_str(), code.size());
	for (auto _ : benchmark_state) {
		lbs::lua_bench_preload_do_or_die(L, code_index);
	}
	lbs::lua_bench_unload(L, code_index);
}

BENCHMARK(plain_c_global_string_get_measure);
BENCHMARK(plain_c_global_string_set_measure);
BENCHMARK(plain_c_table_get_measure);
BENCHMARK(plain_c_table_set_measure);
BENCHMARK(plain_c_table_chained_get_measure);
BENCHMARK(plain_c_table_chained_set_measure);
BENCHMARK(plain_c_c_function_measure);
BENCHMARK(plain_c_c_through_lua_function_measure);
BENCHMARK(plain_c_lua_function_measure);
BENCHMARK(plain_c_member_function_call_measure);
BENCHMARK(plain_c_userdata_variable_access_measure);
BENCHMARK(plain_c_userdata_variable_access_large_measure);
BENCHMARK(plain_c_userdata_variable_access_last_measure);
BENCHMARK(plain_c_multi_return_measure);
BENCHMARK(plain_c_stateful_function_object_measure);
BENCHMARK(plain_c_base_derived_measure);
BENCHMARK(plain_c_return_userdata_measure);
BENCHMARK(plain_c_optional_measure);
BENCHMARK(plain_c_implicit_inheritance_measure);