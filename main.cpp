#include <fmt/core.h>
#include <memory>
#include <vector>
#include "compiled_with.h"
#include "git_hash.h"

int main() {
	fmt::println("Hello, World!");
	fmt::println("Compiled With: {}", COMPILED_WITH);
	fmt::println("Git: {} {}", GIT_REV, GIT_BRANCH);
	return 0;
}
