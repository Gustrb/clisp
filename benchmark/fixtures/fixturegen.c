#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define KB(x) ((x) * 1024)
#define MAX_OUTPUT_SIZE KB(1024)

const char *letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const char *visible_chars = "!#$%&'()*+,-/0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

FILE *output_file;
static char buffer[KB(2048)];
static size_t size = 0;

char random_digit(void);
int random_percent(int chance);
int random_between(int min, int max);
void flushbuffer(void);
void append_char(char c);

int generate_form_list(void);
int generate_form(void);
int generate_atom(void);
int generate_list(void);
int generate_number(void);
int generate_symbol(void);
int generate_string(void);
int generate_integer(void);
int generate_sign(void);
int generate_digit(void);
int generate_float(void);
int generate_visible_string_char(void);
int generate_letter(void);

typedef enum {
    PROGRAM = 0,
    FORM_LIST,
    FORM,
    ATOM,
    NUMBER,
    INTEGER,
    FLOAT,
    SIGN,
    DIGITS,
    DIGIT,
    SYMBOL,
    SYMBOL_REST,
    STRING,
    STRING_CHARS,
    VISIBLE_STRING_CHAR,
    LETTER,
    LIST,
} non_terminals_t;

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "[ERROR]: Usage: %s <filepath> <size (in kb)>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filepath = argv[1];
    int size_in_kb = atoi(argv[2]);
    if (size_in_kb <= 0)
    {
        fprintf(stderr, "[ERROR]: Size must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    output_file = fopen(filepath, "w");
    if (!output_file) {
        perror("fopen");
        return EXIT_FAILURE;
    }


    srand(time(NULL));

    int outputlen = 0;
    while (outputlen < KB(size_in_kb))
    {
        outputlen += generate_form();
        append_char('\n');
    }

    flushbuffer();
    fclose(output_file);

    return EXIT_SUCCESS;
}

int random_between(int min, int max)
{
    return min + rand() % (max - min + 1);
}

char random_digit(void)
{
    const char *digits = "0123456789";
    return digits[rand() % 10];
}

int random_percent(int chance) {
    return rand() % 100 < chance;
}

void flushbuffer(void)
{
    if (size > 0)
    {
        fwrite(buffer, 1, size, output_file);
        size = 0;
    }
}

void append_char(char c)
{
    if (size + 1 >= sizeof(buffer))
    {
        flushbuffer();
    }

    buffer[size++] = c;
}


int generate_form_list(void)
{
    int choice = rand() % 2;
    if (choice == 0) return 0;

    return generate_form();
}

int generate_form(void)
{
    int choice = rand() % 2;
    if (choice == 0)
    {
        return generate_atom();
    }

    return generate_list();
}

int generate_atom(void)
{
    int choice = rand() % 3;
    if (choice == 0)
    {
        return generate_number();
    }
    else if (choice == 1)
    {
        return generate_symbol();
    }
    else
    {
        return generate_string();
    }
}

int generate_list(void)
{
    int a = 2;
    append_char('(');
    a += generate_form_list();
    append_char(')');
    return a;
}

int generate_number(void)
{
    int choice = rand() % 2;

    if (choice == 0)
    {
        return generate_integer();
    }

    return generate_float();
}

int generate_integer(void)
{
    int size = random_between(1, 10);
    int total = 0;

    for (int i = 0; i < size; i++)
    {
        total += generate_digit();
    }

    return total;
}

int generate_digit(void)
{
    char digit = random_digit();
    append_char(digit);
    return 1;
}

int generate_sign(void)
{
    int choice = rand() % 2;
    if (choice == 0)
    {
        append_char('+');
    }
    else
    {
        append_char('-');
    }

    return 1;
}

int generate_float(void)
{
    int size = random_between(4, 10);
    int total = 0;
    for (int i = 0; i < size/2; i++)
    {
        total += generate_digit();
    }
    append_char('.');
    for (int i = 0; i < size/2; i++)
    {
        total += generate_digit();
    }

    return total;    
}


int generate_letter(void)
{
    char letter = letters[rand() % 52];
    append_char(letter);
    return 1;
}

int generate_symbol(void)
{
    size_t size = random_between(1, 10);
    int total = 0;
    total += generate_letter();
    for (size_t i = 1; i < size; i++)
    {
        int choice = rand() % 2;
        if (choice == 0)
        {
            total += generate_letter();
        }
        else
        {
            total += generate_digit();
        }
    }
    return total;
}

int generate_string(void)
{
    int total = 2;
    append_char('"');
    int size = random_between(1, 10);
    for (int i = 0; i < size; i++)
    {
        int choice = rand() % 2;
        if (choice == 0)
        {
            total += generate_visible_string_char();
        }
        else
        {
            total += generate_digit();
        }
    }
    append_char('"');
    return total;
}

int generate_visible_string_char(void)
{
    char visible_char = visible_chars[rand() % strlen(visible_chars)];
    append_char(visible_char);
    return 1;
}
