#include "app_settings.h"
#include "bat.h"
#include "i18n/language.h"
#include "mui_icons.h"
#include "settings.h"
#include "settings_scene.h"

#include "hal_spi_flash.h"
#include "nrf52.h"

#include <stdio.h>

static void settings_scene_about_list_view_on_selected(mui_list_view_event_t event, mui_list_view_t *p_list_view,
                                                       mui_list_item_t *p_item) {
    app_settings_t *app = p_list_view->user_data;
    if (p_item->icon == ICON_BACK) {
        mui_scene_dispatcher_previous_scene(app->p_scene_dispatcher);
    }
}

void settings_scene_about_on_enter(void *user_data) {
    app_settings_t *app = user_data;
    char txt[64];

#if defined(OLED_SCREEN)
    snprintf(txt, sizeof(txt), "OLED");
#elif defined(LCD_SCREEN)
    snprintf(txt, sizeof(txt), "LCD");
#else
    snprintf(txt, sizeof(txt), "???");
#endif
    mui_list_view_add_item_ext(app->p_list_view, ICON_FILE, "Display", txt, NULL_USER_DATA);

    uint32_t part = NRF_FICR->INFO.PART;
    snprintf(txt, sizeof(txt), "nRF%lu", (unsigned long)part);
    mui_list_view_add_item_ext(app->p_list_view, ICON_FILE, "Chip", txt, NULL_USER_DATA);

    uint32_t ram_kb = (NRF_FICR->INFO.RAM == 0xFFFFFFFFUL) ? 0 : NRF_FICR->INFO.RAM * 4;
    if (ram_kb > 0) {
        snprintf(txt, sizeof(txt), "%lu KB", (unsigned long)ram_kb);
    } else {
        snprintf(txt, sizeof(txt), "???");
    }
    mui_list_view_add_item_ext(app->p_list_view, ICON_FILE, "RAM", txt, NULL_USER_DATA);

    flash_info_t flash_info = {0};
    ret_code_t flash_ret = hal_spi_flash_info(&flash_info);
    if (flash_ret == NRF_SUCCESS && flash_info.block_count > 0) {
        uint32_t storage_kb = (flash_info.block_size * flash_info.block_count) / 1024;
        if (storage_kb >= 1024) {
            snprintf(txt, sizeof(txt), "%lu MB", (unsigned long)(storage_kb / 1024));
        } else {
            snprintf(txt, sizeof(txt), "%lu KB", (unsigned long)storage_kb);
        }
    } else {
        snprintf(txt, sizeof(txt), "N/A");
    }
    mui_list_view_add_item_ext(app->p_list_view, ICON_FILE, "Storage", txt, NULL_USER_DATA);

    uint8_t bat_level = bat_get_level();
    bool lipo = settings_get_data()->bat_mode;
    if (lipo && get_stats()) {
        snprintf(txt, sizeof(txt), "%d%% LiPo [+]", bat_level);
    } else if (lipo) {
        snprintf(txt, sizeof(txt), "%d%% LiPo", bat_level);
    } else {
        snprintf(txt, sizeof(txt), "%d%% CR2032", bat_level);
    }
    mui_list_view_add_item_ext(app->p_list_view, ICON_FILE, "Battery", txt, NULL_USER_DATA);

    mui_list_view_add_item(app->p_list_view, ICON_BACK, getLangString(_L_BACK), NULL_USER_DATA);

    mui_list_view_set_selected_cb(app->p_list_view, settings_scene_about_list_view_on_selected);
    mui_view_dispatcher_switch_to_view(app->p_view_dispatcher, SETTINGS_VIEW_ID_MAIN);
}

void settings_scene_about_on_exit(void *user_data) {
    app_settings_t *app = user_data;
    mui_list_view_clear_items(app->p_list_view);
    mui_list_view_set_selected_cb(app->p_list_view, NULL);
}