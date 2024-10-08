
#include "lexer.h"
#include <string.h>
#define MIN_ARGS 2
#define CMD_NAME 0
#define MAX_CMD_ARG_AMOUNT 3
#define UNSET -1


size_t DC(int prompt, size_t amount)
{
	static int _DC = 0;
	if (prompt == RESET) {
		_DC = 0;
		return _DC;
	}

	if (prompt == GET)
		return _DC;

	if (prompt == INCREMENT) {
		_DC += amount;
	}

	return _DC;
}

size_t IC(int prompt, size_t amount)
{
	static int _IC = 100;

	if (prompt == RESET) {
		_IC = 100;
		return _IC;
	}
	if (prompt == GET)
		return _IC;
	if (prompt == INCREMENT) {
		_IC += amount;
	}

	return _IC;
}

inst_table *lex(char *am_filename, char *as_filename, label_table *_label_table, keyword *keyword_table, int *syntax_error_count) {
	syntax_state *state = NULL;
	inst_table *_inst_table = NULL;
	FILE *file = NULL;


	file = my_fopen(am_filename, "r");
	if (file == NULL) {
		return NULL;
	}

	state = create_syntax_state();
	if (state == NULL) {
		fclose(file);
		return NULL;
	}

	state->am_filename = am_filename;
	state->as_filename = as_filename;
	state->k_table = keyword_table;
	state->l_table = _label_table;
	state->line_number = 0;

	/* Create an instance of an instruction table */
	if (create_instruction_table(&_inst_table) != success) {
		print_syntax_error(state, m3_inst_tab_creation);
		quit_lex(&state, &_inst_table, file);
		return NULL;
	}

	while (fgets(state->buffer, MAX_LINE_LENGTH, file)) { /* Read every line */
		if (create_instruction(&state->_inst) != success) {
			print_syntax_error(state, m4_inst_creation);
			quit_lex(&state, &_inst_table, file);;
			return NULL;
		}

		state->buffer_without_offset = state->buffer;

		/* Skip empty lines */
		if (is_empty_line(state->buffer)) {
			destroy_instruction(&state->_inst);
			reset_syntax_state(state);
			continue;
		}

		/* Remove leading spaces */
		state->buffer = trim_whitespace(state->buffer);

		/* If the line is not a comment, count it as an assembly line */
		state->line_number++;

		if (strncmp(state->buffer, ".entry", 6) == 0 || strncmp(state->buffer, ".extern", 7) == 0) {
			state->buffer = state->buffer_without_offset;
			destroy_instruction(&state->_inst);
			reset_syntax_state(state);
			continue;
		}
		/* Skip comment lines */
		if (state->buffer[0] == ';') {
			destroy_instruction(&state->_inst);
			reset_syntax_state(state);
			continue;
		}



		/* Insert the line number to the instruction */
		state->_inst->line_number = state->line_number;

		/* If the instruction starts with a label definition, skip it */
		skip_label_name(state, _label_table);


		/* Skip unnecessary spaces */
		state->buffer = trim_whitespace(state->buffer);

		/* Identify the command key from the keyword table */
		state->cmd_key = identify_command(state, _label_table, keyword_table);

		/* If the command does not exist, exit */
		if (state->cmd_key == UNDEFINED) {
			print_syntax_error(state, e1_undef_cmd);
			destroy_instruction(&(state->_inst));
			state->buffer = state->buffer_without_offset;
			state->buffer_without_offset = NULL;
			reset_syntax_state(state);
			(*syntax_error_count)++;
			continue;
		}
		/* If the command key is valid, assign the command key to the instruction */
		state->_inst->cmd_key = state->cmd_key;

		if (state->label_name_detected)
			state->_inst->label_key = state->label_key;

		if (generate_tokens(state, keyword_table, _label_table) != success) {
			destroy_instruction(&state->_inst);
			state->buffer = state->buffer_without_offset;
			state->buffer_without_offset = NULL;
			reset_syntax_state(state);
			(*syntax_error_count)++;
			continue;
		}

		/* Insert the instruction to the instruction table */
		if (insert_inst_to_table(_inst_table, state->_inst) != success) {
			print_system_error(NULL, state, m5_inst_insert);
			quit_lex(&state, &_inst_table, file);
			return NULL;
		}

		state->buffer = state->buffer_without_offset;

		reset_syntax_state(state);
	}

	if (assign_addresses(_inst_table, _label_table, keyword_table) != success) {
		printf("Failed to assign addresses to instructions.\n");
		quit_lex(&state, &_inst_table, file);
		return NULL;
	}

	fclose(file);
	state->buffer_without_offset = state->buffer;
	destroy_syntax_state(&state);

	_inst_table->IC = IC(GET, 0);
	_inst_table->DC = DC(GET, 0);

	return _inst_table;
}

