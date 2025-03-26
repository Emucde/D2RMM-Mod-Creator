#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

#define MAX_LINE_LENGTH 40960
#define MAX_FIELDS 2000

char** split_tabs(const char* line, int* count) {
    char** fields = malloc(MAX_FIELDS * sizeof(char*));
    char* copy = strdup(line);
    char* token = strtok(copy, "\t");
    *count = 0;

    while (token && *count < MAX_FIELDS) {
        fields[(*count)++] = strdup(token);
        token = strtok(NULL, "\t\n");
    }
    
    free(copy);
    return fields;
}

void escape_quotes(char* str) {
    char* p = str;
    while (*p) {
        if (*p == '"' || *p == '\\') {
            memmove(p+1, p, strlen(p)+1);
            *p++ = '\\';
        }
        p++;
    }
}

int main(int argc, char* argv[]) {

    FILE* input = fopen("cubemain.txt", "r");
    FILE* output = fopen("mod_cubemain.js", "w");
    char line[MAX_LINE_LENGTH];
    int header_count = 0;
    char** headers = NULL;

    // Read headers
    if (fgets(line, MAX_LINE_LENGTH, input)) {
        headers = split_tabs(line, &header_count);
    }
    fprintf(output, "const cubemainFileName = 'global\\\\excel\\\\cubemain.txt';\n");
    fprintf(output, "cubemain.rows = [];\n\n");

    // Process rows
    while (fgets(line, MAX_LINE_LENGTH, input)) {
        int field_count = 0;
        char** fields = split_tabs(line, &field_count);
        
        fprintf(output, "cubemain.rows.push({\n");
        
        for (int i = 0; i < header_count && i < field_count; i++) {
            escape_quotes(fields[i]);
            
            fprintf(output, "    '%s': ", headers[i]);
            
            if (isdigit(fields[i][0])) {
                fprintf(output, "%s", fields[i]);
            } else {
                fprintf(output, "'%s'", fields[i]);
            }
            
            if (i < header_count - 1 && i < field_count - 1) {
                fprintf(output, ",");
            }
            fprintf(output, "\n");
            
            free(fields[i]);
        }
        
        fprintf(output, "});\n\n");
        free(fields);
    }

    // Cleanup
    for (int i = 0; i < header_count; i++) {
        free(headers[i]);
    }
    free(headers);

    fprintf(output, "D2RMM.writeTsv(cubemainFileName, cubemain);\n");
    
    fclose(input);
    fclose(output);
    return 0;
}
