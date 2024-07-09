#ifndef MACRO_H_
#define MACRO_H_
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "file_util.h"


#define as_extension ".as"
#define am_extension ".am"
#define MAX_MACRO_AMOUNT 100
#define MAX_LINE_LENGTH 80
#define INITIAL_MACRO_CAPACITY 3
#define INITIAL_MACRO_TABLE_CAPACITY 3
#define MIN_MACRO_AMOUNT 3
#define MIN_LINE_AMOUNT 3

typedef struct macro {
    char **lines;      /* Vector of strings- the lines that the macro expands to */
    char *name;         /* The name of the macro */
    int line_count;     /* Number of lines in the macro */
    int line_capacity;
}macro;

typedef struct macro_table {
    macro **macros;            /* Vector of macro pointers */
    int macro_count;           /* Number of macros in the table */
    int capacity;
}macro_table;

/**
 * @brief Create a new macro object
 *
 * @param name The name of the macro
 * @return macro*
 */
status create_macro(char *macro_name, macro **new_macro);


status insert_line_to_macro(macro *mac, char *line);

/**
 * @brief Destroy a macro object
 *
 * @param macro The macro to destroy
 */
void macro_destructor(macro *macro);

/**
 * @brief Create a new macro table object
 *
 * @return macro_table*
 */
macro_table *create_macro_table();

/**
 * @brief Insert a macro into the macro table
 *
 * @param table The macro table to insert the macro into
 * @param macro The macro to insert
 */
status insert_macro_to_table(macro_table *table, macro *mac);

/**
 * @brief Get a macro from the macro table
 *
 * @param table The macro table to get the macro from
 * @param name The name of the macro to get
 * @return macro*
 */
macro *find_macro_in_table(macro_table *table, char *name);


/**
 *@brief Get the macro table object
 *
 * @return macro_table*
 */
macro_table *get_macro_table();


/**
 * @brief Destroy a macro table object
 *
 * @param table The macro table to destroy
 */
void macro_table_destructor(macro_table *table);

status print_macro_lines(macro *mac);

status print_macro_table(macro_table *table);


#endif