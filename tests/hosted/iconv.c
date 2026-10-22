#include <iconv.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    iconv_t conversion;
    char input[4];
    char output[8];
    char *input_pointer, *output_pointer;
    size_t input_left, output_left;
    strcpy(input, "A");
    input_pointer = input;
    output_pointer = output;
    input_left = 1;
    output_left = sizeof(output);
    conversion = iconv_open("UTF-16LE", "UTF-8");
    if (conversion == (iconv_t)-1)
        return 1;
    if (iconv(conversion, &input_pointer, &input_left, &output_pointer, &output_left) == (size_t)-1)
        return 2;
    if (input_left != 0 || (unsigned char)output[0] != 0x41 || output[1] != 0)
        return 3;
    iconv_close(conversion);
    puts("hosted iconv ok");
    return 0;
}
