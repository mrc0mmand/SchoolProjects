/**
 * @file main.c (original)
 * @author Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>
 * @date 14.12.2016
 * @brief Enigma implementation for MCU
 *
 * @see https://www.codesandciphers.org.uk/enigma/index.htm
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <lcd/display.h>
#include <keyboard/keyboard.h>
#include <fitkitlib.h>

#define LCD_SIZE (16 * 2)
#define ROTORS_ACTIVE 3
#define ROTORS_TOTAL 3
#define IDX2C(x) ('A' + x)
#define C2IDX(x) (x - 'A')

/**
 * @brief Rotor configuration data structure
 */
typedef struct {
    short int id;               /**< Rotor ID */
    // 0 - 25
    short int position;         /**< Current rotor position */
    // 'A' - 'Z'
    char switch_char;           /**< The CARRY notch */
    const char out_chars[27];   /**< Rotor permutation string */
} rotor_t;

/**
 * @brief Internal LCD state data structure
 */
typedef struct {
    char content[LCD_SIZE];     /**< LCD content */
    short int position;         /**< Cursor position */
} lcd_control_t;

//                                    ABCDEFGHIJKLMNOPQRSTUVWXYZ
static rotor_t rotor_I = {1, 0, 'R', "EKMFLGDQVZNTOWYHXUSPAIBRCJ"};

//                                     ABCDEFGHIJKLMNOPQRSTUVWXYZ
static rotor_t rotor_II = {2, 0, 'F', "AJDKSIRUXBLHWTMCQGZNPYFVOE"};

//                                      ABCDEFGHIJKLMNOPQRSTUVWXYZ
static rotor_t rotor_III = {3, 0, 'W', "BDFHJLCPRTXVZNYEIWGAKMUSQO"};

