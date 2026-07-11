#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linked_list.h"

// 根据名称查找食物
Food* find_food_by_name(Food* head, const char* name) {
    Food* current = head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// 查找最低保存温度
float find_min_storage_temp(Food* head) {
    if (head == NULL) {//链表为空
        return 10.0; // 默认温度
    }
    
    float min_temp = head->storage_temp;
    Food* current = head->next;
    while (current != NULL) {
        if (current->storage_temp < min_temp) {
            min_temp = current->storage_temp;
        }
        current = current->next;
    }
    return min_temp;
}

// 插入食物到链表尾部
void insert_food(Food** head, Food* food) {
    if (food == NULL) {
        return;
    }
    if (*head == NULL) {
        *head = food;
    } else {
        Food* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = food;
    }
    food->next = NULL;
}

// 根据名称删除所有同名食物
int remove_food_by_name(Food** head, const char* name) {
    if (*head == NULL || name == NULL) {
        return 0;
    }

    int deleted = 0;
    Food* current = *head;
    Food* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            Food* to_delete = current;
            if (prev == NULL) {
                *head = current->next;
                current = *head;
            } else {
                prev->next = current->next;
                current = prev->next;
            }
            free(to_delete);
            deleted++;
        } else {
            prev = current;
            current = current->next;
        }
    }
    return deleted;
}

// 释放整个链表
void free_list(Food** head) {
    Food* current = *head;
    while (current != NULL) {
        Food* temp = current;
        current = current->next;
        free(temp);
    }
    *head = NULL;
}

// 计算食物数量
int count_foods(Food* head) {
    int count = 0;
    Food* current = head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// 计算总体积
float calculate_total_volume(Food* head) {
    float total = 0.0;
    Food* current = head;
    while (current != NULL) {
        total += current->volume;
        current = current->next;
    }
    return total;
}

// 按体积排序（插入法）
void sort_foods_by_volume(Food** head_ref) {
    if (head_ref == NULL || *head_ref == NULL || (*head_ref)->next == NULL) {
        return; // 空链表或只有一个元素
    }

    Food* sorted = NULL; 

    Food* current = *head_ref;
    while (current != NULL) {
        Food* next = current->next;

        if (sorted == NULL || current->volume < sorted->volume) {
            // 插入头部
            current->next = sorted;
            sorted = current;
        } else {
            Food* temp = sorted;
            while (temp->next != NULL && temp->next->volume <= current->volume) {
                temp = temp->next;
            }
            current->next = temp->next;
            temp->next = current;
        }

        current = next;
    }

    *head_ref = sorted; // 更新头指针
}
