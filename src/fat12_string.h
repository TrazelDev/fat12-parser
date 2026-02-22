#pragma once
#include <stdint.h>

/** converts filename in the format of fat12 to a regular file name, handles errors internally and
 * terminates the program on fail
 * @param a file name in the format of fat12(11 chars long [8 for name][3 for extension])
 * @return an allocated string containing the name
 */
char* fatFileNameToStr(char* filenameFatFormat);
