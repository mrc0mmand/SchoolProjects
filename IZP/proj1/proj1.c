/*
 * @brief   Projekt 1 - Vypocty v tabulce
 * @author  Frantisek Sumsal (xsumsa01@stud.fit.vutbr.cz)
 *          VUT 1 BIT (BIB - 37)
 * @date    29.9.2014
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define FATAL(...) { fprintf(stderr, __VA_ARGS__); exit(1); }

/* Data types */

/**
 * @brief Operation codes.
 */
enum op_code {
    OP_INVALID = -1,
    OP_SELECT = 0,
    OP_MIN,
    OP_MAX,
    OP_SUM,
    OP_AVG
};

/** 
 * @brief Selection codes.
 */
enum sl_code {
    SL_INVALID = -1,
    SL_ROW = 0,
    SL_COL,
    SL_ROWS,
    SL_COLS,
    SL_RANGE
};

/**
 * @brief Structure for defining valid opeations.
 */
struct op_t {
    const char *name;   /**< Operation name */
    enum op_code code;  /**< Operation code */
    void (*f)();        /**< Operation processing function */
};

/**
 * @brief Structure for defining valid selections.
 */
struct sl_t {
    const char *name;       /**< Selection name */
    enum sl_code code;      /**< Selection code */
    unsigned short int ac;  /**< Selection argument count */
};

/* Function declarations */

/**
 * @brief Checks, if given string contains only numbers.
 * 
 * @param s null terminated string to check
 * @param f canf/printf format of number to check (check scanf/printf docs)
 * 
 * @return true if string contains only numbers, false otherwise
 */
bool str_is_num(const char *s, const char *f);

/**
 * @brief Converts given string to integer.
 * @details String should be checked by str_is_num before
 * calling str_to_int. Actually, this function is not
 * 'bulletproof', because it strips number's fractional part
 * (if it has one) without proper rounding. But for needs of 
 * this project it should be enough.
 * 
 * @param s null terminated string which contains number to convert
 * @return converted number as integer
 */
int str_to_int(const char *s);

 /**
  * @brief Compares given operation with array of valid operations.
  * 
  * @param op null terminated string which contains name of operation to check
  * @return op_code of valid operation on success, OP_INVALID otherwise
  */
int check_operation(const char *op);

/**
 * @brief Compares given selection with array of valid selections.
 * @details Function also checks number of arguments of given selection method.
 * 
 * @param sl null terminated string which contains name of selection method
 * @param ra number of remaining arguments in argv array
 * 
 * @return sl_code of valid operation on success, SL_INVALID otherwise
 */
int check_selection(const char *sl, unsigned int ra);

/**
 * @brief Checks coordinations passed from command line.
 * 
 * @param slc selection code
 * @param ra array of remaining arguments from command line
 */
void check_coords(enum sl_code slc, char *ra[]);

/**
 * @brief Prints content of cells from given range.
 * 
 * @param srow start row
 * @param erow end row
 * @param scol start column
 * @param ecol end column
 */
void op_select(unsigned int srow, unsigned int erow, unsigned int scol, unsigned int ecol);

/**
 * @brief Find minimum valie from given range of cells.
 * @details Works ONLY for cells with valid numbers.
 * 
 * @param srow start row
 * @param erow end row
 * @param scol start column
 * @param ecol end column
 */
void op_min(unsigned int srow, unsigned int erow, unsigned int scol, unsigned int ecol);

/**
 * @brief Finds maximum value from given range of cells.
 * @details Works ONLY for cells with valid numbers.
 * 
 * @param srow start row
 * @param erow end row
 * @param scol start column
 * @param ecol end column
 */
void op_max(unsigned int srow, unsigned int erow, unsigned int scol, unsigned int ecol);

/**
 * @brief Calculates total sum of cells from given range.
 * @details Works ONLY for cells with valid numbers.
 * 
 * @param srow start row
 * @param erow end row
 * @param scol start column
 * @param ecol end column
 */
void op_sum(unsigned int srow, unsigned int erow, unsigned int scol, unsigned int ecol);

/**
 * @brief Calculates average value of cells from given range.
 * @details Works ONLY for cells with valid numbers.
 * 
 * @param srow start row
 * @param erow end row
 * @param scol start column
 * @param ecol end column
 */
