/*
 *   string 2 hid event
 *
 *   program will take a string and generate HID events for each character
 *   HID events will be written to /dev/hidg0 (can be changed via argument 2)
 *
 *   note: \ needs to be escaped with \\
 *         enter key can be produced via \n
 *         ctrl, alt, tab, backspace, esc, delete, win, shift via: c,a,t,b,e,d,g,s
 *         delay input by 1 second via \-
 *
 *   multiple keys at the same time:
 *      enclose the key sequnce (upto 6) in escaped double quotes (\")
 *
 *   examples:
 *       string2hid  abc123
 *          will type abc123
 *       string2hid "ls -la\n"
 *          will type ls -la (and press enter)
 *       string2hid "bla*" /dev/hidg1
 *          will type bla* using /dev/hidg1 (default is hidg0)
 *       string2hid "\\\"\a\t\\\""
 *	    will press alt + tab
 *       string2hid "\\\"\af\\\"\-\-test\n"
 *          will press alt + f sleep for 2 seconds and type "test" + enter
 *
 *
 *   Collin Mulliner (collin AT mulliner.org)
 *   http://www.mulliner.org/
 *
 *   license: for personal use only (non commercial only)
 *
 *   Relevant specifications:
 *   [0] USB Device Class Definition for Human Interface Devices
 *   [1] USB HID Usage Tables
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Key structure. Holds a character, its HID
 * Usage ID, and a bit field that indicates
 * what modifier keys must be pressed to produce it.
 *
 * k:   ASCII character
 * c:   corresponding HID Usage ID
 * mod: modifier bitfield
 *
 * See: [0] Appendix B.1
 */
struct key_t {
	char k;
	char c;
	char mod;
};

/**
 * Table of key_t for digits 0-9.
 *
 * See: [0] Section 10
 */
static struct key_t keys_num[] = {
	{.k = '0', .c = 0x27, .mod=0x00},
	{.k = '1', .c = 0x1e, .mod=0x00},
	{.k = '2', .c = 0x1f, .mod=0x00},
	{.k = '3', .c = 0x20, .mod=0x00},
	{.k = '4', .c = 0x21, .mod=0x00},
	{.k = '5', .c = 0x22, .mod=0x00},
	{.k = '6', .c = 0x23, .mod=0x00},
	{.k = '7', .c = 0x24, .mod=0x00},
	{.k = '8', .c = 0x25, .mod=0x00},
	{.k = '9', .c = 0x26, .mod=0x00},
};

/**
 * Table of key_t for 'special' keys. Special
 * keys are keys that need modifiers to produce
 * them, as well as modifier keys themselves.
 *
 * Uppercase alphabet characters have the same
 * Usage ID as lowercase characters, but simply
 * have the 'shift' bit set in the modifier byte.
 * These are thus omitted from the table and
 * handled programatically.
 *
 * See: [0] Section 10
 */
static struct key_t keys_special[] = {
	{.k = '!', .c = 0x1e, .mod=0x20},
	{.k = '@', .c = 0x1f, .mod=0x20},
	{.k = '#', .c = 0x20, .mod=0x20},
	{.k = '$', .c = 0x21, .mod=0x20},
	{.k = '%', .c = 0x22, .mod=0x20},
	{.k = '^', .c = 0x23, .mod=0x20},
	{.k = '&', .c = 0x24, .mod=0x20},
	{.k = '*', .c = 0x25, .mod=0x20},
	{.k = '(', .c = 0x26, .mod=0x20},
	{.k = ')', .c = 0x27, .mod=0x20},
	{.k = '-', .c = 0x2d, .mod=0x00},
	{.k = '_', .c = 0x2d, .mod=0x20},
	{.k = '+', .c = 0x2e, .mod=0x20},
	{.k = '=', .c = 0x2e, .mod=0x00},
	{.k = '[', .c = 0x2f, .mod=0x00},
	{.k = '{', .c = 0x2f, .mod=0x20},
	{.k = ']', .c = 0x30, .mod=0x00},
	{.k = '}', .c = 0x30, .mod=0x20},
	{.k = '\\', .c = 0x31, .mod=0x00},
	{.k = '|', .c = 0x31, .mod=0x20},
	{.k = ';', .c = 0x33, .mod=0x00},
	{.k = ':', .c = 0x33, .mod=0x20},
	{.k = '\'', .c = 0x34, .mod=0x00},
	{.k = '"', .c = 0x34, .mod=0x20},
	{.k = ',', .c = 0x36, .mod=0x00},
	{.k = '<', .c = 0x36, .mod=0x20},
	{.k = '.', .c = 0x37, .mod=0x00},
	{.k = '>', .c = 0x37, .mod=0x20},
	{.k = '/', .c = 0x38, .mod=0x00},
	{.k = '?', .c = 0x38, .mod=0x20},
	{.k = '`', .c = 0x35, .mod=0x00},
	{.k = '~', .c = 0x35, .mod=0x20},
	{.k = ' ', .c = 0x2c, .mod=0x00},
	{.k = 'n', .c = 0x28, .mod=0x00}, // enter
	{.k = 'c', .c = 0x00, .mod=0x01}, // ctrl
	{.k = 's', .c = 0x00, .mod=0x02}, // shift
	{.k = 'a', .c = 0x00, .mod=0x04}, // alt
	{.k = 'g', .c = 0x00, .mod=0x08}, // gui/win
	{.k = 't', .c = 0x2B, .mod=0x00}, // tab
	{.k = 'd', .c = 0x4C, .mod=0x00}, // delete
	{.k = 'b', .c = 0x2A, .mod=0x00}, // backspace
	{.k = 'e', .c = 0x29, .mod=0x00}, // esc
	{.k = 0, .c = 0x00, .mod=0x00}
};

