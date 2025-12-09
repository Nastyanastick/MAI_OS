#include "contract.h"
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024

void write_string(const char* str) {
    write(STDOUT_FILENO, str, strlen(str));
}

void write_int(int num) {
    char buffer[32];
    int i = 0;
    int is_negative = 0;
    
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    if (num == 0) {
        buffer[i++] = '0';
    } else {
        char temp[32];
        int j = 0;
        while (num > 0) {
            temp[j++] = '0' + (num % 10);
            num /= 10;
        }
        while (j > 0) {
            buffer[i++] = temp[--j];
        }
    }
    
    if (is_negative) {
        char temp[32];
        temp[0] = '-';
        for (int k = 0; k < i; k++) {
            temp[k + 1] = buffer[k];
        }
        write(STDOUT_FILENO, temp, i + 1);
    } else {
        write(STDOUT_FILENO, buffer, i);
    }
}

void write_float(float num) {

    if (num < 0) {
        write_string("-");
        num = -num;
    }
    
    int int_part = (int)num;
    write_int(int_part);
    write_string(".");
    
    float frac = num - int_part;
    for (int i = 0; i < 6; i++) {
        frac *= 10;
        int digit = (int)frac;
        char c = '0' + digit;
        write(STDOUT_FILENO, &c, 1);
        frac -= digit;
    }
    write_string("\n");
}

float parse_float(const char* str) {
    float result = 0.0;
    float fraction = 0.1;
    int sign = 1;
    int decimal = 0;
    
    if (*str == '-') {
        sign = -1;
        str++;
    }
    
    while (*str) {
        if (*str == '.') {
            decimal = 1;
        } else if (*str >= '0' && *str <= '9') {
            if (decimal) {
                result += (*str - '0') * fraction;
                fraction *= 0.1;
            } else {
                result = result * 10 + (*str - '0');
            }
        }
        str++;
    }
    
    return result * sign;
}

int main() {
    char buffer[BUFFER_SIZE];
    char command[10];
    float args[3];
    
    write_string("Program 1 (compile-time linking)\n");
    write_string("Commands:\n");
    write_string("  0 - not used in program 1\n");
    write_string("  1 a b e - calculate sin integral from a to b with step e\n");
    write_string("  2 a dx - calculate cos derivative at point a with dx\n");
    write_string("  q - quit\n");
    
    while (1) {
        write_string("> ");
        
        int n = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
        if (n <= 0) break;
        
        buffer[n] = '\0';
        
        if (buffer[0] == 'q' || buffer[0] == 'Q') {
            break;
        }
        
        int num_args = 0;
        char* token = strtok(buffer, " \t\n");
        
        if (token == NULL) continue;
        
        strcpy(command, token);
        
        if (strcmp(command, "1") == 0) {
            token = strtok(NULL, " \t\n");
            while (token != NULL && num_args < 3) {
                args[num_args++] = parse_float(token);
                token = strtok(NULL, " \t\n");
            }
            
            if (num_args == 3) {
                float result = sin_integral(args[0], args[1], args[2]);
                write_string("Result: ");
                write_float(result);
            } else {
                write_string("Error: need 3 arguments: a b e\n");
            }
        }
        else if (strcmp(command, "2") == 0) {
            token = strtok(NULL, " \t\n");
            while (token != NULL && num_args < 2) {
                args[num_args++] = parse_float(token);
                token = strtok(NULL, " \t\n");
            }
            
            if (num_args == 2) {
                float result = cos_derivative(args[0], args[1]);
                write_string("Result: ");
                write_float(result);
            } else {
                write_string("Error: need 2 arguments: a dx\n");
            }
        }
        else if (strcmp(command, "0") == 0) {
            write_string("Command '0' not used in program 1 (compile-time linking)\n");
        }
        else {
            write_string("Unknown command. Use 1, 2, or q\n");
        }
    }
    
    return 0;
}