void op_avg(unsigned int srow, unsigned int erow, unsigned int scol, unsigned int ecol);

/* Global variables **/
const struct op_t ops[] = {
    {"select",  OP_SELECT,  op_select},
    {"min",     OP_MIN,     op_min},
    {"max",     OP_MAX,     op_max},
    {"sum",     OP_SUM,     op_sum},
    {"avg",     OP_AVG,     op_avg},
    {NULL,      OP_INVALID, NULL}
};

const struct sl_t sls[] = {
    {"row",     SL_ROW,     1},
    {"col",     SL_COL,     1},
    {"rows",    SL_ROWS,    2},
    {"cols",    SL_COLS,    2},
    {"range",   SL_RANGE,   4},
    {NULL,      SL_INVALID, 0}
};

/* Function definitions */
int main(int argc, char *argv[])
{
    enum op_code opc = OP_INVALID;
    enum sl_code slc = SL_INVALID;
    unsigned int a, b, x, y;

    if(argc == 2 && strcmp(argv[1], "--help") == 0) {
        printf("Usage:\n\n"
                "%s --help\n\nor\n\n"
                "%s <operation> <selection> <coord(s)>\n\n"
                "Valid operations:\n"
                "\tselect\tselects and prints cells' values from given range\n"
                "\tmin\tfinds and prints minimum value from given range (works ONLY for cells with numerical values!)\n"
                "\tmax\tfinds and prints maximum value from given range (works ONLY for cells with numerical values!)\n"
                "\tsum\tcalculates total sum of numbers from given range (works ONLY for cells with numerical values!)\n"
                "\tavg\tcalculates average value from given range (works ONLY for cells with numerical values!)\n\n"
                "Valid selections:\n"
                "\trow X\t\tselects all cells from row X where X > 0\n"
                "\tcol X\t\tselects all cells from column X where X > 0\n"
                "\trows X Y\tselects all cells from row X to Y (inclusive) where 0 < X <= Y\n"
                "\tcols X Y\tselects all cells from column X to Y (inclusive) where 0 < X <= Y\n"
                "\trange X Y A B\tselects all cells from row X to Y (inclusive) and from column A to B (inclusive) "
                    "where 0 < X <= Y, 0 < A <= B\n",
                argv[0], argv[0]);
    } else if(argc >= 4) {
        if((opc = check_operation(argv[1])) == OP_INVALID) {
            FATAL("Invalid operation '%s'.\nAvailable operations: select, min, max, sum, avg.\n", argv[1]);
        }

        if((slc = check_selection(argv[2], argc - 3)) == SL_INVALID) {
            FATAL("Invalid selection (%s) or number of arguments (%d).\n"
                    "Available selections: row(1), col(1), rows(2), cols(2), range(4)\n", argv[2], argc - 3);
        }

        check_coords(slc, &argv[3]);

        a = str_to_int(argv[3]);
        b = (argc >= 5 ? str_to_int(argv[4]) : 0);
        x = (argc >= 6 ? str_to_int(argv[5]) : 0);
        y = (argc >= 7 ? str_to_int(argv[6]) : 0);

        switch(slc) {
            case SL_ROW:
                ops[opc].f(a, a, 0, 0);
            break;
            case SL_COL:
                ops[opc].f(0, 0, a, a);
            break;
            case SL_ROWS:
                ops[opc].f(a, b, 0, 0);
            break;
            case SL_COLS:
                ops[opc].f(0, 0, a, b);
            break;
            case SL_RANGE:
                ops[opc].f(a, b, x, y);
            break;
            /* This should never happen. Added just to make compiler happy. */
            case SL_INVALID:
            default:
                FATAL("Invalid selection.\n");
        }
    } else {
        FATAL("Invalid arguments, try %s --help\n", argv[0]);
    }

    return 0;
}

bool str_is_num(const char *s, const char *f)
{
    double n;

    if(sscanf(s, f, &n) != 1)
        return false;

    return true;
}

int str_to_int(const char *s)
{
    int n;

    sscanf(s, "%d", &n);

    return n;
}

