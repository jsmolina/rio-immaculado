#include "allegro.h"
#include "allegro/system.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILENAME_LEN 16

// Estructura para almacenar la cabecera de cada archivo (nombre y tamaño)
typedef struct {
    char filename[MAX_FILENAME_LEN];
    long long filesize;
} FileHeader;

void extract_data() {
    FILE *input = fopen("DATA.DAT", "rb");
    if (!input) {
        allegro_message("Error al abrir el archivo de entrada");
        exit(EXIT_FAILURE);
    }
    struct stat st = {0};
    if (stat("data", &st) == -1) {
        mkdir("data", 0700);
    }
    chdir("data");

    while (1) {
        FileHeader header;
        // Leer la cabecera
        size_t read_size = fread(&header, sizeof(FileHeader), 1, input);
        if (read_size != 1) {
            if (feof(input))
                break;
            perror("Error al leer la cabecera");
            exit(EXIT_FAILURE);
        }
        allegro_message("fname: %s\n", header.filename);
        // Abrir el archivo de salida con el nombre original
        FILE *output = fopen(header.filename, "wb");
        if (!output) {
            allegro_message("Error al abrir el archivo de salida %s", header.filename);
            exit(EXIT_FAILURE);
        }

        // Leer y escribir los datos del archivo
        size_t block_size = 4096; // 4 KB
        char *buffer = (char *)malloc(block_size);
        size_t bytes_read;
        long long bytes_remaining = header.filesize;
        int max_iter = 0;

        while (bytes_remaining > 0) {
            if (bytes_remaining > block_size) {
                bytes_read = fread(buffer, 1, block_size, input);
            } else {
                bytes_read = fread(buffer, 1, bytes_remaining, input);
            }
            fwrite(buffer, 1, bytes_read, output);
            bytes_remaining = bytes_remaining - bytes_read;
        }
        free(buffer);

        fclose(output);

        allegro_message("Archivo '%s' descomprimido.\n\n", header.filename);
    }

    fclose(input);
}

void cleanup_data() {
    chdir("..");
}