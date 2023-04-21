/*
 * mp3-dl.c
 * 
 * Copyright 2023 Luis Negrón <negron100197@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>

void download_audio(char *url) {
    // Descargar audio
    char command[500];
    sprintf(command, "yt-dlp -x --audio-format mp3 -o '/home/luis/Música/nuevas/%%(title)-s[%%(id)s].%%(ext)s' %s", url);
    system(command);

    // Obtener nombre del archivo
    char filename[500];
    sprintf(filename, "yt-dlp -o '%%(title)s' --get-filename %s", url);
    FILE *fp;
    fp = popen(filename, "r");
    fgets(filename, 500, fp);
    pclose(fp);
    filename[strcspn(filename, "\n")] = 0;
    
    // Guardar en base de datos
    sqlite3 *conn;
    sqlite3_stmt *res;
    int rc = sqlite3_open("music.db", &conn);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error opening SQLite3 database: %s\n", sqlite3_errmsg(conn));
        sqlite3_close(conn);
        return;
    }

    const char *query = "INSERT INTO music (name, link, created_at) VALUES (?, ?, ?)";
    rc = sqlite3_prepare_v2(conn, query, -1, &res, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing SQLite3 query: %s\n", sqlite3_errmsg(conn));
        sqlite3_close(conn);
        return;
    }

    sqlite3_bind_text(res, 1, filename, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(res, 2, url, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(res, 3, (long long)time(NULL));

    rc = sqlite3_step(res);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Error executing SQLite3 query: %s\n", sqlite3_errmsg(conn));
    }

    sqlite3_finalize(res);
    sqlite3_close(conn);
}

int main() {
    char url[1000];
    printf("Introduce el link del video de Youtube: ");
    fgets(url, 1000, stdin);
    url[strcspn(url, "\n")] = 0;
    download_audio(url);
    return 0;
}

//It is compiled with the following command: gcc -o mp3-dl mp3-dl.c -lsqlite3