int check_operation(const char *op)
{
    for(unsigned int i = 0; ops[i].name; i++) {
        if(strcmp(op, ops[i].name) == 0)
            return ops[i].code;
    }

    return OP_INVALID;
}

int check_selection(const char *sl, unsigned int ra)
{
    for(unsigned int i = 0; sls[i].name; i++) {
        if(strcmp(sl, sls[i].name) == 0 && ra == sls[i].ac)
            return sls[i].code;
    }

    return SL_INVALID;
}

void check_coords(enum sl_code slc, char *ra[])
{
    /* This is MAYBE a BIT ugly... */
    /* Checking of ra array is unnecessary here, we've already 
     * checked number of arguments in check_selection
     */
    switch(slc) {
        case SL_ROW:
        case SL_COL:
            if(!(str_is_num(ra[0], "%d") && str_to_int(ra[0]) > 0))
                FATAL("Invalid coord [x]: [%s] => x must be an integer where x > 0\n", ra[0]);
        break;
        case SL_ROWS:
        case SL_COLS:
            if(!(str_is_num(ra[0], "%d") && str_to_int(ra[0]) > 0 &&
                str_is_num(ra[1], "%d") && str_to_int(ra[1]) > str_to_int(ra[0])))
                    FATAL("Invalid coords [x; y]: [%s; %s] => x, y must be integers where 0 > x > y\n", ra[0], ra[1]);
        break;
        case SL_RANGE:
            if(!(str_is_num(ra[0], "%d") && str_to_int(ra[0]) > 0 &&
                str_is_num(ra[1], "%d") && str_to_int(ra[1]) >= str_to_int(ra[0])))
                    FATAL("Invalid coords [a; b]: [%s; %s] => a, b must be integers where 0 > a >= b\n", ra[0], ra[1]);

            if(!(str_is_num(ra[2], "%d") && str_to_int(ra[2]) > 0 &&
                str_is_num(ra[3], "%d") && str_to_int(ra[3]) >= str_to_int(ra[2])))
                    FATAL("Invalid coords [x; y]: [%s; %s] => x, y must be integers where 0 > x >= y\n", ra[2], ra[3]);
        break;
        /* This should never happen. Added just to make compiler happy. */
        case SL_INVALID:
        default:
            FATAL("Invalid selection.\n");
    }
}

void op_select(unsigned int srow, unsigned int erow, unsigned int scol, unsigned int ecol)
{
    /* Max lenght of one line is 1024 chars + null char */
    char buffer[1025] = "\0";
    char cell[1025] = "\0";
    unsigned int col = 0, row = 0;
    int offset = 0, read = 0;
    double n = 0;

    while(fgets(buffer, 1025, stdin)) {
        row++;
        if(srow && row < srow) continue;
        if(erow && row > erow) break;

        for(col = 1; ((ecol != 0) ? col <= ecol : 1); col++) {
            if(sscanf(&buffer[offset], "%1024s%n", (char *)&cell, &read) == 1) {
                offset += read;

                if(scol && col < scol) continue;

                if(str_is_num(cell, "%lf")) {
                    sscanf(cell, "%lf", &n);
                    printf("%.10g\n", n);
                } else {
                    printf("%s\n", cell);
                }
            } else {
                if(ecol != 0) {
                    FATAL("Invalid input data. (row: %u, col: %u)\n", row, col);
                } else {
                    break;
                }
            }
        }

        offset = 0;
        *cell = '\0';
    }
}

void op_min(unsigned int srow, unsigned int erow, unsigned int scol, unsigned int ecol)
{
    char buffer[1025] = "\0";
    char cell[1025] = "\0";
    unsigned int col = 0, row = 0;
    int offset = 0, read = 0;
    double min = 0, n = 0;
    bool first = true;

    while(fgets(buffer, 1025, stdin)) {
        row++;
        if(srow && row < srow) continue;
        if(erow && row > erow) break;

        for(col = 1; ((ecol != 0) ? col <= ecol : 1); col++) {
            if(sscanf(&buffer[offset], "%1024s%n", (char *)&cell, &read) == 1) {
                offset += read;

                if(scol && col < scol) continue;

                if(str_is_num(cell, "%lf")) {
                    sscanf(cell, "%lf", &n);
                    if(first) {
                        min = n;
                        first = false;
                    } else {
                        min = (n < min) ? n : min;
                    }
                } else {
                    FATAL("Invalid input data (operation min accepts only cells with numbers).\n");
                }
            } else {
                if(ecol != 0) {
                    FATAL("Invalid input data. (row: %u, col: %u)\n", row, col);
                } else {
                    break;
                }
            }
        }

        offset = 0;
        *cell = '\0';
    }

    printf("%.10g\n", min);
}

