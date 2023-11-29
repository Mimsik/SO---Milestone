#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>


#pragma pack(1)
typedef struct BMP_Header
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

} BMP_Header;
#pragma pack()


void get_permissions(char *array, mode_t mode)
{
    array[0] = (mode & S_IRUSR) ? 'R' : '-';
    array[1] = (mode & S_IWUSR) ? 'W' : '-';
    array[2] = (mode & S_IXUSR) ? 'X' : '-';
    array[3] = (mode & S_IRGRP) ? 'R' : '-';
    array[4] = (mode & S_IWGRP) ? 'W' : '-';
    array[5] = (mode & S_IXGRP) ? 'X' : '-';
    array[6] = (mode & S_IROTH) ? 'R' : '-';
    array[7] = (mode & S_IWOTH) ? 'W' : '-';
    array[8] = (mode & S_IXOTH) ? 'X' : '-';
}


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Insufficient arguments");
        exit(-1);
    }

    int input_file = open(argv[1], O_RDONLY);
    if (input_file == -1)
    {
        perror("The given file cannot be opened");
        exit(-1);
    }

    BMP_Header bmp_header;
    if (read(input_file, &bmp_header, sizeof(BMP_Header)) != sizeof(BMP_Header))
    {
        perror("Error reading BMP header");
        exit(-1);
    }

    int width = bmp_header.width;
    int height = bmp_header.height;

    char *file_name = strrchr(argv[1], '/');
    if (file_name == NULL)
    {
        file_name = (char *)argv[1];
    }
    else
    {
        file_name++;
    }

    struct stat file_data;
    if (fstat(input_file, &file_data) == -1)
    {
        perror("Error getting file info");
        close(input_file);
        exit(-1);
    }

    char mod[20];
    strcpy(mod, (ctime(&file_data.st_mtime)));

    int output_file = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (output_file == -1)
    {
        perror("Error opening output file");
        close(input_file);
        exit(-1);
    }

    char permission1[10], permission2[10], permission3[10];
    get_permissions(permission1, file_data.st_mode);
    get_permissions(permission2, file_data.st_mode >> 3);
    get_permissions(permission3, file_data.st_mode >> 6);

    char buffer[1024];
    int count = sprintf(buffer, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %lu\nidentificatorul Utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %lu\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n",
                           file_name, height, width, (unsigned long)file_data.st_size, file_data.st_uid, mod, (unsigned long)file_data.st_nlink, permission1, permission2, permission3);

    write(output_file, buffer, count);
    close(input_file);
    close(output_file);

    return 0;
}
