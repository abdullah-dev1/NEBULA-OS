#ifndef NEBULA_STORAGE_MANAGER_H
#define NEBULA_STORAGE_MANAGER_H

#include <stdio.h>

void init_storage();
void list_hdd_files(); 
int  file_exists_in_hdd(const char* filename);
long get_file_size(const char* filename);
void get_hdd_path(const char* filename, char* out);

#endif 
