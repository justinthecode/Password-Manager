#pragma once

#include "application.h"

/*
	An action for the program to perform
*/
class action_t
{
	protected:
	int option_num = 0;

	public:
	enum type_t
	{
		FILENAME,
		LOGIN,
		DELETE,
		ADD,
		MODIFY,
		SEC_LEVEL,
		ADD_QUESTION,
		DEL_QUESTION,
		ADD_BACKUP,
		DEL_BACKUP,
		PRINT,
		SEARCH,
		SET_KEY
	};

	type_t type;	// Type of action
	std::vector<std::string> options;
	bool completed = false;

	action_t() {}
	action_t(type_t);
	action_t(type_t, int, char*[], int&, int);

	virtual bool exec() = 0;
	void add_options(int, char*[], int&, int);

	virtual action_t& operator+=(std::string) = 0;
};

class singleval_action_t : public action_t
{
	protected:
	std::string value;

	public:
	singleval_action_t(type_t type, std::string value) : action_t(type)
	{
		this->value = value;
	}

	singleval_action_t& operator+=(std::string option)
	{
		value = option;
		option_num++;
		return *this;
	}
};

class namedlist_action_t : public action_t
{
	protected:
	std::string name;
	std::vector<std::string> items;

	public:
	namedlist_action_t(type_t type) : action_t(type) {}

	namedlist_action_t& operator+=(std::string option)
	{
		switch (option_num)
		{
		case 0:
			name = option;
			break;
		default:
			items.push_back(option);
		}

		option_num++;
		return *this;
	}
};

class filename_action_t : public singleval_action_t
{
	public:
	filename_action_t(std::string value) : singleval_action_t(FILENAME, value) {}

	bool exec()
	{
		credentials_filename = value;
		return true;
	}
};

class login_action_t : public singleval_action_t
{
	public:
	login_action_t(std::string value) : singleval_action_t(LOGIN, value) {}

	bool exec()
	{
		if (!session)
			session = new session_t(value, credentials_filename, keystore_filename, seclevel_filename);

		if (!session->is_logged_in())
		{
			delete session;
			session = nullptr;
		}

		if (session)
			std::cout << "Logged in successfully" << std::endl;
		else
			std::cout << "Error logging in" << std::endl;

		return session;
	}
};

class delete_action_t : public singleval_action_t
{
	public:
	delete_action_t(std::string value) : singleval_action_t(DELETE, value) {}

	bool exec()
	{
		if (!confirm_deletion(value))	// Only delete if user confirms
			return false;

		bool out = session && session->delete_credentials(value);

		if (out)
			std::cout << "Credentials deleted successfully" << std::endl;
		else
			std::cout << "Error deleting credentials" << std::endl;

		return out;
	}
};

class add_action_t : public action_t
{
	std::string name, username, password;

	public:
	add_action_t() : action_t(ADD) {}
	add_action_t(int argc, char* argv[], int& i) : action_t(ADD, argc, argv, i, 3) {}

	bool exec()
	{
		if (session)
		{
			session->add_credentials(name, username, password);
			std::cout << "Credentials added sucessfully" << std::endl;
		}
		else
			std::cout << "Could not add credentials given no login" << std::endl;

		return session;
	}

	add_action_t& operator+=(std::string option)
	{
		switch (option_num)
		{
			case 0:
				name = option;
				break;
			case 1:
				username = option;
				break;
			case 2:
				password = option;
		}

		option_num++;
		return *this;
	}
};

class modify_action_t : public action_t
{
	std::string name, field, value;

	public:
	modify_action_t() : action_t(MODIFY) {}
	modify_action_t(int argc, char* argv[], int& i) : action_t(MODIFY, argc, argv, i, 3) {}

	bool exec()
	{
		bool out = false;

		if (session)
			out = session->modify_credentials(name, field, value);
		else
			std::cout << "Could not modify credentials given no login" << std::endl;

		if (out)
			std::cout << "Credentials modified sucessfully" << std::endl;
		else
			std::cout << "Error modifying credentials" << std::endl;

		return out;
	}

	modify_action_t& operator+=(std::string option)
	{
		switch (option_num)
		{
			case 0:
				name = option;
				break;
			case 1:
				field = option;
				break;
			case 2:
				value = option;
		}

		option_num++;
		return *this;
	}
};

class seclevel_action_t : public namedlist_action_t
{
	public:
	seclevel_action_t() : namedlist_action_t(SEC_LEVEL) {}

