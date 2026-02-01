#include <functional>

#include <fmt/printf.h>
#include <fmt/color.h>

using test_impl = std::function <void ()>;

struct test {
	std::string name;
	test_impl impl;
};

struct suite {
	bool pass;
	std::vector <test> tests;
} g_suite;

int main(int argc, char *argv[])
{
	bool fail = false;
	for (auto &test : g_suite.tests) {
		g_suite.pass = true;

		test.impl();

		if (g_suite.pass) {
			fmt::print(
				fmt::emphasis::bold
				| fmt::fg(fmt::color::light_green),
				"passed: ", test.name
			);
		} else {
			fmt::print(
				fmt::emphasis::bold
				| fmt::fg(fmt::color::orange_red),
				"failed: "
			);

			fail = true;
		}

		fmt::println("{}", test.name);
	}

	return fail ? EXIT_FAILURE : EXIT_SUCCESS;
}

#define mark_fail g_suite.pass = false

auto operator*(const std::string &name, const test_impl &impl)
{
	return test(name, impl);
}

auto operator<<(suite &s, const test &item)
{
	s.tests.push_back(item);
	return item;
}

#define add_test(name) auto name = g_suite << #name * []
