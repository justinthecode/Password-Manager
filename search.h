#pragma once

#include <string>
#include <cctype>
#include <algorithm>

/*
	Return whether the query is in the specified string
*/
bool contains(std::string in, std::string query)
{
	return in.find(query) != std::string::npos;
}

/*
	Return whether the lowercase query is in the lowercase specified string
*/
bool lowercase_contains(std::string in, std::string query)
{
	std::transform(in.begin(), in.end(), in.begin(),
		[](unsigned char c) { return std::tolower(c); });	// Convert in to lowercase

	std::transform(query.begin(), query.end(), query.begin(),
		[](unsigned char c) { return std::tolower(c); });	// Convert query to lowercase

	return in.find(query) != std::string::npos;
}