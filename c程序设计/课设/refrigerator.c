#include <stdio.h>
#include <stdlib.h>
#include "refrigerator.h"
#include "linked_list.h"

#define MIN_TEMP -20.0
#define MAX_TEMP 10.0

// 创建冰柜
Refrigerator* create_refrigerator(float capacity) {
    Refrigerator* fridge = (Refrigerator*)malloc(sizeof(Refrigerator));
    if (fridge == NULL) {
        return NULL;
    }
    
    fridge->capacity = capacity;
    fridge->temperature = MAX_TEMP; // 初始温度设为最高温度
    fridge->foods = NULL;
    
    return fridge;
}

// 销毁冰柜
void destroy_refrigerator(Refrigerator* fridge) {
    if (fridge != NULL) {
        free_list(&fridge->foods);
        free(fridge);
    }
}

// 获取剩余容积
float get_remaining_capacity(const Refrigerator* fridge) {
    if (fridge == NULL) {
        return 0.0;
    }
    float used = calculate_total_volume(fridge->foods);
    return fridge->capacity - used;
}

// 添加食物到冰柜
int add_food(Refrigerator* fridge, Food* food) {
    if (fridge == NULL || food == NULL) {
        return 0;
    }
    
    // 检查温度
    if (food->storage_temp < MIN_TEMP||food->storage_temp > MAX_TEMP) {
        return 0;
    }
    // 检查体积
    float remaining = get_remaining_capacity(fridge);
    if (food->volume > remaining) {
        return 0;
    }
    
    // 添加到链表
    insert_food(&fridge->foods, food);

    // 对链表按体积排序
    sort_foods_by_volume(&fridge->foods);
    
    // 更新冰柜温度：设置为所有食物中最低的保存温度
    fridge->temperature = find_min_storage_temp(fridge->foods);
    
    return 1;
}

// 从冰柜中取出食物
int remove_food(Refrigerator* fridge, const char* name) {
    if (fridge == NULL || name == NULL) {
        return 0;
    }

    int deleted = remove_food_by_name(&fridge->foods, name);
    if (deleted == 0) {
        return 0;
    }

    // 更新温度
    if (fridge->foods == NULL) {
        fridge->temperature = MAX_TEMP;
    } else {
        fridge->temperature = find_min_storage_temp(fridge->foods);
    }

    return deleted;
}

// 修改食物信息
int modify_food(Refrigerator* fridge, const char* name, float new_volume, 
                float new_temp, FoodType new_type) {
    if (fridge == NULL || name == NULL) {
        return 0;
    }
    
    Food* food = find_food_by_name(fridge->foods, name);
    if (food == NULL) {
        return 0;
    }
    // 检查新温度是否有效
    if (new_temp < MIN_TEMP) {
        return 0;
    }
    // 检查新体积是否超过剩余容量
    float old_volume = food->volume;
    float volume_change = new_volume - old_volume;
    float remaining = get_remaining_capacity(fridge);
    if (volume_change > remaining) {
        return 0;
    }
    
    // 更新信息
    food->volume = new_volume;
    food->storage_temp = new_temp;
    food->type = new_type;
    fridge->temperature = find_min_storage_temp(fridge->foods);
    
    return 1;
}
