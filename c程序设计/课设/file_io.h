#ifndef FILE_IO_H
#define FILE_IO_H

#include "refrigerator.h"

int save_to_file(const Refrigerator* fridge, const char* filename);
int load_from_file(Refrigerator* fridge, const char* filename);

#endif