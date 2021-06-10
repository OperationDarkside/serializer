#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <list>
#include <type_traits>

enum obj_type {
	SIMPLE,
	CONTAINER,
	RECURSIVE
};

template<typename T>
struct member {
	std::string_view name;
	T value;
};

struct member_serial {
	std::string_view name;
	std::string value;
	obj_type type;
	std::vector<member_serial> sub_members;
};


// https://stackoverflow.com/questions/31762958/check-if-class-is-a-template-specialization
template <class T, template <class...> class Template>
struct is_specialization : std::false_type {};
template <template <class...> class Template, class... Args>
struct is_specialization<Template<Args...>, Template> : std::true_type {};

template<typename Derv>
class serializer {

	using serialized_members = std::vector<member_serial>;
	
	/*
	template<typename T, typename I, typename F>
	constexpr bool is_container(){
		return std::is_same<T, std::array<I, F>>::value || std::is_same<T, std::vector<I>>::value || std::is_same<T, std::list<I>>::value;
	}
	*/
	
	template<typename Tnew>
	void stringify(serialized_members& all_membs, std::size_t& ser_len, std::string_view name, const Tnew& value) {
		if constexpr (std::is_arithmetic_v<Tnew>) {
			std::string stringified = std::to_string(value);
			
			ser_len += name.size() + stringified.size();
			ser_len += 2; // TODO make this serialization target dependent
			all_membs.emplace_back(name, stringified, obj_type::SIMPLE);
		} else if constexpr (std::is_same<Tnew, std::string>::value || std::is_same<Tnew, const char *>::value) {
			using namespace std::string_literals;
			std::string stringified = "\""s; // remove " for non-json serialization
			stringified.append(value);
			stringified += "\""s; // remove " for non-json serialization
			
			ser_len += name.size() + stringified.size();
			ser_len += 2; // TODO make this serialization target dependent
			all_membs.emplace_back(name, stringified, obj_type::SIMPLE);
		} else if constexpr (is_specialization<Tnew, std::vector>{}) {
			member_serial ms {name, "", obj_type::CONTAINER};
			ms.sub_members.reserve(value.size());
			
			std::size_t i = 0;
			for(const auto& v : value){
				stringify(ms.sub_members, ser_len, "", v);
				++i;
			}
			all_membs.push_back(ms); // std::move???
			ser_len += 2 + (value.size() * 1); // TODO make this serialization target dependent
		} else if constexpr (std::is_base_of_v<serializer, Tnew>) {
			
			std::string stringified = "TODO";
		} else {
			std::string stringified = "???";
		}
	}

	template<typename T>
	void unroll_Args(serialized_members& all_membs, std::size_t& ser_len, const member<T>& mem){
		Derv* derv = static_cast<Derv*>(this);
		using Tnew = std::remove_reference_t<decltype((derv->*(mem.value)))>;
		
		stringify<Tnew>(all_membs, ser_len, mem.name, (derv->*(mem.value)));	
	}
	
	template<typename T, typename... sub_Args>
	void unroll_Args(serialized_members& all_membs, std::size_t& ser_len, const member<T>& pair, const member<sub_Args>&... args){
		unroll_Args(all_membs, ser_len, pair);
		unroll_Args(all_membs, ser_len, args...);
	}
	
protected:

	template<typename... sub_Args>
	std::string impl_serialize(const member<sub_Args>&... args){
		// prepare list of serialized members
		serialized_members all_membs;
		all_membs.reserve(sizeof...(sub_Args));
		
		std::size_t acc_len = 0;
		unroll_Args(all_membs, acc_len, args...);
		
		// TODO send to specific serializers
		using namespace std::string_literals;
		acc_len += 2 /*begin&end closing bracket*/ + (all_membs.size() * (2 /*QuotMark of name*/ + 1 /*colon*/ + 1 /*comma*/));
		std::string result;
		result.reserve(acc_len);
		result += "{";
		for (const auto& m : all_membs) {
			result += "\""s;
			result.append(m.name);
			result += "\":"s;
			if(m.type == SIMPLE){
				result += m.value;
			} else if (m.type == CONTAINER) {
				
				result += "["s;
				
				for (const auto& sub_m : m.sub_members) {
					// TODO recursive
					result += sub_m.value + ","s;
				}
				result = result.substr(0, result.size() - 1);
				
				result += "]"s;
			}
			result += ","s;
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
	std::string bla = "ggggggg";
	std::vector<int> list{1,2,3,4,5};
	
protected:
	virtual std::string serialize() override {
		return impl_serialize(
			member{"i", &A::i}, 
			member{"g", &A::g},
			member{"bla", &A::bla},
			member{"list", &A::list}
		);
	}
public:

};

int main () {
	A a;
	
	std::cout << a.to_string() << std::endl;

	return 0;
}
