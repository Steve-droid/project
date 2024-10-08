#include <string.h>
#include "parser.h"

#include <file_util.h>

#define ARE_SHIFT 0
#define DEST_SHIFT 3
#define SRC_SHIFT 7
#define OPCODE_SHIFT 11
#define SRC_REG_SHIFT 6
#define DEST_REG_SHIFT 3
#define FIRST_OP_GROUP 2
#define SECOND_OP_GROUP 1
#define THIRD_OP_GROUP 0


uint16_t to_twos_complement(int16_t number) {
    return (uint16_t)number;
}

data_image *create_data_image(inst_table *_inst_table) {
    size_t _inst_indx;
    inst *current_inst = NULL;
    char *temp = NULL;
    char *token = NULL;
    int i = 0;
    int name_count = 0;

    data_image *_data_image = (data_image *)calloc(1, sizeof(data_image));
    if (_data_image == NULL) {
        return NULL;
    }

    _data_image->binary_word_vec = NULL;
    _data_image->num_words = 0;
    _data_image->num_names = 0;
    _data_image->binary_word_vec = (uint16_t *)calloc(_inst_table->DC, sizeof(uint16_t));
    if (_data_image->binary_word_vec == NULL) {
        free(_data_image);
        return NULL;
    }

    _data_image->names = (char **)calloc(MAX_LINE_LENGTH, sizeof(char *));
    if (_data_image->names == NULL) {
        free(_data_image->binary_word_vec);
        free(_data_image);
        return NULL;
    }

    _inst_indx = 0;
    while (_inst_indx < _inst_table->num_instructions) {
        current_inst = _inst_table->inst_vec[_inst_indx];
        if (current_inst->is_dot_data) {
            _data_image->names[name_count] = current_inst->tokens[1];
            name_count++;
            temp = my_strdup(current_inst->tokens[1]);
            token = strtok(temp, ",");
            i = 0;
            while (token != NULL && i < current_inst->num_dot_data_members) {
                _data_image->binary_word_vec[_data_image->num_words] = to_twos_complement(atoi(token));
                token = strtok(NULL, ", ");
                _data_image->num_words++;
            }

            free(temp);
            temp = NULL;
        }

        if (current_inst->is_dot_string) {
            _data_image->names[name_count] = current_inst->tokens[1];
            name_count++;
            for (i = 0;i < current_inst->num_words_to_generate; i++) {
                _data_image->binary_word_vec[_data_image->num_words] = (current_inst->tokens[1][i]);
                _data_image->num_words++;
            }
        }

        _inst_indx++;
    }

    if (_data_image->num_words != _inst_table->DC) {
        return NULL;
    }


    _data_image->num_names = name_count;
    return _data_image;
}

void destroy_data_image(data_image **_data_image) {
    int i;
    if (*(_data_image) == NULL) return;

    if ((*_data_image)->binary_word_vec != NULL) {
        free((*_data_image)->binary_word_vec);
        (*_data_image)->binary_word_vec = NULL;
    }
    if ((*_data_image)->names != NULL) {
        for (i = 0;i < (*_data_image)->num_names;i++) {
            (*_data_image)->names[i] = NULL;
        }
        free((*_data_image)->names);
        (*_data_image)->names = NULL;
    }

    free(*_data_image);
    (*_data_image) = NULL;
}

