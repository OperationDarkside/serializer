#include <iostream>

#include "serializer.h"

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