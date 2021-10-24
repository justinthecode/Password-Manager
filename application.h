#pragma once

#include "key.h"
#include "session.h"

#define VERSION "1.0"
#define DEFAULT_CREDENTIALS_FILENAME "credentials.dat"
#define DEFAULT_KEYSTORE_FILENAME "key.dat"
#define DEFAULT_SECLEVEL_FILENAME "seclevel.dat"

session_t* session = nullptr;
std::string credentials_filename = DEFAULT_CREDENTIALS_FILENAME;	// Where to read/store credentials
std::string keystore_filename = DEFAULT_KEYSTORE_FILENAME;	// Where to read/store keys
std::string seclevel_filename = DEFAULT_SECLEVEL_FILENAME;	// Where to read/store security-level information

namespace application
{
	void run();
	void edit_credentials();
	void edit_seclevels();

	void display_title();
	void display_main_menu();
	void display_file_menu();
	void display_modify_credentials_menu();
	void display_update_seclevels_menu();
	void display_modify_seclevel_menu();
	void display_help();

	void add_credentials();
	void delete_credentials();
	void modify_credentials();
	seclevel_t* get_seclevel();

	void add_seclevel();
	void delete_seclevels();
	void modify_seclevels();

	bool login();
	void save_credentials();
	void save_seclevels();

	/*
		Run like a windowed application
	*/
	void run()
	{
		display_title();

		if (login())
		{
			std::string response;
			bool done = false;
			do
			{
				display_main_menu();

				do
				{
					response = get("Enter command: ");
				} while (response.empty());

				std::string filename_response;
				switch (std::tolower(response[0]))
				{
					case 'o':	// Open file
						filename_response = get("Enter filename, or press <Enter> to open the default credentials file (" + credentials_filename + "): ");
						if (!filename_response.empty())
							credentials_filename = filename_response;
						edit_credentials();
						credentials_filename = DEFAULT_CREDENTIALS_FILENAME;
						break;

					case 'k':	// Set program key
						if (session && confirm("Would you like to update the program key?"))
							session->set_key(get("Enter new program key: "));
						break;

					case 'l':	// Update security levels
						edit_seclevels();
						break;

					case 'h':	// Help
						display_help();
						break;

					case 'q':
						done = true;
				}
			} while (!done);
		}
	}

	/*
		Module to edit credentials in credentials_filename
	*/
	void edit_credentials()
	{
		if (session || login())
		{
			if (!session->read(credentials_filename))
			{
				std::cout << "Incorrect static key. Make sure the key.dat file in this folder is the same as the one you used in the past with this file." << std::endl;
				return;
			}

			std::string response;
			std::vector<credentials_t*> update_credentials = session->get_old_passwords();
			std::vector<printable_t*> printable_update_credentials;	// Same vector cast to printable objects for UI
			for (std::vector<credentials_t*>::iterator it = update_credentials.begin(); it < update_credentials.end(); it++)
				printable_update_credentials.push_back((printable_t*)*it);

			if (!update_credentials.empty() && confirm("One or more passwords set by a security level have since been updated. Would you like to update these credentials?"))
			{
				do
				{
					print_list(printable_update_credentials, session->get_crypt_key());
					response = get("Enter the index of the set of credentials you would like to update, or enter \"a\" to update all credentials in this list. Press <Enter> to cancel: ");

					if (response.empty())
						break;

					if (std::tolower(response[0]) == 'a')
					{
						for (std::vector<credentials_t*>::iterator it = update_credentials.begin(); it < update_credentials.end(); it++)
							session->update_password(*it);

						break;
					}
					else if (response[0] >= '0' && response[0] <= '9')
					{
						unsigned int index = std::stoi(response) - 1;

						if (index >= 0 && index < update_credentials.size())
						{
							session->update_password(update_credentials.at(index));

							update_credentials.erase(update_credentials.begin() + index);
							printable_update_credentials.erase(printable_update_credentials.begin() + index);
						}
						else
							std::cout << "Invalid index. No credentials updated." << std::endl;
					}
				} while (!update_credentials.empty() && confirm("Would you like to update more credentials?"));
			}

			bool done = false;
			do
			{
				display_file_menu();

				do
				{
					response = get("Enter command: ");
				} while (response.empty());

				switch (std::tolower(response[0]))
				{
					case 'a':	// Add credentials
						add_credentials();
						break;

					case 'd':	// Delete credentials
						delete_credentials();
						break;

					case 'm':	// Modify credentials
						modify_credentials();
						break;

					case 'p':	// Print credentials
						session->print_credentials();
						break;

					case 'q':
						save_credentials();
						done = true;
				}
			} while (!done);

			session->unload();
		}
	}

