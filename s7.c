#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <pwd.h>

#pragma pack(1)
typedef struct BMP_Header
{
    char signature[2];
    int32_t file_size, reserved, data_offset, header_size, width, height;
    int16_t planes, bit_number;
    int32_t compression, img_size, x_pixel_arrays, y_pixel_arrays, colors_used, colors_important;

} BMP_Header;
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
}


void processBMPImage(const char *file_name, int output_file)
{
    int input_file = open(file_name, O_RDONLY);
    if (input_file == -1)
    {
        perror("The given BMP file cannot be opened");
        exit(-1);
    }

    BMP_Header bmp_header;
    if (read(input_file, &bmp_header, sizeof(BMP_Header)) != sizeof(BMP_Header))
    {
        perror("Error reading BMP header");
        close(input_file);
        exit(-1);
    }

    int width = bmp_header.width; //determina latime imaginii
    int height = bmp_header.height; //determina inaltimea imaginii

    char *first_name = strrchr(file_name, '/'); //sparge calea primita prin file_name si separa numele fisierului gasit in director
    if (first_name == NULL)
    {
        first_name = (char *)file_name;
    }
    else
    {
        first_name++;
    }

    char buffer_file[256];
    snprintf(buffer_file, sizeof(buffer_file), "%s", first_name); //salveaza in buffer_file numele fisierului gasit in director


    struct stat file_data;
    if (stat(file_name, &file_data) == -1)
    {
        perror("Error getting file data");
        close(input_file);
        exit(-1);
    }

    char permission1[10], permission2[10], permission3[10];
    get_permissions(permission1, file_data.st_mode);
    get_permissions(permission2, file_data.st_mode >> 3);
    get_permissions(permission3, file_data.st_mode >> 6);

    char buffer[1024];
    char changes_time[20];
    strcpy(changes_time, ctime(&file_data.st_mtime)); //determina timpul la care a fost facuta ultima modificare
  
    int number = sprintf(buffer, "\nnume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %lu\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %lu\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n\n",
                           buffer_file, height, width, (unsigned long)file_data.st_size, file_data.st_uid, changes_time, (unsigned long)file_data.st_nlink, permission1, permission2, permission3);

    if (write(output_file, buffer, number) == -1) //verifica daca scrie in fisierul de output 
      perror("Error writing to the output file");

    close(input_file);
}


void processOtherFile(const char *file_name, int output_file)
{
    struct stat file_data;
    if (stat(file_name, &file_data) == -1)
    {
        perror("Error getting file data");
        exit(-1);
    }

    char *first_name = strrchr(file_name, '/'); //sparge calea primita prin file_name si separa numele fisierului gasit in director
    if (first_name == NULL)
    {
        first_name = (char *)file_name; 
    }
    else
    {
        first_name++;
    }

    char buffer_file[256];
    snprintf(buffer_file, sizeof(buffer_file), "%s", first_name); //salveaza in buffer_file numele fisierului gasit in director

    char permission1[10], permission2[10], permission3[10];
    get_permissions(permission1, file_data.st_mode);
    get_permissions(permission2, file_data.st_mode >> 3);
    get_permissions(permission3, file_data.st_mode >> 6);

    char buffer[1024];
    char changes_time[20];
    strcpy(changes_time, ctime(&file_data.st_mtime)); //determina timpul la care a fost facuta ultima modificare

    int number = sprintf(buffer, "\nnume fisier: %s\ndimensiune: %lu\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %lu\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n\n", buffer_file, (unsigned long)file_data.st_size, file_data.st_uid, changes_time, (unsigned long)file_data.st_nlink,permission1, permission2, permission3);

    if (write(output_file, buffer, number) == -1) //verifica daca scrie in fisierul de output
        {
            perror("Error writing to the output file");
            exit(-1);
        }
}

void processDirectory(const char *directory, int output_file)
{
    struct stat dir_data;
    if (stat(directory, &dir_data) == -1)
    {
        perror("Error getting directory data");
        exit(-1);
    }

    char buffer[1024];
    char permission[10];
    get_permissions(permission, dir_data.st_mode);
    
    int number;
    number = sprintf(buffer, "\nnume director: %s\nidentificatorul utilizatorului: %d\ndrepturi de acces user: %s\ndrepturi de acces grup: R--\ndrepturi de acces altii: ---\n\n",
                       directory, dir_data.st_uid, permission);

     if (write(output_file, buffer, number) == -1) //verifica daca scrie in fisierul de output
        {
            perror("Error writing to the output file");
            exit(-1);
        }
}

void processSymbolicLink(const char *link, int output_file)
{
    struct stat link_data;
    if (stat(link, &link_data) == -1)
    {
        perror("Error getting symbolic link data");
        exit(-1);
    }

    struct stat target_data;
    if (stat(link, &target_data) == -1)
    {
        perror("Error getting target file data");
        exit(-1);
    }

    char path[1024];
    ssize_t size = readlink(link, path, sizeof(path) - 1);
    path[size] = '\0';

    char permission[10];
    get_permissions(permission, link_data.st_mode);

    char buffer[1024];
    int number = sprintf(buffer, "\nnume legatura: %s\ndimensiune legatura: %ld\ndimensiune fisier target: %ld\ndrepturi de acces user legatura: %s\ndrepturi de acces grup legatura: R--\ndrepturi de acces altii legatura: ---\n\n",
                       link, (long)link_data.st_size, (long)size, permission);

    if (write(output_file, buffer, number) == -1) //verifica daca scrie in fisierul de output
        {
            perror("Error writing to the output file");
            exit(-1);
        }
}

void processFile(char *file, int output_file)
{
    struct stat file_data;
    if (stat(file, &file_data) == -1)
    {
        perror("Error getting file data");
        exit(-1);
    }

    char *file_type;
    if (S_ISREG(file_data.st_mode))
    {
        if (strstr(file, ".bmp"))
        {
            file_type = "fișier BMP";
            processBMPImage(file, output_file);
        }
        else
        {
            file_type = "fișier obișnuit";
            processOtherFile(file, output_file);
        }
    }
    else if (S_ISLNK(file_data.st_mode))
    {
        file_type = "legătură simbolică";
        processSymbolicLink(file, output_file);
    }
    else if (S_ISDIR(file_data.st_mode))
    {
        file_type = "director";
        processDirectory(file, output_file);
    }
    else
    {
        return; //pentru alte tipuri de fisiere 
    }

    printf("%s: %s\n", file, file_type);
}

void processDirectoryExtra(char *directory, int output_file)
{
    DIR *dir = opendir(directory);
    if (dir == NULL)
    {
        perror("Error opening directory");
        exit(-1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char file[1024];
            snprintf(file, sizeof(file), "%s/%s", directory, entry->d_name);

            processFile(file, output_file);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Insufficient arguments");
        exit(-1);
    }

    int output_file = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (output_file == -1)
    {
        perror("Error opening the output file");
        exit(-1);
    }

    processDirectoryExtra(argv[1], output_file);
    close(output_file);

    return 0;
}