void assign_bits_operation(inst_table *_inst_table, size_t index) {
    inst *_inst = _inst_table->inst_vec[index];
    uint16_t combined_word = 0;
    size_t operand_count = 0;
    addressing_method _addressing_method = NO_ADDRESSING_METHOD;

    _inst->bin_ARE = 4; /* 100 -> A = 1, R = E = 0 */


    switch (_inst->src_addressing_method) {
    case IMMEDIATE:
        _inst->bin_src_method = 1; /* 0b0001 */
        break;
    case DIRECT:
        _inst->bin_src_method = 2; /* 0b0010 */
        break;
    case INDIRECT_REGISTER:
        _inst->bin_src_method = 4; /* 0b0100 */
        break;
    case DIRECT_REGISTER:
        _inst->bin_src_method = 8; /* 0b100 */
        break;
    case UNDEFINED_METHOD:
    case NO_ADDRESSING_METHOD:
    default:
        break;
    }
    switch (_inst->dest_addressing_method) {
    case IMMEDIATE:
        _inst->bin_dest_method = 1;
        break;
    case DIRECT:
        _inst->bin_dest_method = 2;
        break;
    case INDIRECT_REGISTER:
        _inst->bin_dest_method = 4;
        break;
    case DIRECT_REGISTER:
        _inst->bin_dest_method = 8;
        break;
    case UNDEFINED_METHOD:
    case NO_ADDRESSING_METHOD:
    default:
        break;
    }


    combined_word = (_inst->bin_ARE << ARE_SHIFT) | /* ARE bits (bits 0, 1, 2)*/
        (_inst->bin_dest_method << DEST_SHIFT) | /* dest method (bits 3, 4, 5, 6*/
        (_inst->bin_src_method << SRC_SHIFT) | /* src method (bits 7, 8, 9, 10)*/
        (_inst->bin_opcode << OPCODE_SHIFT); /* opcode (bits 11, 12, 13, 14)*/


    /*Assign the first word*/
    _inst->binary_word_vec[0] = combined_word;
    combined_word = 0;

    /*Get the number of operands the operation operates on*/
    operand_count = _inst->num_tokens - 1;

    /*
    * rts
    * stop
    */
    if (operand_count == 0) {
        return; /*No addressing methods*/
    }

    /*
    * clr
    * not
    * inc
    * dec
    * jmp
    * bne
    * red
    * prn
    * jsr
    */
    if (operand_count == 1) {
        _addressing_method = _inst->dest_addressing_method;

        switch (_addressing_method) {
        case IMMEDIATE:
            _inst->binary_word_vec[1] = to_twos_complement(_inst->immediate_val_dest);
            _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] << DEST_SHIFT);
            _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] | _inst->bin_ARE);
            return;
        case DIRECT:
            if (_inst->is_dest_extern)
                _inst->bin_ARE = 1;
            else
                _inst->bin_ARE = 2;
            _inst->binary_word_vec[1] = to_twos_complement(_inst->direct_label_address_dest);
            _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] << DEST_SHIFT);
            _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] | _inst->bin_ARE);
            return;
        case INDIRECT_REGISTER:
            _inst->binary_word_vec[1] = (_inst->indirect_reg_num_dest << DEST_REG_SHIFT);
            _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] | _inst->bin_ARE);
            return;
        case DIRECT_REGISTER:
            _inst->binary_word_vec[1] = (_inst->direct_reg_num_dest << DEST_REG_SHIFT);
            _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] | _inst->bin_ARE);
            return;
        case UNDEFINED_METHOD:
        case NO_ADDRESSING_METHOD:
        default: return;

        }


    }


    /*
     * mov
     * cmp
     * add
     * sub
     * lea
     */
    if (operand_count == 2) {
        _addressing_method = _inst->src_addressing_method;
        switch (_addressing_method) {
        case IMMEDIATE:
            _inst->binary_word_vec[1] = to_twos_complement(_inst->immediate_val_src);
            _inst->binary_word_vec[1] = _inst->binary_word_vec[1] << DEST_SHIFT;
            _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] | _inst->bin_ARE);
            break;
        case DIRECT:
            _inst->binary_word_vec[1] = to_twos_complement(_inst->direct_label_address_src);
            _inst->binary_word_vec[1] = _inst->binary_word_vec[1] << DEST_SHIFT;
            if (_inst->is_src_extern)
                _inst->bin_ARE = 1; /* 001 */
            else
                _inst->bin_ARE = 2; /* 010 */
            _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] | _inst->bin_ARE);
            break;
        case INDIRECT_REGISTER:
            _inst->bin_ARE = 4;
            _inst->binary_word_vec[1] = (_inst->indirect_reg_num_src << SRC_REG_SHIFT);
            _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] | _inst->bin_ARE);
            break;

        case DIRECT_REGISTER:
            _inst->bin_ARE = 4; /*100*/
            _inst->binary_word_vec[1] = (_inst->direct_reg_num_src << SRC_REG_SHIFT);
            _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] | _inst->bin_ARE);
            break;
        case UNDEFINED_METHOD:
        case NO_ADDRESSING_METHOD:
        default: break;
        }

        _addressing_method = _inst->dest_addressing_method;
        switch (_addressing_method) {
        case IMMEDIATE:
            _inst->bin_ARE = 4;
            _inst->binary_word_vec[2] = to_twos_complement(_inst->immediate_val_dest);
            _inst->binary_word_vec[2] = _inst->binary_word_vec[2] << DEST_SHIFT;
            _inst->binary_word_vec[2] = (_inst->binary_word_vec[2] | _inst->bin_ARE);
            return;
        case DIRECT:
            _inst->binary_word_vec[2] = to_twos_complement(_inst->direct_label_address_dest);
            _inst->binary_word_vec[2] = _inst->binary_word_vec[2] << DEST_SHIFT;
            if (_inst->is_dest_extern)
                _inst->bin_ARE = 1;
            else
                _inst->bin_ARE = 2;
            _inst->binary_word_vec[2] = (_inst->binary_word_vec[2] | _inst->bin_ARE);
            return;
        case INDIRECT_REGISTER:
            _inst->bin_ARE = 4;
            if (_inst->num_words_to_generate == 2) {
                _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] |
                    _inst->indirect_reg_num_dest << DEST_SHIFT);
                _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] | _inst->bin_ARE);
                break;
            }

            _inst->binary_word_vec[2] = _inst->indirect_reg_num_dest << DEST_SHIFT;
            _inst->binary_word_vec[2] = (_inst->binary_word_vec[2] | _inst->bin_ARE);
            return;
        case DIRECT_REGISTER:
            _inst->bin_ARE = 4;
            if (_inst->num_words_to_generate == 2) {
                _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] |
                    _inst->direct_reg_num_dest << DEST_SHIFT);
                _inst->binary_word_vec[1] = (_inst->binary_word_vec[1] | _inst->bin_ARE);
                break;
            }

            _inst->binary_word_vec[2] = _inst->direct_reg_num_dest << DEST_SHIFT;
            _inst->binary_word_vec[2] = (_inst->binary_word_vec[2] | _inst->bin_ARE);

            return;

        case UNDEFINED_METHOD:
        case NO_ADDRESSING_METHOD:
        default: return;

        }
    }
}

