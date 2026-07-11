#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "food.h"

// 创建食物
Food* create_food(const char* name, float volume, float storage_temp, FoodType type) {
    Food* food = (Food*)malloc(sizeof(Food));
    if (food == NULL) {
        return NULL;
    }
    
    strncpy(food->name, name, sizeof(food->name) - 1);
    food->name[sizeof(food->name) - 1] = '\0';
    food->volume = volume;
    food->storage_temp = storage_temp;
    food->type = type;
    food->next = NULL;
    
    return food;
}

// 获取食物类型名称
const char* get_food_type_name(FoodType type) {
    switch (type) {
        case VEGETABLE: return "蔬菜";
        case MEAT: return "肉类";
        case FRUIT: return "水果";
        case OTHER: return "其他";
        default: return "未知";
    }
}


