#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

class printable_t
{
	public:
	virtual void print(std::ostream&, std::string) = 0;
};

void set_print(std::ostream& output)
{
	output << std::setw(50) << std::setfill(' ') << std::left;
}

void print_list(std::vector<printable_t*> list, std::string key)
{
	for (unsigned int i = 0; i < list.size(); i++)
	{
		set_print(std::cout);
		std::cout << (i + 1);
		list.at(i)->print(std::cout, key);
	}
}