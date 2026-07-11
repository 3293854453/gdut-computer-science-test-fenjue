#ifndef REFRIGERATOR_H
#define REFRIGERATOR_H

#include "food.h"

typedef struct {
    float capacity;
    float temperature;
    Food* foods;
} Refrigerator;

Refrigerator* create_refrigerator(float capacity);
void destroy_refrigerator(Refrigerator* fridge);
int add_food(Refrigerator* fridge, Food* food);
int remove_food(Refrigerator* fridge, const char* name);
float get_remaining_capacity(const Refrigerator* fridge);
int modify_food(Refrigerator* fridge, const char* name, float new_volume, 
                float new_temp, FoodType new_type);
Food* find_food_by_name(Food* head, const char* name);

#endif