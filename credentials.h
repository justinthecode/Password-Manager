#pragma once

#include <initializer_list>
#include <utility>
#include "crypt.h"
#include "search.h"
#include "response.h"

#define NO_SECURITY_LEVEL "N/A"

/*
	A set of credentials
*/
class credentials_t : public printable_t
{
	public:
	class secquestion_t : public secret_t
	{
		enum unit_code
		{
			QUESTION = 'Q'
		};

		std::string question;

		public:
		secquestion_t() {}
		secquestion_t(std::ifstream&);
		secquestion_t(std::string, std::string, std::string);
		secquestion_t(std::pair<std::string, std::string>, std::string);

		void set_question(std::string);
		std::string get_question();

		void print(std::ostream&, std::string);
		void store(std::ofstream&);
	};

	private:
	enum group_code
	{
		NAMES = 'N',
		PWORD = 'P',
		SECLV = 'L',
		SECQS = 'S',
		BKPCS = 'B'
	};

	enum unit_code
	{
		NAME = 'N',
		USERNAME = 'U'
	};

	std::string name;
	std::string username;

	secret_t password;
	secret_t security_level;

	std::vector<secquestion_t*> secret_questions;
	std::vector<secret_t*> backup_codes;

	public:
	credentials_t() {}
	credentials_t(std::ifstream&);
	credentials_t(std::string, std::string, std::string, std::string);
	credentials_t(std::string, std::string, std::string, std::string, std::string);
	credentials_t(std::string, std::string, std::string, std::string, std::initializer_list<std::pair<std::string, std::string>>, std::initializer_list<std::string>);
	~credentials_t();

	void set_name(std::string);
	void set_username(std::string);
	void set_password(std::string, std::string);
	bool set_security_level(std::string, std::string);
	bool set_security_level(seclevel_t*, std::string);
	void add_questions(std::initializer_list<std::pair<std::string, std::string>>, std::string);
	void add_questions(std::vector<std::pair<std::string, std::string>>, std::string);
	int delete_questions(std::vector<std::string>);
	bool delete_question(unsigned int);
	void add_backups(std::initializer_list<std::string>, std::string);
	void add_backups(std::vector<std::string>, std::string);
	int delete_backups(std::vector<std::string>, std::string);
	bool delete_backup(unsigned int);
	void set_key(std::string, std::string);

	std::string get_name();
	std::string get_username();
	std::string get_password(std::string);
	std::string get_security_level(std::string);
	std::vector<secquestion_t*> get_questions();
	std::vector<secret_t*> get_backups();

	void print(std::ostream&, std::string);
	void store(std::ofstream&);
};

credentials_t::credentials_t(std::ifstream& input)
{
	if (input.is_open())
	{
		char group_code;
		while (!storage::is_eor(input) && storage::read_group(group_code, input))	// Read next group code
		{
			if (group_code == NAMES)
			{
				char unit_code;
				while (!storage::is_eog(input) && storage::read_unit(unit_code, input))	// Read next unit code
				{
					switch (unit_code)
					{
						case NAME:
							storage::read(name, input);
							break;
						case USERNAME:
							storage::read(username, input);
					}
				}
			}

			if (group_code == PWORD)
				password = secret_t(input);

			if (group_code == SECLV)
				security_level = secret_t(input);

			if (group_code == SECQS)
				while (!storage::is_eog(input))
					secret_questions.push_back(new secquestion_t(input));

			if (group_code == BKPCS)
				while (!storage::is_eog(input))
					backup_codes.push_back(new secret_t(input));
		}

		storage::consume_rs(input);
	}
}

credentials_t::credentials_t(std::string name, std::string username, std::string password, std::string key)
{
	set_name(name);
	set_username(username);
	set_password(password, key);
	set_security_level(NO_SECURITY_LEVEL, key);
}

credentials_t::credentials_t(std::string name, std::string username, std::string password, std::string security_level, std::string key)
{
	set_name(name);
	set_username(username);
	set_password(password, key);
	set_security_level(security_level, key);
}

