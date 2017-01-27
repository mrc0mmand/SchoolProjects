// https://www.codesandciphers.org.uk/enigma/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define ROTORS_ACTIVE 3
#define ROTORS_TOTAL 3
#define IDX2C(x) ('A' + x)
#define C2IDX(x) (x - 'A')

typedef struct {
    // 0 - 25
    short int position;
    // 'A' - 'Z'
    char switch_char;
    const char out_chars[27];
} rotor_t;

//                                 ABCDEFGHIJKLMNOPQRSTUVWXYZ
static rotor_t rotor_I = {0, 'R', "EKMFLGDQVZNTOWYHXUSPAIBRCJ"};

//                                  ABCDEFGHIJKLMNOPQRSTUVWXYZ
static rotor_t rotor_II = {0, 'F', "AJDKSIRUXBLHWTMCQGZNPYFVOE"};

//                                   ABCDEFGHIJKLMNOPQRSTUVWXYZ
static rotor_t rotor_III = {0, 'W', "BDFHJLCPRTXVZNYEIWGAKMUSQO"};

//                        ABCDEFGHIJKLMNOPQRSTUVWXYZ
const char reflector[] = "YRUHQSLDPXKGONMIEBFZCWVJAT";
char plugboard[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

rotor_t *rotor_config[ROTORS_ACTIVE] = {NULL,};

int init_plugboard(const char *str);
int init_rotors(const char *str);
char *process_string(char *str);
char process_char(char c);

int main(int argc, char *argv[])
{
    char *result;

    if(argc < 3)
        goto help;

    if(init_rotors(argv[1]) != 0)
        goto help;

    if(init_plugboard(argv[2]) != 0)
        goto help;

    if(argc >= 4) {
        result = process_string(argv[3]);
        puts(result);
    }

    goto end;

help:
    fprintf(stderr, "Usage: %s \"R RS R RS R RS\" \"CC CC CC ...(x10)\"\n"
        "where:\n"
        "\tR\t\trotor number (1 - %d)\n"
        "\tRS\t\tinitial rotor state (0 - 25)\n"
        "\tCC\t\tswap combination for plugboard - eg.: AB CD EF\n",
        argv[0], ROTORS_TOTAL);
    return 1;

end:
    return 0;
}

int init_plugboard(const char *str)
{
    if(str == NULL)
        return 1;

    bool alphabet[26] = {false, };
    char pair[2] = {0,};
    size_t offset = 0;
    int read = 0;
    int i;

    // FORMAT: AB CD EF ... (max x10)
    for(i = 0; i < 10; i ++) {
        if(sscanf(&str[offset], "%2s%n", pair, &read) != 1)
            return 1;

        if(!isalpha(pair[0]) || !isalpha(pair[1]))
            return 1;

        pair[0] = toupper(pair[0]);
        pair[1] = toupper(pair[1]);

        if(alphabet[pair[0] - 'A'] || alphabet[pair[1] - 'A'])
            return 1;

        if(pair[0] == pair[1])
            return 1;

        plugboard[pair[0] - 'A'] = pair[1];
        plugboard[pair[1] - 'A'] = pair[0];

        alphabet[pair[0] - 'A'] = true;
        alphabet[pair[1] - 'A'] = true;

        offset += read;
        if(offset >= strlen(str))
            break;
    }

    return 0;
}

int init_rotors(const char *str)
{
    if(str == NULL)
        return 1;

    size_t offset = 0;
    int read;
    int rnum;
    int rstate;
    int i;

    for(i = 0; i < ROTORS_ACTIVE; i++) {
        if(sscanf(&str[offset], "%d %d%n", &rnum, &rstate, &read) != 2)
            return 1;

        if(rnum < 1 || rnum > ROTORS_TOTAL)
            return 1;

        if(rstate < 0 || rstate > 25)
            return 1;

        switch(rnum) {
        case 1:
            rotor_config[i] = &rotor_I;
            break;
        case 2:
            rotor_config[i] = &rotor_II;
            break;
        case 3:
            rotor_config[i] = &rotor_III;
            break;
        default:
            return 1;
        };

        rotor_config[i]->position = rstate;

        offset += read;
        if(offset >= strlen(str))
            break;
    }

    return 0;
}

char *process_string(char *str)
{
    if(str == NULL)
        return NULL;

    int i;

    for(i = 0; str[i] != '\0'; i++) {
        str[i] = process_char(str[i]);
    }

    return str;
}

char process_char(char c)
{
    char r = toupper(c);
    int i, j;
    int idx;
    int pos;

    if(!isalpha(c))
        return c;

    // Plugboard
    r = plugboard[r - 'A'];

    // Rotor movement
    rotor_config[0]->position = (rotor_config[0]->position + 1) % 26;
    for(i = 0; i < ROTORS_ACTIVE- 1; i++) {
        if(rotor_config[i]->position == C2IDX(rotor_config[i]->switch_char)) {
            pos = rotor_config[i + 1]->position;
            rotor_config[i + 1]->position = (pos + 1) % 26;
        }
    }

    // First rotor pass
    // -> R0 -> R1 -> R2
    for(i = 0; i < ROTORS_ACTIVE; i++) {
        pos = rotor_config[i]->position;
        // Get index of the character on the rotor's output
        idx = (C2IDX(r) + pos) % 26;
        r = rotor_config[i]->out_chars[idx];
    }

    // -> REFLECTOR
    r = reflector[C2IDX(r)];

    // Second rotor pass
    // -> R2 -> R1 -> R0
    for(i = ROTORS_ACTIVE - 1; i >= 0; i--) {
        pos = rotor_config[i]->position;
        for(j = 0; j < 26; j++) {
            if(r == rotor_config[i]->out_chars[j]) {
                idx = (j - pos) % 26;
                if(idx < 0)
                    idx += 26;

                break;
            }
        }

        r = IDX2C(idx);
    }

    // Plugboard
    r = plugboard[r - 'A'];

    return r;
}
