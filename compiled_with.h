#include <string>

#if defined(_MSC_VER)
const std::string COMPILED_WITH = "MSVC " + std::to_string(_MSC_VER);
#elif defined(__clang__)
const std::string COMPILED_WITH = "Clang." + std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__) +
								  "." + std::to_string(__clang_patchlevel__);
#elif defined(__GNUC__)
const std::string COMPILED_WITH = "GCC." + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__) + "." +
								  std::to_string(__GNUC_PATCHLEVEL__);
#elif defined(__INTEL_COMPILER)
const std::string COMPILED_WITH = "Intel C++." + std::to_string(__INTEL_COMPILER);
#else
const std::string COMPILED_WITH = "Unknown compiler";
#endif
