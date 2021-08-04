#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "utils.h"

int read_name_of_file(int argc, char **argv, int *index, char **out_name)
{
    (*index)++;
    int i = *index;
    if(*out_name)
    {
        puts("Такой аргумент уже есть");
        return 1;
    }
    if(argc <= i)
    {
        puts("Ожидалось название файла");
        return 1;
    }
    *out_name = argv[i];
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        puts("Invalid number of parameters");
        return 1;
    }
    char *name_o = NULL, *name_f = NULL;
    char **mas = (char **)malloc(sizeof(char *) * (argc - 1));
    int size = 0;

    for(int i = 1; i < argc; i++)
    {
        if(size > 0 && name_f != NULL)
        {
            puts("Data entry is not possible from file and console at the same time");
            free(mas);
            return 1;
        }
        if(strcmp(argv[i], "-f") == 0)
        {
            if(read_name_of_file(argc, argv, &i, &name_f)){
                free(mas);
                return 1;
            }
            continue;
        }
        if(strcmp(argv[i], "-o") == 0)
        {
            if(read_name_of_file(argc, argv, &i, &name_o)) {
                free(mas);
                return 1;
            }
            continue;
        }
        mas[size] = argv[i];
        size++;
    }
    for (int i = 0; i < size; i++)
        scan_server(mas[i]);
    if(name_f)
    {
        puts("The function of reading from a file is not implemented");
    }
    if(name_o)
    {
        puts("The function of writing in file is not implemented");
    }
    free(mas);
    return 0;
}