status generate_tokens(syntax_state *state, keyword *_keyword_table, label_table *_label_table) {
	int cmd_index = 0;
	int command_length = 0;
	size_t i = 0;
	int _tok_amount_to_allocate = 0;

	if (state == NULL || state->buffer == NULL || state->_inst == NULL || _keyword_table == NULL || _label_table == NULL) {
		return success; /* No arguments to process */
	}

	/* Get the number representing the command in the instruction */
	cmd_index = command_number_by_key(_keyword_table, state->cmd_key);

	/* Check if the command exists */
	if (cmd_index == UNDEFINED_KEYWORD) {
		print_syntax_error(state, e1_undef_cmd);
		return failure;
	}


	/* Get the number of arguments required for the command */
	_tok_amount_to_allocate = get_command_argument_count(state->cmd_key);

	if (_tok_amount_to_allocate != UNKNOWN_NUMBER_OF_ARGUMENTS) {
		/* Allocate memory for the string tokens and for the binary words */
		for (i = 0;i < _tok_amount_to_allocate;i++) {
			if (create_empty_token(state->_inst) != success) {
				return failure;
			}
		}

		state->_inst->opcode = get_command_opcode(_keyword_table, state->cmd_key);
		if (state->_inst->opcode < 0 || state->_inst->opcode>15) {
			print_syntax_error(state, e9_inval_opcode);
			return failure;
		}

		state->_inst->bin_opcode = state->_inst->opcode;

	}
	else {
		for (i = 0;i < MIN_ARGS;i++) {
			if (create_empty_token(state->_inst) != success) {
				return failure;
			}
		}

	}

	/*Initiallize both addressing methods*/
	state->_inst->dest_addressing_method = NO_ADDRESSING_METHOD;
	state->_inst->src_addressing_method = NO_ADDRESSING_METHOD;

	state->_inst->num_words_to_generate = state->_inst->num_tokens;

	/* Get the length of the command */
	command_length = _keyword_table[cmd_index].length;

	/* Copy the command name to the first field of the instruction */
	strcpy(state->_inst->tokens[CMD_NAME], _keyword_table[cmd_index].name);

	/* Null terminate the command name */
	state->_inst->tokens[CMD_NAME][command_length] = '\0';

	/* Skip the command name in the buffer- point to the argument section */
	state->buffer += command_length;

	/* Skip any leading/trailing whitepace */
	state->buffer = trim_whitespace(state->buffer);


	/* Check if there is a comma between the command and the arguments */
	if (state->buffer && state->buffer[0] == ',') {
		print_syntax_error(state, e4_extra_comma);
		return failure;
	}

	/* Check if the command requires an unknown number of arguments */
	if (_tok_amount_to_allocate == UNKNOWN_NUMBER_OF_ARGUMENTS) { /* keep all the arguments in array[1] */

		return assign_data(state, _label_table, _keyword_table);
	}

	else return assign_args(state, _label_table, _keyword_table);

}

