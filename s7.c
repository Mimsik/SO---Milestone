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

// Structura pentru header-ul BMP
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

// Functie pentru obtinerea permisiunilor sub forma de string
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

// Functie pentru procesarea unui fisier BMP
void processBMP(const char *fileName, int output_file)
{
    char outputBuffer[1024];  // Declaram variabila outputBuffer
    int input_file = open(fileName, O_RDONLY);
    if (input_file == -1)
    {
        perror("The given BMP file cannot be opened");
        return;
    }

    BMPheader bmpHeader;
    if (read(input_file, &bmpHeader, sizeof(BMPheader)) != sizeof(BMPheader))
    {
        perror("Error reading BMP header");
        close(input_file);
        return;
    }

    int width = bmpHeader.width;
    int height = bmpHeader.height;

    char *baseFileName = strrchr(fileName, '/');
    char fileNameBuffer[256];
    if (baseFileName == NULL)
    {
        baseFileName = (char *)fileName;
    }
    else
    {
        baseFileName++;
    }
    snprintf(fileNameBuffer, sizeof(fileNameBuffer), "%s", baseFileName);

    struct stat fileInfo;
    if (stat(fileName, &fileInfo) == -1)
    {
        perror("Error getting file info");
        close(input_file);
        return;
    }

    char modificationTime[20];
    strcpy(modificationTime, ctime(&fileInfo.st_mtime));

    char permission1[10], permission2[10], permission3[10];
    get_permissions(permission1, fileInfo.st_mode);
    get_permissions(permission2, fileInfo.st_mode >> 3);
    get_permissions(permission3, fileInfo.st_mode >> 6);

    int numChars = sprintf(outputBuffer, "\nnume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %lu\nidentificatorul Utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %lu\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n\n",
                           fileNameBuffer, height, width, (unsigned long)fileInfo.st_size, fileInfo.st_uid, modificationTime, (unsigned long)fileInfo.st_nlink, permission1, permission2, permission3);

    if (write(output_file, outputBuffer, numChars) == -1)
    {
        perror("Error writing to the output file");
    }

    close(input_file);
}

// Functie pentru procesarea altor tipuri de fisiere
void processOtherFile(const char *fileName, int outputFd)
{
    struct stat fileInfo;
    if (stat(fileName, &fileInfo) == -1)
    {
        perror("Error getting file info");
        return;
    }

    char buffer[1024];
    int numChars;

    char permission1[10], permission2[10], permission3[10];
    get_permissions(permission1, fileInfo.st_mode);
    get_permissions(permission2, fileInfo.st_mode >> 3);
    get_permissions(permission3, fileInfo.st_mode >> 6);
    
    numChars = sprintf(buffer, "\nnume fisier: %s\ndimensiune: %lu\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %lu\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n\n",
                       fileName, (unsigned long)fileInfo.st_size, fileInfo.st_uid, ctime(&fileInfo.st_mtime), (unsigned long)fileInfo.st_nlink, permission1, permission2, permission3);

    write(outputFd, buffer, numChars);
}

// Functie pentru procesarea directorului
void processDirectory(const char *dirName, int outputFd)
{
    struct stat dirInfo;
    if (stat(dirName, &dirInfo) == -1)
    {
        perror("Error getting directory info");
        return;
    }

    char buffer[1024];
    int numChars;

    char permission[10];
    get_permissions(permission, dirInfo.st_mode);

    numChars = sprintf(buffer, "\nnume director: %s\nidentificatorul utilizatorului: %d\ndrepturi de acces user: %s\ndrepturi de acces grup: R--\ndrepturi de acces altii: ---\n\n",
                       dirName, dirInfo.st_uid, permission);

    write(outputFd, buffer, numChars);
}

// Functie pentru procesarea legaturii simbolice
void processSymbolicLink(const char *linkName, int outputFd)
{
    struct stat linkInfo;
    if (stat(linkName, &linkInfo) == -1)
    {
        perror("Error getting symbolic link info");
        return;
    }

    struct stat targetInfo;
    if (stat(linkName, &targetInfo) == -1)
    {
        perror("Error getting target file info");
        return;
    }

    char buffer[1024];
    int numChars;

    // Informații speciale pentru legătură simbolică
    char target_path[1024];
    ssize_t target_size = readlink(linkName, target_path, sizeof(target_path) - 1);
    target_path[target_size] = '\0';

    char permission[10];
    get_permissions(permission, linkInfo.st_mode);

    numChars = sprintf(buffer, "\nnume legatura: %s\ndimensiune legatura: %ld\ndimensiune fisier target: %ld\ndrepturi de acces user legatura: %s\ndrepturi de acces grup legatura: R--\ndrepturi de acces altii legatura: ---\n\n",
                       linkName, (long)linkInfo.st_size, (long)target_size, permission);

    write(outputFd, buffer, numChars);
}

// Functie pentru procesarea unui fisier
void process_file(char *file_path, int output_fd)
{
    struct stat file_info;
    if (stat(file_path, &file_info) == -1)
    {
        perror("Error getting file info");
        return;
    }

    char *file_type;
    if (S_ISREG(file_info.st_mode))
    {
        if (strstr(file_path, ".bmp"))
        {
            file_type = "fișier BMP";
            processBMP(file_path, output_fd);
        }
        else
        {
            file_type = "fișier obișnuit";
            processOtherFile(file_path, output_fd);
        }
    }
    else if (S_ISLNK(file_info.st_mode))
    {
        file_type = "legătură simbolică";
        processSymbolicLink(file_path, output_fd);
    }
    else if (S_ISDIR(file_info.st_mode))
    {
        file_type = "director";
        processDirectory(file_path, output_fd);
    }
    else
    {
        return; // Ignorăm alte tipuri de fișiere
    }

    printf("%s: %s\n", file_path, file_type);
}

// Functie pentru procesarea unui director
void process_directory(char *dir_path, int output_fd)
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL)
    {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char file_path[1024];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);

            process_file(file_path, output_fd);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <director_intrare>\n", argv[0]);
        exit(-1);
    }

    int output_file = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (output_file == -1)
    {
        perror("Error opening the output file");
        exit(-1);
    }

    process_directory(argv[1], output_file);

    close(output_file);

    return 0;
}
