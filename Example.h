#pragma once
#include <string>
#include <iostream>
class Example {
public:
	Example() = default;
	~Example() = default;
	void sayHello() const {
		std::cout << "Hello from Example class!" << std::endl;
	}
	std::string getGreeting() const {
		return "Greetings from Example class!";
	}
};