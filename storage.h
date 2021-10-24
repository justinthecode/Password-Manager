#pragma once

#include <string>
#include <fstream>

#define UNIT_SEPARATOR 31
#define GROUP_SEPARATOR 29
#define RECORD_SEPARATOR 30

namespace storage
{
	void read(unsigned int&, std::ifstream&);
	bool read(char&, std::ifstream&);
	void read(int&, std::ifstream&);
	void read(std::string&, std::ifstream&);
	void read(uint8_t[], std::ifstream&, size_t&);
	void read(uint8_t[], std::ifstream&);

	void store_us(const char, std::ofstream&);
	void store_gs(const char, std::ofstream&);
	void store_rs(std::ofstream&);

	void store(int, std::ofstream&);
	void store(const char, int, std::ofstream&);
	void store(const char, std::string, std::ofstream&);
	void store(const char, uint8_t[], std::ofstream&, unsigned int);

	bool read_unit(char&, std::ifstream&);
	bool read_group(char&, std::ifstream&);

	bool is_eof(std::ifstream&);
	bool is_eor(std::ifstream&);
	bool is_eog(std::ifstream&);
	void consume_rs(std::ifstream&);

	/*
		Read an unsigned int. Generally used to read in length values.
	*/
	void read(unsigned int& n, std::ifstream& input)
	{
		input.read(reinterpret_cast<char*>(&n), sizeof(int));
	}

	/*
		Read a char. Generally used to read in group and unit codes.
	*/
	bool read(char& c, std::ifstream& input)
	{
		return !(input.read(reinterpret_cast<char*>(&c), sizeof(char))).eof();
	}

	/*
		Read an int
	*/
	void read(int& n, std::ifstream& input)
	{
		input.read(reinterpret_cast<char*>(&n), sizeof(int));
	}

	/*
		Read a string stored using the store() method
	*/
	void read(std::string& out, std::ifstream& input)
	{
		out = "";
		unsigned int n;
		char temp_char;

		read(n, input);
		for (unsigned int i = 0; i < n; i++)
		{
			input.read(reinterpret_cast<char*>(&temp_char), sizeof(char));
			out += temp_char;
		}
	}

	/*
		Read an array of uint8_t's and save the length of the array
	*/
	void read(uint8_t out[], std::ifstream& input, size_t& length)
	{
		unsigned int n;

		read(n, input);
		length = n;
		for (unsigned int i = 0; i < n; i++)
			input.read(reinterpret_cast<char*>(&out[i]), sizeof(uint8_t));
	}

	/*
		Read an array of uint8_t's
	*/
	void read(uint8_t out[], std::ifstream& input)
	{
		unsigned int n;

		read(n, input);
		for (unsigned int i = 0; i < n; i++)
			input.read(reinterpret_cast<char*>(&out[i]), sizeof(uint8_t));
	}

	/*
		Denote start of unit
	*/
	void store_us(const char code, std::ofstream& output)
	{
		char separator_code = UNIT_SEPARATOR;
		output.write((char*)&separator_code, sizeof(separator_code));
		output.write((char*)&code, sizeof(code));
	}

	/*
		Denote start of group
	*/
	void store_gs(const char code, std::ofstream& output)
	{
		char separator_code = GROUP_SEPARATOR;
		output.write((char*)&separator_code, sizeof(separator_code));
		output.write((char*)&code, sizeof(code));
	}

	/*
		Denote end of record
	*/
	void store_rs(std::ofstream& output)
	{
		char separator_code = RECORD_SEPARATOR;
		output.write((char*)&separator_code, sizeof(separator_code));
	}

	/*
		Store an int
	*/
	void store(int n, std::ofstream& output)
	{
		output.write((char*)&n, sizeof(n));
	}

	/*
		Store an int with a unit code
	*/
	void store(const char code, int n, std::ofstream& output)
	{
		store_us(code, output);
		output.write((char*)&n, sizeof(n));
	}

	/*
		Store a string with a unit code
	*/
	void store(const char code, std::string string, std::ofstream& output)
	{
		store_us(code, output);
		store(string.length(), output);
		for (unsigned int i = 0; i < string.length(); i++)
			output.write((char*)&string[i], sizeof(string[i]));
	}

	/*
		Store an array of uint8_t's with a unit code
	*/
	void store(const char code, uint8_t block[], std::ofstream& output, unsigned int n)
	{
		store_us(code, output);
		store(n, output);
		for (unsigned int i = 0; i < n; i++)
			output.write((char*)&block[i], sizeof(block[i]));
	}

	/*
		Read a unit code
	*/
	bool read_unit(char& code, std::ifstream& input)
	{
		char separator_check;

		if (input.peek() == UNIT_SEPARATOR)	// If this is the start of a group
		{
			read(separator_check, input);
			read(code, input);
			return true;
		}
		else
			return false;
	}

	/*
		Read a group code
	*/
	bool read_group(char& code, std::ifstream& input)
	{
		char separator_check;

		if (input.peek() == GROUP_SEPARATOR && read(separator_check, input))	// If this is the start of a group and not the end of the file
		{
			read(code, input);
			return true;
		}
		else
			return false;
	}

	/*
		Check if input is at the end of the file
	*/
	bool is_eof(std::ifstream& input)
	{
		return input.peek() == EOF;
	}

	/*
		Check if input is at the end of a record
	*/
	bool is_eor(std::ifstream& input)
	{
		return input.peek() == RECORD_SEPARATOR || is_eof(input);
	}

	/*
		Check if input is at the end of a group
	*/
	bool is_eog(std::ifstream& input)
	{
		return input.peek() == GROUP_SEPARATOR || is_eor(input);
	}

	/*
		Confirm end of record
	*/
	void consume_rs(std::ifstream& input)
	{
		if (!input.eof())
		{
			char separator;
			read(separator, input);
		}
	}
}