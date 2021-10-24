#pragma once

#include "Salsa20.h"
#include "storage.h"
#include "print.h"

#define IV_LENGTH 8
#define MAX_BLOCK_LENGTH 64
#define MAX_KEY_LENGTH 32

void to_block(std::string, uint8_t[], size_t);
std::string to_string(uint8_t[], size_t);
void crypt(uint8_t[], uint8_t[], size_t, uint8_t[], uint8_t[]);
void encrypt(uint8_t[], uint8_t[], std::string, uint8_t[], size_t);
void decrypt(uint8_t[], uint8_t[], std::string, uint8_t[], size_t);
std::string encrypt(std::string, std::string, uint8_t[]);
std::string decrypt(std::string, std::string, uint8_t[]);

/*
	A string stored as encrypted text
*/
class secret_t
{
	protected:
	enum unit_code
	{
		DATA = 'D',
		IV = 'V'
	};

	uint8_t data[MAX_BLOCK_LENGTH];
	size_t data_length;
	uint8_t iv[IV_LENGTH];	// Initialization vector

	public:
	secret_t() {}
	secret_t(std::ifstream&);
	secret_t(std::string, std::string);

	void set_data(std::string, std::string);
	std::string get_data(std::string);
	void set_key(std::string, std::string);

	void print(std::ostream&, std::string, bool = true);
	void store(std::ofstream&);
};

/*
	Convert string to array of uint8_t's
*/
void to_block(std::string in, uint8_t out[], size_t n)
{
	unsigned int i;
	for (i = 0; i < in.length() && i < n; i++)
	{
		out[i] = (uint8_t)in[i];
	}
	for (; i < n; i++)
	{
		out[i] = '0';
	}
}

/*
	Convert array of uint8_t's to string
*/
std::string to_string(uint8_t in[], size_t n)
{
	std::string out = "";

	for (unsigned int i = 0; in[i] != '\0' && i < n; i++)
	{
		out += (char)in[i];
	}

	return out;
}

/*
	Perform the Salsa20 block cypher algorithm
*/
void crypt(uint8_t in[], uint8_t out[], size_t n, uint8_t key[MAX_KEY_LENGTH], uint8_t iv[IV_LENGTH])
{
	ucstk::Salsa20 salsa20(key);
	salsa20.setIv(iv);

	salsa20.processBytes(in, out, n);
	/*
	if (n >= 64)
	{
		salsa20.processBlocks(in, out, n / 64);
	}
	if (n % 64)
	{
		salsa20.processBytes(in + ((n / 64) * 64), out + ((n / 64) * 64), n % 64);
	}
	*/
}

void encrypt(uint8_t in[], uint8_t out[], std::string key, uint8_t iv[IV_LENGTH], size_t n)
{
	for (unsigned int i = 0; i < IV_LENGTH; i++)	// Generate initialization vector
	{
		iv[i] = rand() % (2 ^ 8);	// Pick any number that can be expressed in 8 bits
	}

	uint8_t block_key[MAX_KEY_LENGTH];
	to_block(key, block_key, MAX_KEY_LENGTH);
	crypt(in, out, n, block_key, iv);
}

void decrypt(uint8_t in[], uint8_t out[], std::string key, uint8_t iv[8], size_t n)
{
	uint8_t block_key[MAX_KEY_LENGTH];
	to_block(key, block_key, MAX_KEY_LENGTH);
	crypt(in, out, n, block_key, iv);
}

std::string encrypt(std::string in, std::string key, uint8_t iv[8])
{
	for (unsigned int i = 0; i < IV_LENGTH; i++)	// Generate initialization vector
	{
		iv[i] = rand() % (2 ^ 8);	// Pick any number that can be expressed in 8 bits
	}
	
	uint8_t block_key[32];
	uint8_t block_in[64];

	to_block(key, block_key, 32);
	to_block(in, block_in, 64);

	uint8_t* result = (uint8_t*)malloc(in.length());	// Allocate space to store result
	crypt(block_in, result, in.length(), block_key, iv);
	std::string out = to_string(result, in.length());
	free(result);

	return out;
}

std::string decrypt(std::string in, std::string key, uint8_t iv[8])
{
	uint8_t block_key[32];
	uint8_t block_in[64];

	to_block(key, block_key, 32);
	to_block(in, block_in, 64);

	uint8_t* result = (uint8_t*)malloc(in.length());	// Allocate space to store result
	crypt(block_in, result, in.length(), block_key, iv);
	std::string out = to_string(result, in.length());
	free(result);

	return out;
}

secret_t::secret_t(std::ifstream& input)
{
	if (input.is_open())
	{
		char unit_code;
		while (!storage::is_eor(input) && storage::read_unit(unit_code, input))	// Read next unit code
		{
			switch (unit_code)
			{
			case DATA:
				storage::read(data, input, data_length);
				break;
			case IV:
				storage::read(iv, input);
			}
		}

		storage::consume_rs(input);
	}
}

secret_t::secret_t(std::string data, std::string key)
{
	set_data(data, key);
}

void secret_t::set_data(std::string data, std::string key)
{
	data_length = data.length();

	uint8_t block_password[64];
	to_block(data, block_password, data_length);
	encrypt(block_password, this->data, key, iv, data_length);	// Insert encrypted data into data member
}

std::string secret_t::get_data(std::string key)
{
	uint8_t block_out[64];
	decrypt(data, block_out, key, iv, data_length);

	return to_string(block_out, data_length);
}

/*
	Re-encrypt data with a new key
*/
void secret_t::set_key(std::string new_key, std::string key)
{
	set_data(get_data(key), new_key);
}

void secret_t::print(std::ostream& output, std::string key, bool newline)
{
	set_print(output);
	output << get_data(key);
	if (newline)
		output << std::endl;
}

void secret_t::store(std::ofstream& output)
{
	if (output.is_open())
	{
		storage::store(DATA, data, output, data_length);
		storage::store(IV, iv, output, 8);

		storage::store_rs(output);
	}
}