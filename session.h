#pragma once

#include <stdlib.h>
#include <time.h>
#include "key.h"
#include "seclevel.h"
#include "credentials.h"

/*
	An object to streamline user interaction with credentials
*/
class session_t
{
	enum group_code
	{
		KEY = 'K',
		CREDENTIALS = 'C'
	};

	std::string key;	// Program key
	std::string crypt_key;	// Encryption key
	bool logged_in = false;
	std::vector<credentials_t*> credentials_list;
	seclevel_manager_t* seclevel_manager;

	std::string keystore_filename;

	public:
	session_t(std::string, std::string, std::string);
	session_t(std::string, std::string, std::string, std::string);
	~session_t();

	bool login(std::string);
	void logout();

	void add_credentials(std::string, std::string, std::string, seclevel_t* = nullptr);
	void add_credentials(std::string, std::string, seclevel_t*);
	void add_credentials(std::string, std::string, std::string, std::initializer_list<std::pair<std::string, std::string>>, std::initializer_list<std::string>);
	bool modify_credentials(std::string, std::string, std::string);
	int set_security_level(std::string, std::vector<std::string>);
	bool set_security_level(std::string, seclevel_t*);
	bool add_questions(std::string, std::vector<std::pair<std::string, std::string>>);
	int delete_questions(std::string, std::vector<std::string>);
	bool delete_question(std::string, int);
	bool add_backups(std::string, std::vector<std::string>);
	int delete_backups(std::string, std::vector<std::string>);
	bool delete_backup(std::string, int);
	bool delete_credentials(std::string);

	void set_key(std::string);

	void add_seclevel(std::string, std::string, int, int, int, int);
	void add_seclevel(std::string, int, int, int, int);
	bool delete_seclevel(std::string);
	bool set_seclevel_password(std::string, std::string);
	bool clear_seclevel_password(std::string);
	std::vector<credentials_t*> get_old_passwords();
	std::vector<seclevel_t*> get_exp_passwords();
	bool update_password(credentials_t*);
	void update_seclevel(seclevel_t*, std::string);

	std::vector<credentials_t*>::iterator find_credentials(std::string);
	bool is_end(std::vector<credentials_t*>::iterator);
	std::string get_crypt_key();
	seclevel_t* find_seclevel(std::string);
	bool is_logged_in();
	bool are_credentials_loaded();
	
	void print_credentials();
	void print_questions(std::string);
	void print_backups(std::string);
	void print_seclevels();
	void search_credentials(std::string);

	bool read(std::string);
	void store(std::string);
	void store_credentials(std::string);
	void store_seclevels();
	void unload();
};

/*
	Create session without reading credentials and log in
*/
session_t::session_t(std::string key, std::string keystore_filename, std::string seclevel_filename)
{
	srand(static_cast<unsigned int>(std::time(nullptr)));

	this->keystore_filename = keystore_filename;
	login(key);
	seclevel_manager = new seclevel_manager_t(seclevel_filename);
}

/*
	Create session and log in
*/
session_t::session_t(std::string key, std::string credentials_filename, std::string keystore_filename, std::string seclevel_filename)
{
	srand(static_cast<unsigned int>(std::time(nullptr)));

	this->keystore_filename = keystore_filename;
	login(key);
	seclevel_manager = new seclevel_manager_t(seclevel_filename);
	read(credentials_filename);
}

session_t::~session_t()
{
	unload();
	delete seclevel_manager;
}

/*
	Log in using the provided key
*/
bool session_t::login(std::string key)
{
	keystore_t keystore = keystore_t(keystore_filename);

	logged_in = keystore.login(key);

	if (logged_in)
	{
		this->key = key;
		crypt_key = keystore.get_static_key(key);
	}

	return logged_in;
}

/*
	Reset key information
*/
void session_t::logout()
{
	key = crypt_key = "";
	logged_in = false;
}

