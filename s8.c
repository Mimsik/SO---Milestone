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
#include <sys/wait.h>
#include <errno.h>
#include <libgen.h>

#pragma pack(1)
typedef struct BMP_Header
{
    char signature[2];
    int32_t file_size, reserved, data_offset, header_size, width, height;
    int16_t planes, bit_number;
    int32_t compression, img_size, x_pixel_arrays, y_pixel_arrays, colors_used, colors_important;

} BMP_Header;
#pragma pack()

void getPermission(char *array, mode_t mode)
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

void convertToGrayscale(const char *file_name)
{
    int input_file = open(file_name, O_RDWR);
    if (input_file == -1)
    {
        perror("File open error");
        exit(-1);
    }

    BMP_Header bmp_header;
    if (read(input_file, &bmp_header, sizeof(BMP_Header)) != sizeof(BMP_Header))
    {
        perror("Error reading BMP header");
        close(input_file);
        exit(-1);
    }

    lseek(input_file, bmp_header.data_offset, SEEK_SET); //muta cursorul la dreapta

    int width = bmp_header.width;
    int height = bmp_header.height;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            unsigned char pixel_array[3];
            if (read(input_file, &pixel_array, sizeof(pixel_array)) != sizeof(pixel_array))
            {
                perror("Error reading pixel data");
                close(input_file);
                exit(-1);
            }

            unsigned char gray_processing = (unsigned char)(0.299 * pixel_array[0] + 0.587 * pixel_array[1] + 0.114 * pixel_array[2]);

            pixel_array[0] = pixel_array[1] = pixel_array[2] = gray_processing;
            lseek(input_file, -sizeof(pixel_array), SEEK_CUR); //muta cursorul inapoi, la stanga

            if (write(input_file, &pixel_array, sizeof(pixel_array)) != sizeof(pixel_array))
            {
                perror("Error writing modified pixels data");
                close(input_file);
                exit(-1);
            }
        }
    }
    close(input_file);
}

void processBMPImage(const char *file_name, int output_file)
{
    int input_file = open(file_name, O_RDWR);
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

    int width = bmp_header.width;
    int height = bmp_header.height;

    char *name = strrchr(file_name, '/'); //sparge calea primita prin file_name si separa numele fisierului gasit in director
    if (name == NULL)
      name = (char *)file_name;
    else
      name++;

    //obtine numele fisierului fara extensia de dupa caracterul '.'
    size_t length = strcspn(name, ".");
    char single_name[length + 1];
    strncpy(single_name, name, length); 
    single_name[length] = '\0';

    struct stat file_data;
    if (stat(file_name, &file_data) == -1)
    {
        perror("Error getting file data");
        close(input_file);
        return;
    }

    char changes_time[20];
    strncpy(changes_time, ctime(&file_data.st_mtime), sizeof(changes_time) - 1);  //determina timpul ultimei schimbari facute
    changes_time[sizeof(changes_time) - 1] = '\0';

    char permission1[10], permission2[10], permission3[10];
    getPermission(permission1, file_data.st_mode); 
    getPermission(permission2, file_data.st_mode >> 3);  //shifteaza cu 3 pozitii pentru a accesa urmatorii biti
    getPermission(permission3, file_data.st_mode >> 6);  //shifteaza cu 6 pozitii pentru a accesa urmatorii biti

    pid_t grayscale = fork();
    if (grayscale == -1)
    {
        perror("ERROR: Process grayscale conversion");
        exit(-1);
    }
    if (grayscale == 0)
    {
        convertToGrayscale(file_name);
        exit(EXIT_SUCCESS);
    }
    else
    {
        char buffer[1024];
        int state;
        waitpid(grayscale, &state, 0);

	//folosim un buffer pentru a stoca in el datele pe care trebuie sa le scriem in fisierul de statistica aferent imaginii bmp
        int number = sprintf(buffer, "\nfile name: %s\nheight: %d\nwidth: %d\nsize: %lu\nuser ID: %d\nmodification time: %s\nlink number: %lu\nuser permissions: %s\ngroup permissions: %s\nothers permissions: %s\n\n", single_name, height, width, (unsigned long)file_data.st_size, file_data.st_uid, changes_time, (unsigned long)file_data.st_nlink, permission1, permission2, permission3);

        if (write(output_file, buffer, number) == -1) //verifica daca scrie in fisierul de output
        {
            perror("Error writing to the output file");
            exit(-1);
        }
        close(input_file);
    }
}

