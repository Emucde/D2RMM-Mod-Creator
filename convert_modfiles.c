#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // for tolower

#define MAX_FILES 100
#define MAX_FILENAME 256

#define MAX_LINE_LENGTH 409600
#define MAX_FIELDS 10000

int get_txt_files(char files[][MAX_FILENAME]) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    int fileCount = 0;

    // Get the path of the current executable
    char currentDirectory[MAX_PATH];
    GetModuleFileName(NULL, currentDirectory, MAX_PATH);

    // Remove the executable name to get the folder path
    char* lastSlash = strrchr(currentDirectory, '\\');
    if (lastSlash) {
        *lastSlash = '\0';
    }

    // Append wildcard to search for .txt files
    char searchPath[MAX_PATH];
    snprintf(searchPath, sizeof(searchPath), "%s\\*.txt", currentDirectory);

    // Start searching for .txt files
    hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            strncpy(files[fileCount], findFileData.cFileName, MAX_FILENAME - 1);
            files[fileCount][MAX_FILENAME - 1] = '\0';  // Ensure null-termination
            fileCount++;

            if (fileCount >= MAX_FILES) {
                break;  // Prevent array overflow
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return fileCount;
}

char** split_tabs(const char* line, int* count) {
    char** fields = malloc(MAX_FIELDS * sizeof(char*));
    char* copy = strdup(line);
    char* token = copy;
    *count = 0;

    while (*token && *count < MAX_FIELDS) {
        char* next_tab = strchr(token, '\t');
        if (next_tab) {
            *next_tab = '\0'; // Null-terminate the current token
            fields[(*count)++] = strdup(token);
            token = next_tab + 1; // Move to the next character after the tab
        } else {
            fields[(*count)++] = strdup(token); // Last token (no more tabs)
            break;
        }
    }

    // Handle trailing tabs (empty fields)
    while (*count < MAX_FIELDS && token && *token == '\0') {
        fields[(*count)++] = strdup("");
        token++;
    }

    free(copy);
    return fields;
}

void escape_quotes(char* str) {
    char* src = str;
    char* dst = str;
    while (*src) {
        if (*src != '"' && *src != '\\') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0'; // Null-terminate the modified string
}

void remove_newline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        // Replace '\n' with '\', 'r', and '\0'
        str[len - 1] = '\\';
        str[len] = 'r';
        str[len + 1] = '\0';
    }
}

// Function to convert a string to lowercase
void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Function to find the default value in config.txt
int get_default_value(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char key[50];
        int value;
        if (sscanf(line, "%49[^:]:%d", key, &value) == 2) {
            to_lowercase(key);
            if (strcmp(key, "default") == 0) {
                fclose(file);
                return value;
            }
        }
    }

    fclose(file);
    return 0; // Default fallback if "default" key is not found
}

// Function to find values for filenames in config.txt
void get_file_values(const char *filename, char files[MAX_FILES][MAX_FILENAME], int values[MAX_FILES], int default_value) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char key[50];
        int value;
        if (sscanf(line, "%49[^:]:%d", key, &value) == 2) {
            to_lowercase(key);
            for (int i = 0; i < MAX_FILES; i++) {
                char temp[MAX_FILENAME];
                strcpy(temp, files[i]);
                to_lowercase(temp);
                if (strcmp(key, temp) == 0) {
                    values[i] = value;
                }
            }
        }
    }

    // Fill missing values with the default value
    for (int i = 0; i < MAX_FILES; i++) {
        if (values[i] == -1) { // -1 indicates uninitialized
            values[i] = default_value;
        }
    }

    fclose(file);
}

FILE* seek_to_line(const char* filename, int target_line) {
    if (target_line < 1) return NULL;
    
    FILE* fp = fopen(filename, "r");
    if (!fp) return NULL;

    char buffer[2048];
    int current_line = 1;
    
    // Read lines until target_line-1
    while (current_line < target_line && fgets(buffer, sizeof(buffer), fp)) {
        current_line++;
    }

    // Verify we reached the target line and line exists
    long position = ftell(fp);
    if (current_line != target_line || !fgets(buffer, sizeof(buffer), fp)) {
        fclose(fp);
        return NULL;
    }
    
    fseek(fp, position, SEEK_SET);  // Reset to start of target line
    return fp;
}