//                        ABCDEFGHIJKLMNOPQRSTUVWXYZ
const char reflector[] = "YRUHQSLDPXKGONMIEBFZCWVJAT";
char plugboard[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int delay_counter = 0;

rotor_t *rotor_config[ROTORS_ACTIVE] = {&rotor_I, &rotor_II, &rotor_III};
lcd_control_t lcd_control = {{0,}, 0};

/**
 * @brief Write a character to LCD
 * @details Function updates internal strucuture with LCD state and then
 *          calls LCD_append_char.
 *
 * @param c Character to write
 */
void LCD_add_char(char c);

/**
 * @brief Remove character from LCD
 */
void LCD_remove_last_char();

/**
 * @brief Clear LCD
 * @details Funtion clears the internal strucuture with LCD state as well
 *          as the LCD itself.
 */
void LCD_clear_display();

/**
 * @brief Redraw all characters on LCD.
 */
void LCD_redraw();

/**
 * @brief Print user help
 */
void print_user_help(void);

/**
 * @brief Decode user command
 *
 * @param ucmd User string in uppercase
 * @param cmd User string
 */
unsigned char decode_user_cmd(char *ucmd, char *cmd);

/**
 * @brief FPGA initialization callback
 */
void fpga_initialized();

/**
 * @brief Keyboard handler
 *
 * @return Always 0 (in this case)
 */
int keyboard_idle();

/**
 * @brief Initialize plugboard of the Enigma machine
 * @details Simulates a plugboard of typical Enigma machine, which can
 *          switch 10 letter pair at most.
 *
 * @param conf Configuration string (PLUGBOARD AB CD EF ...)
 * @return 0 on success, 1 otherwise
 */
int init_plugboard(char *conf);

/**
 * @brief Print plugboard configuration to terminal
 */
void print_plugboard_configuration();

/**
 * @brief Initialize rotors of the Enigma machine
 * @detals Simulates rotors of typical Enigma machine, which has three rotors,
 *         where each of them has 26 states.
 *
 * @param conf Configuration string (ROTORS ID STATE ID STATE ID STATE)
 * @return 0 on success, 1 otherwise
 */
int init_rotors(char *conf);

/**
 * @brief Print rotor configuration to terminal
 */
void print_rotor_configuration();

/**
 * @brief Encrypt/decrypt given string using pre-configured Enigma machine
 *
 * @param str String to process
 * @param size Size of the string
 *
 * @return Processed string (which is equal to str parameter)
 */
char *process_string(char *str, int size);

/**
 * @brief Process one character through the Enigma machine
 *
 * @param c Character to process
 *
 * @return Processed character
 */
char process_char(char c);

int main(void)
{
    initialize_hardware();
    keyboard_init();

    while (1) {
        delay_ms(10);
        delay_counter += 10;

        terminal_idle();
        keyboard_idle();
    }

    return 0;
}

void LCD_add_char(char c)
{
    if(lcd_control.position >= LCD_SIZE - 1)
        return;

    lcd_control.content[lcd_control.position++] = c;
    LCD_redraw();
}

void LCD_remove_last_char()
{
    if(lcd_control.position == 0)
        return;

    lcd_control.position--;
    LCD_redraw();
}

void LCD_redraw()
{
    int i;

    LCD_clear();

    for(i = 0; i < lcd_control.position; i++)
        LCD_append_char(lcd_control.content[i]);
}

void LCD_clear_display()
{
    lcd_control.position = 0;
    LCD_clear();
}

void print_user_help(void)
{
    term_send_str_crlf(" ROTORS R RS R RS R RS .. configure rotors");
    term_send_str_crlf("                             R - rotor ID (1 - 3)");
    term_send_str_crlf("                             RS - state (0 - 25)");
    term_send_str_crlf(" PLUGBOARD AB CD EF ..... configure plugboard, "
                       "up to 10 combinations");
}

unsigned char decode_user_cmd(char *ucmd, char *cmd)
{
    if(strncmp(ucmd, "ROTORS", 6) == 0) {
        init_rotors(cmd);
        return (USER_COMMAND);
    } else if(strncmp(ucmd, "PLUGBOARD", 9) == 0) {
        init_plugboard(cmd);
        return (USER_COMMAND);
    }

    return (CMD_UNKNOWN);
}

void fpga_initialized()
{
    LCD_init();
    LCD_clear();
    LCD_write_string("ENIGMA");
}

int keyboard_idle()
{
    static char last_ch = 0;
    static char last_real_ch = 0;
    static char pc = 0;
    char printable;
    char ch;

    ch = key_decode(read_word_keyboard_4x4());
    if(ch != last_ch) {
        last_ch = ch;
        if(ch != 0) {
            if(ch == last_real_ch && delay_counter <= 500) {
                if(ch >= '2' && ch <= '9') {
                    pc = (pc + 1) % 4;
                } else if(ch == '0') {
                    pc = (pc + 1) % 3;
                }

                if(isdigit(ch))
                    lcd_control.position--;
            } else {
                pc = 0;
            }

            if(pc == 0)
                printable = ch;
            else if(ch != '0')
                printable = 'A' + (pc - 1) + ((ch - '0' - 2) * 3);
            else
                printable = 'A' + (pc - 1) + ((10 - 2) * 3);

            if(isdigit(ch)) {
                LCD_add_char(printable);
            } else if(ch == 'A'){
                process_string(lcd_control.content, lcd_control.position);
                LCD_redraw();
                print_rotor_configuration();
                print_plugboard_configuration();
            } else if(ch == 'B') {
                LCD_remove_last_char();
            } else if(ch == 'C') {
                LCD_clear_display();
            } else if(ch == 'D') {
                (void)0;
            } else if(ch == '*') {
                LCD_add_char(' ');
            } else if(ch == '#') {
                LCD_add_char('.');
            }

            delay_counter = 0;
            last_real_ch = ch;
        }
    }

    return 0;
}

int init_plugboard(char *conf)
{
    bool alphabet[26] = {false, };
    char pair[2] = {0,};
    char *token;
    int i;

    token = strtok(conf, " ");

    // FORMAT: AB CD EF ... (max x10)
    for(i = 0; i < 10; i ++) {
        token = strtok(NULL, " ");

        if(token == NULL)
            break;

        if(strlen(token) != 2) {
            term_send_str_crlf("Plugboard configuration failed - "
                               "invalid letter pair");
            return 1;
        }

        if(!isalpha(token[0]) || !isalpha(token[1])) {
            term_send_str_crlf("Plugboard configuration failed - "
                               "invalid letter pair");
            return 1;
        }

        pair[0] = toupper(token[0]);
        pair[1] = toupper(token[1]);

        if(alphabet[pair[0] - 'A'] || alphabet[pair[1] - 'A']) {
            term_send_str_crlf("Plugboard configuration failed - "
                               "letter(s) are already used");
            return 1;
        }

        if(pair[0] == pair[1]) {
            term_send_str_crlf("Plugboard configuration failed - "
                               "letters must be different");
            return 1;
        }

        plugboard[pair[0] - 'A'] = pair[1];
        plugboard[pair[1] - 'A'] = pair[0];

        alphabet[pair[0] - 'A'] = true;
        alphabet[pair[1] - 'A'] = true;
    }

    print_plugboard_configuration();

    return 0;
}

void print_plugboard_configuration()
{
    term_send_str("Plugboard configuration: ");
    term_send_str_crlf(plugboard);
}

int init_rotors(char *conf)
{
    char *tok1;
    char *tok2;
    int rnum;
    int rstate;
    int i;

    tok1 = strtok(conf, " ");
    if(tok1 == NULL) {
        term_send_str_crlf("Rotor configration failed - invalid format");
        return 1;
    }

    for(i = 0; i < ROTORS_ACTIVE; i++) {
        tok1 = strtok(NULL, " ");
        tok2 = strtok(NULL, " ");

        if(tok1 == NULL || tok2 == NULL) {
            term_send_str_crlf("Rotor configuration failed - "
                               "not enough configuration data");
            return 1;
        }

        // atoi() is evil, but it should be enough for this case
        rnum = atoi(tok1);
        rstate = atoi(tok2);

        if(rnum < 1 || rnum > ROTORS_TOTAL) {
            term_send_str_crlf("Rotor configuration failed - "
                               "rotor number is out of range");
            return 1;
        }

        if(rstate < 0 || rstate > 25) {
            term_send_str_crlf("Rotor configuration failed - "
                               "rotor state is out of range");
            return 1;
        }

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
    }

    print_rotor_configuration();

    return 0;
}

void print_rotor_configuration()
{
    int i;

    term_send_str_crlf("Current rotor configuration (<ID; state>): ");

    for(i = 0; i < ROTORS_ACTIVE; i++) {
        term_send_str("Rotor #");
        term_send_num(i + 1);
        term_send_str(": <");
        term_send_num(rotor_config[i]->id);
        term_send_str("; ");
        term_send_num(rotor_config[i]->position);
        term_send_str(">");
        term_send_crlf();
    }

}

char *process_string(char *str, int size)
{
    if(str == NULL)
        return NULL;

    int i;

    for(i = 0; i < size; i++) {
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
