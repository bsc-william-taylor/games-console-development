
#include <iostream>
#include <altivec.h>

typedef union {
	vector char v;
	char e[16];
} char_type;

template<typename T, typename V>
class simd_vector {
	T contents;
	int insert;
	int size;
public:
	simd_vector() {
		size = sizeof(contents.e) / sizeof(V);
		insert = -1;
	}

	void add(V value) {
		contents.e[++insert] = value;
	}

	V get(int index) {
		return contents.e[index];
	}

	int length() { return size; };
};

int main(int argc, char * argv[])
{
	simd_vector<char_type, char> elements;
	elements.add('w');
	elements.add('t');
	
	std::cout << elements.get(0) << elements.get(1) << std::endl;
	std::cout << elements.length() << std::endl;
	return 0;
}