status assign_data(syntax_state *state, label_table *_label_table, keyword *keyword_table) {
	int command_key = state->cmd_key;
	char *line = state->buffer;
	char *temp = NULL;
	char *token = NULL;
	int char_indx = 0;
	int copy_indx = 0;

	/* Check if the command is .data, .string, .entry, or .extern */
	update_command(state, keyword_table, command_key);

	/* If the command is neither .data, .string, .entry, nor .extern, return an error */
	if (!state->is_data && !state->is_string) {
		print_syntax_error(state, e1_undef_cmd);
		return failure;
	}

	state->index = 0;
	state->_inst->num_words_to_generate = 0;

	/* Parse the instruction and call the appropriate function based on the command */
	do {

		if (state->is_data) {  /* Process .data command */
			state->_inst->is_dot_data = true;
			if (process_data_command(state, _label_table) == failure)
				return failure;
		}

		else if (state->is_string) { /* Process .string command */
			state->_inst->is_dot_string = true;
			if (process_string_command(state, _label_table) == failure)
				return failure;
		}

		/* Move the index to the next character */
		state->index++;
	} while (continue_reading(line, state));

	if (state->is_data) {
		temp = my_strdup(state->_inst->tokens[1]);
		token = strtok(temp, ",");

		while (token != NULL) {
			state->_inst->num_dot_data_members++;
			state->_inst->num_words_to_generate++;
			DC(INCREMENT, 1);
			IC(INCREMENT, 1);
			token = strtok(NULL, ", ");
		}

		free(temp);
		temp = NULL;

	}

	if (state->is_string) {
		temp = NULL;
		temp = my_strdup(state->_inst->tokens[1]);
		char_indx = 0;
		copy_indx = 1;
		while (temp != NULL && temp[copy_indx] != '\"' && temp[copy_indx] != '\0') {
			state->_inst->tokens[1][char_indx] = temp[copy_indx];
			state->_inst->num_dot_string_members++;
			state->_inst->num_words_to_generate++;
			DC(INCREMENT, 1);
			IC(INCREMENT, 1);
			copy_indx++;
			char_indx++;
		}

		state->_inst->tokens[1][char_indx] = '\0';
		state->_inst->num_dot_string_members++;
		state->_inst->num_words_to_generate++;
		DC(INCREMENT, 1);
		IC(INCREMENT, 1);
		free(temp);
		temp = NULL;
	}

	return success;
}

status assign_args(syntax_state *state, label_table *_label_table, keyword *keyword_table) {

	int arg_section_len = 0;
	int arg_len = 0;
	char *_instruction_args = NULL;
	addressing_method src_reg = NO_ADDRESSING_METHOD;
	addressing_method dest_reg = NO_ADDRESSING_METHOD;

	if (state == NULL || state->buffer == NULL || state->_inst == NULL || _label_table == NULL) {
		return success; /* No arguments to process */
	}

	arg_section_len = strlen(state->buffer);
	_instruction_args = state->buffer;



	/* Check each argument- look for syntax errors */
	for (state->arg_count = 1; state->arg_count < state->_inst->num_tokens; state->arg_count++) {
		state->index = 0;
		arg_len = 0;

		/* Check if the intruction terminated before any arguments arguments are found */
		if (_instruction_args && (_instruction_args[state->index] == '\0' || _instruction_args[state->index] == '\n')) {
			print_syntax_error(state, e5_missing_args);
			return failure;
		}

		state->comma = false;
		state->null_terminator = false;
		state->new_line = false;
		state->whitespace = false;


		do {

			if (_instruction_args) {
				state->comma = _instruction_args[state->index] == ',';
				state->null_terminator = _instruction_args[state->index] == '\0';
				state->new_line = _instruction_args[state->index] == '\n';
				state->whitespace = isspace(_instruction_args[state->index]);

				state->continue_reading = !(state->comma || state->null_terminator || state->new_line || state->whitespace);

			}
			else {
				break;
			}

			if (state->continue_reading == false) {
				break;
			}

			state->_inst->tokens[state->arg_count][arg_len] = _instruction_args[state->index];

			arg_len++;
			state->index++;

		} while (_instruction_args && state->continue_reading && (arg_len < arg_section_len));

		/* Null terminate the copied arg field */
		state->_inst->tokens[state->arg_count][arg_len] = '\0';

		/* Point to the first character after the argument we just read */
		_instruction_args += arg_len;
		_instruction_args = trim_whitespace(_instruction_args);

		state->null_terminator = (*_instruction_args) == '\0';
		state->new_line = (*_instruction_args) == '\n';

		/* If we reached the end of the argument section, break out of the loop */
		if (state->null_terminator || state->new_line) {
			if (state->arg_count < (state->_inst->num_tokens - 1)) {
				print_syntax_error(state, e5_missing_args);
				return failure;
			}
		}

		if (assign_addressing_method(state, state->_inst->tokens[state->arg_count], _label_table, keyword_table) != success) {
			return failure;
		}

		/* Check if there is a comma after the last argument */
		if ((state->null_terminator == false) && _instruction_args && *(_instruction_args) == ',') {
			state->comma = true;

			/* Skip the comma */
			_instruction_args++;

			/* Skip any leading/trailing whitespace */
			_instruction_args = trim_whitespace(_instruction_args);
		}

		/*Check if there are multiple consecutive commas*/
		if ((state->comma == true) && _instruction_args && *(_instruction_args) == ',') {
			print_syntax_error(state, e10_series_of_commas);
			return failure;
		}

		/* If there is no comma after the last argument check if there are any more arguments */
		if ((state->comma == true) && (state->null_terminator || state->new_line) && state->arg_count == (state->_inst->num_tokens - 1)) {
			/* If the argument count is less than the number of expected arguments, there is a missing comma */
			print_syntax_error(state, e11_missing_comma);
			return failure;
		}

	}


	src_reg = state->_inst->src_addressing_method;
	dest_reg = state->_inst->dest_addressing_method;
	if (state->_inst->num_tokens == MAX_CMD_ARG_AMOUNT) {
		if (src_reg == DIRECT_REGISTER || src_reg == INDIRECT_REGISTER)
			if (dest_reg == DIRECT_REGISTER || dest_reg == INDIRECT_REGISTER)
				state->_inst->num_words_to_generate = 2;
	}

	IC(INCREMENT, state->_inst->num_words_to_generate);



	/*
	If we reached this point, all arguments for the specific command (i.e operation) have been successfully processed.
	Check if there are any additional arguments after the last one
	*/
	_instruction_args = trim_whitespace(_instruction_args);

	if (_instruction_args == NULL) {
		return success;
	}

	if (_instruction_args[0] != '\0' && _instruction_args[0] != '\n') {
		print_syntax_error(state, e12_too_much_args);
		return failure;
	}

	state->comma = *(_instruction_args) == ',';
	if (state->comma == true) {
		print_syntax_error(state, e13_comma_after_last_arg);
		return failure;
	}

	return success;
}

