#include <string_view>

template<typename T>
struct member {
	const char* name;
	T value;
};