void op_max(unsigned int srow, unsigned int erow, unsigned int scol, unsigned int ecol)
{
    char buffer[1025] = "\0";
    char cell[1025] = "\0";
    unsigned int col = 0, row = 0;
    int offset = 0, read = 0;
    double max = 0, n = 0;
    bool first = true;

    while(fgets(buffer, 1025, stdin)) {
        row++;
        if(srow && row < srow) continue;
        if(erow && row > erow) break;

        for(col = 1; ((ecol != 0) ? col <= ecol : 1); col++) {
            if(sscanf(&buffer[offset], "%1024s%n", (char *)&cell, &read) == 1) {
                offset += read;

                if(scol && col < scol) continue;

                if(str_is_num(cell, "%lf")) {
                    sscanf(cell, "%lf", &n);
                    if(first) {
                        max = n;
                        first = false;
                    } else {
                        max = (n > max) ? n : max;
                    }
                } else {
                    FATAL("Invalid input data (operation max accepts only cells with numbers).\n");
                }
            } else {
                if(ecol != 0) {
                    FATAL("Invalid input data. (row: %u, col: %u)\n", row, col);
                } else {
                    break;
                }
            }
        }

        offset = 0;
        *cell = '\0';
    }

    printf("%.10g\n", max);
}

void op_sum(unsigned int srow, unsigned int erow, unsigned int scol, unsigned int ecol)
{
    char buffer[1025] = "\0";
    char cell[1025] = "\0";
    unsigned int col = 0, row = 0;
    int offset = 0, read = 0;
    double sum = 0, n = 0;

    while(fgets(buffer, 1025, stdin)) {
        row++;
        if(srow && row < srow) continue;
        if(erow && row > erow) break;

        for(col = 1; ((ecol != 0) ? col <= ecol : 1); col++) {
            if(sscanf(&buffer[offset], "%1024s%n", (char *)&cell, &read) == 1) {
                offset += read;

                if(scol && col < scol) continue;

                if(str_is_num(cell, "%lf")) {
                    sscanf(cell, "%lf", &n);
                    sum += n;
                } else {
                    FATAL("Invalid input data (operation sum accepts only cells with numbers).\n");
                }
            } else {
                if(ecol != 0) {
                    FATAL("Invalid input data. (row: %u, col: %u)\n", row, col);
                } else {
                    break;
                }
            }
        }

        offset = 0;
        *cell = '\0';
    }

    printf("%.10g\n", sum);
}

void op_avg(unsigned int srow, unsigned int erow, unsigned int scol, unsigned int ecol)
{
    char buffer[1025] = "\0";
    char cell[1025] = "\0";
    unsigned int col = 0, row = 0;
    int offset = 0, read = 0;
    double avg = 0, n = 0, count = 0;

    while(fgets(buffer, 1025, stdin)) {
        row++;
        if(srow && row < srow) continue;
        if(erow && row > erow) break;

        for(col = 1; ((ecol != 0) ? col <= ecol : 1); col++) {
            if(sscanf(&buffer[offset], "%1024s%n", (char *)&cell, &read) == 1) {
                offset += read;

                if(scol && col < scol) continue;

                if(str_is_num(cell, "%lf")) {
                    sscanf(cell, "%lf", &n);
                    avg += n;
                    count++;
                } else {
                    FATAL("Invalid input data (operation avg accepts only cells with numbers).\n");
                }
            } else {
                if(ecol != 0) {
                    FATAL("Invalid input data. (row: %u, col: %u)\n", row, col);
                } else {
                    break;
                }
            }
        }

        offset = 0;
        *cell = '\0';
    }

    printf("%.10g\n", avg/count);
}