status process_data_command(syntax_state *state, label_table *_label_table) {
	int i = state->index;
	char *line = state->buffer;
	size_t next = state->_inst->num_tokens - 1;

	if (isspace(line[i]) && !(state->comma)) {
		state->end_of_argument_by_space = true;
		i++;
	}

	trim_whitespace(&line[i]);
	/* 	if (line[0] == ',') {
			print_syntax_error(state, e14_comma_before_data);
			return failure;
		} */
	if (!(isdigit(line[i]) || line[i] == '-' || line[i] == '+' || line[i] == ',' || line[i] == '\0' || line[i] == '\n' || isspace(line[i]))) {
		print_syntax_error(state, e15_inval_data_symbol);
		return failure;
	}
	if (state->end_of_argument_by_space && line[i] != ',' && line[i] != '\0' && line[i] != '\n') {
		print_syntax_error(state, e16_no_comma_betw_ints_data);
		return failure;
	}
	if (state->comma && line[i] == ',') {
		print_syntax_error(state, e17_extre_comma_data_mid);
		return failure;
	}

	state->_inst->tokens[next][i] = line[i];
	state->_inst->tokens[next][i + 1] = '\0';

	if (line[i] == ',') {
		state->comma = true;
	}
	else {
		state->end_of_argument_by_space = state->comma = false;
	}
	if (line[i] == '\n' || line[i + 1] == '\n' || line[i] == '\0' || line[i + 1] == '\0') {
		if (state->comma) {
			print_syntax_error(state, e18_extra_comma_data_end);
			return failure;
		}
		if (validate_data_members(state) == failure) {
			return failure;
		}
	}


	return success;
}

status process_string_command(syntax_state *state, label_table *_label_table) {
	int i = state->index;
	int token_index = 1;
	char *line = state->buffer;
	if (i == 0) {
		if (line[0] == '\"') {
			state->first_quatiotion_mark = true;
			state->_inst->tokens[token_index][i] = line[i];
		}
		return success;
	}
	if (line[i] == '\"') {
		state->last_quatiotion_mark = true;
	}
	else {
		state->last_quatiotion_mark = false;
	}


	state->_inst->tokens[token_index][i] = line[i];

	if (line[i + 1] == '\n' || line[i + 1] == '\0') {
		if (!state->first_quatiotion_mark && !state->last_quatiotion_mark) {
			print_syntax_error(state, e21_str_no_qm);
			return failure;
		}
		else if (!state->first_quatiotion_mark && state->last_quatiotion_mark) {
			print_syntax_error(state, e22_str_no_qm_start);
			return failure;
		}
		if (state->first_quatiotion_mark && !state->last_quatiotion_mark) {
			print_syntax_error(state, e23_str_no_qm_end);
			return failure;
		}
		{
			while (i > 0 && state->_inst->tokens[token_index][i] != '\"') {
				i--;
			}
		}
	}

	return success;
}

