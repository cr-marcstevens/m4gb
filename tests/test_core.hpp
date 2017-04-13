#include <iostream>

#undef CHECK
#define CHECK(s) if (!(s)) { std::cerr << "Check FAILED in " __FILE__ " at line " << __LINE__ << "!" << std::endl; return 1; }

int test();

int main(int argc, char** argv)
{
	return test();
}
