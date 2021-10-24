#pragma once

#include <iostream>
#include <sstream>
#include <string>

std::string get(std::string);
bool confirm(std::string);

std::string get(std::string question)
{
	std::string response;
	std::string check;

	std::cout << question;
	getline(std::cin, response);

	std::stringstream ss(response);

	if (ss >> check)	// Check if user entered anything
		return response;
	else
		return std::string();
}

bool confirm(std::string question)
{
	std::string response = get(question + " (y/n): ");
	
	while (response.empty())
	{
		response = get("");
	}

	switch (response[0])
	{
		case 'y':
			return true;
		default:
			return false;
	}
}

bool confirm_deletion(std::string name)
{
	return confirm("Are you sure you want to delete \"" + name + "\"?");
}