	/*
		View and update security-level information
	*/
	void edit_seclevels()
	{
		if (session || login())
		{
			std::string response;
			std::vector<std::string> options;
			bool done = false;

			std::vector<seclevel_t*> update_seclevels = session->get_exp_passwords();	// First check if any passwords are due to be updated
			std::vector<printable_t*> printable_update_seclevels;	// Same vector cast to printable objects for UI
			for (std::vector<seclevel_t*>::iterator it = update_seclevels.begin(); it < update_seclevels.end(); it++)
				printable_update_seclevels.push_back((printable_t*)*it);

			if (!update_seclevels.empty() && confirm("One or more security-level passwords have expired. Would you like to update these security levels?"))
			{
				do
				{
					print_list(printable_update_seclevels, session->get_crypt_key());
					response = get("Enter the index of the set of credentials you would like to update, or enter \"a\" to update all credentials in this list. Press <Enter> to cancel: ");

					if (response.empty())
						break;

					if (std::tolower(response[0]) == 'a')
					{
						for (std::vector<seclevel_t*>::iterator it = update_seclevels.begin(); it < update_seclevels.end(); it++)
							session->update_seclevel(*it, get("Enter new password for " + (*it)->get_code() + ": "));

						break;
					}
					else if (response[0] >= '0' && response[0] <= '9')
					{
						unsigned int index = std::stoi(response) - 1;

						if (index >= 0 && index < update_seclevels.size())
						{
							session->update_seclevel(update_seclevels.at(index), get("Enter new password for " + (update_seclevels.at(index))->get_code() + ": "));

							update_seclevels.erase(update_seclevels.begin() + index);
							printable_update_seclevels.erase(printable_update_seclevels.begin() + index);
						}
						else
							std::cout << "Invalid index. No security levels updated." << std::endl;
					}
				} while (!update_seclevels.empty() && confirm("Would you like to update more security levels?"));
			}

			do
			{
				session->print_seclevels();
				display_update_seclevels_menu();

				response = get("Enter command: ");

				switch (response[0])
				{
					case 'a':	// Add security level
						add_seclevel();
						break;

					case 'd':	// Delete security levels
						delete_seclevels();
						break;

					case 'm':	// Modify security levels
						modify_seclevels();
						break;

					case 'q':
						save_seclevels();
						done = true;
				}

				options.clear();
			} while (!done);
		}
	}

	void display_title()
	{
		std::cout
			<< "| Password Manager" << std::endl
			<< "| by Justin Elli" << std::endl
			<< "| " << VERSION << std::endl << std::endl;

		std::cout << "NOTE: DO NOT INPUT SENSITIVE INFORMATION INTO THIS ONLINE APPLICATION." << std::endl
			<< "This application does not meet cryptographic standards and therefore should only be used for demonstration purposes." << std::endl << std::endl;
	}

	void display_main_menu()
	{
		std::cout
			<< "Open File (o)" << std::endl
			<< "Set Program Key (k)" << std::endl
			<< "Update Security Levels (l)" << std::endl
			<< "Help (h)" << std::endl
			<< "Quit (q)" << std::endl << std::endl;
	}

	void display_file_menu()
	{
		std::cout
			<< "Add Credentials (a)" << std::endl
			<< "Delete Credentials (d)" << std::endl
			<< "Modify Credentials (m)" << std::endl
			<< "Print Credentials (p)" << std::endl
			<< "Save and Exit (q)" << std::endl << std::endl;
	}

