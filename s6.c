#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>


#pragma pack(1)
typedef struct
{
    char signature[2];
    int32_t file_size;
    int32_t reserved;
    int32_t data_offset;
    int32_t header_size;
    int32_t width;
    int32_t height;
    int16_t planes;
    int16_t bit_count;
    int32_t compression;
    int32_t img_size;
    int32_t x_pixels;
    int32_t y_pixels;
    int32_t colors_used;
    int32_t colors_important;
} BMPheader;
#pragma pack()


void get_permissions(char *str, mode_t mode)
{
    str[0] = (mode & S_IRUSR) ? 'R' : '-';
    str[1] = (mode & S_IWUSR) ? 'W' : '-';
    str[2] = (mode & S_IXUSR) ? 'X' : '-';
    str[3] = (mode & S_IRGRP) ? 'R' : '-';
    str[4] = (mode & S_IWGRP) ? 'W' : '-';
    str[5] = (mode & S_IXGRP) ? 'X' : '-';
    str[6] = (mode & S_IROTH) ? 'R' : '-';
    str[7] = (mode & S_IWOTH) ? 'W' : '-';
    str[8] = (mode & S_IXOTH) ? 'X' : '-';
    str[9] = '\0';
}


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <fisier_intrare>\n", argv[0]);
        exit(-1);
    }

    int input_file = open(argv[1], O_RDONLY);
    if (input_file == -1)
    {
        perror("The given file cannot be opened");
        exit(-1);
    }

    BMPheader bmp_header;
    if (read(input_file, &bmp_header, sizeof(BMPheader)) != sizeof(BMPheader))
    {
        perror("Error reading BMP header");
        exit(-1);
    }

    int width = bmp_header.width;
    int height = bmp_header.height;

    char *fileName = strrchr(argv[1], '/');
    if (fileName == NULL)
    {
        fileName = (char *)argv[1];
    }
    else
    {
        fileName++;
    }

    struct stat file_info;
    if (fstat(input_file, &file_info) == -1)
    {
        perror("Error getting file info");
        close(input_file);
        return 1;
    }

    char mod_time[20];
    strcpy(mod_time, (ctime(&file_info.st_mtime)));

    int output_file = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (output_file == -1)
    {
        perror("Error opening the output file");
        close(input_file);
        return 1;
    }

    char permission1[10], permission2[10], permission3[10];
    get_permissions(permission1, file_info.st_mode);
    get_permissions(permission2, file_info.st_mode >> 3);
    get_permissions(permission3, file_info.st_mode >> 6);

    char buffer[1024];
    int numChars = sprintf(buffer, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %lu\nidentificatorul Utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %lu\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n",
                           fileName, height, width, (unsigned long)file_info.st_size, file_info.st_uid, mod_time, (unsigned long)file_info.st_nlink, permission1, permission2, permission3);

    write(output_file, buffer, numChars);
    close(input_file);
    close(output_file);

    return 0;
}