void session_t::add_credentials(std::string name, std::string username, std::string password, seclevel_t* seclevel)
{
	if (logged_in)
	{
		std::vector<credentials_t*>::iterator it = find_credentials(name);

		if (!is_end(it))
		{
			(*it)->set_username(username);
			(*it)->set_password(password, crypt_key);
			(*it)->set_security_level(seclevel->get_code(), crypt_key);
		}
		else
		{
			if (seclevel)
				credentials_list.push_back(new credentials_t(name, username, password, seclevel->get_code(), crypt_key));
			else
				credentials_list.push_back(new credentials_t(name, username, password, crypt_key));
		}
	}
}

void session_t::add_credentials(std::string name, std::string username, seclevel_t* seclevel)
{
	add_credentials(name, username, seclevel->get_password(crypt_key), seclevel);
}

/*
	Add credentials with programmatic lists of secret questions and backup codes. The purpose of this function is to give more control to developers in testing.
*/
void session_t::add_credentials(std::string name, std::string username, std::string password, std::initializer_list<std::pair<std::string, std::string>> secret_questions, std::initializer_list<std::string> backup_codes)
{
	if (logged_in)
	{
		std::vector<credentials_t*>::iterator it = find_credentials(name);

		if (!is_end(it))
		{
			(*it)->set_username(username);
			(*it)->set_password(password, crypt_key);
			(*it)->add_questions(secret_questions, crypt_key);
		}
		else
		{
			credentials_list.push_back(new credentials_t(name, username, password, crypt_key, secret_questions, backup_codes));
		}
	}
}

bool session_t::modify_credentials(std::string name, std::string field, std::string value)
{
	if (logged_in)
	{
		std::vector<credentials_t*>::iterator it = find_credentials(name);

		if (!is_end(it))
		{
			switch (std::tolower(field[0]))
			{
				case 'n':	// Site name
					(*it)->set_name(value);
					return true;
				case 'u':	// Username
					(*it)->set_username(value);
					return true;
				case 'p':	// Password
					(*it)->set_password(value, crypt_key);
					return true;
				case 'l':	// Security level
					(*it)->set_security_level(value, crypt_key);
					return true;
			}
		}
	}

	return false;
}

int session_t::set_security_level(std::string security_level, std::vector<std::string> names)
{
	int out = 0;

	std::vector<credentials_t*>::iterator it;
	for (unsigned int i = 0; i < names.size(); i++)
	{
		it = find_credentials(names.at(i));

		if (it != credentials_list.end() && (*it)->set_security_level(security_level, crypt_key))
			out++;
	}

	return out;
}

bool session_t::set_security_level(std::string name, seclevel_t* security_level)
{
	std::vector<credentials_t*>::iterator it = find_credentials(name);

	if (it != credentials_list.end() && (*it)->set_security_level(security_level, crypt_key))
		return true;

	return false;
}

bool session_t::add_questions(std::string name, std::vector<std::pair<std::string, std::string>> questions)
{
	if (logged_in)
	{
		std::vector<credentials_t*>::iterator it = find_credentials(name);

		if (!is_end(it))
		{
			(*it)->add_questions(questions, crypt_key);
			return true;
		}
	}

	return false;
}

int session_t::delete_questions(std::string name, std::vector<std::string> queries)
{
	if (logged_in)
	{
		std::vector<credentials_t*>::iterator it = find_credentials(name);

		if (!is_end(it))
		{
			return (*it)->delete_questions(queries);
		}
	}
	
	return 0;
}

bool session_t::delete_question(std::string name, int index)
{
	if (logged_in)
	{
		std::vector<credentials_t*>::iterator it = find_credentials(name);

		if (!is_end(it))
		{
			return (*it)->delete_question(index);
		}
	}

	return false;
}

bool session_t::add_backups(std::string name, std::vector<std::string> backups)
{
	if (logged_in)
	{
		std::vector<credentials_t*>::iterator it = find_credentials(name);

		if (!is_end(it))
		{
			(*it)->add_backups(backups, crypt_key);
			return true;
		}
	}

	return false;
}

