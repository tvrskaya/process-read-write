#include <iostream>

int main(void)
{
	int data = 100;
	std::cout << &data << std::endl;

	std::cin.get();

	while (true)
	{
		std::cout << data << std::endl;
	}

	return 0;
}