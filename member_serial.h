#include <string>
#include <string_view>
#include <vector>

#include "type.h"

struct member_serial {
	std::string_view name;
	std::string value;
	obj_type type;
	std::vector<member_serial> sub_members;
};