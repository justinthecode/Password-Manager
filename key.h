#pragma once

#include <algorithm>
#include "crypt.h"

/*
	A key stored as its hash value
*/
class key_t
{
	enum unit_code
	{
		KEY = 'K',
		SALT = 'S'
	};

	int key;
	std::string salt;	// Salt to add to the key before hashing to deflect rainbow-table attacks

	void generate_salt();
	int get_hash(std::string);

	public:
	key_t() {}
	key_t(std::string);
	key_t(std::ifstream&);

	bool equals(std::string);
	void store(std::ofstream&);
};

/*
	A handler of keys, connected to a keystore file
*/
class keystore_t
{
	enum group_code
	{
		KEY = 'K',
		STATIC_KEY = 'S'
	};

	key_t key;	// Master key
	secret_t static_key;	// Key used to encrypt secrets

	std::string filename;
	bool file_exists;

	void read();
	std::string generate_static_key();

	public:
	keystore_t(std::string);

	bool is_loaded();
	bool login(std::string);
	void set_key(std::string, std::string);
	std::string get_static_key(std::string);

	void store();
};

void key_t::generate_salt()
{
	salt = "";

	for (int i = 0; i < 8; i++)
	{
		salt += rand() % 32 + 'A';
	}
}

int key_t::get_hash(std::string key)
{
	return std::hash<std::string>{}(key + salt);
}

/*
	Create a new key
*/
key_t::key_t(std::string key)
{
	generate_salt();
	this->key = get_hash(key);
}

/*
	Load an existing key
*/
key_t::key_t(std::ifstream& input)
{
	if (input.is_open())
	{
		char unit_code;
		while (!storage::is_eor(input) && storage::read_unit(unit_code, input))	// Read next unit code
		{
			switch (unit_code)
			{
				case KEY:
					storage::read(key, input);
					break;
				case SALT:
					storage::read(salt, input);
			}
		}

		storage::consume_rs(input);
	}
}

/*
	Return whether a string equals the key the stored hash represents
*/
bool key_t::equals(std::string attempt)
{
	return get_hash(attempt) == key;
}

void key_t::store(std::ofstream& output)
{
	if (output.is_open())
	{
		storage::store(KEY, key, output);
		storage::store(SALT, salt, output);

		storage::store_rs(output);
	}
}

/*
	Read in keys from the file on record
*/
void keystore_t::read()
{
	std::ifstream file(filename);
	file_exists = file.is_open();

	if (file.is_open())
	{
		char group_code;
		while (storage::read_group(group_code, file))
		{
			switch (group_code)
			{
				case KEY:
					key = key_t(file);
					break;
				case STATIC_KEY:
					static_key = secret_t(file);
			}
		}

		file.close();
	}
}

std::string keystore_t::generate_static_key()
{
	std::string out = "";

	for (int i = 0; i < 32; i++)
	{
		out += rand() % 92 + '!';
	}

	return out;
}

keystore_t::keystore_t(std::string filename)
{
	this->filename = filename;
	read();
}

bool keystore_t::is_loaded()
{
	return file_exists;	// By the time this function is run, this criterion will be synonymous with the keystore being initialized
}

/*
	Verify key or use it to generate keystore file
*/
bool keystore_t::login(std::string key)
{
	if (file_exists)
	{
		return this->key.equals(key);
	}
	else	// If there is no keystore file, perform first-time setup
	{
		this->key = key_t(key);
		std::string static_key = generate_static_key();
		this->static_key = secret_t(static_key, key);
		store();

		return true;
	}
}

/*
	Update master key and encrypt static key with it
*/
void keystore_t::set_key(std::string new_key, std::string key)
{
	this->key = key_t(new_key);
	static_key.set_key(new_key, key);

	store();
}

std::string keystore_t::get_static_key(std::string key)
{
	return this->key.equals(key) ? static_key.get_data(key) : "";
}

/*
	Store keys in the file on record
*/
void keystore_t::store()
{
	std::ofstream file(filename, std::ios::trunc | std::ios::binary);

	if (file.is_open())
	{
		storage::store_gs(KEY, file);
		this->key.store(file);

		storage::store_gs(STATIC_KEY, file);
		this->static_key.store(file);

		file.close();
	}
}