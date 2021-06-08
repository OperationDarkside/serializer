#include <iostream>
#include <string>

template<typename Derv>
class json_base1 {

	template<typename T>
	std::string unroll_Args(T t){
		Derv* derv = static_cast<Derv*>(this);
		return std::to_string((derv->*t));
	}
	
	template<typename T, typename... sub_Args>
	std::string unroll_Args(T t, sub_Args... args){
		return unroll_Args<T>(t) + unroll_Args<sub_Args...>(args...);
	}
	
protected:

	template<typename... sub_Args>
	std::string impl_to_string(sub_Args... args){
		return unroll_Args(args...);
	}
	
	virtual std::string serialize() = 0;
	
public:
	std::string to_string() {
		return serialize();
	}
};

class A : public json_base1<A> {

	int i = 5;
	int g = 3;
	
protected:
	virtual std::string serialize() override {
		return impl_to_string(&A::i, &A::g);
	}
public:

};

int main () {
	A a;
	
	std::cout << a.to_string() << std::endl;

	return 0;
}
