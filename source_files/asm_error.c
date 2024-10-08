#include "asm_error.h"

void reset_main(int file_amount, filenames **fnames, macro_table **_macro_table, keyword **keyword_table, label_table **_label_table, inst_table **_instruction_table) {
   quit_filename_creation(fnames);
   destroy_macro_table(_macro_table);
   destroy_keyword_table(keyword_table);
   destroy_label_table(_label_table);
   destroy_instruction_table(_instruction_table);
}

void quit_filename_creation(filenames **fnames) {
   int i;
   filenames *names = NULL;
   if (fnames == NULL || (*fnames == NULL) || (*fnames)->amount == 0)
      return; /* Memory already freed */

   names = (*fnames);
   if (names->am) {
      for (i = 0;i < (*fnames)->amount;i++) {
         free(names->am[i]);
      }

      free(names->am);
      names->am = NULL;
   }

   if (names->as) {
      for (i = 0;i < (*fnames)->amount;i++) {
         free(names->as[i]);
      }

      free(names->as);
      names->as = NULL;
   }

   if (names->generic) {
      for (i = 0;i < (*fnames)->amount;i++) {
         free(names->generic[i]);
      }

      free(names->generic);
      names->generic = NULL;
   }

   if (names->errors) {
      free(names->errors);
      names->errors = NULL;
   }


   free(names);
   (*fnames) = NULL;

}

void quit_lex(syntax_state **state, inst_table **_inst_table, FILE *am_file_ptr) {
   destroy_syntax_state(state);
   destroy_instruction_table(_inst_table);
   if (am_file_ptr) fclose(am_file_ptr);
}

void quit_pre_assembler(syntax_state **state, macro_table **_macro_table, FILE *am_file_ptr, FILE *as_file_ptr) {
   destroy_syntax_state(state);
   destroy_macro_table(_macro_table);
   if (am_file_ptr) fclose(am_file_ptr);
   if (as_file_ptr) fclose(as_file_ptr);
}

void quit_label_parsing(label_table **_label_table, syntax_state **state, FILE *am_file_ptr) {
   destroy_label_table(_label_table);
   destroy_syntax_state(state);
   if (am_file_ptr) fclose(am_file_ptr);
}