status generate_binary_words(inst_table *_inst_table, size_t index) {
    inst *current_inst = _inst_table->inst_vec[index];

    current_inst->binary_word_vec = (uint16_t *)calloc(current_inst->num_words_to_generate, sizeof(uint16_t));
    if (current_inst->binary_word_vec == NULL) {
        return failure;
    }

    return success;
}

status parse(inst_table *_inst_table, label_table *_label_table, keyword *keyword_table, char *am_filename) {
    size_t i;
    size_t label_index;
    size_t bin_word_index;
    size_t inst_index;
    size_t am_filename_len = strlen(am_filename);
    inst *tmp_inst = NULL;
    char *object_output_filename = NULL;
    char *extern_output_filename = NULL;
    char *entry_output_filename = NULL;
    FILE *object_file_ptr = NULL;
    FILE *entry_file_ptr = NULL;
    FILE *extern_file_ptr = NULL;
    char *tmp_ptr = NULL;
    label *tmp_label = NULL;
    int create_ext = false;
    int create_ent = false;
    int address = 100;
    data_image *_data_image = NULL;


    object_output_filename = my_strdup(am_filename);
    if (object_output_filename == NULL) {
        return failure;
    }

    tmp_ptr = strstr(object_output_filename, ".am");
    if (tmp_ptr != NULL) {
        tmp_ptr[1] = 'o';
        tmp_ptr[2] = 'b';
        tmp_ptr = NULL;
    }

    object_file_ptr = my_fopen(object_output_filename, "w");
    if (object_file_ptr == NULL) {
        free_filenames(object_output_filename, NULL);
        object_output_filename = NULL;
        object_file_ptr = NULL;
        return failure;
    }


    _data_image = create_data_image(_inst_table);
    if (_data_image == NULL) {
        free_filenames(object_output_filename, NULL);
        close_files(object_file_ptr, NULL);
        object_output_filename = NULL;
        object_file_ptr = NULL;
        return failure;
    }



    for (i = 0; i < _inst_table->num_instructions; i++) {
        if (generate_binary_words(_inst_table, i) != success) {
            free_filenames(object_output_filename, NULL);
            close_files(object_file_ptr, NULL);
            object_output_filename = NULL;
            object_file_ptr = NULL;
            return failure;
        }

        tmp_inst = _inst_table->inst_vec[i];
        if (!(tmp_inst->is_dot_data || tmp_inst->is_dot_string || tmp_inst->is_entry || tmp_inst->is_extern)) {
            assign_bits_operation(_inst_table, i);
        }
    }


    /*Print to the .ob output file*/
    fprintf(object_file_ptr, "\t%lu\t%lu\n", _inst_table->IC - 100 - _inst_table->DC, _inst_table->DC);
    for (inst_index = 0; inst_index < _inst_table->num_instructions; inst_index++) {
        tmp_inst = _inst_table->inst_vec[inst_index];

        if (tmp_inst->is_dot_data || tmp_inst->is_dot_string || tmp_inst->is_entry || tmp_inst->is_extern) {
            continue;
        }

        for (bin_word_index = 0; bin_word_index < _inst_table->inst_vec[inst_index]->num_words_to_generate;
            bin_word_index++, address++) {
            tmp_inst = _inst_table->inst_vec[inst_index];
            if (tmp_inst->is_dot_data || tmp_inst->is_dot_string || tmp_inst->is_entry || tmp_inst->is_extern) continue;
            print_octal(address, _inst_table->inst_vec[inst_index]->binary_word_vec[bin_word_index], object_file_ptr);

        }
    }

    for (bin_word_index = 0; bin_word_index < _data_image->num_words; bin_word_index++, address++) {
        print_octal(address, _data_image->binary_word_vec[bin_word_index], object_file_ptr);
    }
    close_files(object_file_ptr, NULL);
    free_filenames(object_output_filename, NULL);
    object_file_ptr = NULL;
    object_output_filename = NULL;


    /*Check if we need to create .ext or .ent files*/

    for (inst_index = 0; inst_index < _inst_table->num_instructions; inst_index++) {
        tmp_inst = _inst_table->inst_vec[inst_index];
        if (tmp_inst->is_dest_entry || tmp_inst->is_src_entry) {
            create_ent = true;
        }

        if (tmp_inst->is_dest_extern || tmp_inst->is_src_extern) {
            create_ext = true;
        }
    }

    if (create_ent) {
        entry_output_filename = (char *)calloc(am_filename_len + 2, sizeof(char));
        if (entry_output_filename == NULL) {
            return failure;
        }

        strncpy(entry_output_filename, am_filename, am_filename_len);
        tmp_ptr = strstr(entry_output_filename, ".am");
        if (tmp_ptr != NULL) {
            tmp_ptr[1] = 'e';
            tmp_ptr[2] = 'n';
            tmp_ptr[3] = 't';
            tmp_ptr[4] = '\0';
        }

        entry_file_ptr = my_fopen(entry_output_filename, "w");
        if (entry_file_ptr == NULL) {
            free_filenames(entry_output_filename, NULL);
            entry_output_filename = NULL;
            return failure;
        }



        /*Print to the .ent output file*/
        for (label_index = 0; label_index < _label_table->size; label_index++) {
            tmp_label = _label_table->labels[label_index];
            if (tmp_label->declared_as_entry) {
                fprintf(entry_file_ptr, "%s\t0%lu\n", tmp_label->name, tmp_label->address);
            }
        }

        close_files(entry_file_ptr, NULL);
        free_filenames(entry_output_filename, NULL);
        entry_file_ptr = NULL;
        entry_output_filename = NULL;
    }


    if (create_ext) {
        extern_output_filename = (char *)calloc(am_filename_len + 2, sizeof(char));
        if (extern_output_filename == NULL) {
            return failure;
        }


        strncpy(extern_output_filename, am_filename, am_filename_len);
        tmp_ptr = strstr(extern_output_filename, ".am");
        if (tmp_ptr != NULL) {
            tmp_ptr[1] = 'e';
            tmp_ptr[2] = 'x';
            tmp_ptr[3] = 't';
            tmp_ptr = NULL;
        }


        extern_file_ptr = my_fopen(extern_output_filename, "w");
        if (extern_file_ptr == NULL) {
            free_filenames(extern_output_filename, NULL);
            object_file_ptr = NULL;
            return failure;
        }



        /*Print to the .ext output file*/
        for (inst_index = 0; inst_index < _inst_table->num_instructions; inst_index++) {
            tmp_inst = _inst_table->inst_vec[inst_index];

            /*If the instruction is using direct addressing with an external label,
             print the name of that label and the address where it is referenced*/

            if (tmp_inst->num_words_to_generate == FIRST_OP_GROUP + 1) {
                if (tmp_inst->is_src_extern)
                    fprintf(extern_file_ptr, "%s\t\t0%d\n", tmp_inst->direct_label_name_src, tmp_inst->address + 1);


                if (tmp_inst->is_dest_extern)
                    fprintf(extern_file_ptr, "%s\t\t0%d\n", tmp_inst->direct_label_name_dest, tmp_inst->address + 2);
            }

            if (tmp_inst->num_words_to_generate == SECOND_OP_GROUP + 1)
                if (tmp_inst->is_dest_extern)
                    fprintf(extern_file_ptr, "%s\t\t0%d\n", tmp_inst->direct_label_name_dest, tmp_inst->address + 1);
        }
    }

    close_files(extern_file_ptr, NULL);
    free_filenames(extern_output_filename, NULL);
    extern_file_ptr = NULL;
    extern_output_filename = NULL;

    destroy_data_image(&_data_image);
    return success;

}
