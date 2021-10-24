#pragma once

#include <queue>
#include <ctime>
#include <utility>
#include "key.h"

struct basic_tm : public std::tm
{
	private:
	enum unit_code
	{
		TIME = 'T'
	};

	void set_fields(int, int, int);
	void set_fields(std::string);

	public:
	basic_tm();
	basic_tm(int, int, int);
	basic_tm(std::string);
	basic_tm(std::ifstream&);

	int compare(basic_tm);

	std::string to_string();
	void print(std::ostream&, bool = true);
	void store(std::ofstream&);
};

class seclevel_t : public printable_t
{
	public:
	struct prevpwrd_t
	{
		private:
		enum group_code
		{
			PASSWORD = 'P',
			TIMESTAMP = 'T'
		};

		public:
		key_t password;
		basic_tm timestamp;

		prevpwrd_t(key_t, basic_tm);
		prevpwrd_t(std::ifstream&);

		void store(std::ofstream&);
	};

	private:
	enum group_code
	{
		BASIC = 'B',
		PASSWORD = 'P',
		PREV_PWRD = 'O',
		UPDATE = 'U'
	};

	enum unit_code
	{
		CODE = 'C',
		MONTHS = 'M'
	};

	std::string code;

	secret_t* password;
	std::queue<prevpwrd_t*> prev_passwords;

	int months_valid;
	basic_tm update_time;

	void save_password(std::string);

	public:
	seclevel_t() {}
	seclevel_t(std::string, int, int, int, int);
	seclevel_t(std::string, std::string, int, int, int, int, std::string);
	seclevel_t(std::ifstream&);
	~seclevel_t();

	void set_months_valid(int);
	void set_update_time(int, int, int);
	void set_password(std::string, std::string);
	void clear_password(std::string);

	std::string get_code();
	std::string get_password(std::string);
	basic_tm get_update_time();
	bool has_password();
	bool is_old_password(std::string);
	bool is_expired();
	void update_password(std::string, std::string);

	void print(std::ostream&, std::string);
	void print_long(std::ostream&, std::string);
	void store(std::ofstream&);
};

class seclevel_manager_t
{
	enum group_code
	{
		SECLEVELS = 'L'
	};

	std::vector<seclevel_t*> security_levels;

	std::string filename;

	void read();

	public:
	seclevel_manager_t(std::string);
	~seclevel_manager_t();

	void add_seclevel(std::string, std::string, int, int, int, int, std::string);
	void add_seclevel(std::string, int, int, int, int);
	bool delete_seclevel(std::string);
	bool set_seclevel_password(std::string, std::string, std::string);
	bool clear_seclevel_password(std::string, std::string);

	std::vector<seclevel_t*>::iterator find_seclevel(std::string);
	bool is_end(std::vector<seclevel_t*>::iterator);
	bool is_old_password(std::string, std::string);
	std::vector<seclevel_t*> get_exp_passwords();

	void print(std::ostream&, std::string);
	void store();
};

void basic_tm::set_fields(int year, int month, int day)
{
	tm_year = year;
	tm_mon = month;
	tm_mday = day;
}

void basic_tm::set_fields(std::string str)
{
	if (str.length() == 10)
		set_fields(std::stoi(str.substr(0, 4)), std::stoi(str.substr(5, 2)), std::stoi(str.substr(8, 2)));
}

/*
	Generate basic_tm based on current date
*/
basic_tm::basic_tm()
{
	std::time_t now;
	std::time(&now);
	std::tm timeinfo;
	localtime_s(&timeinfo, &now);
	
	set_fields(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday);
}

basic_tm::basic_tm(int year, int month, int day)
{
	set_fields(year, month, day);
}

/*
	Converts YYYY/MM/DD-formatted string to tm
*/
basic_tm::basic_tm(std::string str)
{
	set_fields(str);
}

basic_tm::basic_tm(std::ifstream& input)
{
	if (input.is_open())
	{
		char unit_code;
		while (!storage::is_eor(input) && storage::read_unit(unit_code, input))	// Read next unit code
		{
			std::string str_time;
			switch (unit_code)
			{
				case TIME:
					storage::read(str_time, input);
					set_fields(str_time);
			}
		}

		storage::consume_rs(input);
	}
}