void print_system_error(system_state *sys_state, syntax_state *syn_state, error_code e_code) {

   switch (e_code) {

      /* Memory handling errors */
   case m1_general_memerr:
      printf("Failed to allocate memory in assembly process.\n");
      return;
   case m2_syntax_state:
      printf("Failed to allocate memory for syntax state structre.\n");
      return;
   case m3_inst_tab_creation:
      printf("Failed to allocate memory for the instruction table structre.\n");
      return;
   case m5_inst_insert:
      if (syn_state)
         printf("Error in file %s: line %d: '%s' - ", syn_state->as_filename, syn_state->line_number, syn_state->buffer_without_offset);
      printf("Failed to insert instruction to instruction table.\n");
      return;
   case m6_tok_gen_mem:
      printf("Failed to allocate memory for tokens while lexing instruction.\n");
      return;

   case m7_generic_creation:

      printf("Failed to duplicte original filename recieved as command line argument: '%s'\n", sys_state->generic_filename);
      return;


   case m9_create_sys_state:
      printf("In 'file_util.c' -> 'create_system_state': ");
      printf("Failed to allocate memory for syntax state");
      return;


   case m10_create_backup_filename:
      printf("In 'file_util.c' -> 'dup;icate_filenames': ");
      printf("Memory allocation error while creating a backup '%s' filename for '%s'\n", sys_state->tmp, sys_state->generic_filename);
      return;

   case m11_create_keyword_table:
      printf("In 'data_structs.c' -> 'create_keyword_table': ");
      printf("Failed to allocate memory for keyword table\n");
      return;

   case m12_create_label_table:
      printf("In 'data_structs.c' -> 'create_label_table': ");
      printf("Failed to allocate memory for label table\n");
      return;

   case m13_create_label:
      printf("In 'data_structs.c' -> 'create_label': ");
      printf("Failed to allocate memory for label\n");
      return;

   case m14_copy_label_name:
      printf("In 'data_structs.c' -> 'extract_label_name_from_instruction': ");
      printf("Failed to allocate memory for label name\n");
      return;

   case m15_insert_label:
      printf("In 'data_structs.c' -> 'insert_label': ");
      printf("Failed to insert label to label table\n");
      return;

   case m16_create_macro:
      printf("In 'data_structs.c' -> 'create_macro': ");
      printf("Failed to allocate memory for macro\n");
      return;

   case m17_create_macro_table:
      printf("In 'data_structs.c' -> 'create_macro_table': ");
      printf("Failed to allocate memory for macro table\n");
      return;

   case m18_create_instruction:
      printf("In 'data_structs.c' -> 'create_instruction': ");
      printf("Failed to allocate memory for instruction\n");
      return;

   case m19_create_inst_table:
      printf("In 'data_structs.c' -> 'create_instruction_table': ");
      printf("Failed to allocate memory for instruction table\n");
      return;

   case m20_create_token:
      printf("In 'data_structs.c' -> 'create_empty_token': ");
      printf("Failed to allocate memory for token\n");
      return;

   case m21_insert_macro_to_table:
      printf("In 'data_structs.c' -> 'insert_macro_to_table': ");
      printf("Failed to insert macro to macro table\n");
      return;

   case m22_insert_line_to_macro:
      printf("In 'data_structs.c' -> 'insert_line_to_macro': ");
      printf("Failed to insert line to macro\n");
      return;



      /* File handling errors */
   case f1_file_open:
      printf("Failed to open file\n");
      return;

   case f2_as_creation:
      printf("In 'file_util.c' -> 'generate_filenames': ");
      printf("Failed to add a '.as' extention for the file '%s'\n", sys_state->generic_filename);
      return;
   case f3_backup:
      printf("In 'file_util.c' -> 'generate_filenames': ");
      printf("File backup has failed\n");
      return;
   case f4_am_creation:
      printf("In 'file_util.c' -> 'generate_filenames': ");
      printf("Failed to add a '.am' extention for the file '%s'\n", sys_state->generic_filename);
      return;

   case f5_tmp_file:
      printf("In 'file_util.c' -> 'remove_whitespace_from_file': ");
      printf("Failed tp create a temporary file for '%s'\n", sys_state->full_filename);
      return;
   case f6_open_tmpfile:
      printf("In 'file_util.c' -> 'remove_whitespace_from_file': ");
      printf("Failed to open tmpfile while removing whitespace from '%s'\n", sys_state->as_filename);

      return;

   case f7_write_to_tmp_file:
      printf("In 'file_util.c' -> 'remove_whitespace_from_file': ");
      printf("Failed to write to temporary file while removing whitespace from '%s'\n", sys_state->as_filename);
      return;

   case f8_line_mismatch:
      printf("In 'file_util.c' -> 'remove_whitespace_from_file': ");
      printf("Line count mismatch: original=%d, cleaned=%d\n", sys_state->original_line_count, sys_state->cleaned_line_count);

      return;

   case f9_rmv_original:
      printf("In 'file_util.c' -> 'remove_whitespace_from_file': ");
      printf("Failed to remove original file '%s'\n", sys_state->as_filename);
      return;

   case f10_rename_tmp:
      printf("In 'file_util.c' -> 'remove_whitespace_from_file': ");
      printf("Failed to rename temporary filename to '%s'\n", sys_state->as_filename);
      return;
   case f11_bckup_null:
      printf("In 'file_util.c' -> 'duplicate_files': ");
      printf("Filenames array contains a null filename.\n");




   default:
      break;
   }



}

