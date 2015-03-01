/*
 * @brief   Projekt 2 - Iteracni vypocty
 * @author  Frantisek Sumsal (xsumsa01@stud.fit.vutbr.cz)
 *          VUT 1 BIT (BIB - 37)
 * @date    30.9.2014
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

/**
 * @brief PI constant.
 * @details In C99 M_PI constant is undefined. Workaround
 *          would be to use GNU99 which wasn't allowed.
 */
#define IZP_PI 3.14159265358979323846264338327

/** Data types */

/**
 * @brief Structure for saving pairs of coeficients for Taylor's theorem.
 */
struct taylor_coef_t {
    unsigned long long int num; /**< Numerator */
    unsigned long long int den; /**< Denominator */
};

/**
 * @brief Error codes
 */
enum ecodes {
    E_OK = 0,   /**< Everything's ok */
    E_IRANGE,   /**< Value is out of range */
    E_IINPUT,   /**< Invalid input */
    E_OTHER     /**< Other error */
};

/**
 * @brief Checks, if given string contains only numbers.
 * 
 * @param s null terminated string to check
 * @param f scanf/printf format of number to check
 * 
 * @return true if string contains only numbers, false otherwise
 */
bool str_is_num(const char *s, const char *f);

/**
 * @brief Calculates absolute value of given number.
 * @details [long description]
 * 
 * @param x number
 * 
 * @return absolute value of given number
 */
double my_abs(double x);

/**
 * @brief Calculates tangent using Taylor's theorem.
 * 
 * @param x angle in radians
 * @param n number of iterations (max 13)
 * 
 * @return tangent of given angle
 */
double taylor_tan(double x, unsigned int n);

/**
 * @brief Calculates tangent using continued fractions.
 * 
 * @param x angle in radians
 * @param n number of iterations
 * 
 * @return tangent of given angle
 */
double cfrac_tan(double x, unsigned int n);

/**
 * @brief Prints comparison table for tangent functions.
 * @details Prints comparison table for tan(), taylor_tan(), cfrac_tan()
 *          and their absolute tolerances.
 * 
 * @param a angle in radians
 * @param n initial iteration (for functions taylor_tan() and cfrac_tan())
 * @param m final iteration (for functions taylor_tan() and cfrac_tan())
 */
void compare_tans(double a, int n, int m);

/**
 * @brief Calculates tangent using cfrac_tan() function.
 * @details Approximates tangent by comparing absolute difference between 
 *          last and current result of cfrac_tan() function.
 * 
 * @param a angle in radians
 * @return tangent of given angle
 */
double calc_tan(double a);

/**
 * @brief   Calculates distance between measuring device and object,
 *          and height of the object if beta is specified.
 * @details For more information about this mysterious object, please 
 *          refer to project's description.
 * 
 * @param alpha angle alpha [required]
 * @param beta angle beta [optional]
 * @param c height of the measuring device [optional] (default: 1.5)
 */
void calc_dist(double alpha, double beta, double c);

/** Global variables **/

/**
 * @brief Coeficients for calculating tangent using Taylor's theorem.
 * @details Taken from:
 *          https://oeis.org/A002430
 *          https://oeis.org/A156769
 */ 
struct taylor_coef_t tc[] = {
    {1,                 1},
    {1,                 3},
    {2,                 15},
    {17,                315},
    {62,                2835},
    {1382,              155925},
    {21844,             6081075},
    {929569,            638512875},
    {6404582,           10854718875},
    {443861162,         1856156927625},
    {18888466084,       194896477400625},
    {113927491862,      49308808782358125},
    {58870668456604,    3698160658676859375}
};

