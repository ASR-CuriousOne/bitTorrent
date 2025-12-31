#include <iostream>
#include <format>
#include <string>

int main(){
	std::string hello = "World";

	std::cout << std::format("Hello {}",hello) << std::endl;
}
