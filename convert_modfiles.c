#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILES 100
#define MAX_FILENAME 256

#define MAX_LINE_LENGTH 409600
#define MAX_FIELDS 2000

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
    char* src = str;
    char* dst = str;
    while (*src) {
        if (*src != '\n') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0'; // Null-terminate the modified string
}



int main(int argc, char* argv[]) {

    char files[MAX_FILES][MAX_FILENAME];
    int fileCount = get_txt_files(files);

    printf("Found %d .txt files:\n", fileCount);
    
    FILE* output = fopen("mod.js", "w");

    for (int i = 0; i < fileCount; i++) {
        //printf("%s\n", files[i]);
        char baseName[MAX_FILENAME];
        strncpy(baseName, files[i], MAX_FILENAME - 1);
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
        fprintf(output, "%s.rows = [];\n\n", baseName);

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