status assign_addressing_method(syntax_state *state, char *argument, label_table *_label_table, keyword *keyword_table) {
	size_t i;
	addressing_method _addressing_method = UNDEFINED_METHOD;
	keyword_name command = UNDEFINED_KEYWORD;
	int contains_src_addressing = false;
	int contains_dest_addressing = false;
	int found_label_with_matching_name = false;
	label *tmp_label = NULL;
	char *tmp_ptr = NULL;
	int tmp_val = 0;;

	if (state->_inst->src_addressing_method != NO_ADDRESSING_METHOD) contains_src_addressing = true;
	if (state->_inst->dest_addressing_method != NO_ADDRESSING_METHOD) contains_dest_addressing = true;

	if (contains_dest_addressing && contains_src_addressing) {
		print_syntax_error(state, e26_third_assignment);
		return failure;
	}

	state->tmp_arg = argument;
	_addressing_method = get_addressing_method(state, argument, _label_table);

	if (!(_addressing_method == IMMEDIATE || _addressing_method == DIRECT || _addressing_method == INDIRECT_REGISTER || _addressing_method == DIRECT_REGISTER)) {
		return failure;
	}

	/*Get the number that represents the command in the current instruction*/
	command = command_number_by_key(keyword_table, state->cmd_key);

	switch (command) {
	case MOV:
	case ADD:
	case SUB:
		if (contains_src_addressing) {/*Check dest addressing */
			if (_addressing_method == IMMEDIATE) {

				if (command == MOV)
					print_syntax_error(state, ex28_inval_method_mov);
				else if (command == ADD)
					print_syntax_error(state, ex28_inval_method_add);
				else if (command == SUB)
					print_syntax_error(state, ex28_inval_method_sub);

				return failure;
			}

			/*If the addressing method is valid for the specific command, assign it to the instruction */
			state->_inst->dest_addressing_method = _addressing_method;
			break;
		}
		if (contains_dest_addressing) {
			print_syntax_error(state, e26_third_assignment);
			return failure;
		}
		state->_inst->src_addressing_method = _addressing_method;
		break;
	case CMP:
		if (contains_src_addressing) {
			state->_inst->dest_addressing_method = _addressing_method;
			break;
		}

		if (contains_dest_addressing) {
			print_syntax_error(state, e39_cmp_extra_args);
			return failure;
		}

		state->_inst->src_addressing_method = _addressing_method;
		break;

	case LEA:
		if (!contains_src_addressing) {
			if (_addressing_method != DIRECT) {
				print_syntax_error(state, e7_lea_nondir_src);
				return failure;
			}

			/*If the addressing method is valid for the specific command, assign it to the instruction */
			state->_inst->src_addressing_method = _addressing_method;
			break;
		}

		if (!contains_dest_addressing) {
			if (_addressing_method == IMMEDIATE) {
				print_syntax_error(state, e29_inval_method_imm_lea);
				return failure;
			}
			/*If we got here it means that the argument is a dest argument with a valid addressing method*/
			state->_inst->dest_addressing_method = _addressing_method;
			break;
		}

	case CLR:
		if (contains_src_addressing) {
			print_syntax_error(state, ex30_ext_arg_clr);
			return failure;
		}
		if (contains_dest_addressing) {
			print_syntax_error(state, ex31_mult_assign_clr);
			return failure;
		}

		if (_addressing_method == IMMEDIATE) {
			print_syntax_error(state, ex32_imm_clr);
			return failure;
		}
		/*If we got here it means that the argument is a destination argument with a valid addressing method*/
		state->_inst->dest_addressing_method = _addressing_method;
		break;

	case NOT:
		if (contains_src_addressing) {
			print_syntax_error(state, ex30_ext_arg_not);
			return failure;
		}
		if (contains_dest_addressing) {
			print_syntax_error(state, ex31_mult_assign_not);
			return failure;
		}

		if (_addressing_method == IMMEDIATE) {
			print_syntax_error(state, ex32_imm_not);
			return failure;
		}

		/*If we got here it means that the argument is a destination argument with a valid addressing method*/
		state->_inst->dest_addressing_method = _addressing_method;
		break;

	case INC:
		if (contains_src_addressing) {
			print_syntax_error(state, ex30_ext_arg_inc);
			return failure;
		}
		if (contains_dest_addressing) {
			print_syntax_error(state, ex31_mult_assign_inc);
			return failure;
		}

		if (_addressing_method == IMMEDIATE) {
			print_syntax_error(state, ex32_imm_inc);
			return failure;
		}


		/*If we got here it means that the argument is a destination argument with a valid addressing method*/
		state->_inst->dest_addressing_method = _addressing_method;
		break;

	case DEC:
		if (contains_src_addressing) {
			print_syntax_error(state, ex30_ext_arg_dec);
			return failure;
		}
		if (contains_dest_addressing) {
			print_syntax_error(state, ex31_mult_assign_dec);
			return failure;
		}

		if (_addressing_method == IMMEDIATE) {
			print_syntax_error(state, ex32_imm_dec);
			return failure;
		}

		/*If we got here it means that the argument is a destination argument with a valid addressing method*/
		state->_inst->dest_addressing_method = _addressing_method;
		break;

	case RED:
		if (contains_src_addressing) {
			print_syntax_error(state, ex30_ext_arg_red);
			return failure;
		}

		if (contains_dest_addressing) {
			print_syntax_error(state, ex31_mult_assign_red);
			return failure;
		}

		if (_addressing_method == IMMEDIATE) {
			print_syntax_error(state, ex32_imm_red);
			return failure;
		}

		/*If we got here it means that the argument is a destination argument with a valid addressing method*/
		state->_inst->dest_addressing_method = _addressing_method;
		break;

	case JMP:
	case BNE:
	case JSR:
		if (contains_src_addressing) {
			if (command == JMP) {
				print_syntax_error(state, ex33_toomany_jmp);
				return failure;
			}
			if (command == BNE) {
				print_syntax_error(state, ex33_toomany_bne);
				return failure;
			}
			if (command == JSR) {
				print_syntax_error(state, ex33_toomany_jsr);
				return failure;
			}
		}

		if (contains_dest_addressing) {

			if (command == JMP) {
				print_syntax_error(state, ex34_mul_assign_jmp);
				return failure;
			}

			if (command == BNE) {
				print_syntax_error(state, ex34_mul_assign_bne);
				return failure;
			}

			if (command == JSR) {
				print_syntax_error(state, ex34_mul_assign_jsr);
				return failure;
			}

		}

		if (_addressing_method == IMMEDIATE) {


			if (command == JMP) {
				print_syntax_error(state, ex35_imm_jmp);
				return failure;
			}

			if (command == BNE) {
				print_syntax_error(state, ex35_imm_bne);
				return failure;
			}

			if (command == JSR) {
				print_syntax_error(state, ex35_imm_jsr);
				return failure;
			}

		}

		if (_addressing_method == DIRECT_REGISTER) {

			if (command == JMP) {
				print_syntax_error(state, ex36_dir_jmp);
				return failure;
			}

			if (command == BNE) {
				print_syntax_error(state, ex36_dir_bne);
				return failure;
			}

			if (command == JSR) {
				print_syntax_error(state, ex36_dir_jsr);
				return failure;
			}
		}

		state->_inst->dest_addressing_method = _addressing_method;
		break;

	case PRN:
		if (contains_src_addressing) {
			print_syntax_error(state, e37_toomany_prn);
			return failure;
		}

		if (contains_dest_addressing) {
			print_syntax_error(state, e38_addr_mult_assign_prn);
			return failure;
		}
		state->_inst->dest_addressing_method = _addressing_method;
		break;
	case RTS:
		if (contains_src_addressing || contains_dest_addressing) {
			print_syntax_error(state, ex40_toomany_rts);
			return failure;
		}
		break;
	case STOP:
		if (contains_src_addressing || contains_dest_addressing) {
			print_syntax_error(state, ex40_toomany_stop);
			return failure;
		}

		break;

	default:
		printf("Could not assign addressing method to arguments\n");
		return failure;
	}

	if (_addressing_method == IMMEDIATE) {
		tmp_ptr = strchr(argument, '#');
		tmp_val = atoi(tmp_ptr + 1);

		switch (command) {
		case PRN:
			if (state->_inst->immediate_val_dest != UNDEFINED) {
				break;
			}
			state->_inst->immediate_val_dest = tmp_val;
			break;

		case MOV:
		case ADD:
		case SUB:
			if (state->_inst->immediate_val_src != UNDEFINED) {
				break;
			}
			state->_inst->immediate_val_src = tmp_val;
			break;

		case CMP:
			if (state->_inst->src_addressing_method != IMMEDIATE && state->_inst->dest_addressing_method == IMMEDIATE) {
				if (state->_inst->immediate_val_dest == UNDEFINED) {
					state->_inst->immediate_val_dest = tmp_val;
				}
				break;
			}

			if (state->_inst->src_addressing_method == IMMEDIATE && state->_inst->dest_addressing_method != IMMEDIATE) {
				if (state->_inst->immediate_val_src == UNDEFINED) {
					state->_inst->immediate_val_src = tmp_val;
				}
				break;
			}

			if ((state->_inst->src_addressing_method == IMMEDIATE && state->_inst->dest_addressing_method == IMMEDIATE)) {
				if (state->_inst->immediate_val_src == UNDEFINED) {
					state->_inst->immediate_val_src = tmp_val;
					break;
				}

				if (state->_inst->immediate_val_dest == UNDEFINED) {
					state->_inst->immediate_val_dest = tmp_val;
					break;
				}
			}

		default:
			tmp_ptr = NULL;
			tmp_val = 0;
		}

	}

	if (_addressing_method == DIRECT) {
		for (i = 0;i < _label_table->size;i++) {
			tmp_label = get_label_by_name(_label_table, argument);
			if (tmp_label != NULL) {
				/* Check if the label is declared as .entry but not defined */

				if (tmp_label->declared_as_entry && tmp_label->missing_definition) {
					state->tmp_arg = argument;
					print_syntax_error(state, e67_using_undefined_label);
					return failure;
				}

				found_label_with_matching_name = true;
				break;
			}
		}

		if (found_label_with_matching_name) {

			/* Copy the label name that is used by the instruction */
			if (state->_inst->src_addressing_method == DIRECT) {
				if (state->_inst->direct_label_key_src == -1) {
					state->_inst->direct_label_key_src = tmp_label->key;
					strcpy(state->_inst->direct_label_name_src, tmp_label->name);
					if (tmp_label->declared_as_entry) state->_inst->is_src_entry = true;
					else if (tmp_label->declared_as_extern) state->_inst->is_src_extern = true;
				}
			}
			if (state->_inst->dest_addressing_method == DIRECT) {
				if (state->_inst->direct_label_key_dest == -1) {
					state->_inst->direct_label_key_dest = tmp_label->key;
					strcpy(state->_inst->direct_label_name_dest, tmp_label->name);
					if (tmp_label->declared_as_entry) state->_inst->is_dest_entry = true;
					else if (tmp_label->declared_as_extern) state->_inst->is_dest_extern = true;
				}
			}

		}
	}

	if (_addressing_method == INDIRECT_REGISTER) {
		tmp_ptr = strstr(argument, "r") + 1;
		tmp_val = atoi(tmp_ptr);


		switch (command) {
		case MOV:
		case CMP:
		case ADD:
		case SUB:
			if (state->_inst->src_addressing_method != INDIRECT_REGISTER && state->_inst->dest_addressing_method == INDIRECT_REGISTER) {
				if (state->_inst->indirect_reg_num_dest == UNSET) {
					state->_inst->indirect_reg_num_dest = tmp_val;
				}
				break;
			}

			if (state->_inst->src_addressing_method == INDIRECT_REGISTER && state->_inst->dest_addressing_method != INDIRECT_REGISTER) {
				if (state->_inst->indirect_reg_num_src == UNSET) {
					state->_inst->indirect_reg_num_src = tmp_val;
				}
				break;
			}

			if ((state->_inst->src_addressing_method == INDIRECT_REGISTER && state->_inst->dest_addressing_method == INDIRECT_REGISTER)) {
				if (state->_inst->indirect_reg_num_src == UNSET) {
					state->_inst->indirect_reg_num_src = tmp_val;
					break;
				}

				if (state->_inst->indirect_reg_num_dest == UNSET) {
					state->_inst->indirect_reg_num_dest = tmp_val;
					break;
				}
			}


		case LEA:
		case CLR:
		case NOT:
		case INC:
		case DEC:
		case JMP:
		case BNE:
		case RED:
		case PRN:
		case JSR:
			if (state->_inst->indirect_reg_num_dest == UNDEFINED) {
				state->_inst->indirect_reg_num_dest = tmp_val;
			}
			break;
		default:
			break;
		}


	}

	if (_addressing_method == DIRECT_REGISTER) {
		tmp_ptr = NULL;
		tmp_val = UNSET;

		tmp_ptr = strchr(argument, 'r') + 1;
		tmp_val = atoi(tmp_ptr);

		switch (command) {
		case MOV:
		case CMP:
		case ADD:
		case SUB:
			if (state->_inst->src_addressing_method != DIRECT_REGISTER && state->_inst->dest_addressing_method == DIRECT_REGISTER) {
				if (state->_inst->direct_reg_num_dest == UNSET) {
					state->_inst->direct_reg_num_dest = tmp_val;
				}
				break;
			}

			if (state->_inst->src_addressing_method == DIRECT_REGISTER && state->_inst->dest_addressing_method != DIRECT_REGISTER) {
				if (state->_inst->direct_reg_num_src == UNSET) {
					state->_inst->direct_reg_num_src = tmp_val;
				}
				break;
			}

			if ((state->_inst->src_addressing_method == DIRECT_REGISTER && state->_inst->dest_addressing_method == DIRECT_REGISTER)) {
				if (state->_inst->direct_reg_num_src == UNSET) {
					state->_inst->direct_reg_num_src = tmp_val;
					break;
				}

				if (state->_inst->direct_reg_num_dest == UNSET) {
					state->_inst->direct_reg_num_dest = tmp_val;
					break;
				}

				break;
			}

		case LEA:
		case CLR:
		case NOT:
		case INC:
		case DEC:
		case RED:
		case PRN:
			if (state->_inst->direct_reg_num_dest == UNDEFINED) {
				state->_inst->direct_reg_num_dest = tmp_val;
			}
			break;


		default: break;
		}

	}
	return success;
}