int session_t::delete_backups(std::string name, std::vector<std::string> queries)
{
	if (logged_in)
	{
		std::vector<credentials_t*>::iterator it = find_credentials(name);

		if (!is_end(it))
		{
			return (*it)->delete_backups(queries, crypt_key);
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

bool session_t::delete_backup(std::string name, int index)
{
	if (logged_in)
	{
		std::vector<credentials_t*>::iterator it = find_credentials(name);

		if (!is_end(it))
		{
			return (*it)->delete_backup(index);
		}
	}

	return false;
}

bool session_t::delete_credentials(std::string name)
{
	if (logged_in)
	{
		std::vector<credentials_t*>::iterator it = find_credentials(name);

		if (!is_end(it))
		{
			delete *it;
			credentials_list.erase(it);
			return true;
		}
	}

	return false;
}

/*
	Set master key
*/
void session_t::set_key(std::string new_key)
{
	if (logged_in)
	{
		keystore_t(keystore_filename).set_key(new_key, key);
	}
}

void session_t::add_seclevel(std::string code, std::string password, int months_valid, int update_year, int update_month, int update_day)
{
	seclevel_manager->add_seclevel(code, password, months_valid, update_year, update_month, update_day, crypt_key);
}

void session_t::add_seclevel(std::string code, int months_valid, int update_year, int update_month, int update_day)
{
	seclevel_manager->add_seclevel(code, months_valid, update_year, update_month, update_day);
}

bool session_t::delete_seclevel(std::string code)
{
	return seclevel_manager->delete_seclevel(code);
}

bool session_t::set_seclevel_password(std::string code, std::string password)
{
	return seclevel_manager->set_seclevel_password(code, password, crypt_key);
}

bool session_t::clear_seclevel_password(std::string code)
{
	return seclevel_manager->clear_seclevel_password(code, crypt_key);
}

/*
	Return a list of credentials whose passwords match an older password for their security level
*/
std::vector<credentials_t*> session_t::get_old_passwords()
{
	std::vector<credentials_t*> out;

	for (std::vector<credentials_t*>::iterator it = credentials_list.begin(); it < credentials_list.end(); it++)
		if (seclevel_manager->is_old_password((*it)->get_security_level(crypt_key), (*it)->get_password(crypt_key)))
			out.push_back(*it);

	return out;
}

std::vector<seclevel_t*> session_t::get_exp_passwords()
{
	return seclevel_manager->get_exp_passwords();
}

bool session_t::update_password(credentials_t* credentials)
{
	seclevel_t* ptr = find_seclevel(credentials->get_security_level(crypt_key));	// Get security-level information to update the set of credentials
	if (ptr)
		return modify_credentials(credentials->get_name(), "p", ptr->get_password(crypt_key));
	else
		return false;
}

void session_t::update_seclevel(seclevel_t* seclevel, std::string password)
{
	seclevel->update_password(password, crypt_key);
}

/*
	Find credentials with site name fully matching the name parameter
*/
std::vector<credentials_t*>::iterator session_t::find_credentials(std::string name)
{
	std::vector<credentials_t*>::iterator out;

	for (out = credentials_list.begin(); out < credentials_list.end() && (*out)->get_name() != name; out++) {}

	return out;
}

/*
	Return whether the iterator points to the end of credentials_list
*/
bool session_t::is_end(std::vector<credentials_t*>::iterator it)
{
	return it == credentials_list.end();
}

std::string session_t::get_crypt_key()
{
	return crypt_key;
}

seclevel_t* session_t::find_seclevel(std::string code)
{
	std::vector<seclevel_t*>::iterator it = seclevel_manager->find_seclevel(code);

	if (!seclevel_manager->is_end(it))
		return *it;
	else
		return nullptr;
}

bool session_t::is_logged_in()
{
	return logged_in;
}

bool session_t::are_credentials_loaded()
{
	return !credentials_list.empty();
}

void session_t::print_credentials()
{
	for (unsigned int i = 0; i < credentials_list.size(); i++)
	{
		credentials_list.at(i)->print(std::cout, crypt_key);
	}
}

/*
	Print the secret questions of a set of credentials in an ordered list
*/
void session_t::print_questions(std::string name)
{
	if (logged_in)
	{
		std::vector<credentials_t*>::iterator it = find_credentials(name);

		if (!is_end(it))
		{
			std::vector<credentials_t::secquestion_t*> secret_questions = (*it)->get_questions();

			for (unsigned int i = 0; i < secret_questions.size(); i++)
			{
				set_print(std::cout);
				std::cout << (i + 1);
				secret_questions.at(i)->print(std::cout, crypt_key);
			}
		}
	}
}

/*
	Print the backup codes of a set of credentials in an ordered list
*/
void session_t::print_backups(std::string name)
{
	if (logged_in)
	{
		std::vector<credentials_t*>::iterator it = find_credentials(name);

		if (!is_end(it))
		{
			std::vector<secret_t*> backup_codes = (*it)->get_backups();

			for (unsigned int i = 0; i < backup_codes.size(); i++)
			{
				set_print(std::cout);
				std::cout << (i + 1);
				backup_codes.at(i)->print(std::cout, crypt_key);
			}
		}
	}
}

void session_t::print_seclevels()
{
	seclevel_manager->print(std::cout, crypt_key);
}

/*
	Find credentials with the site name pattern-matching the query
*/
void session_t::search_credentials(std::string query)
{
	for (unsigned int i = 0; i < credentials_list.size(); i++)
	{
		if (lowercase_contains(credentials_list.at(i)->get_name(), query))	// For each site name that pattern-matches the query
		{
			credentials_list.at(i)->print(std::cout, crypt_key);
		}
	}
}

/*
	Read in credentials from a file
*/
bool session_t::read(std::string filename)
{
	bool same_key = true;	// Assume that there is no key stored and, therefore, the file can be read (albeit in a less secure manner)
	std::ifstream file(filename, std::ios::binary);

	if (file.is_open())	// same_key will also stay true if the file isn't open
	{
		char group_code;
		while (storage::read_group(group_code, file))	// Read next group code
		{
			if (group_code == KEY && logged_in)
				same_key = key_t(file).equals(crypt_key);	// Read hashed key used to encrypt the credentials and check it against crypt_key

			if (group_code == CREDENTIALS)
			{
				while (!storage::is_eor(file))	// Push all credential records to credentials_list
					credentials_list.push_back(new credentials_t(file));

				storage::consume_rs(file);	// There is an extra record separator since this list doesn't span the entire file
			}
		}

		file.close();
	}

	if (!same_key)
		unload();

	return same_key;
}

/*
	Store credentials in a file and security-level information in the file on record
*/
void session_t::store(std::string credentials_filename)
{
	store_credentials(credentials_filename);
	store_seclevels();
}

/*
	Store credentials in a file
*/
void session_t::store_credentials(std::string filename)
{
	std::ofstream file(filename, std::ios::trunc | std::ios::binary);

	if (file.is_open())
	{
		if (!credentials_list.empty())
		{
			storage::store_gs(KEY, file);
			key_t(crypt_key).store(file);	// Hash crypt_key and store

			storage::store_gs(CREDENTIALS, file);
			for (unsigned int i = 0; i < credentials_list.size(); i++)
			{
				credentials_list.at(i)->store(file);
			}
			storage::store_rs(file);	// Store an extra record separator since this list doesn't span the entire file
		}

		file.close();
	}
}

/*
	Store security-level information in the file on record
*/
void session_t::store_seclevels()
{
	seclevel_manager->store();
}

/*
	Clear loaded credentials
*/
void session_t::unload()
{
	for (unsigned int i = 0; i < credentials_list.size(); i++)
	{
		delete credentials_list.at(i);
	}

	credentials_list.clear();
}