/*
	Initialize credentials with programmatic lists of secret questions and backup codes. The purpose of this function is to give more control to developers in testing.
*/
credentials_t::credentials_t(std::string name, std::string username, std::string password, std::string key, std::initializer_list<std::pair<std::string, std::string>> secret_questions, std::initializer_list<std::string> backup_codes)
	: credentials_t::credentials_t(name, username, password, key)
{
	add_questions(secret_questions, key);
	add_backups(backup_codes, key);
}

credentials_t::~credentials_t()
{
	for (unsigned int i = 0; i < secret_questions.size(); i++)
	{
		delete secret_questions.at(i);
	}

	secret_questions.clear();

	for (unsigned int i = 0; i < backup_codes.size(); i++)
	{
		delete backup_codes.at(i);
	}

	backup_codes.clear();
}

void credentials_t::set_name(std::string name)
{
	this->name = name;
}

void credentials_t::set_username(std::string username)
{
	this->username = username;
}

void credentials_t::set_password(std::string password, std::string key)
{
	this->password.set_data(password, key);
}

bool credentials_t::set_security_level(std::string security_level, std::string key)
{
	this->security_level.set_data(security_level, key);
	return true;
}

bool credentials_t::set_security_level(seclevel_t* security_level, std::string key)
{
	if (security_level)
		this->security_level.set_data(security_level->get_code(), key);
	else
		this->security_level.set_data(NO_SECURITY_LEVEL, key);

	return true;
}

void credentials_t::add_questions(std::initializer_list<std::pair<std::string, std::string>> secret_questions, std::string key)
{
	for (auto sec_q : secret_questions)
	{
		this->secret_questions.push_back(new secquestion_t(sec_q, key));
	}
}

void credentials_t::add_questions(std::vector<std::pair<std::string, std::string>> secret_questions, std::string key)
{
	for (auto sec_q : secret_questions)
	{
		this->secret_questions.push_back(new secquestion_t(sec_q, key));
	}
}

/*
	Increment through a vector of queries and delete the first secret question to pattern-match each one
*/
int credentials_t::delete_questions(std::vector<std::string> queries)
{
	int out = 0;

	for (unsigned int i = 0; i < queries.size(); i++)
	{
		for (unsigned int j = 0; j < secret_questions.size(); j++)
		{
			if (lowercase_contains(secret_questions.at(j)->get_question(), queries.at(i)))	// Delete first question that pattern-matches the query
			{
				if (confirm_deletion(secret_questions.at(j)->get_question()))	// Only delete if user confirms
				{
					delete secret_questions.at(j);
					secret_questions.erase(secret_questions.begin() + j);
					out++;
				}

				break;	// Only one question maximum should be deleted with each query
			}
		}
	}

	return out;
}

/*
	Delete secret question based on index
*/
bool credentials_t::delete_question(unsigned int index)
{
	if (index >= 0 && index < secret_questions.size())
	{
		secret_questions.erase(secret_questions.begin() + index);
		return true;
	}

	return false;
}

void credentials_t::add_backups(std::initializer_list<std::string> backup_codes, std::string key)
{
	for (auto backup_c : backup_codes)
	{
		this->backup_codes.push_back(new secret_t(backup_c, key));
	}
}

void credentials_t::add_backups(std::vector<std::string> backup_codes, std::string key)
{
	for (auto backup_c : backup_codes)
	{
		this->backup_codes.push_back(new secret_t(backup_c, key));
	}
}

/*
	Increment through a vector of queries and delete the first backup code to pattern-match each one
*/
int credentials_t::delete_backups(std::vector<std::string> queries, std::string key)
{
	int out = 0;

	for (unsigned int i = 0; i < queries.size(); i++)
	{
		for (unsigned int j = 0; j < backup_codes.size(); j++)
		{
			if (lowercase_contains(backup_codes.at(j)->get_data(key), queries.at(i)))	// Delete first backup that pattern-matches the query
			{
				if (confirm_deletion(backup_codes.at(j)->get_data(key)))	// Only delete if user confirms
				{
					delete backup_codes.at(j);
					backup_codes.erase(backup_codes.begin() + j);
					out++;
				}
				
				break;	// Only one backup maximum should be deleted with each query
			}
		}
	}

	return out;
}

