#ifndef PARSER_H
#define PARSER_H

#include "data_structs.h"

 /**
  *@brief Converts a number to two's complement
  *
 * @param number The number to convert
 * @return uint16_t The number in two's complement
 */
uint16_t to_twos_complement(int16_t number);

/**
 * @brief Create a data image
 * This function creates a data image from the instruction table
 * The data image consists of constants defined as '.data' and '.string' directives
 * @param _inst_table The instruction table to create the data image from
 * @return data_image* The data image
 */
data_image *create_data_image(inst_table *_inst_table);

/**
 * @brief Destroy a data image
 * This function frees the memory allocated for a data image
 * It sets the pointer to the data image to NULL
 * This way, the data image can be safely destroyed without causing memory leaks
 * @param _data_image The data image to destroy
 */
void destroy_data_image(data_image **_data_image);

/**
 * @brief Create an instruction code image
 * This function creates a code image from the instruction table
 * The code image consists of the machine code instructions
 * @param _inst_table The instruction table to create the code image from
 * @return code_image* The code image
 */
void assign_bits_operation(inst_table *_inst_table, size_t index);


/**
 *@brief Create 1-3 words of binary code for each instruction
 * Is called by the parse function when the instruction table is scanned
 * @param _inst_table The instruction table to create the binary words from
 * @param index The index of the instruction in the instruction table
 * @return status A status code indicating the success of the operation
 */
status generate_binary_words(inst_table *_inst_table, size_t index);

/**
 *@brief Parse the instruction table and create the binary code
 * The parse function assumes that the instruction table has been filled with valid instructions
 * @param _inst_table The instruction table to parse
 * @param _label_table The label table to use
 * @param keyword_table The keyword table to use
 * @param am_filename The name of the current assembly file
 * @return status A status code indicating the success of the operation
 */
status parse(inst_table *_inst_table, label_table *_label_table, keyword *keyword_table, char *am_filename);

#endif