/*
	Compares tm to another tm
*/
int basic_tm::compare(basic_tm other)
{
	if (tm_year > other.tm_year)	// Check for differences in year
		return 1;
	if (tm_year < other.tm_year)
		return -1;

	if (tm_mon > other.tm_mon)	// Check for differences in month
		return 1;
	if (tm_mon < other.tm_mon)
		return -1;

	if (tm_mday > other.tm_mday)	// Check for differences in day of the month
		return 1;
	if (tm_mday < other.tm_mday)
		return -1;

	return 0;	// If no differences, return 0
}

/*
	Converts tm to YYYY/MM/DD format
*/
std::string basic_tm::to_string()
{
	std::string out = "";
	std::string temp;

	temp = std::to_string(tm_year);
	for (unsigned int i = 0; i < 4 - temp.length(); i++)
		out += "0";	// Pad with 0's as needed
	out += temp + "/";

	temp = std::to_string(tm_mon);
	if (temp.length() < 2)
		out += "0" + temp + "/";
	else
		out += temp + "/";

	temp = std::to_string(tm_mday);
	if (temp.length() < 2)
		out += "0" + temp;
	else
		out += temp;

	return out;
}

void basic_tm::print(std::ostream& output, bool newline)
{
	set_print(output);
	output << to_string();
	if (newline)
		output << std::endl;
}

void basic_tm::store(std::ofstream& output)
{
	storage::store(TIME, to_string(), output);
	storage::store_rs(output);
}

seclevel_t::prevpwrd_t::prevpwrd_t(key_t password, basic_tm timestamp)
{
	this->password = password;
	this->timestamp = timestamp;
}

seclevel_t::prevpwrd_t::prevpwrd_t(std::ifstream& input)
{
	if (input.is_open())
	{
		char group_code;
		while (!storage::is_eor(input) && storage::read_group(group_code, input))	// Read next group code
		{
			if (group_code == PASSWORD)
				password = key_t(input);

			if (group_code == TIMESTAMP)
				timestamp = basic_tm(input);
		}

		storage::consume_rs(input);
	}
}

void seclevel_t::prevpwrd_t::store(std::ofstream& output)
{
	storage::store_gs(PASSWORD, output);
	password.store(output);

	storage::store_gs(TIMESTAMP, output);
	timestamp.store(output);

	storage::store_rs(output);
}

void seclevel_t::save_password(std::string key)
{
	if (this->password)
		prev_passwords.push(new prevpwrd_t(key_t(this->password->get_data(key)), basic_tm()));
}

seclevel_t::seclevel_t(std::string code, int months_valid, int update_year, int update_month, int update_day)
{
	this->code = code;
	this->months_valid = months_valid;
	set_update_time(update_year, update_month, update_day);
	password = nullptr;
}

seclevel_t::seclevel_t(std::string code, std::string password, int months_valid, int update_year, int update_month, int update_day, std::string key)
{
	this->code = code;
	this->months_valid = months_valid;
	set_update_time(update_year, update_month, update_day);
	this->password = new secret_t(password, key);
}

seclevel_t::seclevel_t(std::ifstream& input)
{
	password = nullptr;

	if (input.is_open())
	{
		char group_code;
		while (!storage::is_eor(input) && storage::read_group(group_code, input))	// Read next group code
		{
			if (group_code == BASIC)
			{
				char unit_code;
				while (!storage::is_eog(input) && storage::read_unit(unit_code, input))	// Read next unit code
				{
					switch (unit_code)
					{
						case CODE:
							storage::read(code, input);
							break;
						case MONTHS:
							storage::read(months_valid, input);
					}
				}
			}

			if (group_code == UPDATE)
				update_time = basic_tm(input);

			if (group_code == PASSWORD)
				password = new secret_t(input);

			if (group_code == PREV_PWRD)
			{
				while (!storage::is_eor(input))
					prev_passwords.push(new prevpwrd_t(input));

				storage::consume_rs(input);	// There is an extra record separator since this list doesn't span the entire file
			}
		}

		storage::consume_rs(input);
	}
}

