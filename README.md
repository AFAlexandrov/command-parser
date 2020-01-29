# command-parser
Extensible parser of user-defined commands

* Commands are stored regardless of the parser code.
* Adding / changing / deleting commands or groups of commands without touching the code that executes it.
* Obtaining parameters for post-processing inside the command execution code or somewhere else.

## Commands example
		print voltage
		set brightness 123
		get time
		ble send message "ping"

## Test cases
* Builds with gnu make from recurcive_parser directory.
(on Windows you must previous install msys with mingw)

## Getting started
1. Create header file for your commands. Define max command length.

		#define MAX_CMD_LEN 30 // for example
2. Create command group. Assign the fields you need.

		#define SUBCMD_SET_NUM 2
		START_CMD_GROUP (set, SUBCMD_SET_NUM) {
			[0] = {
				.command = SUBCMD_SOME_PARAMETER,
				.subcommand = SUBCOMMAND(set_some_parameter)
			},
			[1] = {
				.command = SUBCMD_ANOTHER_PARAM,
			}
			END_CMD_GROUP(SUBCMD_SET_NUM)
		};
3. Create subcommand for command "set_some_parameter".

		#define SUBCMD_SET_SOME_PARAMETER_NUM 1
		START_CMD_GROUP (
			set_some_parameter, SUBCMD_SET_SOME_PARAMETER_NUM) {
			[0] = {
				.command = SOME_PARAMETER_NAME,
				.response =  RESP_SET_SOME_PARAMETER,
				.action = set_some_parameter_ParamName
			}
			END_CMD_GROUP_WITH_RESPONSE(
				SUBCMD_SET_SOME_PARAMETER_NUM,
				RESP_INVALID_PARAMETER)
		};
4. Add the definition of used actions at the top of the file.

		void set_some_parameter_ParamName (void);
5. Initialize parser by command group.

		parser_create(&set, MAX_CMD_LEN);
6. Parse user message.

		parse(user_message);
7. Get result.

		char * response = parser_response();
		char * parameter = parser_parameter();
		action_t action = parser_action();
Or execute action directly :)

		parser_action()();

## LICENSE
* This code is licensed under the GNU Lesser General Public License (LGPL) version 3 as published by the Free Software Foundation.
* It is possible to link closed-source code with the LGPL code.
* All code files contain licensing information.

## PS
Now (January 2020) it is very raw version. Code need great refactoring and documentation. 

[But it works and pass test cases :)] 

If you want to help in the development please write your recommendations on this mail: 

* feodor.alexandrov@yandex.ru 

Or use pull-requests.
