/*
 * Copyright © 2019-2020 Feodor A. Alexandrov (feodor.alexandrov@yandex.ru)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v3. See the file LICENSE in the top level
 * directory for more details.
 */

// #include "parser_internal.h"
#include "../inc/parser/parser_internal.h"

static void stub_action (void) {

}
action_t parser_stub_action (void) {
	return stub_action;
}

static dbase_record_t _empty_cmd_dbase = {
	.command = "emty_command_dbase",
	.response = PARSER_STUB_RESPONSE,
	.action = stub_action,
	.subcommand = NULL
};
dbase_record_t * parser_empty_cmd_dbase (void) {
	return &_empty_cmd_dbase;
}

static dbase_record_t * _command_dbase = NULL;
static int _pattern_max_len = 0;
static void (* _action) (void) = stub_action;
static char
	* _response = PARSER_STUB_RESPONSE,
	* _parameter = NULL;

void parser_set_pattern_max_len (int pattern_max_len) {
	_pattern_max_len = pattern_max_len;
}
void parser_set_command_dbase (dbase_record_t ** cmd_dbase) {
	_command_dbase = (dbase_record_t *)cmd_dbase;
}
void parser_set_response (char * response) {
	_response = response;
}
void parser_set_action (void (* action)(void)) {
	_action = action;
}
void parser_set_parameter (char * parameter) {
	_parameter = parameter;
}

int parser_pattern_max_len (void) {
	return _pattern_max_len;
}
dbase_record_t * parser_command_dbase (void) {
	return _command_dbase;
}
char * parser_response (void) {
	return _response;
}
action_t parser_action (void) {
	return _action;
}
char * parser_parameter (void) {
	return _parameter;
}

dbase_record_t * _parser (const char * message) {
	if (parser_command_dbase() == NULL)
		return parser_empty_cmd_dbase();

	dbase_record_t * found_record = find_cmd(message);

	if (found_record != NULL) {
		return found_record;
	}
	return parser_not_found(); // must never occure
}

//---find---// TODO: extract to independend code

#include <string.h>
#include <stdint.h>	
#include <stdbool.h>

bool is_delimeter (char symbol) {
	return (symbol == ' ' || symbol == '\t' || symbol == '\0');
}

#define EXTRACT_MSG_MAX_SIZE 100 //parser_pattern_max_len
static inline bool extraction_possible (char * message, uint16_t from_here, uint16_t cmd_len) {
	register int msg_len = strlen(message);
	return from_here < msg_len &&
		cmd_len <= msg_len &&
		cmd_len < EXTRACT_MSG_MAX_SIZE &&
		is_delimeter(message[from_here + cmd_len])
	;
}

char * extract_pattern (char * message, uint16_t from_here, uint16_t cmd_len) {
	static char pattern [EXTRACT_MSG_MAX_SIZE] = {0};
	*pattern = '\0';

	if (extraction_possible(message, from_here, cmd_len)) {
		strncpy(pattern, &message[from_here], cmd_len);
		pattern[from_here + cmd_len] = '\0';
	}

	return pattern;
}

char * free_spaces (char * source) {
    char * result = source;
    while (*result == ' ' || *result == '\t')
    	result++;
    return result;
}

static inline char * extract_pattern_from_start (char * string, uint16_t pattern_len) {
	return extract_pattern(string, 0, pattern_len);
}

#include <stdbool.h>

static inline bool pattern_matched (char * pattern, char * template) {
	return !strcmp(pattern, template);
}

static inline bool message_payload_continue (char * message) {
	return strlen(free_spaces(message));
}

dbase_record_t * dbase_table_find (dbase_table_t table, char * message) {
	static uint16_t record;	
	static bool not_found;

	not_found = true;
	record = 0;

	while (table[record].command) {
		char * command = table[record].command;
		int cmd_len = strlen(command);

		char
			* msg_after_spaces = free_spaces(message),
			* pattern = extract_pattern_from_start(msg_after_spaces, cmd_len);

		if (pattern_matched(pattern, command)) {
			dbase_record_t * sub_table = table[record].subcommand;
			register char * msg_after_pattern = msg_after_spaces + cmd_len;

			if (sub_table || message_payload_continue(msg_after_pattern)) {
				if (sub_table == NULL) {
					 table[record].parameter = msg_after_pattern;
					 return &table[record];
				}
				return dbase_table_find(sub_table, msg_after_pattern);
			}
			else { // sub-pattern matched (record was found)
				not_found = false;
				break;
			}
		}
		else if (not_found) { // look at next record
			record++;
		}
	}
	return &table[record];
}

dbase_record_t * find_cmd (const char * message) {
	static char msg [PARSER_PATTERN_MAX_LEN_DEFAULT] = {0};

	uint16_t len = strlen(message);
	strncpy(msg, message, len);
	if (len < PARSER_PATTERN_MAX_LEN_DEFAULT) {
		msg[len] = '\0';
	}

	return dbase_table_find(parser_command_dbase(), msg);
}
