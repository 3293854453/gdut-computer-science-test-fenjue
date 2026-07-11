#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_io.h"
#include "food.h"
#include "linked_list.h"

#define BUFFER_SIZE 256

// 保存到文件
int save_to_file(const Refrigerator* fridge, const char* filename) {
    if (!fridge || !filename) return 0;
    
    FILE* file = fopen(filename, "w");
    if (!file) return 0;
    
    // 写入冰柜基本信息
    fprintf(file, "CAPACITY:%.2f\n", fridge->capacity);
    fprintf(file, "TEMPERATURE:%.2f\n", fridge->temperature);
    
    // 写入食物信息
    Food* current = fridge->foods;
    while (current) {
        fprintf(file, "FOOD:%s|%.2f|%.2f|%d\n",
                current->name, current->volume, 
                current->storage_temp, (int)current->type);
        current = current->next;
    }
    
    fclose(file);
    return 1;
}

// 从文件加载
int load_from_file(Refrigerator* fridge, const char* filename) {
    if (!fridge || !filename) return 0;
    
    FILE* file = fopen(filename, "r");
    if (!file) return 0;
    
    // 清空现有食物列表
    free_list(&fridge->foods);
    
    char line[BUFFER_SIZE];
    
    while (fgets(line, sizeof(line), file)) {
        // 移除换行符
        line[strcspn(line, "\r\n")] = '\0';
        
        if (strncmp(line, "CAPACITY:", 9) == 0) {
            sscanf(line + 9, "%f", &fridge->capacity);
        } else if (strncmp(line, "TEMPERATURE:", 12) == 0) {
            sscanf(line + 12, "%f", &fridge->temperature);
        } else if (strncmp(line, "FOOD:", 5) == 0) {
            char name[50] = {0};
            float volume = 0.0, temp = 0.0;
            int type_int = 0;
            
            char* saveptr = NULL;
            char* token = strtok_r(line + 5, "|", &saveptr);//跳过前五个字符用|分隔
            
            if (token) strncpy(name, token, sizeof(name) - 1);
            token = strtok_r(NULL, "|", &saveptr);
            if (token) volume = atof(token);
            token = strtok_r(NULL, "|", &saveptr);
            if (token) temp = atof(token);
            token = strtok_r(NULL, "|", &saveptr);
            if (token) type_int = atoi(token);
            //名字不为空
            if (name[0] != '\0') {
                Food* food = create_food(name, volume, temp, (FoodType)type_int);
                if (food) {
                    insert_food(&fridge->foods, food);
                }
            }
        }
    }
    
    fclose(file);
    return 1;
}