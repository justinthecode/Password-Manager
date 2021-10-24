#include "action.h"

std::vector<action_t*> get_actions(int, char**);
void do_actions(std::vector<action_t*>);

bool set_key(std::string);
bool login(std::string);
void logout();

void save();

int main(int argc, char* argv[])
{
	/*
	action_t* action;

	action = new login_action_t("Pa55W0rd");
	action->exec();
	delete action;

	action = new add_questions_action_t();
	*action += "SITE2";
	*action += "What is red?";
	*action += "Red";
	*action += "What is orange?";
	*action += "Orange";
	action->exec();
	delete action;

	action = new print_action_t();
	action->exec();
	delete action;

	save();
	logout();
	return 0;
	*/
	if (argc == 1)
	{
		application::run();
		return 0;
	}
	if (argc == 2 && contains(argv[1], "."))
	{
		credentials_filename = argv[1];
		application::edit_credentials();
		return 0;
	}

	bool no_actions = false;

	for (int i = 1; i < argc && !no_actions; i++)
	{
		if (!strcmp(argv[i], "--version"))	// Check if --version is mentioned anywhere in the arguments
		{
			std::cout << VERSION << std::endl;
			no_actions = true;
		}

		if (!strcmp(argv[i], "--help"))	// Check if --help is mentioned anywhere in the arguments
		{
			application::display_help();
			no_actions = true;
		}

		if (!strcmp(argv[i], "--key"))	// Check if --key is mentioned anywhere in the arguments
		{
			if (application::login())
				if (confirm("Would you like to update the program key?"))
					set_key(get("Enter new program key: "));

			no_actions = true;
		}

		if (!strcmp(argv[i], "--seclevels"))	// Check if --seclevels is mentioned anywhere in the arguments
		{
			if (application::login())
				application::edit_seclevels();

			no_actions = true;
		}
	}

	if (!no_actions)
		do_actions(get_actions(argc, argv));

	save();
	logout();
}

/*
	Parse command-line arguments into an actions vector
*/
std::vector<action_t*> get_actions(int argc, char** argv)
{
	std::vector<action_t*> out;

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-k"))
			if (++i < argc)	// Make sure that there is at least one more argument to utilize
				out.push_back(new login_action_t(argv[i]));

		if (!strcmp(argv[i], "-f"))
			if (++i < argc)
				out.push_back(new filename_action_t(argv[i]));

		if (!strcmp(argv[i], "-d"))
			if (++i < argc)
				out.push_back(new delete_action_t(argv[i]));

		if (!strcmp(argv[i], "-a"))
			if (i < argc - 2)	// Make sure that there are at least three more arguments to utilize
				out.push_back(new add_action_t(argc, argv, i));
			else
				break;

		if (!strcmp(argv[i], "-m"))
			if (i++ < argc - 2)
				out.push_back(new modify_action_t(argc, argv, i));
			else
				break;

		if (!strcmp(argv[i], "-l"))
		{
			action_t* action = new seclevel_action_t();
			*action += argv[++i];	// Add the security level code to the options vector

			for (int n = std::stoi(argv[i]); n > 0; n--)	// Add the credentials to modify to the options vector
				if (++i < argc)
					*action += argv[i];

			out.push_back(action);
		}

		if (!strcmp(argv[i], "-q"))
		{
			action_t* action;

			std::string name = std::string();
			int n = 0;
			if (++i < argc)
				name = argv[i];
			if (++i < argc)
				n = std::stoi(argv[i]);

			if (n >= 0)	// Positive n's call for adding secret questions
				action = new add_questions_action_t();
			else	// Negative n's call for deleting secret questions
				action = new delete_questions_action_t();

			*action += name;	// Both types take the site name before other arguments

			int j = 0;
			for (; j < n * 2; j++)	// If n is positive, arguments for adding secret questions will be pushed to the options vector
				if (++i < argc)
					*action += argv[i];
			for (; j > n; j--)	// If n is negative, arguments for deleting secret questions will be pushed
				if (++i < argc)
					*action += argv[i];

			out.push_back(action);
		}

		if (!strcmp(argv[i], "-b"))
		{
			action_t* action;

			std::string name = std::string();
			int n = 0;
			if (++i < argc)
				name = argv[i];
			if (++i < argc)
				n = std::stoi(argv[i]);

			if (n >= 0)	// Positive n's call for adding backup codes
				action = new add_backups_action_t();
			else	// Negative n's call for deleting backup codes
				action = new delete_backups_action_t();

			*action += name;	// Both types take the site name before other arguments

			for (int j = 0; j < abs(n); j++)	// The same number of arguments is required whether n is positive or negative
				if (++i < argc)
					*action += argv[i];

			out.push_back(action);
		}

		if (!strcmp(argv[i], "-u"))
			if (++i < argc)
				out.push_back(new set_key_action_t(argv[i]));

		if (!strcmp(argv[i], "-p"))
			out.push_back(new print_action_t());

		if (!strcmp(argv[i], "-s"))
			if (++i < argc)
				out.push_back(new search_action_t(argv[i]));
	}

	return out;
}

void do_actions(std::vector<action_t*> actions)
{
	action_t* action;
	do
	{
		action = nullptr;

		for (int type = action_t::FILENAME; type <= action_t::SET_KEY; type++)	// Actions have a predefined order they are completed in
		{
			for (unsigned int i = 0; !action && i < actions.size(); i++)
			{
				if (actions.at(i)->type == type)
				{
					actions.at(i)->exec();
					delete actions.at(i);
					actions.erase(actions.begin() + i);
					i--;
				}
			}
		}
	} while (action);
}

/*
	Update master key
*/
bool set_key(std::string new_key)
{
	if (session)
	{
		session->set_key(new_key);
		return true;
	}

	return false;
}

bool login(std::string key)
{
	if (!session)
		session = new session_t(key, credentials_filename, keystore_filename, seclevel_filename);

	if (!session->is_logged_in())
	{
		delete session;
		session = nullptr;
	}

	if (session)
		return true;
	else
		return false;
}

void logout()
{
	if (session)
	{
		session->logout();
		delete session;
		std::cout << "Logged out successfully" << std::endl;
	}
}

void save()
{
	if (session)
	{
		std::cout << "Saving credentials and security levels" << std::endl;
		session->store(credentials_filename);
	}
}