/** Function definitions **/
int main(int argc, char *argv[])
{
    /* These aren't probably the best names for variables, so let's have a brief descrption:
     *  a => angle in radians (procedure compare_tans)
     *  n => first iteration for comparison (procedure compare_tans)
     *  m => last iteration for comparison (procedure compare_tans)
     *  c => height of measuring device (procedure calc_dist)
     *  alpha => angle alpha (procedure calc_dist)
     *  beta => angle beta (procedure calc_dist)
     *  idx => index of next argument (used for parsing optional argument -c)
     *  ec => error code
     */
    double a, c = 1.5, alpha, beta = -1;
    int n, m, idx = 1, ec = E_OK;

    /* Parsing arguments without proper parser is horrible... */
    if(argc == 2 && strcmp(argv[1], "--help") == 0) {
        printf("Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>\n" 
                "Usage:\n\n"
                "%s --help\n\nor\n\n"
                "%s --tan A N M\n\nor\n\n"
                "%s [-c X] -m A [B]\n\n"
                "Arguments:\n"
                "\t--tan A N M\tCompares results of tangent functions tan (from math.h), taylor_tan (Taylor's theorem)\n"
                    "\t\t\tand cfrac_tan (continued fractions).\n"
                    "\t\t\tA = angle in radians, N = initial iteration, M = final iteration.\n"
                "\t-c X\t\t[Optional] Specifies height of measuring device for calculating distances by argument -m (default: 1.5)\n"
                "\t-m A [B]\tCalculates distance between measuring device and the object.\n"
                    "\t\t\tIf B is specified, also calculates height of the object.\n"
                    "\t\t\tA = angle alpha, B = [optional] angle beta.\n", 
                argv[0], argv[0], argv[0]);
    } else if(argc == 5 && strcmp(argv[1], "--tan") == 0) {
        /* Check, if arguments 3, 4 and 5 are valid numbers. */
        if(str_is_num(argv[2], "%lf") && str_is_num(argv[3], "%d") && str_is_num(argv[4], "%d")) {
            /* If so, convert them and save to appropriate variables. */
            sscanf(argv[2], "%lf", &a);
            sscanf(argv[3], "%d", &n);
            sscanf(argv[4], "%d", &m);

            /* Check, if given numbers are in valid ranges. If so, call the final procedure. */
            if(a > -IZP_PI/2 && a < IZP_PI/2 && n > 0 && m >= n && m < 14) {
                compare_tans(a, n, m);
            } else {
                fprintf(stderr, "Invalid range of arguments.\n");
                ec = E_IRANGE;
            }
        } else {
            fprintf(stderr, "Arguments for --tan must be valid numbers where -PI/2 <= A <= PI/2 and 0 < N <= M < 14\n");
            ec = E_IINPUT;
        }
    } else if(argc >= 3) {
        /* If second argument is "-c" followed by valid number, parse it and set index of next argument */
        if(argc >= 5 && strcmp(argv[1], "-c") == 0 && str_is_num(argv[2], "%lf")) {
            sscanf(argv[2], "%lf", &c);
            if(c < 0 || c > 100) {
                fprintf(stderr, "X is out of range: 0 < X <= 100\n");
                ec = E_IRANGE;
            }
            idx = 3;
        }

        /* Parse angles alpha (required) and beta (optional) */
        if(ec == E_OK) {
            if(strcmp(argv[idx], "-m") == 0 && str_is_num(argv[idx + 1], "%lf")) {
                sscanf(argv[idx + 1], "%lf", &alpha);
                if(alpha < 0 || alpha > 1.4) {
                    fprintf(stderr, "A is out of range: 0 < A <= 1.4\n");
                    ec = E_IRANGE;
                } 

                if(ec == E_OK && argc >= idx + 3 && str_is_num(argv[idx + 2], "%lf")) {
                    sscanf(argv[idx + 2], "%lf", &beta);
                    if(beta < 0 || beta > 1.4) {
                        fprintf(stderr, "B is out of range: 0 < B <= 1.4\n");
                        ec = E_IRANGE;
                    }
                }
            } else {
                fprintf(stderr, "Invalid arguments, try %s --help\n", argv[0]);
                ec = E_IINPUT;
            }
        }

        if(ec == E_OK) calc_dist(alpha, beta, c);
    } else {
        fprintf(stderr, "Invalid arguments, try %s --help\n", argv[0]);
        ec = E_IINPUT;
    } 

    return ec;
}

bool str_is_num(const char *s, const char *f)
{
    double n;

    if(sscanf(s, f, &n) != 1)
        return false;

    return true;
}

double my_abs(double x)
{
    return (x < 0.0 ? x * -1.0 : x);
}

double taylor_tan(double x, unsigned int n)
{
    double res = x;
    double base = x;

    x *= base * base;
    for(unsigned int i = 1; i < n; i++) {
        res += ((tc[i].num * x) / (tc[i].den));
        x *= base * base;
    }

    return res;
}

double cfrac_tan(double x, unsigned int n)
{
    int d = n * 2 - 1;
    double res = d - (x * x) / d + 2;

    while(n-- > 0) {
        res = d - ((x * x) / res);
        d -= 2;
    }

    return x / res;
}

void compare_tans(double a, int n, int m)
{
    double mtan, ttan, cftan;
    for(int i = n; i <= m; i++) {
        mtan = tan(a);
        ttan = taylor_tan(a, i);
        cftan = cfrac_tan(a, i);

        printf("%d %e %e %e %e %e\n", i, mtan, ttan, my_abs(ttan - mtan), cftan, my_abs(cftan - mtan));
    }
}

double calc_tan(double a)
{
    unsigned int it = 1;
    double eps = 0.0000000001;
    double tan_prev = cfrac_tan(a, it);
    double tan_act = cfrac_tan(a, ++it);

    while(my_abs(tan_act - tan_prev) > eps) {
        tan_prev = tan_act;
        tan_act = cfrac_tan(a, ++it);
    }

    return tan_act;
}

void calc_dist(double alpha, double beta, double c)
{
    double d, v;

    d = c / calc_tan(alpha);
    printf("%.10e\n", d);

    if(beta != -1) {
        v = c + (calc_tan(beta) * d);
        printf("%.10e\n", v);
    }
}