/**
 * Converts an array of up to 6 characters to an HID report.
 * The array does not necessarily have to be 6 characters long,
 * as escapes may be 2 characters in length.
 *
 * Report format:
 * Byte    0: Bit field for modifier keys (shift, alt, win, etc)
 * Byte    1: Reserved (0x00)
 * Bytes 2-7: usage id's of character data ()
 *
 * See: [0] Appendix B.1
 *      [1] Section 10
 *
 * @param report the buffer to write the report to
 * @param input_chars the characters to include in this report
 * @param input_length the length of input_chars
 */
int char2event(char *report, char *input_chars, int input_length)
{
	// clear the report
	memset(report, 0x00, 8);
	char input;

	// key data starts at byte 2
	int index = 2;
	int ic;

	// fill bytes 2-7 with Usage ID's for character data
	for (ic = 0; ic < input_length; ic++) {
		input = input_chars[ic];
		char lower = tolower(input);

		// handle case where character is a letter
		if (lower >= 'a' && lower <= 'z') {
			// calculate Usage ID; ASCII lowercase letters map
			// directly to their Usage ID's by subtracting 93.
			report[index] = lower - ('a' - 4);
			index++;
			// if the character is a capital letter, set bits 1
			// and 5 of modifier byte (for l+r shift)
			if (lower != input) {
				report[0] = 0x22;
			}
		}
		// handle case where character is a digit
		else if (lower >= '0' && lower <= '9') {
			// get Usage ID from lookup table
			report[index] = keys_num[lower - '0'].c;
			index++;
		}
		// otherwise handle special characters, i.e. modifiers and
		// escaped characters
		else {
			int i;
			// if the character is \, skip it; the next character
			// should be handled as a special in the lookup table
			if (input == '\\') {
				ic++;
				input = input_chars[ic];
			}
			// search for character in special table
			for (i = 0; i < sizeof(keys_special); i++) {
				if (input == keys_special[i].k) {
					if (keys_special[i].c != 0) {
						report[index] = keys_special[i].c;
					}
					// set any bits in the modifier field
					// needed to produce the character
					report[0] |= keys_special[i].mod;
					index++;
					break;
				}
				if (keys_special[i].k == 0)
					break;
			}
		}
		if (report[2] == 0 && report[0] == 0) {
			printf("error for >%c<\n", input);
			return 0;
		}
	}

	return 1;
}

int main(int argc, char **argv)
{
	char report[8];
	int fd = -1;

	if (argc <= 1) {
		printf("syntax: %s <string> [/dev/hidgX] (\\ needs to be escaped with a \\, enter key is produced via \\n)\n", argv[0]);
		return 0;
	}

	//printf("string >%s<\n", argv[1]);

	char *filename = "/dev/hidg0";

	if (argc > 2)
		filename = argv[2];

	if ((fd = open(filename, O_RDWR, 0666)) == -1) {
		perror(filename);
		return 3;
	}

	int i;
	//printf("input = %s\n", argv[1]);
	for (i = 0; i < strlen(argv[1]); i++) {
		if (argv[1][i] == '\\') {
			i++;
			if (argv[1][i] == '-') {
				sleep(1);
				continue;
			}
			else if (argv[1][i] == '"') {
				int m = i + 1;
				int ml = 0;
				while (1) {
					if (argv[1][m + ml] == '\\') {
						printf("bs\n");
						if (argv[1][m + ml + 1] == '"') {
							break;
						}
					}
					ml++;
				}
				char2event(report, &argv[1][m], ml);
				i = i + ml + 2;
			}
			else {
				char2event(report, &argv[1][i-1], 2);
			}
		}
		else {
			char2event(report, &argv[1][i], 1);
		}
		//printf("%0.2x %0.2x\n", report[0], report[2]);
		// send key
		if (write(fd, report, 8) != 8) {
			perror(filename);
			return 2;
		}
		// clear key ... no key pressed after
		memset(report, 0x0, sizeof(report));
		if (write(fd, report, 8) != 8) {
			perror(filename);
			return 4;
		}
	}

	close(fd);
}