seclevel_t::~seclevel_t()
{
	if (password)
		delete password;

	for (int i = prev_passwords.size(); i > 0; i--)	 // With this setup, no extra variables are needed
	{
		delete prev_passwords.front();
		prev_passwords.pop();
	}
}

void seclevel_t::set_months_valid(int months_valid)
{
	this->months_valid = months_valid;
}

void seclevel_t::set_update_time(int year, int month, int day)
{
	update_time.tm_year = year;
	update_time.tm_mon = month;
	update_time.tm_mday = day;
}

void seclevel_t::set_password(std::string password, std::string key)
{
	save_password(key);
	if (this->password)
		delete this->password;
	this->password = new secret_t(password, key);
}

void seclevel_t::clear_password(std::string key)
{
	save_password(key);
	if (password)
		delete password;
	password = nullptr;
}

std::string seclevel_t::get_code()
{
	return code;
}

std::string seclevel_t::get_password(std::string key)
{
	if (password)
		return password->get_data(key);
	else
		return std::string();
}

basic_tm seclevel_t::get_update_time()
{
	return update_time;
}

bool seclevel_t::has_password()
{
	if (password)
		return true;
	else
		return false;
}

bool seclevel_t::is_old_password(std::string password)
{
	if (!has_password())
		return false;

	bool out = false;

	for (unsigned int i = 0; i < prev_passwords.size(); i++)
	{
		if (prev_passwords.front()->password.equals(password))
			out = true;	// Don't return here so the queue can get back to its original state

		prev_passwords.push(prev_passwords.front());
		prev_passwords.pop();
	}

	return out;
}

/*
	Whether the date specified in update_time has been reached
*/
bool seclevel_t::is_expired()
{
	return update_time.compare(basic_tm()) <= 0;
}

void seclevel_t::update_password(std::string password, std::string key)
{
	basic_tm now = basic_tm();

	while (months_valid > 0 && update_time.compare(now) <= 0)	// Advance update_time in months_valid increments until update_time is in the future
	{
		update_time.tm_mon += months_valid;
		while (update_time.tm_mon > 12)
		{
			update_time.tm_mon -= 12;
			update_time.tm_year++;
		}
	}

	set_password(password, key);
}

void seclevel_t::print(std::ostream& output, std::string key)
{
	set_print(output);
	output << get_code();
	
	if (password)
		password->print(output, key, false);
	else
	{
		set_print(output);
		output << "No Password";
	}

	set_print(output);
	get_update_time().print(output, false);
	output << std::endl;
}

void seclevel_t::print_long(std::ostream& output, std::string key)
{
	print(output, key);

	set_print(output);
	output << "";
	set_print(output);
	output << "Update every " + std::to_string(months_valid) + " month" + (months_valid != 1 ? "s" : "");
	output << std::endl;
}

void seclevel_t::store(std::ofstream& output)
{
	storage::store_gs(BASIC, output);
	storage::store(CODE, code, output);
	storage::store(MONTHS, months_valid, output);
	
	storage::store_gs(UPDATE, output);
	update_time.store(output);

	if (password)
	{
		storage::store_gs(PASSWORD, output);
		password->store(output);
	}

	if (!prev_passwords.empty())
	{
		storage::store_gs(PREV_PWRD, output);
		for (unsigned int i = 0; i < prev_passwords.size(); i++)
		{
			prev_passwords.front()->store(output);
			prev_passwords.push(prev_passwords.front());	// Rotate through values
			prev_passwords.pop();
		}

		storage::store_rs(output);	// Store an extra record separator since this list doesn't span the entire file
	}

	storage::store_rs(output);
}

