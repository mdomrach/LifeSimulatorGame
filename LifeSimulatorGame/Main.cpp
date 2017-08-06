#include <iostream>

#include "GameManager.h"

int main() 
{
	FGameManager gameManager;

	try 
	{
		gameManager.Run();
	}
	catch (const std::runtime_error& e) 
	{
		std::cerr << e.what() << std::endl;

#ifndef NDEBUG
		int n;
		std::cin >> n;
#endif 

		return EXIT_FAILURE;
	}

//#ifndef NDEBUG
//	int m;
//	std::cin >> m;
//#endif 

	return EXIT_SUCCESS;
}