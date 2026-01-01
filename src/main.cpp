#include <logger/logger.hpp>

int main(){
	Logger::setLevel(Logger::LogLevel::WARNING);
	Logger::fatal("Hello There");
}
