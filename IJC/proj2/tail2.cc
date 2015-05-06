/*
 * Author: Frantisek Sumsal <xsumsal01@stud.fit.vutbr.cz> BUT FIT
 * Date: 31.3.2015
 * File: tail2.cc
 * Compiled with: gcc 4.9.2, gcc 4.8.4 and 4.9.1
 * 
 * This file is part of IJC project.
 * This solution is slow as well as the 
 * one in C, because it has to continuously manage
 * buffer queue.
 * 
 * Benchmarks:
 * # Built-in tail
 * $ tail 30mbfile
 * real    0m0.003s
 * user    0m0.000s
 * sys     0m0.001s
 * 
 * # This tail
 * $./tail 30mbfile
 * real    0m0.160s
 * user    0m0.139s
 * sys     0m0.015s
 */ 

#include <iostream>
#include <fstream>
#include <queue>
#include <cstring>

using namespace std;

/**
 * @brief Checks, if given string is a valid number
 * @details This implementation should be better than the one
 *          from tail.c, but it still lacks proper error handling
 * 
 * @param str C string which should be checked
 * @return True, if given string is a valid number, false otherwise
 */
bool is_int(const char *str);

/**
 * @brief Converts given string to positive integer
 * @details Again, this should be a better implementation but (again)
 *          it lacks proper error handling.
 * 
 * @param str C string which should be converted
 * @return Converted integer if the number is >= 0, -1 otherwise
 */
int str_to_pint(const char *str);

/**
 * @brief Prints last max lines from file or start+ lines if start is initialized
 * @details Does (or should do) the same as tail command on *NIX systems
 * 
 * @param file Pointer to string with file name. If set to NULL, stdin will be used
 * @param start First line which should be printed. If set to -1, only last max lines will be printed
 * @param max Limits how many lines will be printed. If set to -1, all lines until the end of file will be printed
 */
void parse_file(const char *file, int start, int max);

int main(int argc, char *argv[])
{
    char *file = NULL;
    int start_line = -1;
    int max_line = 10;

    /* This turns off synchronization of C++ streams with their C equivalents. */
    std::ios::sync_with_stdio(false);

    /* Shamelessly copied from tail.c */
    /* This is a little bit messy, but I just didn't want to use getops for one parameter */
    if(argc == 2) { 
        file = argv[1];
    } else if(argc >= 3 && strcmp(argv[1], "-n") == 0) {
        if(strlen(argv[2]) >= 2 && argv[2][0] == '+' && is_int(&argv[2][1])) {
            start_line = str_to_pint(&argv[2][1]);

            if(start_line < 0) {
                cerr << "Invalid value for -n parameter." << endl;
                exit(1);
            }

            if(start_line > 0)
                start_line--;

            max_line = -1;
        } else if(is_int(argv[2])) {
            max_line = str_to_pint(argv[2]);

            if(max_line < 0) {
                cerr << "Invalid value for -n parameter." << endl;
                exit(1);
            }
        } else {
            cerr << "Invalid value for -n parameter." << endl;
            exit(1);
        }

        if(argc >= 4) {
            file = argv[3];
        }
    }

    parse_file(file, start_line, max_line);

    return 0;
}

bool is_int(const char *str)
{
    char *pc;

    strtol(str, &pc, 10);

    return (*pc == '\0');
}

int str_to_pint(const char *str)
{
    char *pc;
    int x;

    x = strtol(str, &pc, 10);

    if(*pc == '\0' && x >= 0)
        return x;

    return -1;
}

void parse_file(const char *file, int start, int max)
{
    istream *in = &cin;
    ifstream f_in;
    string read_buff;
    queue<string> print_buff;
    int current_line = 0;

    /* Little fun with pointers to have one variable for fstream/istream */
    if(file != NULL) {
        f_in.open(file);

        if(f_in.is_open() == false) {
            cerr << "Unable to open file " << file << endl;
            exit(2);
        }

        in = &f_in;
    }

    while(getline(*in, read_buff)) {
        if(start != -1) {
            if(current_line >= start)
                cout << read_buff << endl;
        } else {
            /* Maintain only max items in print queue */
            print_buff.push(read_buff);

            /* In this part we are sure that max is a positive (or zero) integer
             * so we can typecast it to unsigned int to make compiler happy */
            if(print_buff.size() > (unsigned int)max)
                print_buff.pop();
        }
        
        current_line++;
    }

    /* This part gets interesting when you use text file created on windows
     * on *nix system and you forgot to print 'endl' after each line */ 
    while(print_buff.empty() == false) {
        cout << print_buff.front() << endl;
        print_buff.pop();
    }

    if(f_in.is_open())
        f_in.close();
}