	void display_modify_credentials_menu()
	{
		std::cout
			<< "Change Name (n)" << std::endl
			<< "Change Username (u)" << std::endl
			<< "Change Password (p)" << std::endl
			<< "Change Security Level (l)" << std::endl
			<< "Edit Secret Questions (s)" << std::endl
			<< "Edit Backup Codes (b)" << std::endl
			<< "Finish (q)" << std::endl << std::endl;
	}

	void display_update_seclevels_menu()
	{
		std::cout
			<< "Add Security Level (a)" << std::endl
			<< "Delete Security Levels (d)" << std::endl
			<< "Modify Security Levels (m)" << std::endl
			<< "Save and Exit (q)" << std::endl << std::endl;
	}

	void display_modify_seclevel_menu()
	{
		std::cout
			<< "Change Password (p)" << std::endl
			<< "Change Update Period (m)" << std::endl
			<< "Change Update Time (u)" << std::endl
			<< "Finish (q)" << std::endl << std::endl;
	}

	void display_help()
	{
		std::ifstream readme_file("readme.md");

		if (readme_file.is_open())
			std::cout << std::endl << readme_file.rdbuf() << std::endl;
	}

	/*
		Prompt the user to add a set (or multiple sets) of credentials
	*/
	void add_credentials()
	{
		if (session)
		{
			std::string response;
			std::vector<std::string> options;
			seclevel_t* seclevel = nullptr;
			bool no_seclevel = false;

			do
			{
				response = get("Enter site name, or press <Enter> to cancel: ");
				if (!response.empty())
					options.push_back(response);
				else
					break;

				options.push_back(get("Enter username: "));

				seclevel = get_seclevel();
				if (!seclevel || !seclevel->has_password())	// Password needed
				{
					options.push_back(get("Enter password: "));
					session->add_credentials(options.at(0), options.at(1), options.at(2), seclevel);
				}
				else	// Password determined by security level
					session->add_credentials(options.at(0), options.at(1), seclevel);

				options.clear();
			} while (confirm("Would you like to add more credentials?"));

			session->print_credentials();
		}
	}

	/*
		Prompt the user to delete a set (or multiple sets) of credentials
	*/
	void delete_credentials()
	{
		std::string response;

		do
		{
			response = get("Enter site name, or press <Enter> to cancel: ");
			if (response.empty())
				return;
			while (!response.empty() && session->is_end(session->find_credentials(response)))
				response = get("Site name not found. Please enter a valid site name, or press <Enter> to cancel: ");

			if (confirm_deletion((*(session->find_credentials(response)))->get_name()))	// Not the most efficient implementation, but it'll do
				session->delete_credentials(response);
		} while (confirm("Would you like to delete more credentials?"));
	}

