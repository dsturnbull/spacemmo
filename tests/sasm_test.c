#include "src/tests.h"
#include "src/lib/cpu/sasm.h"

sasm_t *sasm;

bool
test_read_value_hex_number()
{
    uint32_t value = read_value(sasm, "0x10", NULL);
    assert_equals(value, 0x10);

    value = read_value(sasm, "0x1f", NULL);
    assert_equals(value, 0x1f);
    return true;
}

bool
test_read_value_char()
{
    uint32_t value = read_value(sasm, "'a'", NULL);
    assert_equals(value, 0x61);
    return true;
}

bool
test_read_value_constant()
{
    uint32_t value = read_value(sasm, "hello", NULL);
    assert_equals(value, 0xdeadbeef);
    return true;
}

bool
test_read_value_label()
{
    uint32_t value = read_value(sasm, "_hello", NULL);
    assert_equals(value, 0xdeadbeef);
    return true;
}

bool
test_define_constant()
{
    define_constant(sasm, "test_define_constant", 0x20);
    assert_equals(strcmp(sasm->labels[0].name, "test_define_constant"), 0);
    assert_equals(sasm->labels[0].addr, 0x20);
    return true;
}

bool
test_define_data_string()
{
    char *data = strdup("\"hello\"");
    define_data(sasm, "test_define_data_string", data);
    assert_equals(strcmp(sasm->labels[1].name, "test_define_data_string"), 0);
    assert_equals((int)sasm->labels[1].data_len, 5);
    for (size_t i = 0; i < sasm->labels[1].data_len; i++)
        assert_equals(sasm->labels[1].data[i], (uint32_t)data[i + 1]);
    return true;
}

bool
test_define_data_len()
{
    char *data = strdup("@-test_define_data_string");
    define_data(sasm, "test_define_data_string_len", data);
    assert_equals(strcmp(sasm->labels[2].name,
                "test_define_data_string_len"), 0);
    assert_equals(sasm->labels[2].addr, 5);
    assert_equals((int)sasm->labels[2].data_len, 0);

    return true;
}

bool
test_define_variable()
{
    define_variable(sasm, "test_define_variable", 1);
    assert_equals(strcmp(sasm->labels[3].name, "test_define_variable"), 0);
    return true;
}

bool
test_make_label()
{
    make_label(sasm, strdup("test_make_label\n"));
    assert_equals(strcmp(sasm->labels[4].name, "test_make_label"), 0);
    return true;
}

bool
test_parse_op()
{
    assert_equals(parse_op(strdup("HLT")),  HLT);
    assert_equals(parse_op(strdup("LOAD")), LOAD);
    assert_equals(parse_op(strdup("ADD")),  ADD);
    assert_equals(parse_op(strdup("SUB")),  SUB);
    assert_equals(parse_op(strdup("MUL")),  MUL);
    assert_equals(parse_op(strdup("DIV")),  DIV);
    assert_equals(parse_op(strdup("AND")),  AND);
    assert_equals(parse_op(strdup("OR")),   OR);
    assert_equals(parse_op(strdup("JMP")),  JMP);
    assert_equals(parse_op(strdup("JE")),   JE);
    assert_equals(parse_op(strdup("JZ")),   JZ);
    assert_equals(parse_op(strdup("JNZ")),  JNZ);
    assert_equals(parse_op(strdup("CALL")), CALL);
    assert_equals(parse_op(strdup("RET")),  RET);
    assert_equals(parse_op(strdup("DUP")),  DUP);
    assert_equals(parse_op(strdup("PUSH")), PUSH);
    assert_equals(parse_op(strdup("POP")),  POP);
    assert_equals(parse_op(strdup("SWAP")), SWAP);
    assert_equals(parse_op(strdup("INT")),  INT);
    return true;
}

bool
test_normalise_line_tabs_are_spaces()
{
    char *line = strdup("blah\tblah\n");
    normalise_line(&line);
    assert_equals(line[4], ' ');
    return true;
}

bool
test_normalise_strip_leading_whitespace()
{
    char *line = strdup("blah blah\n");
    normalise_line(&line);
    assert_equals(line[4], ' ');
    return true;
}

bool
test_normalise_skip_comments()
{
    char *line = strdup("blah ; hihi");
    normalise_line(&line);
    assert_equals(strcmp(line, "blah"), 0);
    return true;
}

bool
test_normalise_coalesce_whitespace()
{
    char *line = strdup("blah\t blah \t blah  ");
    normalise_line(&line);
    assert_equals(strcmp(line, "blah blah blah"), 0);
    return true;
}

bool
test_push_single_instruction()
{
    uint8_t *orig = sasm->ip;
    push(sasm, NULL, HLT, 0);
    assert_equals((int)(sasm->ip - orig), 1);
    assert_equals(*(sasm->ip - 1), HLT);
    return true;
}

bool
test_push_instruction_with_arg()
{
    uint8_t *orig = sasm->ip;
    char *line = strdup("0x20");
    push(sasm, &line, PUSH, 1);
    assert_equals((int)(sasm->ip - orig), 5);
    assert_equals(*(sasm->ip - 5), PUSH);
    assert_equals(*((uint32_t *)(sasm->ip - 4)), 0x20);
    return true;
}

bool
test_parse_empty_file();

bool
test_parse_flat_file();

bool
test_assemble_prog_len();

bool
test_parse_file_with_imports();

bool
test_assemble_main_intro();

bool
test_assemble_no_main();

bool
test_assemble_data_labels_written();

bool
test_assemble_sentinels_written();

bool
test_assemble_constants_defined();

void
setup()
{
    sasm = init_sasm();
}

void
teardown()
{
    free(sasm);
}

test_t tests[] = {
      &test_read_value_hex_number
    , &test_read_value_char
    , &test_read_value_constant
    , &test_read_value_label
    , &test_define_constant
    , &test_define_data_string
    , &test_define_data_len
    , &test_define_variable
    , &test_make_label
    , &test_parse_op
    , &test_normalise_line_tabs_are_spaces
    , &test_normalise_skip_comments
    , &test_normalise_coalesce_whitespace
    , &test_push_single_instruction
    , &test_push_instruction_with_arg
    , NULL
};

