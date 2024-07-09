#include <stdio.h>
#include <stdlib.h>
#include "pre_assembler.h"
#include "file_util.h"
#include <string.h>

char *as_filename;
char *am_filename;
macro_table *m_table;




int main(int argc, char *argv[]) {

    char **backup_filenames = (char **)malloc(sizeof(char *) * (argc - 1));
    if (backup_filenames == NULL) err(errno, "Memory allocation error while creating backup file names");

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Starting file backup...\n");
    if (backup_files(&backup_filenames, argc - 1, argv + 1) != STATUS_OK) {
        printf("File backup did not execute properly. Exiting..");
        exit(EXIT_FAILURE);
    }
    printf("Backup succesful\n\n");

    printf("Creating .am file... ");
    if (initallize_file_names(argv[1], &am_filename, &as_filename) != STATUS_OK) {
        printf("ERROR: .am file creation did not execute properly. Exiting..");
        exit(EXIT_FAILURE);
    }
    printf("Done\n\n");

    m_table = create_macro_table();

    printf("Starting pre assembly...\n\n");
    status result = pre_assemble(as_filename, am_filename, m_table);
    rename("test1.as.backup", "test1.as");


    switch (result) {
    case STATUS_OK:
        if (print_macro_table(m_table) == STATUS_ERROR_MACRO_TABLE_IS_EMPTY) {
            printf("Macro table is empty. Exiting...");
            exit(EXIT_FAILURE);
        }
        printf("Pre-assembly completed successfully.\n");
        break;
    case STATUS_ERROR_OPEN_SRC:
        printf("Error: Could not open source file.\n");
        break;
    case STATUS_ERROR_OPEN_DEST:
        printf("Error: Could not open destination file.\n");
        break;
    case STATUS_ERROR_READ:
        printf("Error: Could not read from source file.\n");
        break;
    case STATUS_ERROR_WRITE:
        printf("Error: Could not write to destination file.\n");
        break;
    case STATUS_ERROR_MACRO_REDEFINITION:
        printf("Error: Macro redefinition detected.\n");
        break;
    case STATUS_ERROR_MEMORY_ALLOCATION:
        printf("Error: Memory allocation failed.\n");
        break;
    case STATUS_ERROR_MACRO_NOT_FOUND:
        printf("Error: Macro not found.\n");
        break;
    default:
        printf("Unknown error.\n");
        break;
    }

    free(as_filename);
    free(am_filename);
    macro_table_destructor(m_table);
    delete_backup_names(argc - 1, backup_filenames);
    free(backup_filenames);

    return result == STATUS_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
