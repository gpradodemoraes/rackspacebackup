#include <fmt/core.h>
#include "compiled_with.h"
#include "git_hash.h"
extern const char *build_date;

int main() {
	fmt::println("Hello, World!");
	fmt::println("Compiled With: {}", COMPILED_WITH);
	fmt::println("Git: {} {}", GIT_REV, GIT_BRANCH);
	fmt::println("Build Time: {}", build_date);
	return 0;
}