void print_syntax_error(syntax_state *state, error_code e_code) {
   char *ptr = NULL;
   char *tmp = NULL;
   int len = 0;
   static int error_count = 0;
   static int warning_count = 0;
   if (e_code < 0) return;

   switch (e_code) {
   case n1_prefix_op:
      printf("Note: In file %s: line %d: '%s': ", state->as_filename, state->line_number, state->buffer_without_offset);
      printf("A prefix of the invalid operation name '");
      ptr = state->buffer;
      while (ptr != NULL && !isspace(*ptr) && (*ptr) != '\0') {
         putchar(*ptr);
         ptr++;
      }

      printf("'");


      printf(" is a valid operation name called '%s'. Did you mean to write '%s'? \n",
         state->k_table[state->index].name, state->k_table[state->index].name);

      return;


   case w1_label_declared_not_defined:
      warning_count++;
      printf("\n(Warning #%d)~ In file %s:\nA label with the name '%s' is declared as an entry but is not defined in the file '%s'.\nUsing this label as an argument will result in a syntax error.\n\n", warning_count, state->as_filename, state->tmp_arg, state->as_filename);
      return;
   default:
      break;
   }

   error_count++;
   printf("\n(Error #%d)~ file '%s' on line: '%d': %s -> ", error_count, state->as_filename, state->line_number, state->buffer_without_offset);
   switch (e_code) {

      /* General errors */
   case e1_undef_cmd:
      printf("Undefined command name: '");
      ptr = state->buffer;
      while (ptr != NULL && !isspace(*ptr) && (*ptr) != '\0') {
         putchar(*ptr);
         ptr++;
      }

      printf("'\n");

      break;

   case e2_tokgen:
      printf("Failed to lex instruction.\n");
      break;


   case e3_addr_assign:
      printf("Failed to assign addresses.\n");
      break;


   case e4_extra_comma:
      printf("Unnecessary comma between the command '%s' and the first argument\n", state->_inst->tokens[0]);
      break;

   case e5_missing_args:
      printf("Trying to pass less than %lu argument to the '%s' instruction\n", state->_inst->num_tokens - 1, state->_inst->tokens[0]);
      break;
   case e6_arg_order:
      printf("Wrong order of arguments.\n");
      break;
   case e7_lea_nondir_src:
      printf("Trying to assign an non-direct addressing method to a source argument of 'lea' instruction\n");
      break;
   case e8_lea_imm_dest:
      printf("Trying to assign an immediate addressing method to a destination argument of 'lea' instruction\n");
      break;
   case e9_inval_opcode:
      printf("Command opcode not found\n");
      break;
   case e10_series_of_commas:
      printf("A series of commas without an argument between them\n");
      break;
   case e11_missing_comma:
      printf("Missing a comma - ',' between arguments of the instruction\n");
      break;
   case e12_too_much_args:
      printf("Trying to pass more than %lu arguments for the '%s' instruction\n", state->_inst->num_tokens - 1, state->_inst->tokens[0]);
      break;
   case e13_comma_after_last_arg:
      printf("An invalid comma- ',' after the last argument of the '%s' instruction\n", state->buffer_without_offset);
      break;
   case e14_comma_before_data:
      printf("Additional unnecessary comma- ',' before '.data' integer list\n");
      break;
   case e15_inval_data_symbol:
      printf("An invalid symbol '%c' in '.data' integer list\n", state->buffer[state->index]);
      break;
   case e16_no_comma_betw_ints_data:
      printf("Missing a comma - ',' between integers in '.data' definition\n");
      break;
   case e17_extre_comma_data_mid:
      printf("A series of commas without an integer between them\n");
      break;
   case e18_extra_comma_data_end:
      printf("An unnecessary comma in the end of '.data' integer list\n");
      break;
   case e19_pm_wrong_position:
      printf("Invalid position of '+' or '-' sign when defining a '.data' integer list\n");
      break;
   case e20_pm_no_int:
      printf("No matching integer for a '+' or '-' sign when defining a '.data' integer list\n");
      break;
   case e21_str_no_qm:
      printf("Trying to define a string without any quotation marks - '\"'\n");
      break;
   case e22_str_no_qm_start:
      printf("Trying to define a string without opening quotation marks- '\"'\n");
      break;
   case e23_str_no_qm_end:
      printf("Trying to define a string without closing quotation marks- '\"'\n");
      break;
   case e24_inval_label_name:
      printf("Invalid character(non english letter or digit) detected in label name\n");
      break;

   case e25_ent_ex_args:
      printf("Additional arguments in an 'entry'/'extern' instruction\n");
      break;

   case e26_third_assignment:
      printf("Trying to assign addressing methods to arguments for a 3rd time.\n");
      break;

   case e27_inval_addr_mehod:
      printf("Trying to pass an invalid argument '%s' that does not match any addressing method.\n", state->tmp_arg);
      break;

   case ex28_inval_method_mov:
      printf("Trying to assign an immediate addressing method to a destination argument of a 'mov' instruction\n");
      break;

   case ex28_inval_method_add:
      printf("Trying to assign an immediate addressing method to a destination argument of an 'add' instruction\n");
      break;

   case ex28_inval_method_sub:
      printf("Trying to assign an immediate addressing method to a destination argument of a 'sub' instruction\n");
      break;


   case e39_cmp_extra_args:
      printf("Trying to pass more than one argument to a 'cmp' instruction.\n");
      break;

   case e29_inval_method_imm_lea:
      printf("Trying to assign an immediate addressing method to destination argument of a 'lea' instruction\n");
      break;

   case ex30_ext_arg_clr:
      printf("Trying to pass more than one argument as input to a 'clr' instruction\n");
      break;

   case ex30_ext_arg_not:
      printf("Trying to pass more than one argument as input to a 'not' instruction\n");
      break;

   case ex30_ext_arg_inc:
      printf("Trying to pass more than one argument as input to a 'inc' instruction\n");
      break;

   case ex30_ext_arg_dec:
      printf("Trying to pass more than one argument as input to a 'dec' instruction\n");
      break;

   case ex30_ext_arg_red:
      printf("Trying to pass more than one argument as input to a 'red' instruction\n");
      break;


   case ex31_mult_assign_clr:
      printf("Trying to assign more than one addressing method to a destination argument of a 'clr' instruction\n");
      break;

   case ex31_mult_assign_not:
      printf("Trying to assign more than one addressing method to a destination argument of a 'not' instruction\n");
      break;

   case ex31_mult_assign_inc:
      printf("Trying to assign more than one addressing method to a destination argument of a 'inc' instruction\n");
      break;

   case ex31_mult_assign_dec:
      printf("Trying to assign more than one addressing method to a destination argument of a 'dec' instruction\n");
      break;

   case ex31_mult_assign_red:
      printf("Trying to assign more than one addressing method to a destination argument of a 'red' instruction\n");
      break;


   case ex32_imm_clr:
      printf("Trying to assign an immediate addressing method to a destination argument of a 'clr' instruction\n");
      break;

   case ex32_imm_not:
      printf("Trying to assign an immediate addressing method to a destination argument of a 'not' instruction\n");
      break;

   case ex32_imm_inc:
      printf("Trying to assign an immediate addressing method to a destination argument of a 'inc' instruction\n");
      break;

   case ex32_imm_dec:
      printf("Trying to assign an immediate addressing method to a destination argument of a 'dec' instruction\n");
      break;

   case ex32_imm_red:
      printf("Trying to assign an immediate addressing method to a destination argument of a 'red' instruction\n");
      break;

   case ex33_toomany_jmp:
      printf("Trying to pass more than one argument as input to a 'jmp' instruction\n");
      break;

   case ex33_toomany_bne:
      printf("Trying to pass more than one argument as input to a 'bne' instruction\n");
      break;

   case ex33_toomany_jsr:
      printf("Trying to pass more than one argument as input to a 'jsr' instruction\n");
      break;

   case e34_mul_assign_jmp_bne_jsr:
      printf("Multiple assignment attempts for a destination argument addressing method of a 'jmp'/'bne'/'jsr' instruction\n");
      break;

   case ex34_mul_assign_jmp:
      printf("Trying to assign more than one addressing method to a destination argument of a 'jmp' instruction\n");
      break;

   case ex34_mul_assign_bne:
      printf("Trying to assign more than one addressing method to a destination argument of a 'bne' instruction\n");
      break;

   case ex34_mul_assign_jsr:
      printf("Trying to assign more than one addressing method to a destination argument of a 'jsr' instruction\n");
      break;

   case ex35_imm_jmp:
      printf("Trying to assign an immediate addressing method to a destination argument of a 'jmp' instruction\n");
      break;

   case ex35_imm_bne:
      printf("Trying to assign an immediate addressing method to a destination argument of a 'bne' instruction\n");
      break;

   case ex35_imm_jsr:
      printf("Trying to assign an immediate addressing method to a destination argument of a 'jsr' instruction\n");
      break;

   case ex36_dir_jmp:
      printf("Trying to assign a direct register addressing method to a destination argument of a 'jmp' instruction\n");
      break;

   case ex36_dir_bne:
      printf("Trying to assign a direct register addressing method to a destination argument of a 'bne' instruction\n");
      break;

   case ex36_dir_jsr:
      printf("Trying to assign a direct register addressing method to a destination argument of a 'jsr' instruction\n");
      break;

   case e37_toomany_prn:
      printf("Trying to pass more than one argument as input to a 'prn' instruction\n");
      break;
   case e38_addr_mult_assign_prn:
      printf("Multiple assignment attempts for a destination argument addressing method of a 'prn' instruction\n");
      break;

   case e40_toomany_rts_stop:
      printf("Trying to pass an argument as input to a 'rts'/'stop' instruction\n");
      break;

   case ex40_toomany_rts:
      printf("Trying to pass an argument as input to a 'rts' instruction\n");
      break;

   case ex40_toomany_stop:
      printf("Trying to pass an argument as input to a 'stop' instruction\n");
      break;

   case e41_lbl_arg:
      printf("Trying to use an invalid label name as an argument\n");
      break;

   case e42_imm_val_not_digit:
      printf("The character following '#' in '%s' neither a digit not a '+' or '-' sign\n", state->buffer);
      break;
   case e43_inval_indirect_reg:
      printf("Expected a register identifier 'rx' after the '*' character\n");
      break;

   case e46_imm_inv_after_pm:
      printf("The character following the sign character in '%s' neither a digit not a valid digit\n", state->buffer);
      break;

   case e44_indirect_reg_number_not_in_range:
      printf("The character following '*r' in '%s' is not a digit between 0 and 7\n", state->buffer);
      break;

   case e47_ext_chars_after_indirect_reg:
      printf("Detected additional characters after '*rx' in '%s'\n", state->buffer);
      break;

   case e48_no_num_after_dir_reg:
      tmp = my_strdup(state->buffer);
      ptr = strchr(tmp, 'r');
      ptr++;
      if (ptr) (*ptr) = '\0';

      printf("\n%s  missing a digit between 0 and 7 after 'r'\n", tmp);

      putchar(' ');
      printf("^\n");
      free(tmp);
      break;

   case e49_ext_chars_after_direct_reg:
      printf("Detected additional characters after 'rx' in '%s'\n", state->buffer);
      break;

   case e50_direct_reg_num_not_in_range:
      printf("The character following 'r' in '%s' is not a digit between 0 and 7\n", state->buffer);
      break;

   case e51_unknown_label:
      printf("Unrecognized argument '%s'\nNote:'%s' could be a label name that has an invalid definition. Make sure label definitions in your code are valid\n",
         state->tmp_arg, state->tmp_arg);
      break;

   case e52_inval_ext:
      printf("Trying to remove extention from a filename '%s' that has no extention\n", state->tmp_arg);
      break;

   case e53_ext_chars_after_endmacr:
      printf("Additional characters after 'endmacr' are not allowed\n");
      break;

   case e54_macr_already_defined:
      printf("Macro with the name '%s' is already defined\n", state->tmp_arg);
      break;

   case e55_macro_not_found:
      printf("Failed to expand macro with the name '%s'\n", state->tmp_arg);
      break;

   case e56_macro_name_not_valid:
      printf("Macro name definition contains more than one word.\n");
      break;

   case e57_macroname_same_as_keyword:
      printf("Trying to define a macro with the same name as a keyword '%s'\n", state->tmp_arg);
      break;

   case e58_cannot_insert_macro:
      printf("Failed to insert macro '%s' to macro table\n", state->tmp_arg);
      break;

   case e59_label_redef:
      printf("Label with the name '%s' is already defined\n", state->tmp_arg);
      break;

   case e60_label_name_is_too_long:
      printf("Label name '%s' is too long. Max length is %d\n", state->buffer, MAX_LABEL_LENGTH);
      break;

   case e61_label_name_is_keyword:
      tmp = my_strdup(state->buffer);
      ptr = strchr(tmp, ':');
      if (ptr) *ptr = '\0';
      printf("Label name cannot be the same as the keyword '%s'\n", tmp);
      free(tmp);
      break;

   case e62_label_name_is_macro:
      printf("Label name '%s' is already defined as a macro\n", state->buffer);
      break;

   case e63_label_name_not_alphanumeric:
      tmp = my_strdup(state->buffer);
      ptr = strchr(tmp, ':');
      if (ptr) *ptr = '\0';
      ptr = tmp;
      while (ptr != NULL && (*ptr) != '\0' && (isdigit(*ptr) || isalpha(*ptr))) {
         ptr++;
      }

      len = ptr - tmp + 1;
      printf("\n'%s' contains non alphanumeric characters and cannot be used as a label name\n", tmp);
      while (len--) putchar(' ');
      printf("^\n");
      free(tmp);
      break;

   case e64_whitespace_between_label_and_colon:
      tmp = my_strdup(state->buffer);
      ptr = strchr(tmp, ':');
      if (ptr) *ptr = '\0';
      len = ptr - tmp;
      printf("\n'%s:' Label definition contains whitespace between the name and the colon\n", tmp);
      while (len--) putchar(' ');
      printf("^\n");
      free(tmp);
      break;

   case e66_redef_directive:
      printf("Trying to redefine a directive '%s'\n", state->buffer);
      break;

   case e67_using_undefined_label:
      printf("Trying to reference an undefined label with the name '%s'\n", state->tmp_arg);
      break;





   default:
      break;

   }

}