/*
	Delete backup code based on index
*/
bool credentials_t::delete_backup(unsigned int index)
{
	if (index >= 0 && index < backup_codes.size())
	{
		backup_codes.erase(backup_codes.begin() + index);
		return true;
	}

	return false;
}

/*
	Re-encrypt all secrets with a new key
*/
void credentials_t::set_key(std::string new_key, std::string key)
{
	set_password(get_password(key), new_key);

	for (unsigned int i = 0; i < secret_questions.size(); i++)
	{
		secret_questions.at(i)->set_data(secret_questions.at(i)->get_data(key), new_key);
	}

	for (unsigned int i = 0; i < backup_codes.size(); i++)
	{
		backup_codes.at(i)->set_data(backup_codes.at(i)->get_data(key), new_key);
	}
}

std::string credentials_t::get_name()
{
	return name;
}

std::string credentials_t::get_username()
{
	return username;
}

std::string credentials_t::get_password(std::string key)
{
	return password.get_data(key);
}

std::string credentials_t::get_security_level(std::string key)
{
	return security_level.get_data(key);
}

std::vector<credentials_t::secquestion_t*> credentials_t::get_questions()
{
	return secret_questions;
}

std::vector<secret_t*> credentials_t::get_backups()
{
	return backup_codes;
}

void credentials_t::print(std::ostream& output, std::string key)
{
	set_print(output);
	output << get_name();
	set_print(output);
	output << get_username();
	set_print(output);
	output << get_password(key);
	set_print(output);
	output << get_security_level(key);
	output << std::endl;

	for (unsigned int i = 0; i < secret_questions.size(); i++)
	{
		set_print(output);
		output << "";
		secret_questions.at(i)->print(output, key);
	}

	for (unsigned int i = 0; i < backup_codes.size(); i++)
	{
		set_print(output);
		output << "";
		backup_codes.at(i)->print(output, key);
	}
}

void credentials_t::store(std::ofstream& output)
{
	if (output.is_open())
	{
		storage::store_gs(NAMES, output);
		storage::store(NAME, name, output);
		storage::store(USERNAME, username, output);

		storage::store_gs(PWORD, output);
		password.store(output);

		storage::store_gs(SECLV, output);
		security_level.store(output);

		if (!secret_questions.empty())
		{
			storage::store_gs(SECQS, output);
			for (unsigned int i = 0; i < secret_questions.size(); i++)
			{
				secret_questions.at(i)->store(output);
			}
		}

		if (!backup_codes.empty())
		{
			storage::store_gs(BKPCS, output);
			for (unsigned int i = 0; i < backup_codes.size(); i++)
			{
				backup_codes.at(i)->store(output);
			}
		}

		storage::store_rs(output);
	}
}

credentials_t::secquestion_t::secquestion_t(std::ifstream& input)
{
	if (input.is_open())
	{
		char unit_code;
		while (!storage::is_eor(input) && storage::read_unit(unit_code, input))	// Read next unit code
		{
			switch (unit_code)
			{
				case QUESTION:
					storage::read(question, input);
					break;
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

credentials_t::secquestion_t::secquestion_t(std::string question, std::string answer, std::string key) : secret_t::secret_t(answer, key)
{
	set_question(question);
}

credentials_t::secquestion_t::secquestion_t(std::pair<std::string, std::string> sec_q, std::string key) : secret_t::secret_t(sec_q.second, key)
{
	set_question(sec_q.first);
}

void credentials_t::secquestion_t::set_question(std::string question)
{
	this->question = question;
}

std::string credentials_t::secquestion_t::get_question()
{
	return question;
}

void credentials_t::secquestion_t::print(std::ostream& output, std::string key)
{
	set_print(output);
	output << get_question();
	secret_t::print(output, key);
}

void credentials_t::secquestion_t::store(std::ofstream& output)
{
	if (output.is_open())
	{
		storage::store(QUESTION, question, output);
		secret_t::store(output);
	}
}