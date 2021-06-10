#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

template<typename T>
struct member {
	std::string_view name;
	T value;
};

struct member_serial {
	std::string_view name;
	std::string value;
};

template<typename Derv>
class serializer {

	using serialized_members = std::vector<member_serial>;

	template<typename T>
	void unroll_Args(serialized_members& all_membs, std::size_t& ser_len, const member<T>& mem){
		using namespace std::string_literals;
		Derv* derv = static_cast<Derv*>(this);
		using Tnew = std::remove_reference_t<decltype((derv->*(mem.value)))>;
		
		std::string stringified;
		if constexpr (std::is_arithmetic_v<Tnew>) {
			stringified = std::to_string((derv->*(mem.value)));
		} else if constexpr (std::is_same<Tnew, std::string>::value || std::is_same<Tnew, const char *>::value) {
			stringified = "\""s; // remove " for non-json serialization
			stringified.append((derv->*(mem.value)));
			stringified += "\""s; // remove " for non-json serialization
			ser_len += 2; // TODO make this serialization target dependent
		} else {
			stringified = "???";
		}
		
		ser_len += mem.name.size() + stringified.size();
		
		all_membs.push_back({mem.name, stringified});
	}
	
	template<typename T, typename... sub_Args>
	void unroll_Args(serialized_members& all_membs, std::size_t& ser_len, const member<T>& pair, const member<sub_Args>&... args){
		unroll_Args(all_membs, ser_len, pair);
		unroll_Args(all_membs, ser_len, args...);
	}
	
protected:

	template<typename... sub_Args>
	std::string impl_serialize(const member<sub_Args>&... args){
		using namespace std::string_literals;
		serialized_members all_membs;
		all_membs.reserve(sizeof...(sub_Args));
		
		std::size_t acc_len = 0;
		unroll_Args(all_membs, acc_len, args...);
		
		// TODO send to specific serializers
		acc_len += 2 /*begin&end closing bracket*/ + (all_membs.size() * (2 /*QuotMark of name*/ + 1 /*colon*/ + 1 /*comma*/));
		std::string result;
		result.reserve(acc_len);
		result += "{";
		for (const auto& m : all_membs) {
			result += "\""s;
			result.append(m.name);
			result += "\":"s + m.value + ","s;
		}
		// remove last comma
		result = result.substr(0, result.size() - 1);
		result += "}";
		
		return result;
	}
	
	virtual std::string serialize() = 0;
	
public:
	std::string to_string() {
		return serialize();
	}
};

class A : public serializer<A> {

	int i = 5;
	int g = 3;
	
protected:
	virtual std::string serialize() override {
		return impl_serialize(member{"i", &A::i}, member{"g", &A::g});
	}
public:

};

int main () {
	A a;
	
	std::cout << a.to_string() << std::endl;

	return 0;
}
