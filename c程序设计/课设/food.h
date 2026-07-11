#ifndef FOOD_H
#define FOOD_H

typedef enum {
    VEGETABLE,
    MEAT, 
    FRUIT,
    OTHER
} FoodType;

typedef struct Food {
    char name[50];
    float volume;
    float storage_temp;
    FoodType type;
    struct Food* next;
} Food;

Food* create_food(const char* name, float volume, float storage_temp, FoodType type);
const char* get_food_type_name(FoodType type);
#endif