/*
	Read in security-level information from the file on record
*/
void seclevel_manager_t::read()
{
	std::ifstream file(filename, std::ios::binary);

	if (file.is_open())
	{
		char group_code;
		while (storage::read_group(group_code, file))	// Read next group code
		{
			if (group_code == SECLEVELS)
			{
				while (!storage::is_eor(file))	// Push all security-level records to security_levels
					security_levels.push_back(new seclevel_t(file));

				storage::consume_rs(file);
			}
		}

		file.close();
	}
}

seclevel_manager_t::seclevel_manager_t(std::string filename)
{
	this->filename = filename;
	read();
}

seclevel_manager_t::~seclevel_manager_t()
{
	for (unsigned int i = 0; i < security_levels.size(); i++)
	{
		delete security_levels.at(i);
	}

	security_levels.clear();
}

void seclevel_manager_t::add_seclevel(std::string code, std::string password, int months_valid, int update_year, int update_month, int update_day, std::string key)
{
	std::vector<seclevel_t*>::iterator it = find_seclevel(code);

	if (is_end(it))
		security_levels.push_back(new seclevel_t(code, password, months_valid, update_year, update_month, update_day, key));
}

void seclevel_manager_t::add_seclevel(std::string code, int months_valid, int update_year, int update_month, int update_day)
{
	std::vector<seclevel_t*>::iterator it = find_seclevel(code);

	if (is_end(it))
		security_levels.push_back(new seclevel_t(code, months_valid, update_year, update_month, update_day));
}

bool seclevel_manager_t::delete_seclevel(std::string code)
{
	std::vector<seclevel_t*>::iterator it = find_seclevel(code);

	if (!is_end(it))
	{
		delete *it;
		security_levels.erase(it);
		return true;
	}

	return false;
}

bool seclevel_manager_t::set_seclevel_password(std::string code, std::string password, std::string key)
{
	std::vector<seclevel_t*>::iterator it = find_seclevel(code);

	if (!is_end(it))
	{
		(*it)->set_password(password, key);
		return true;
	}

	return false;
}

bool seclevel_manager_t::clear_seclevel_password(std::string code, std::string key)
{
	std::vector<seclevel_t*>::iterator it = find_seclevel(code);

	if (!is_end(it))
	{
		(*it)->clear_password(key);
		return true;
	}

	return false;
}

/*
	Find security level with code fully matching the code parameter
*/
std::vector<seclevel_t*>::iterator seclevel_manager_t::find_seclevel(std::string code)
{
	std::vector<seclevel_t*>::iterator out;

	for (out = security_levels.begin(); out < security_levels.end() && (*out)->get_code() != code; out++) {}

	return out;
}

/*
	Whether an iterator is at the end of the list of security levels
*/
bool seclevel_manager_t::is_end(std::vector<seclevel_t*>::iterator it)
{
	return it == security_levels.end();
}

/*
	Whether a password matches a previous password for its security level
*/
bool seclevel_manager_t::is_old_password(std::string code, std::string password)
{
	std::vector<seclevel_t*>::iterator it = find_seclevel(code);

	if (!is_end(it))
		return (*it)->is_old_password(password);

	return false;
}

/*
Return a list of security levels whose passwords are expired
*/
std::vector<seclevel_t*> seclevel_manager_t::get_exp_passwords()
{
	std::vector<seclevel_t*> out;

	for (std::vector<seclevel_t*>::iterator it = security_levels.begin(); it < security_levels.end(); it++)
		if ((*it)->is_expired())
			out.push_back(*it);

	return out;
}

void seclevel_manager_t::print(std::ostream& output, std::string key)
{
	for (unsigned int i = 0; i < security_levels.size(); i++)
		security_levels.at(i)->print(output, key);
}

/*
	Store security-level information in the file on record
*/
void seclevel_manager_t::store()
{
	std::ofstream file(filename, std::ios::trunc | std::ios::binary);

	if (file.is_open())
	{
		if (!security_levels.empty())
		{
			storage::store_gs(SECLEVELS, file);
			for (unsigned int i = 0; i < security_levels.size(); i++)
			{
				security_levels.at(i)->store(file);
			}

			storage::store_rs(file);
		}

		file.close();
	}
}