|------------------------------- Password Manager -------------------------------|
|-------------------------------- by Justin Elli --------------------------------|

Command-line command: passmngr

|----------------------------------- Options ------------------------------------|

[filename]
	Opens the specified filename for editing.
	
	Example:
		passmngr credentials.dat

--help
	Prints the readme.md file to the console.

--version
	Prints the program version to the console.

--key
	Update the program key.

--seclevels
	View/modify security levels.

-k	Key

	Specify the program key.

	Example:
	passmngr -k Pa55W0rd

-p	Print

	Print all credentials before exiting the program.

	Example:
	passmngr -k Pa55W0rd -p

-f	Filename

	Specify the filename to use for credential data.

	Example:
	passmngr -k Pa55W0rd -f credentials_new.dat

-s	Search

	Print credentials to site names that pattern-match the query.

	Example:
	passmngr -k Pa55W0rd -s SITE

-a	Add

	Add a set of credentials.

	Order: [Site Name] [Username] [Password]

	Example:
	passmngr -k Pa55W0rd -a SITE1 MyUsername MyPa55W0rd

-d	Delete

	Delete a set of credentials.

	Example:
	passmngr -k Pa55W0rd -d SITE1

-m	Modify

	Modify a set of credentials.

	Order: [Site Name] [Field] [Value]

	Examples:
	passmngr -k Pa55W0rd -m SITE1 n SITE2
	passmngr -k Pa55W0rd -m SITE1 u NewUsername
	passmngr -k Pa55W0rd -m SITE1 p NewPa55W0rd

-l	Security Level

	Set the security level on a specified number of credentials.
	
	Order: [Security Level Code] [Number of Sites to Modify] [Site 1]
			[Site 2] ...
	
	Example:
	passmngr -k Pa55W0rd -l BASIC 3 SITE1 SITE2 SITE3

-q	Secret Questions

	Add secret questions to, or delete them from, a set of credentials. Queries
	are run over the questions and not the answers.

	Order: [Site Name] [Number of Questions to Add] [Question 1] [Answer 1]
			[Question 2] [Answer 2]...
	OR [Site Name] -[Number of Questions to Delete] [Query 1] [Query 2] ...

	Examples:
	passmngr -k Pa55W0rd -q SITE1 2 "What is red?" Red "What is blue?" Blue
	passmngr -k Pa55W0rd -q SITE1 -2 "what is red" blue

-b	Backup Codes

	Add two-factor authentication backup codes to, or delete them from, a set of
	credentials.

	Order: [Site Name] [Number of Codes to Add] [Code 1] [Code 2] ...
	OR [Site Name] -[Number of Codes to Delete] [Query 1] [Query 2] ...

	Examples:
	passmngr -k Pa55W0rd -b SITE1 2 12345678 87654321
	passmngr -k Pa55W0rd -b SITE1 -1 12345678

-u	Update Master Key

	Change the program key.
	
	Example:
	passmngr -k Pa55W0rd -u NewPa55W0rd