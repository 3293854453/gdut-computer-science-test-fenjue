#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <stdint.h>
#include <math.h>
#include <windows.h>

#include "refrigerator.h"
#include "file_io.h"
#include "linked_list.h"
#include "food.h"

#define DATA_FILE "refrigerator_data.txt"
#define CAPACITY 100.0f
//枚举定义常量
enum {
    COL_NAME = 0,
    COL_VOLUME,
    COL_TEMP,
    COL_TYPE,
    N_COLS
};

Refrigerator* g_fridge = NULL;
GtkListStore* g_store = NULL;
GtkWidget* g_capacity_label = NULL;
GtkWidget* g_temp_label = NULL;
GtkWidget* g_count_label = NULL;

// 信息更新
static void update_info() {
    char buf[256];
    float remaining = get_remaining_capacity(g_fridge);
    int count = count_foods(g_fridge->foods);
    
    sprintf(buf, "%.2f / %.2f", g_fridge->capacity - remaining, g_fridge->capacity);
    gtk_label_set_text(GTK_LABEL(g_capacity_label), buf);
    sprintf(buf, "%.1f °C", g_fridge->temperature);
    gtk_label_set_text(GTK_LABEL(g_temp_label), buf);
    sprintf(buf, "%d", count);
    gtk_label_set_text(GTK_LABEL(g_count_label), buf);
}

// 列表刷新
static void refresh_list() {
    gtk_list_store_clear(g_store);
    
    Food* cur = g_fridge->foods;
    char volume_str[32], temp_str[32];
    
    while (cur) {
        GtkTreeIter iter;
        gtk_list_store_append(g_store, &iter);
        
        sprintf(volume_str, "%.2f", cur->volume);
        sprintf(temp_str, "%.1f", cur->storage_temp);
        
        gtk_list_store_set(g_store, &iter,
                          COL_NAME, cur->name,
                          COL_VOLUME, volume_str,
                          COL_TEMP, temp_str,
                          COL_TYPE, get_food_type_name(cur->type), -1);
        cur = cur->next;
    }
    update_info();
}

// 选择获取
static int get_selected_name(GtkTreeView* view, char* out) {
    GtkTreeSelection* sel = gtk_tree_view_get_selection(view);
    GtkTreeModel* model = gtk_tree_view_get_model(view);
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
        char* name = NULL;
        gtk_tree_model_get(model, &iter, COL_NAME, &name, -1);
        if (name) {
            strcpy(out, name);
            free(name);
            return 1;
        }
    }
    return 0;
}

// 添加食物对话框
static void on_add(GtkButton* btn, gpointer user_data) {
    GtkWidget* dialog = gtk_dialog_new_with_buttons("添加食物",
                                                    GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(btn))),
                                                    GTK_DIALOG_MODAL,
                                                    "取消", GTK_RESPONSE_CANCEL,
                                                    "添加", GTK_RESPONSE_OK, NULL);

    GtkWidget* content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content), grid);

    GtkWidget* name_entry = gtk_entry_new();
    GtkWidget* volume_spin = gtk_spin_button_new_with_range(0.1, 1000.0, 0.1);
    GtkWidget* temp_spin = gtk_spin_button_new_with_range(-20.0, 30.0, 0.1);
    GtkWidget* type_combo = gtk_combo_box_text_new();

    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "蔬菜");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "肉类");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "水果");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "其他");

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("食物名称："), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("体积："), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), volume_spin, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("温度："), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), temp_spin, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("种类："), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), type_combo, 1, 3, 1, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char* name = gtk_entry_get_text(GTK_ENTRY(name_entry));
        double volume = gtk_spin_button_get_value(GTK_SPIN_BUTTON(volume_spin));
        double temp = gtk_spin_button_get_value(GTK_SPIN_BUTTON(temp_spin));
        int type_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(type_combo));

        if (volume <= 0) {
            // 体积必须大于0
            GtkWidget* msg = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                    GTK_DIALOG_MODAL,
                                                    GTK_MESSAGE_WARNING,
                                                    GTK_BUTTONS_OK,
                                                    "体积必须大于0！");
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
        } else if (type_idx < 0) {
            // 种类必须选择
            GtkWidget* msg = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                    GTK_DIALOG_MODAL,
                                                    GTK_MESSAGE_WARNING,
                                                    GTK_BUTTONS_OK,
                                                    "请选择食物种类！");
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
        } else {
           Food* food = create_food(name, (float)volume, (float)temp, type_idx);
            if (food && add_food(g_fridge, food)) {
                save_to_file(g_fridge, DATA_FILE);  
                refresh_list();
            } else if (food) {
                free(food); //free
            }
        }
    }
    gtk_widget_destroy(dialog);
}