	/*
		Prompt the user to modify a set (or multiple sets) of credentials
	*/
	void modify_credentials()
	{
		if (session)
		{
			std::string response;
			std::vector<std::string> options;
			bool done;
			std::string name, value;
			std::vector<credentials_t*>::iterator credentials_ptr;
			seclevel_t* seclevel;
			std::vector<std::pair<std::string, std::string>> secret_questions;
			std::vector<std::string> backup_codes;

			do
			{
				name = get("Enter site name, or press <Enter> to cancel: ");
				if (name.empty())	// Cancel if user pressed <Enter> without entering any characters
					return;
				while (session->is_end(credentials_ptr = session->find_credentials(name)))
					name = get("Credentials not found. Please enter a valid site name: ");

				(*credentials_ptr)->print(std::cout, session->get_crypt_key());	// Print credential information

				done = false;
				do
				{
					display_modify_credentials_menu();

					do
					{
						response = get("Enter command: ");
					} while (response.empty());

					value = std::string();

					std::string filename_response;
					switch (std::tolower(response[0]))
					{
						case 'n':	// Site name
							value = get("Enter the new site name: ");
						case 'u':	// Username
							if (value.empty())	// Ensure value is only queried once
								value = get("Enter the new username: ");
						case 'p':	// Password
							if (value.empty())
								value = get("Enter the new password: ");
							session->modify_credentials(name, response, value);	// All of n, u, and p will run this line
							break;
						case 'l':	// Security level
							seclevel = get_seclevel();
							if (seclevel && confirm("Would you like to set the password to that defined by the security level?"))
								session->modify_credentials(name, "p", seclevel->get_password(session->get_crypt_key()));
							session->set_security_level(name, seclevel);
							break;

						case 's':	// Secret questions
							session->print_questions(name);

							response = get("Enter the index of the secret question you would like to delete, or enter \"a\" to add secret questions: ");
							if (std::tolower(response[0]) == 'a')
							{
								do
								{
									options.push_back(get("Enter question: "));
									options.push_back(get("Enter answer: "));
									secret_questions.push_back(std::make_pair(options.at(0), options.at(1)));
									options.clear();
								} while (confirm("Would you like to add more secret questions?"));

								session->add_questions(name, secret_questions);
								secret_questions.clear();
							}
							else if (response[0] >= '0' && response[0] <= '9')
								if (session->delete_question(name, std::stoi(response) - 1))
									std::cout << "Secret question deleted successfully" << std::endl;
								else
									std::cout << "Error deleting secret question" << std::endl;

							break;

						case 'b':	// Backup codes
							session->print_backups(name);

							response = get("Enter the index of the backup code you would like to delete, or enter \"a\" to add backup codes: ");
							if (std::tolower(response[0]) == 'a')
							{
								do
								{
									backup_codes.push_back(get("Enter code: "));
								} while (confirm("Would you like to add more backup codes?"));

								session->add_backups(name, backup_codes);
								backup_codes.clear();
							}
							else if (response[0] >= '0' && response[0] <= '9')
								if (session->delete_backup(name, std::stoi(response) - 1))
									std::cout << "Backup code deleted successfully" << std::endl;
								else
									std::cout << "Error deleting backup code" << std::endl;

							break;

						case 'q':
							done = true;
					}
				} while (!done);

				session->print_credentials();
			} while (confirm("Would you like to modify another set of credentials?"));
		}
	}

	/*
		Prompt the user to input a security level and check against existing security levels
	*/
	seclevel_t* get_seclevel()
	{
		seclevel_t* out = nullptr;
		std::string response;
		bool no_seclevel = false;

		response = get("Enter the security level code, or press <Enter> for no security level: ");
		if (!response.empty())
			out = session->find_seclevel(response);
		else	// Denote no security level if the user pressed <Enter> without entering any characters
			no_seclevel = true;
		while (!out && !no_seclevel)	// Repeat until a valid security level code is entered or it is denoted that there should be none
		{
			response = get("Security level not found. Enter a valid security level code, or press <Enter> for no security level: ");
			if (!response.empty())
				out = session->find_seclevel(response);
			else	// Denote no security level if the user pressed <Enter> without entering any characters
				no_seclevel = true;
		}

		return out;
	}

	/*
		Prompt the user to add a security level
	*/
	void add_seclevel()
	{
		std::string response;
		std::vector<std::string> options;
		
		response = get("Enter the security level code, or press <Enter> to cancel: ");
		if (!response.empty())
			options.push_back(response);
		else	// Cancel if user pressed <Enter> without entering any characters
			return;

		response = get("Enter the security level password, or press <Enter> for no password: ");
		if (!response.empty())	// Only set the security level password if the user entered characters before pressing <Enter>
			options.push_back(response);

		options.push_back(get("Enter the number of months each new password should be valid: "));

		response = get("Enter the year of the next password update: ");
		while (std::stoi(response) < 2000)	// Ensure that the year entered is a valid year
		{
			response = get("Error. Please enter a valid year: ");
		}
		options.push_back(response);

		response = get("Enter the month of the next password update: ");
		while (!(std::stoi(response) > 0 && std::stoi(response) <= 12))	// Ensure that the month entered is a valid month
		{
			response = get("Error. Please enter a valid month: ");
		}
		options.push_back(response);

		response = get("Enter the day of the month of the next password update: ");
		while (!(std::stoi(response) > 0 && std::stoi(response) <= 31))	// Ensure that the day entered is a valid day of the month
		{
			response = get("Error. Please enter a valid day of the month: ");
		}
		options.push_back(response);

		if (options.size() == 6)	// Include password
			session->add_seclevel(options.at(0), options.at(1), std::stoi(options.at(2)), std::stoi(options.at(3)), std::stoi(options.at(4)), std::stoi(options.at(5)));

		if (options.size() == 5)	// No password
			session->add_seclevel(options.at(0), std::stoi(options.at(1)), std::stoi(options.at(2)), std::stoi(options.at(3)), std::stoi(options.at(4)));
	}