void processOtherFile(const char *file_name, int output_file)
{
    struct stat file_data;
    if (stat(file_name, &file_data) == -1)
    {
        perror("Error getting file data");
        exit(-1);
    }

    char *name = strrchr(file_name, '/'); //sparge calea primita prin file_name si separa numele fisierului gasit in director
    if (name == NULL)
      name = (char *)file_name;
    else
      name++;

    //obtine numele fisierului fara extensia de dupa caracterul '.'
    size_t length = strcspn(name, ".");
    char single_name[length + 1];
    strncpy(single_name, name, length); 
    single_name[length] = '\0';

    char buffer[1024];
    char changes_time[20];
    strncpy(changes_time, ctime(&file_data.st_mtime), sizeof(changes_time) - 1);  //determina timpul ultimei schimbari facute
    changes_time[sizeof(changes_time) - 1] = '\0';

    char permission1[10], permission2[10], permission3[10];
    getPermission(permission1, file_data.st_mode);
    getPermission(permission2, file_data.st_mode >> 3);  //shifteaza cu 3 pozitii pentru a accesa urmatorii biti
    getPermission(permission3, file_data.st_mode >> 6);  //shifteaza cu 6 pozitii pentru a accesa urmatorii biti

    //folosim un buffer pentru a stoca in el datele pe care trebuie sa le scriem in fisierul de statistica aferent fisierului curent
    int number = sprintf(buffer, "\nfile name: %s\nsize: %lu\nuser ID: %d\nmodification time: %s\nlink number: %lu\nuser permissions: %s\ngroup permissions: %s\nothers permissions: %s\n\n", single_name, (unsigned long)file_data.st_size, file_data.st_uid, changes_time, (unsigned long)file_data.st_nlink, permission1, permission2, permission3);

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
    getPermission(permission, dir_data.st_mode);
	
    //folosim un buffer pentru a stoca in el datele pe care trebuie sa le scriem in fisierul de statistica aferent folderului
    int number = sprintf(buffer, "\ndirectory name: %s\nuser ID: %d\nuser permissions: %s\ngroup permissions: R--\nothers permissions: ---\n\n",
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
        perror("Error getting symbolic link info");
        exit(-1);
    }

    struct stat targetInfo;
    if (stat(link, &targetInfo) == -1)
    {
        perror("Error getting target file info");
        exit(-1);
    }

    char target_file[1024];
    ssize_t size = readlink(link, target_file, sizeof(target_file) - 1);
    target_file[size] = '\0';

    char permission[10];
    getPermission(permission, link_data.st_mode);

    char buffer[1024];
	//folosim un buffer pentru a stoca in el datele pe care trebuie sa le scriem in fisierul de statistica aferent pentru symbolic link
    int number = sprintf(buffer, "\nlink name: %s\nlink size: %ld\ntarget file size: %ld\nuser permissions: %s\ngroup permissions: R--\nothers permissions: ---\n\n",
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
            file_type = "BMP File";
            processBMPImage(file, output_file);
        }
        else
        {
            file_type = "Regular File";
            processOtherFile(file, output_file);
        }
    }
    else if (S_ISLNK(file_data.st_mode))
    {
        file_type = "Symbolic Link";
        processSymbolicLink(file, output_file);
    }
    else if (S_ISDIR(file_data.st_mode))
    {
        file_type = "Directory";
        processDirectory(file, output_file);
    }
    else
      return;  //pentru alte tipuri de fisiere

    printf("%s: %s\n", file, file_type);
}

void processDirectoryExtra(char *directory, char *output_dir)
{
    DIR *inputDir = opendir(directory);
    if (inputDir == NULL)
    {
        perror("Error opening input directory");
        exit(-1);
    }

    DIR *outputDir = opendir(output_dir);
    if (outputDir == NULL)
    {
        perror("Error opening output directory");
        exit(-1);
    }

    struct dirent *entry;
    while ((entry = readdir(inputDir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            pid_t pid = fork();
            if (pid == -1)
            {
                perror("ERROR: fork process");
                exit(-1);
            }
            if (pid == 0)
            {
                char file[1024];
                snprintf(file, sizeof(file), "%s/%s", directory, entry->d_name);
       
		size_t length = strcspn(entry->d_name, ".");
		char single_name[length + 1];
		strncpy(single_name, entry->d_name, length); 
		single_name[length] = '\0';

                char buffer_file[1024];
                snprintf(buffer_file, sizeof(buffer_file), "%s/%s_statistica.txt", output_dir, single_name);

                int output_file = open(buffer_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (output_file == -1)
                {
                    perror("Error opening the output file");
                    exit(-1);
                }
                processFile(file, output_file);
                close(output_file);
                exit(EXIT_SUCCESS);
            }
        }
    }
    closedir(inputDir);
    closedir(outputDir);

    int state;
    while (wait(&state) > 0)
        if (WIFEXITED(state))
            printf("Child process: SUCCESS\n");
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Insufficient arguments");
        exit(-1);
    }

    processDirectoryExtra(argv[1], argv[2]);
    return 0;
}