status validate_data_members(syntax_state *state) {
	size_t i;
	int minus_or_plus = false;
	int number = false;
	char *buffer = state->_inst->tokens[1];


	for (i = 0; buffer[i] != '\0' && buffer[i] != '\n'; i++) {
		trim_whitespace(&buffer[i]);
		if (buffer[i] == '-' || buffer[i] == '+') {
			if (number == true && minus_or_plus == true) { /*in case of 1,2,3-,4 or 1,+-2,3,4*/
				print_syntax_error(state, e19_pm_wrong_position);
				return failure;
			}
			minus_or_plus = true;
			number = false;
		}
		else if (buffer[i] == ',') {
			if (minus_or_plus == true) { /*in case of 1,-,2,3 */
				print_syntax_error(state, e20_pm_no_int);
				return failure;
			}
			number = false;
			minus_or_plus = false;
		}
		else {   /*data_buffer[i] == number*/
			number = true;
			minus_or_plus = false;
		}
	}

	return success;
}

status assign_addresses(inst_table *_inst_table, label_table *_label_table, keyword *_keyword_table) {
	int initial_address = 100;
	int inst_index = 0;
	int words_generated = 0;
	int label_key = 0;
	label *tmp_label = NULL;
	inst *tmp_inst = NULL;

	if (_inst_table == NULL || _label_table == NULL || _keyword_table == NULL) {
		return failure;
	}



	while (inst_index < (_inst_table->num_instructions)) {
		if (_inst_table->inst_vec) {
			tmp_inst = _inst_table->inst_vec[inst_index];
			tmp_inst->address = initial_address + words_generated;
			if (tmp_inst->label_key != UNSET) {

				label_key = tmp_inst->label_key;
				tmp_label = get_label_by_key(_label_table, label_key);
				if (tmp_label != NULL) {
					tmp_label->address = tmp_inst->address;
				}
			}
			words_generated += tmp_inst->num_words_to_generate;
			tmp_inst->bin_address = tmp_inst->address;
			tmp_inst = NULL;
			tmp_label = NULL;
			inst_index++;
		}
	}


	for (inst_index = 0;inst_index < _inst_table->num_instructions;inst_index++) {
		tmp_inst = _inst_table->inst_vec[inst_index];
		if (tmp_inst->src_addressing_method == DIRECT) {
			label_key = tmp_inst->direct_label_key_src;
			tmp_label = get_label_by_key(_label_table, label_key);
			tmp_inst->direct_label_address_src = tmp_label->address;

			tmp_label = NULL;
			label_key = UNSET;
		}

		if (tmp_inst->dest_addressing_method == DIRECT) {
			label_key = tmp_inst->direct_label_key_dest;
			tmp_label = get_label_by_key(_label_table, label_key);
			tmp_inst->direct_label_address_dest = tmp_label->address;
			tmp_label = NULL;
			label_key = UNSET;
		}

	}

	return success;
}