// 修改食物对话框
static void on_modify(GtkButton* btn, gpointer user_data) {
    GtkTreeView* view = GTK_TREE_VIEW(user_data);
    char name[64];
    
    if (!get_selected_name(view, name)) {
        return;
    }
    
    Food* selected_food = find_food_by_name(g_fridge->foods, name);
    if (!selected_food) {
        return;
    }
    
    GtkWidget* dialog = gtk_dialog_new_with_buttons("修改食物信息",
                                                    GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(btn))),
                                                    GTK_DIALOG_MODAL,
                                                    "取消", GTK_RESPONSE_CANCEL,
                                                    "确认", GTK_RESPONSE_OK, NULL);
    
    GtkWidget* content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content), grid);
    
    GtkWidget* name_label = gtk_label_new(selected_food->name);

    // 文本输入框修改数据
    GtkWidget* volume_entry = gtk_entry_new();
    GtkWidget* temp_entry = gtk_entry_new();

    // 设置初始文本为当前值
    char vol_str[32], temp_str[32];
    snprintf(vol_str, sizeof(vol_str), "%.2f", selected_food->volume);
    snprintf(temp_str, sizeof(temp_str), "%.2f", selected_food->storage_temp);
    gtk_entry_set_text(GTK_ENTRY(volume_entry), vol_str);
    gtk_entry_set_text(GTK_ENTRY(temp_entry), temp_str);
    
    GtkWidget* type_combo = gtk_combo_box_text_new();    
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "蔬菜");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "肉类");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "水果");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "其他");
    gtk_combo_box_set_active(GTK_COMBO_BOX(type_combo), selected_food->type);
    
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("食物名称："), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_label, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("体积："), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), volume_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("温度："), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), temp_entry, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("种类："), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), type_combo, 1, 3, 1, 1);
    
    gtk_widget_show_all(dialog);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char* vol_text = gtk_entry_get_text(GTK_ENTRY(volume_entry));
        const char* temp_text = gtk_entry_get_text(GTK_ENTRY(temp_entry));
        int new_type_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(type_combo));
        
        char* endptr;
        float new_volume = strtof(vol_text, &endptr);
        if (endptr == vol_text || *endptr != '\0') {
            GtkWidget* msg = gtk_message_dialog_new(GTK_WINDOW(dialog),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK,
                "体积必须是有效数字！");
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
            gtk_widget_destroy(dialog);
            return;
        }
        if (new_volume <= 0 || new_volume > 1000.0f) {
            GtkWidget* msg = gtk_message_dialog_new(GTK_WINDOW(dialog),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK,
                "体积必须在0.1到1000之间！");
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
            gtk_widget_destroy(dialog);
            return;
        }
        
        float new_temp = strtof(temp_text, &endptr);
        if (endptr == temp_text || *endptr != '\0') {
            GtkWidget* msg = gtk_message_dialog_new(GTK_WINDOW(dialog),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK,
                "温度必须是有效数字！");
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
            gtk_widget_destroy(dialog);
            return;
        }
        if (new_temp < -20.0f || new_temp > 10.0f) {
            GtkWidget* msg = gtk_message_dialog_new(GTK_WINDOW(dialog),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK,
                "温度必须在-20到10之间！");
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
            gtk_widget_destroy(dialog);
            return;
        }
        
        if (new_type_idx < 0) {
            GtkWidget* msg = gtk_message_dialog_new(GTK_WINDOW(dialog),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK,
                "请选择食物种类！");
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
            gtk_widget_destroy(dialog);
            return;
        }
        
        modify_food(g_fridge, name, new_volume, new_temp, new_type_idx);
        save_to_file(g_fridge, DATA_FILE);
        refresh_list();
    }
    
    gtk_widget_destroy(dialog);
}