int count_lines(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1;

    int ch, line_count = 0, last_char = 0;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') line_count++;
        last_char = ch;
    }
    fclose(fp);

    // Add 1 if file ends without newline and isn't empty
    if (last_char != '\n' && last_char != 0) line_count++;
    
    return line_count;
}

int main(int argc, char* argv[]) {

    char files[MAX_FILES][MAX_FILENAME];
    int line_numbers[MAX_FILES];
    char warning_msg[512] = "";

    int fileCount = get_txt_files(files);

    // File containing configuration data
    const char *config_filename = "config.txt";

    // Step 1: Get the default value
    int default_value = get_default_value(config_filename);

    // Step 2: Get the values for filenames or use the default value
    get_file_values(config_filename, files, line_numbers, default_value);

    printf("Found %d .txt files:\n", fileCount);

    // Count line numbers
    for (int i = 0; i < fileCount; i++) {
        int lines = count_lines(files[i]);
        printf("%s:%d\n", files[i], lines);
    }

    // Output the numbers for verification
    for (int i = 0; i < fileCount; i++) {
        printf("File: %s, Number: %d\n", files[i], line_numbers[i]);
    }

    FILE* output = fopen("mod.js", "w");

    for (int i = 0; i < fileCount; i++) {
        //printf("%s\n", files[i]);
        char baseName[MAX_FILENAME];
        strncpy(baseName, files[i], MAX_FILENAME - 1);

        // if filename is "config.txt" skip this file
        if (strcmp(baseName, "config.txt") == 0) {
            continue;
        }

        baseName[MAX_FILENAME - 1] = '\0';  // Ensure null-termination
        char* dot = strrchr(baseName, '.');
        if (dot) {
            *dot = '\0';  // Remove the file extension
        }

        FILE* input = fopen(files[i], "r");

        char line[MAX_LINE_LENGTH];
        int header_count = 0;
        char** headers = NULL;

        // Read headers
        if (fgets(line, MAX_LINE_LENGTH, input)) {
            headers = split_tabs(line, &header_count);
        }
        fprintf(output, "const %sFileName = 'global\\\\excel\\\\%s.txt';\n", baseName, baseName);
        fprintf(output, "const %s = D2RMM.readTsv(%sFileName);\n", baseName, baseName);

        // Seek to the target line, +1 to skip the header line
        input = seek_to_line(files[i], line_numbers[i]+1);

        // Process rows
        while (fgets(line, MAX_LINE_LENGTH, input)) {
            int field_count = 0;
            char** fields = split_tabs(line, &field_count);
            
            fprintf(output, "%s.rows.push({\n", baseName);
            
            for (int i = 0; i < header_count; i++) {
                if (fields[i] && fields[i][0] != '\0') { // Check if the field is not empty
                    escape_quotes(fields[i]);
                    remove_newline(fields[i]);
                    remove_newline(headers[i]);
                    
                    fprintf(output, "    \"%s\": ", headers[i]);
                    
                    fprintf(output, "\"%s\"", fields[i]);
                    
                    if (i < header_count - 1) {
                        fprintf(output, ",");
                    }
                    fprintf(output, "\n");
                }
                
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

        fprintf(output, "D2RMM.writeTsv(%sFileName, %s);\n", baseName, baseName);
        fclose(input);
    }
    
    fclose(output);

    FILE* modJson = fopen("mod.json", "w");
    if (modJson) {
        fprintf(modJson, "{\n");
        fprintf(modJson, "  \"name\": \"Mod Merge\",\n");
        fprintf(modJson, "  \"description\": \"For merging hardcoded opensource mods\",\n");
        fprintf(modJson, "  \"author\": \"Emu\",\n");
        fprintf(modJson, "  \"website\": \"\",\n");
        fprintf(modJson, "  \"version\": \"1.0.0\"\n");
        fprintf(modJson, "}\n");
        fclose(modJson);
    } else {
        fprintf(stderr, "Failed to create mod.json\n");
    }

    return 0;
}