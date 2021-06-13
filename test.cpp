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
	template<typename T>
	constexpr bool is_container(){
		return is_specialization<T, std::array>{} || is_specialization<T, std::vector>{} || is_specialization<T, std::list>{};
	}
	*/
	
	template<typename Tnew>
	void stringify(serialized_members& all_membs, std::size_t& ser_len, std::string_view name, Tnew& value) {
		if constexpr (std::is_arithmetic_v<Tnew>) {
			std::string stringified = std::to_string(value);
			
			ser_len += name.size() + stringified.size();
			all_membs.emplace_back(name, stringified, obj_type::SIMPLE);
		} else if constexpr (std::is_same<Tnew, std::string>::value || std::is_same<Tnew, const char *>::value) {
			using namespace std::string_literals;
			std::string stringified = "\""s; // remove " for non-json serialization
			stringified.append(value);
			stringified += "\""s; // remove " for non-json serialization
			
			ser_len += name.size() + stringified.size();
			ser_len += 2; // TODO make this serialization target dependent
			all_membs.emplace_back(name, stringified, obj_type::SIMPLE);
		} else if constexpr (is_specialization<Tnew, std::vector>{}) { // TODO add array and list
			member_serial ms {name, "", obj_type::CONTAINER};
			ms.sub_members.reserve(value.size());
			
			std::size_t i = 0;
			for(const auto& v : value){
				stringify(ms.sub_members, ser_len, "", v);
				++i;
			}
			all_membs.push_back(ms); // std::move???
			ser_len += name.size() + 2 + value.size(); // TODO make this serialization target dependent
		} else if constexpr (std::is_base_of_v<serializer<Tnew>, Tnew>) {
			// TODO make serializer public?
			std::string stringified = value.serialize(); // Maybe create new method to get raw members
			
			ser_len += name.size() + stringified.size();
			all_membs.emplace_back(name, stringified, obj_type::RECURSIVE);
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
	
	void container_to_json(std::string& result, const serialized_members& all_membs){
		result += "[";
		
		for (const auto& sub_m : all_membs) {
			if(sub_m.type == SIMPLE){
				result += sub_m.value;
			} else if (sub_m.type == CONTAINER) {
				container_to_json(result, sub_m.sub_members);
			} else {
				// recursive
				object_to_json(result, sub_m.sub_members);
			}
			result += ",";
		}
		result = result.substr(0, result.size() - 1);
		
		result += "]";
	}
	
	void object_to_json(std::string& result, const serialized_members& all_membs) {
		result += "{";
		for (const auto& m : all_membs) {
			result += "\"";
			result.append(m.name);
			result += "\":";
			if(m.type == SIMPLE){
				result += m.value;
			} else if (m.type == CONTAINER) {
				container_to_json(result, m.sub_members);
			} else {
				// recursive
				result += m.value;
			}
			result += ",";
		}
		// remove last comma
		result = result.substr(0, result.size() - 1);
		result += "}";
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
		object_to_json(result, all_membs);
		
		return result;
	}

public:

	virtual std::string serialize() = 0;
};

class B : public serializer<B> {

	int d = 5;
	int e = 3;
	
public:
	virtual std::string serialize() override {
		return impl_serialize(
			member{"d", &B::d}, 
			member{"e", &B::e}
		);
	}
};

class A : public serializer<A> {

	int i = 5;
	int g = 3;
	std::string bla = "ggggggg";
	std::vector<int> list{1,2,3,4,5};
	B b;
	
public:
	virtual std::string serialize() override {
		return impl_serialize(
			member{"i", &A::i}, 
			member{"g", &A::g},
			member{"bla", &A::bla},
			member{"list", &A::list},
			member{"b", &A::b}
		);
	}
};

int main () {
	A a;
	
	std::cout << a.serialize() << std::endl;

	return 0;
}
