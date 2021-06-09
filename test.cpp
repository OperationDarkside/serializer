#include <iostream>
#include <string>

template<typename T>
struct ser_pair {
	const char* name;
	T value;
};


template<typename Derv>
class serializer {

	template<typename T>
	std::string unroll_Args(const ser_pair<T>& pair){
		using namespace std::string_literals;
		Derv* derv = static_cast<Derv*>(this);
		return "{"s + pair.name + ":"s + std::to_string((derv->*(pair.value))) + "}"s;
	}
	
	template<typename T, typename... sub_Args>
	std::string unroll_Args(const ser_pair<T>& pair, const ser_pair<sub_Args>&... args){
		return unroll_Args(pair) + unroll_Args(args...);
	}
	
protected:

	template<typename... sub_Args>
	std::string impl_serialize(const ser_pair<sub_Args>&... args){
		return unroll_Args(args...);
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
		return impl_serialize(ser_pair{"i", &A::i}, ser_pair{"g", &A::g});
	}
public:

};

int main () {
	A a;
	
	std::cout << a.to_string() << std::endl;

	return 0;
}