	bool exec()
	{
		unsigned int successes = 0;

		if (session)
			successes = session->set_security_level(name, items);
		else
			std::cout << "Could not set security levels given no login" << std::endl;

		if (successes == 0)
			std::cout << "No security levels set" << std::endl;	// There are two possibilities here; the first is that all of the security-level setting was erronous and the second is that there were no names
		else if (successes < items.size())
			std::cout << "Error setting some security levels" << std::endl;
		else
			std::cout << "Security levels set successfully" << std::endl;

		return successes == items.size();
	}
};

class add_questions_action_t : public namedlist_action_t
{
	std::vector<std::pair<std::string, std::string>> get_questions()
	{
		std::vector<std::pair<std::string, std::string>> out;
		
		for (unsigned int i = 0; i + 1 < items.size(); i += 2)	// Iterate in question/answer pairs
			out.push_back(std::pair<std::string, std::string>(items.at(i), items.at(i + 1)));

		return out;
	}

	public:
	add_questions_action_t() : namedlist_action_t(ADD_QUESTION) {}

	bool exec()
	{
		bool out = false;

		if (session)
			out = session->add_questions(name, get_questions());
		else
			std::cout << "Could not add secret questions given no login" << std::endl;

		if (out)
			std::cout << "Secret questions added successfully" << std::endl;
		else
			std::cout << "Error adding secret questions" << std::endl;

		return out;
	}
};

class delete_questions_action_t : public namedlist_action_t
{
	public:
	delete_questions_action_t() : namedlist_action_t(DEL_QUESTION) {}

	bool exec()
	{
		unsigned int successes = 0;

		if (session)
			successes = session->delete_questions(name, items);
		else
			std::cout << "Could not delete secret questions given no login" << std::endl;

		if (successes == 0)
			std::cout << "No secret questions deleted" << std::endl;
		else if (successes < items.size())
			std::cout << "One or more secret questions not deleted" << std::endl;
		else
			std::cout << "Secret questions deleted successfully" << std::endl;

		return successes == items.size();
	}
};

class add_backups_action_t : public namedlist_action_t
{
	public:
	add_backups_action_t() : namedlist_action_t(ADD_BACKUP) {}

	bool exec()
	{
		bool out = false;

		if (session)
			out = session->add_backups(name, items);
		else
			std::cout << "Could not add backup codes given no login" << std::endl;

		if (out)
			std::cout << "Backup codes added successfully" << std::endl;
		else
			std::cout << "Error adding backup codes" << std::endl;

		return out;
	}
};

class delete_backups_action_t : public namedlist_action_t
{
	public:
	delete_backups_action_t() : namedlist_action_t(DEL_BACKUP) {}

	bool exec()
	{
		unsigned int successes = 0;

		if (session)
			successes = session->delete_backups(name, items);
		else
			std::cout << "Could not delete backup codes given no login" << std::endl;

		if (successes == 0)
			std::cout << "No backup codes deleted" << std::endl;
		else if (successes < items.size())
			std::cout << "One or more backup codes not deleted" << std::endl;
		else
			std::cout << "Backup codes deleted successfully" << std::endl;

		return successes == items.size();
	}
};

class print_action_t : public action_t
{
	public:
	print_action_t() : action_t(PRINT) {}
	
	bool exec()
	{
		if (session)
			session->print_credentials();
		else
			std::cout << "Could not print credentials given no login" << std::endl;

		return session;
	}

	print_action_t& operator+=(std::string option)
	{
		option_num++;
		return *this;
	}
};

class search_action_t : public singleval_action_t
{
	public:
	search_action_t(std::string value) : singleval_action_t(SEARCH, value) {}
	
	bool exec()
	{
		if (session)
			session->search_credentials(value);
		else
			std::cout << "Could not filter credentials given no login" << std::endl;

		return session;
	}
};

class set_key_action_t : public singleval_action_t
{
	public:
	set_key_action_t(std::string value) : singleval_action_t(SET_KEY, value) {}

	bool exec()
	{
		if (session)
			session->set_key(value);
		else
			std::cout << "Could not set program key given no login" << std::endl;

		return session;
	}
};

action_t::action_t(type_t type)
{
	this->type = type;
}

/*
	Initialize action and read in arguments from the command line
*/
action_t::action_t(action_t::type_t type, int argc, char* argv[], int& i, int n) : action_t(type)
{
	add_options(argc, argv, i, n);
}

/*
	Read in n arguments from the command line and apply them to this action
*/
void action_t::add_options(int argc, char* argv[], int& i, int n)
{
	for (; n > 0; n--)
		if (++i < argc)
			*this += argv[i];
}