	/*
		Prompt the user to delete a security level (or multiple security levels)
	*/
	void delete_seclevels()
	{
		std::string response;
		do
		{
			response = get("Enter the security level code, or press <Enter> to cancel: ");
			if (!response.empty())
				if (session->delete_seclevel(response))
					std::cout << "Security level deleted successfully" << std::endl;
				else
					std::cout << "Error deleting security level" << std::endl;
		} while (!response.empty() && confirm("Would you like to delete more security levels?"));
	}

	/*
		Prompt the user to modify security-level information
	*/
	void modify_seclevels()
	{
		std::string response;
		std::string code;
		std::vector<std::string> options;
		seclevel_t* seclevel;
		bool done;

		do
		{
			code = get("Enter security-level code, or press <Enter> to cancel: ");
			if (code.empty())	// Cancel if user pressed <Enter> without entering any characters
				return;
			while (!(seclevel = session->find_seclevel(code)))
				code = get("Security level not found. Please enter a valid security-level code: ");

			seclevel->print_long(std::cout, session->get_crypt_key());	// Print security-level information

			done = false;

			do
			{
				display_modify_seclevel_menu();

				do
				{
					response = get("Enter command: ");
				} while (response.empty());

				switch (std::tolower(response[0]))
				{
					case 'p':	// Change password
						response = get("Enter the new password for " + code + ", or press <Enter> to clear the password: ");
						if (response.empty())	// No password
							session->clear_seclevel_password(code);
						else	// New password
							session->set_seclevel_password(code, response);
						break;

					case 'm':	// Change update period
						seclevel->set_months_valid(std::stoi(get("Enter the number of months each password for the " + code + " security level should be valid: ")));
						break;

					case 'u':	// Change update time
						response = get("Enter the year of the next password update: ");
						while (std::stoi(response) < 2000)	// Ensure that the year entered is a valid year
						{
							response = get("Error. Please enter a valid year: ");
						}
						options.push_back(response);

						response = get("Enter the month of the next password update: ");
						while (!(std::stoi(response) > 0 && std::stoi(response) <= 12))	// Ensure that the month entered is a valid month
						{
							response = get("Error. Please enter a valid month: ");
						}
						options.push_back(response);

						response = get("Enter the day of the month of the next password update: ");
						while (!(std::stoi(response) > 0 && std::stoi(response) <= 31))	// Ensure that the day entered is a valid day of the month
						{
							response = get("Error. Please enter a valid day of the month: ");
						}
						options.push_back(response);

						seclevel->set_update_time(std::stoi(options.at(0)), std::stoi(options.at(1)), std::stoi(options.at(2)));
						break;

					case 'q':
						done = true;

				}

				options.clear();
			} while (!done);
		} while (confirm("Would you like to modify more security levels?"));
	}

	/*
		Prompt the user to log in
	*/
	bool login()
	{
		std::string key = get("Enter current program key: ");

		if (session && !session->is_logged_in())	// Run this first so that the user has a chance to log in if the session is logged out
		{
			delete session;
			session = nullptr;
		}

		if (!session)
			session = new session_t(key, keystore_filename, seclevel_filename);	// No credentials filename specified here so it can be modified before opening

		if (session->is_logged_in())
		{
			std::cout << "Logged in successfully" << std::endl;
			return true;
		}
		else
		{
			std::cout << "Login attempt unsuccessful" << std::endl;
			if (confirm("Would you like to try again?"))
				return login();
			else
				return false;
		}
	}

	void save_credentials()
	{
		if (session && session->are_credentials_loaded())
			session->store_credentials(credentials_filename);
	}

	void save_seclevels()
	{
		if (session)
			session->store_seclevels();
	}
}