// 删除食物
static void on_remove(GtkButton* btn, gpointer user_data) {
    GtkTreeView* view = GTK_TREE_VIEW(user_data);
    char name[64];
    
    if (get_selected_name(view, name)) {
        remove_food(g_fridge, name);
        save_to_file(g_fridge, DATA_FILE);
        refresh_list();
    }
}

static void on_sort(GtkButton* btn, gpointer user_data) {
    sort_foods_by_volume(&g_fridge->foods);
    refresh_list();
}

static void on_save(GtkButton* btn, gpointer user_data) {
    save_to_file(g_fridge, DATA_FILE);
}

static void on_load(GtkButton* btn, gpointer user_data) {
    load_from_file(g_fridge, DATA_FILE);
    refresh_list();
}

static void on_destroy(GtkWidget* widget, gpointer user_data) {
    save_to_file(g_fridge, DATA_FILE);
    destroy_refrigerator(g_fridge);
    gtk_main_quit();
}

// 界面创建
static GtkWidget* create_info_panel() {
    GtkWidget* frame = gtk_frame_new(NULL);
    GtkWidget* grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(frame), grid);
    
    g_capacity_label = gtk_label_new("0.00 / 100.00");
    g_temp_label = gtk_label_new("10.0 °C");
    g_count_label = gtk_label_new("0");
    
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("已用容量："), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), g_capacity_label, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("当前温度："), 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), g_temp_label, 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("食物数量："), 4, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), g_count_label, 5, 0, 1, 1);
    
    return frame;
}

static GtkWidget* build_view() {
    g_store = gtk_list_store_new(N_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(g_store));
    
    GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, "食物名称", renderer, "text", COL_NAME, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, "体积", renderer, "text", COL_VOLUME, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, "保存温度", renderer, "text", COL_TEMP, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, "类型", renderer, "text", COL_TYPE, NULL);
    
    return view;
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");
    gtk_init(&argc, &argv);
    
    g_fridge = create_refrigerator(CAPACITY);
    load_from_file(g_fridge, DATA_FILE);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "冰柜管理系统");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 650);
    
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    GtkWidget* title = gtk_label_new("冰柜管理系统");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);
    
    GtkWidget* info_panel = create_info_panel();
    gtk_box_pack_start(GTK_BOX(vbox), info_panel, FALSE, FALSE, 0);
    
    GtkWidget* view = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget* tree = build_view();
    gtk_container_add(GTK_CONTAINER(view), tree);
    gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 0);

    //按钮栏
    GtkWidget* btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget* add_btn = gtk_button_new_with_label("添加");
    g_signal_connect(add_btn, "clicked", G_CALLBACK(on_add), NULL);
    gtk_box_pack_start(GTK_BOX(btn_box), add_btn, FALSE, FALSE, 0);
    GtkWidget* remove_btn = gtk_button_new_with_label("删除");
    g_signal_connect(remove_btn, "clicked", G_CALLBACK(on_remove), tree);
    gtk_box_pack_start(GTK_BOX(btn_box), remove_btn, FALSE, FALSE, 0);
    GtkWidget* modify_btn = gtk_button_new_with_label("修改");
    g_signal_connect(modify_btn, "clicked", G_CALLBACK(on_modify), tree);
    gtk_box_pack_start(GTK_BOX(btn_box), modify_btn, FALSE, FALSE, 0);
    GtkWidget* sort_btn = gtk_button_new_with_label("排序");
    g_signal_connect(sort_btn, "clicked", G_CALLBACK(on_sort), NULL);
    gtk_box_pack_start(GTK_BOX(btn_box), sort_btn, FALSE, FALSE, 0);
    GtkWidget* save_btn = gtk_button_new_with_label("保存");
    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save), NULL);
    gtk_box_pack_start(GTK_BOX(btn_box), save_btn, FALSE, FALSE, 0);
    GtkWidget* load_btn = gtk_button_new_with_label("加载");
    g_signal_connect(load_btn, "clicked", G_CALLBACK(on_load), NULL);
    gtk_box_pack_start(GTK_BOX(btn_box), load_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_box, FALSE, FALSE, 0);

    refresh_list();
    g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), NULL);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}