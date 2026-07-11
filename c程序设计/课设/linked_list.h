#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "food.h"

void insert_food(Food** head, Food* food);
int remove_food_by_name(Food** head, const char* name);
Food* find_food_by_name(Food* head, const char* name);
void free_list(Food** head);
int count_foods(Food* head);
float calculate_total_volume(Food* head);
float find_min_storage_temp(Food* head);
void sort_foods_by_volume(Food** head_ref);

#endif