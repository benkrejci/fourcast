#include <pebble.h>

const int OUTBOX_SEND_MAX_TRIES = 6;
const int WEATHER_UPDATE_MINUTES = 30;
const int GET_WEATHER_TIMEOUT = 30;
const int TIME_SIZE = 6;
const char TIME_FORMAT_24HR[] = "%k:%M";
const char TIME_FORMAT_12HR[] = "%l:%M";
const int DATE_SIZE = 11;
const char DATE_FORMAT[] = "%a %h %e";
/*const int DAY_SIZE = 2;
const char DAY_FORMAT[] = "%a";*/

#define GColorWhiteARGB8 ((uint8_t)0b11111111)
#define GColorBlackARGB8 ((uint8_t)0b11000000)
#ifdef PBL_COLOR
  #define supports_color true
  #define COLOR_BATT ((uint8_t)GColorWhiteARGB8)
  #define COLOR_BATT_LOW ((uint8_t)GColorRedARGB8)
  #define COLOR_BG ((uint8_t)GColorBlackARGB8)
  #define COLOR_TEXT ((uint8_t)GColorWhiteARGB8)
#else
  #define supports_color false
  #define COLOR_BATT ((uint8_t)GColorWhiteARGB8)
  #define COLOR_BATT_LOW ((uint8_t)COLOR_BATT)
  #define COLOR_BG ((uint8_t)GColorBlackARGB8)
  #define COLOR_TEXT ((uint8_t)GColorWhiteARGB8)
#endif

enum {
  KEY_WEATHER_TEMP_C = 0,
  KEY_WEATHER_TEMP_F = 1,
  KEY_WEATHER_CONDITIONS = 2,
  KEY_FORECAST_0_DAY = 3,
  KEY_FORECAST_0_MIN_C = 4,
  KEY_FORECAST_0_MIN_F = 5,
  KEY_FORECAST_0_MAX_C = 6,
  KEY_FORECAST_0_MAX_F = 7,
  KEY_FORECAST_0_CONDITIONS = 8,
  KEY_FORECAST_1_DAY = 9,
  KEY_FORECAST_1_MIN_C = 10,
  KEY_FORECAST_1_MIN_F = 11,
  KEY_FORECAST_1_MAX_C = 12,
  KEY_FORECAST_1_MAX_F = 13,
  KEY_FORECAST_1_CONDITIONS = 14,
  KEY_FORECAST_2_DAY = 15,
  KEY_FORECAST_2_MIN_C = 16,
  KEY_FORECAST_2_MIN_F = 17,
  KEY_FORECAST_2_MAX_C = 18,
  KEY_FORECAST_2_MAX_F = 19,
  KEY_FORECAST_2_CONDITIONS = 20,
  KEY_CITY = 21,
  
  KEY_TEMP_UNITS = 22,
  KEY_BG_COLOR = 23,
  KEY_TEXT_COLOR = 24,
  
  KEY_GET_WEATHER_LAST_RECEIVED = 25,
  KEY_ACTION_STORE_SETTINGS = 26,
  KEY_ACTION_GET_WEATHER = 27,
  KEY_SUPPORTS_COLOR = 28,
  KEY_READY = 29,
  KEY_WEATHER_SERVICE = 30
};

static char s_temp_units[] = "f";
static uint8_t s_bg_color = COLOR_BG;
static uint8_t s_text_color = COLOR_TEXT;
static char s_weather_service[32] = "OpenWeatherMap";
static int s_batt_percent = 0;
/*static char s_battery_buffer[] = "---";*/
static char s_time_buffer[] = "XX:XX";
static char s_date_buffer[] = "XXX XXX XX";
/*static char s_day_buffer[] = "X";
static char s_date_combined_buffer[] = "X XXX XX";*/
/*static char s_city_buffer[13];*/
static char s_weather_temp_c_buffer[5];
static char s_weather_temp_f_buffer[5];
static char s_weather_conditions_buffer[8];
static char s_forecast_0_day_buffer[4];
static char s_forecast_0_min_c_buffer[5];
static char s_forecast_0_min_f_buffer[5];
static char s_forecast_0_max_c_buffer[5];
static char s_forecast_0_max_f_buffer[5];
static char s_forecast_0_conditions_buffer[8];
static char s_forecast_1_day_buffer[4];
static char s_forecast_1_min_c_buffer[5];
static char s_forecast_1_min_f_buffer[5];
static char s_forecast_1_max_c_buffer[5];
static char s_forecast_1_max_f_buffer[5];
static char s_forecast_1_conditions_buffer[8];
static char s_forecast_2_day_buffer[4];
static char s_forecast_2_min_c_buffer[5];
static char s_forecast_2_min_f_buffer[5];
static char s_forecast_2_max_c_buffer[5];
static char s_forecast_2_max_f_buffer[5];
static char s_forecast_2_conditions_buffer[8];

static Window *s_main_window;
/*static BitmapLayer *s_bitmap_layer;*/
/*static GBitmap *s_background_bitmap;*/
static TextLayer *s_time_layer;
static Layer *s_battery_layer;
static TextLayer *s_date_layer;
/*static TextLayer *s_day_layer;*/
/*static TextLayer *s_city_layer;*/
static Layer *s_weather_border_layer;
static TextLayer *s_weather_temp_layer;
static TextLayer *s_weather_conditions_layer;
static TextLayer *s_forecast_0_day_layer;
static TextLayer *s_forecast_0_min_layer;
static TextLayer *s_forecast_0_max_layer;
static TextLayer *s_forecast_0_conditions_layer;
static TextLayer *s_forecast_1_day_layer;
static TextLayer *s_forecast_1_min_layer;
static TextLayer *s_forecast_1_max_layer;
static TextLayer *s_forecast_1_conditions_layer;
static TextLayer *s_forecast_2_day_layer;
static TextLayer *s_forecast_2_min_layer;
static TextLayer *s_forecast_2_max_layer;
static TextLayer *s_forecast_2_conditions_layer;

static GFont s_font_big;
static GFont s_font_small;
static GFont s_font_small_text;
static GFont s_font_weather_icons;

static void update_time(struct tm *tick_time) {
  if (clock_is_24h_style() == true) {
    strftime(s_time_buffer, TIME_SIZE, TIME_FORMAT_24HR, tick_time);
  } else {
    strftime(s_time_buffer, TIME_SIZE, TIME_FORMAT_12HR, tick_time);
  }
  strftime(s_date_buffer, DATE_SIZE, DATE_FORMAT, tick_time);
  /*strftime(s_day_buffer, DAY_SIZE, DAY_FORMAT, tick_time);
  snprintf(s_date_combined_buffer, sizeof(s_date_combined_buffer), "%s %s", s_day_buffer, s_date_buffer);*/
  /*strftime(s_day_buffer, DAY_SIZE, DAY_FORMAT, tick_time);*/
  
  text_layer_set_text(s_time_layer, s_time_buffer);
  /*text_layer_set_text(s_date_layer, s_date_combined_buffer);*/
  text_layer_set_text(s_date_layer, s_date_buffer);
  /*text_layer_set_text(s_day_layer, s_day_buffer);*/
}

static void battery_handler(BatteryChargeState new_state) {
  /*snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d", (int)((float)new_state.charge_percent / 10.0 + 0.5));
  text_layer_set_text(s_battery_layer, s_battery_buffer);*/
  s_batt_percent = new_state.charge_percent;
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, s_batt_percent > 20 ? (GColor)s_text_color : (GColor)COLOR_BATT_LOW);
  graphics_context_set_fill_color(ctx, s_batt_percent > 20 ? (GColor)s_text_color : (GColor)COLOR_BATT_LOW);
  graphics_draw_round_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h), 1);
  
  int batt_px;
  batt_px = (int)((float)s_batt_percent / 100.0 * (float)(bounds.size.w - 4) + 0.5);
  graphics_fill_rect(ctx, GRect(2, 2, batt_px, bounds.size.h - 4), 0, GCornersAll);
}

static void weather_border_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, (GColor)s_text_color);
  graphics_draw_round_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h), 9);
}

/*static void update_background_color() {
  #ifdef PBL_COLOR
    GColor BACKGROUND_COLORS[] = {
      GColorBulgarianRose,
      GColorBlack,
      GColorOxfordBlue,
      GColorImperialPurple,
      GColorDarkGreen
    };
    window_set_background_color(s_main_window, BACKGROUND_COLORS[rand() % sizeof(BACKGROUND_COLORS)]);
  #else
    window_set_background_color(s_main_window, GColorBlack);
  #endif
}

#ifdef PBL_COLOR
  static void tap_handler(AccelAxisType axis, int32_t direction) {
    update_background_color();
  }
#endif*/

static void update_ui() {
  bool temp_c = strcmp(s_temp_units, "c") == 0;
  text_layer_set_text(s_weather_temp_layer, temp_c ? s_weather_temp_c_buffer : s_weather_temp_f_buffer);
  text_layer_set_text(s_weather_conditions_layer, s_weather_conditions_buffer);
  text_layer_set_text(s_forecast_0_day_layer, s_forecast_0_day_buffer);
  text_layer_set_text(s_forecast_0_min_layer, temp_c ? s_forecast_0_min_c_buffer : s_forecast_0_min_f_buffer);
  text_layer_set_text(s_forecast_0_max_layer, temp_c ? s_forecast_0_max_c_buffer : s_forecast_0_max_f_buffer);
  text_layer_set_text(s_forecast_0_conditions_layer, s_forecast_0_conditions_buffer);
  text_layer_set_text(s_forecast_1_day_layer, s_forecast_1_day_buffer);
  text_layer_set_text(s_forecast_1_min_layer, temp_c ? s_forecast_1_min_c_buffer : s_forecast_1_min_f_buffer);
  text_layer_set_text(s_forecast_1_max_layer, temp_c ? s_forecast_1_max_c_buffer: s_forecast_1_max_f_buffer);
  text_layer_set_text(s_forecast_1_conditions_layer, s_forecast_1_conditions_buffer);
  text_layer_set_text(s_forecast_2_day_layer, s_forecast_2_day_buffer);
  text_layer_set_text(s_forecast_2_min_layer, temp_c ? s_forecast_2_min_c_buffer : s_forecast_2_min_f_buffer);
  text_layer_set_text(s_forecast_2_max_layer, temp_c ? s_forecast_2_max_c_buffer : s_forecast_2_max_f_buffer);
  text_layer_set_text(s_forecast_2_conditions_layer, s_forecast_2_conditions_buffer);
  
  window_set_background_color(s_main_window, (GColor)s_bg_color);
  text_layer_set_text_color(s_date_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_time_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_weather_conditions_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_weather_temp_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_forecast_0_day_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_forecast_0_conditions_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_forecast_0_max_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_forecast_0_min_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_forecast_1_day_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_forecast_1_conditions_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_forecast_1_max_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_forecast_1_min_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_forecast_2_day_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_forecast_2_conditions_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_forecast_2_max_layer, (GColor)s_text_color);
  text_layer_set_text_color(s_forecast_2_min_layer, (GColor)s_text_color);
}

static void main_window_load(Window *window) {
  s_font_big = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SQUARE_54));
  s_font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SQUARE_18));
  s_font_small_text = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SQUARE_TEXT_22));
  s_font_weather_icons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_WEATHER_ICONS_26));
  
  /*update_background_color();*/
  
  /*s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  s_bitmap_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_bitmap_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bitmap_layer));*/
  
  

  /*s_city_layer = text_layer_create(GRect(-1, 0, 145, 25));
  text_layer_set_background_color(s_city_layer, GColorClear);
  text_layer_set_text_color(s_city_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_city_layer, s_font_small_text);
  text_layer_set_text_alignment(s_city_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_city_layer));*/
  
  /*s_battery_layer = text_layer_create(GRect(119, -4, 25, 25));
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_battery_layer, s_font_small);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  battery_handler(battery_state_service_peek());*/
  
  s_battery_layer = layer_create(GRect(117, 3, 24, 14));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  battery_handler(battery_state_service_peek());

  s_date_layer = text_layer_create(GRect(1, -4, 119, 25));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, s_font_small_text);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  /*s_day_layer = text_layer_create(GRect(2, 12, 144, 25));
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_day_layer, s_font_small_text);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_layer));*/
  

  s_time_layer = text_layer_create(GRect(0, 24, 144, 60));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, s_font_big);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  
  s_weather_border_layer = layer_create(GRect(0, 100, 36, 68));
  layer_set_update_proc(s_weather_border_layer, weather_border_update_proc);
  layer_add_child(window_get_root_layer(window), s_weather_border_layer);
  
  s_weather_conditions_layer = text_layer_create(GRect(0, 112, 36, 25));
  text_layer_set_background_color(s_weather_conditions_layer, GColorClear);
  text_layer_set_font(s_weather_conditions_layer, s_font_weather_icons);
  text_layer_set_text_alignment(s_weather_conditions_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_conditions_layer));
  
  s_weather_temp_layer = text_layer_create(GRect(0, 138, 36, 25));
  text_layer_set_background_color(s_weather_temp_layer, GColorClear);
  text_layer_set_font(s_weather_temp_layer, s_font_small);
  text_layer_set_text_alignment(s_weather_temp_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_temp_layer));

  
  s_forecast_0_day_layer = text_layer_create(GRect(36, 93, 36, 25));
  text_layer_set_background_color(s_forecast_0_day_layer, GColorClear);
  text_layer_set_font(s_forecast_0_day_layer, s_font_small_text);
  text_layer_set_text_alignment(s_forecast_0_day_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_0_day_layer));
  
  s_forecast_0_conditions_layer = text_layer_create(GRect(36, 114, 36, 25));
  text_layer_set_background_color(s_forecast_0_conditions_layer, GColorClear);
  text_layer_set_font(s_forecast_0_conditions_layer, s_font_weather_icons);
  text_layer_set_text_alignment(s_forecast_0_conditions_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_0_conditions_layer));
  
  s_forecast_0_max_layer = text_layer_create(GRect(36, 134, 36, 25));
  text_layer_set_background_color(s_forecast_0_max_layer, GColorClear);
  text_layer_set_font(s_forecast_0_max_layer, s_font_small);
  text_layer_set_text_alignment(s_forecast_0_max_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_0_max_layer));
  
  s_forecast_0_min_layer = text_layer_create(GRect(36, 149, 36, 25));
  text_layer_set_background_color(s_forecast_0_min_layer, GColorClear);
  text_layer_set_font(s_forecast_0_min_layer, s_font_small);
  text_layer_set_text_alignment(s_forecast_0_min_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_0_min_layer));

  
  s_forecast_1_day_layer = text_layer_create(GRect(72, 93, 36, 25));
  text_layer_set_background_color(s_forecast_1_day_layer, GColorClear);
  text_layer_set_font(s_forecast_1_day_layer, s_font_small_text);
  text_layer_set_text_alignment(s_forecast_1_day_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_1_day_layer));
  
  s_forecast_1_conditions_layer = text_layer_create(GRect(72, 114, 36, 25));
  text_layer_set_background_color(s_forecast_1_conditions_layer, GColorClear);
  text_layer_set_font(s_forecast_1_conditions_layer, s_font_weather_icons);
  text_layer_set_text_alignment(s_forecast_1_conditions_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_1_conditions_layer));
  
  s_forecast_1_max_layer = text_layer_create(GRect(72, 134, 36, 25));
  text_layer_set_background_color(s_forecast_1_max_layer, GColorClear);
  text_layer_set_font(s_forecast_1_max_layer, s_font_small);
  text_layer_set_text_alignment(s_forecast_1_max_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_1_max_layer));
  
  s_forecast_1_min_layer = text_layer_create(GRect(72, 149, 36, 25));
  text_layer_set_background_color(s_forecast_1_min_layer, GColorClear);
  text_layer_set_font(s_forecast_1_min_layer, s_font_small);
  text_layer_set_text_alignment(s_forecast_1_min_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_1_min_layer));
  

  s_forecast_2_day_layer = text_layer_create(GRect(108, 93, 36, 25));
  text_layer_set_background_color(s_forecast_2_day_layer, GColorClear);
  text_layer_set_font(s_forecast_2_day_layer, s_font_small_text);
  text_layer_set_text_alignment(s_forecast_2_day_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_2_day_layer));

  s_forecast_2_conditions_layer = text_layer_create(GRect(108, 114, 36, 25));
  text_layer_set_background_color(s_forecast_2_conditions_layer, GColorClear);
  text_layer_set_font(s_forecast_2_conditions_layer, s_font_weather_icons);
  text_layer_set_text_alignment(s_forecast_2_conditions_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_2_conditions_layer));
  
  s_forecast_2_max_layer = text_layer_create(GRect(108, 134, 36, 25));
  text_layer_set_background_color(s_forecast_2_max_layer, GColorClear);
  text_layer_set_font(s_forecast_2_max_layer, s_font_small);
  text_layer_set_text_alignment(s_forecast_2_max_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_2_max_layer));
  
  s_forecast_2_min_layer = text_layer_create(GRect(108, 149, 36, 25));
  text_layer_set_background_color(s_forecast_2_min_layer, GColorClear);
  text_layer_set_font(s_forecast_2_min_layer, s_font_small);
  text_layer_set_text_alignment(s_forecast_2_min_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_2_min_layer));
  
  update_ui();
}

static void main_window_unload(Window *window) {
  /*bitmap_layer_destroy(s_bitmap_layer);*/
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  /*text_layer_destroy(s_day_layer);*/
  layer_destroy(s_battery_layer);
  /*text_layer_destroy(s_city_layer);*/
  layer_destroy(s_weather_border_layer);
  text_layer_destroy(s_weather_temp_layer);
  text_layer_destroy(s_weather_conditions_layer);
  text_layer_destroy(s_forecast_0_day_layer);
  text_layer_destroy(s_forecast_0_min_layer);
  text_layer_destroy(s_forecast_0_max_layer);
  text_layer_destroy(s_forecast_0_conditions_layer);
  text_layer_destroy(s_forecast_1_day_layer);
  text_layer_destroy(s_forecast_1_min_layer);
  text_layer_destroy(s_forecast_1_max_layer);
  text_layer_destroy(s_forecast_1_conditions_layer);
  text_layer_destroy(s_forecast_2_day_layer);
  text_layer_destroy(s_forecast_2_min_layer);
  text_layer_destroy(s_forecast_2_max_layer);
  text_layer_destroy(s_forecast_2_conditions_layer);
  
  fonts_unload_custom_font(s_font_big);
  fonts_unload_custom_font(s_font_small);
  fonts_unload_custom_font(s_font_small_text);
}


static int s_outbox_send_current_try = 0;
static time_t s_outbox_send_last_action = 0;
static int s_get_weather_last_received = 0;
static uint8_t s_action_store_settings = 0;
static uint8_t s_action_get_weather = 0;
static bool s_ready = false;
static bool s_outbox_send_pending_ready = false;

static void _outbox_send() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "_outbox_send()");
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  if (s_action_store_settings) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "action:store_settings");
    dict_write_uint8(iter, KEY_ACTION_STORE_SETTINGS, s_action_store_settings);
    dict_write_uint8(iter, KEY_SUPPORTS_COLOR, supports_color ? 1 : 0);
    dict_write_cstring(iter, KEY_TEMP_UNITS, s_temp_units);
    dict_write_uint8(iter, KEY_BG_COLOR, s_bg_color);
    dict_write_uint8(iter, KEY_TEXT_COLOR, s_text_color);
    dict_write_cstring(iter, KEY_WEATHER_SERVICE, s_weather_service);
  }
  
  if (s_action_get_weather) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "action:get_weather");
    dict_write_uint8(iter, KEY_ACTION_GET_WEATHER, s_action_get_weather);
  }
  
  app_message_outbox_send();
}

static void outbox_send() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "outbox_send()");
  if (s_ready) {
    _outbox_send();
  } else {
    s_outbox_send_pending_ready = true;
  }
}

static bool _inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);
  bool received_weather = false;

  while(t != NULL) {
    switch(t->key) {
      case KEY_READY:
        s_ready = true;
        if (s_outbox_send_pending_ready) {
          _outbox_send();
        }
      break;
      
      case KEY_TEMP_UNITS:
        strncpy(s_temp_units, t->value->cstring, sizeof(s_temp_units));
      break;
      case KEY_BG_COLOR:
        s_bg_color = t->value->uint8;
      break;
      case KEY_TEXT_COLOR:
        s_text_color = t->value->uint8;
      break;
      case KEY_WEATHER_SERVICE:
        strncpy(s_weather_service, t->value->cstring, sizeof(s_weather_service));
      break;
      
      case KEY_CITY:
        /*snprintf(s_city_buffer, sizeof(s_city_buffer), "%s", t->value->cstring);
        text_layer_set_text(s_city_layer, s_city_buffer);*/
      break;
      case KEY_WEATHER_TEMP_C:
        received_weather = true;
        strncpy(s_weather_temp_c_buffer, t->value->cstring, sizeof(s_weather_temp_c_buffer));
      break;
      case KEY_WEATHER_TEMP_F:
        strncpy(s_weather_temp_f_buffer, t->value->cstring, sizeof(s_weather_temp_f_buffer));
      break;
      case KEY_WEATHER_CONDITIONS:
        strncpy(s_weather_conditions_buffer, t->value->cstring, sizeof(s_weather_conditions_buffer));
      break;
      case KEY_FORECAST_0_DAY:
        strncpy(s_forecast_0_day_buffer, t->value->cstring, sizeof(s_forecast_0_day_buffer));
      break;
      case KEY_FORECAST_0_MIN_C:
        strncpy(s_forecast_0_min_c_buffer, t->value->cstring, sizeof(s_forecast_0_min_c_buffer));
      break;
      case KEY_FORECAST_0_MIN_F:
        strncpy(s_forecast_0_min_f_buffer, t->value->cstring, sizeof(s_forecast_0_min_f_buffer));
      break;
      case KEY_FORECAST_0_MAX_C:
        strncpy(s_forecast_0_max_c_buffer, t->value->cstring, sizeof(s_forecast_0_max_c_buffer));
      break;
      case KEY_FORECAST_0_MAX_F:
        strncpy(s_forecast_0_max_f_buffer, t->value->cstring, sizeof(s_forecast_0_max_f_buffer));
      break;
      case KEY_FORECAST_0_CONDITIONS:
        strncpy(s_forecast_0_conditions_buffer, t->value->cstring, sizeof(s_forecast_0_conditions_buffer));
      break;
      case KEY_FORECAST_1_DAY:
        strncpy(s_forecast_1_day_buffer, t->value->cstring, sizeof(s_forecast_1_day_buffer));
      break;
      case KEY_FORECAST_1_MIN_C:
        strncpy(s_forecast_1_min_c_buffer, t->value->cstring, sizeof(s_forecast_1_min_c_buffer));
      break;
      case KEY_FORECAST_1_MIN_F:
        strncpy(s_forecast_1_min_f_buffer, t->value->cstring, sizeof(s_forecast_1_min_f_buffer));
      break;
      case KEY_FORECAST_1_MAX_C:
        strncpy(s_forecast_1_max_c_buffer, t->value->cstring, sizeof(s_forecast_1_max_c_buffer));
      break;
      case KEY_FORECAST_1_MAX_F:
        strncpy(s_forecast_1_max_f_buffer, t->value->cstring, sizeof(s_forecast_1_max_f_buffer));
      break;
      case KEY_FORECAST_1_CONDITIONS:
        strncpy(s_forecast_1_conditions_buffer, t->value->cstring, sizeof(s_forecast_1_conditions_buffer));
      break;
      case KEY_FORECAST_2_DAY:
        strncpy(s_forecast_2_day_buffer, t->value->cstring, sizeof(s_forecast_2_day_buffer));
      break;
      case KEY_FORECAST_2_MIN_C:
        strncpy(s_forecast_2_min_c_buffer, t->value->cstring, sizeof(s_forecast_2_min_c_buffer));
      break;
      case KEY_FORECAST_2_MIN_F:
        strncpy(s_forecast_2_min_f_buffer, t->value->cstring, sizeof(s_forecast_2_min_f_buffer));
      break;
      case KEY_FORECAST_2_MAX_C:
        strncpy(s_forecast_2_max_c_buffer, t->value->cstring, sizeof(s_forecast_2_max_c_buffer));
      break;
      case KEY_FORECAST_2_MAX_F:
        strncpy(s_forecast_2_max_f_buffer, t->value->cstring, sizeof(s_forecast_2_max_f_buffer));
      break;
      case KEY_FORECAST_2_CONDITIONS:
        strncpy(s_forecast_2_conditions_buffer, t->value->cstring, sizeof(s_forecast_2_conditions_buffer));
      break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    t = dict_read_next(iterator);
  }
  
  update_ui();
  
  return received_weather;
}

static void store_settings() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "store_settings()");
  s_action_store_settings = 1;
  s_action_get_weather = 0;
  outbox_send();
}

static void _get_weather() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "_get_weather()");
  s_action_store_settings = 1;
  s_action_get_weather = 1;
  outbox_send();
}

static void get_weather() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "get_weather()");
  time_t current_time = time(NULL);
  if (current_time - s_outbox_send_last_action > GET_WEATHER_TIMEOUT) {
    s_outbox_send_last_action = current_time;
    s_outbox_send_current_try = 0;
    _get_weather();
  }
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "outbox_sent_callback()");
  s_action_get_weather = 0;
  s_action_store_settings = 0;
  s_outbox_send_last_action = time(NULL);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  s_outbox_send_current_try++;
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed: %d (try %d/%d)", reason, s_outbox_send_current_try, OUTBOX_SEND_MAX_TRIES);
  if (s_outbox_send_current_try < OUTBOX_SEND_MAX_TRIES) {
    outbox_send();
  } else {
    s_outbox_send_last_action = 0;
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "inbox_received_callback()");
  if (_inbox_received_callback(iterator, context)) {
    s_get_weather_last_received = (int)time(NULL);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped: %d", reason);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
  
  int elapsed = (int)time(NULL) - s_get_weather_last_received;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "elapsed = %d", elapsed);
  if (elapsed >= WEATHER_UPDATE_MINUTES * 60) {
    get_weather();
  }
}

static void init() {
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  battery_state_service_subscribe(battery_handler);
  /*#ifdef PBL_COLOR
    accel_tap_service_subscribe(tap_handler);
  #endif*/
  
  s_get_weather_last_received = persist_exists(KEY_GET_WEATHER_LAST_RECEIVED) ? persist_read_int(KEY_GET_WEATHER_LAST_RECEIVED) : 0;
  if (persist_exists(KEY_WEATHER_TEMP_C)) {
    persist_read_string(KEY_WEATHER_TEMP_C, s_weather_temp_c_buffer, sizeof(s_weather_temp_c_buffer));
    persist_read_string(KEY_WEATHER_TEMP_F, s_weather_temp_f_buffer, sizeof(s_weather_temp_f_buffer));
    persist_read_string(KEY_WEATHER_CONDITIONS, s_weather_conditions_buffer, sizeof(s_weather_conditions_buffer));
    persist_read_string(KEY_FORECAST_0_DAY, s_forecast_0_day_buffer, sizeof(s_forecast_0_day_buffer));
    persist_read_string(KEY_FORECAST_0_MIN_C, s_forecast_0_min_c_buffer, sizeof(s_forecast_0_min_c_buffer));
    persist_read_string(KEY_FORECAST_0_MIN_F, s_forecast_0_min_f_buffer, sizeof(s_forecast_0_min_f_buffer));
    persist_read_string(KEY_FORECAST_0_MAX_C, s_forecast_0_max_c_buffer, sizeof(s_forecast_0_max_c_buffer));
    persist_read_string(KEY_FORECAST_0_MAX_F, s_forecast_0_max_f_buffer, sizeof(s_forecast_0_max_f_buffer));
    persist_read_string(KEY_FORECAST_0_CONDITIONS, s_forecast_0_conditions_buffer, sizeof(s_forecast_0_conditions_buffer));
    persist_read_string(KEY_FORECAST_1_DAY, s_forecast_1_day_buffer, sizeof(s_forecast_1_day_buffer));
    persist_read_string(KEY_FORECAST_1_MIN_C, s_forecast_1_min_c_buffer, sizeof(s_forecast_1_min_c_buffer));
    persist_read_string(KEY_FORECAST_1_MIN_F, s_forecast_1_min_f_buffer, sizeof(s_forecast_1_min_f_buffer));
    persist_read_string(KEY_FORECAST_1_MAX_C, s_forecast_1_max_c_buffer, sizeof(s_forecast_1_max_c_buffer));
    persist_read_string(KEY_FORECAST_1_MAX_F, s_forecast_1_max_f_buffer, sizeof(s_forecast_1_max_f_buffer));
    persist_read_string(KEY_FORECAST_1_CONDITIONS, s_forecast_1_conditions_buffer, sizeof(s_forecast_1_conditions_buffer));
    persist_read_string(KEY_FORECAST_2_DAY, s_forecast_2_day_buffer, sizeof(s_forecast_2_day_buffer));
    persist_read_string(KEY_FORECAST_2_MIN_C, s_forecast_2_min_c_buffer, sizeof(s_forecast_2_min_c_buffer));
    persist_read_string(KEY_FORECAST_2_MIN_F, s_forecast_2_min_f_buffer, sizeof(s_forecast_2_min_f_buffer));
    persist_read_string(KEY_FORECAST_2_MAX_C, s_forecast_2_max_c_buffer, sizeof(s_forecast_2_max_c_buffer));
    persist_read_string(KEY_FORECAST_2_MAX_F, s_forecast_2_max_f_buffer, sizeof(s_forecast_2_max_f_buffer));
    persist_read_string(KEY_FORECAST_2_CONDITIONS, s_forecast_2_conditions_buffer, sizeof(s_forecast_2_conditions_buffer));
    
    persist_read_string(KEY_TEMP_UNITS, s_temp_units, sizeof(s_temp_units));
    s_bg_color = persist_read_int(KEY_BG_COLOR);
    s_text_color = persist_read_int(KEY_TEXT_COLOR);
    persist_read_string(KEY_WEATHER_SERVICE, s_weather_service, sizeof(s_weather_service));
    
    #ifndef PBL_COLOR
      if (( s_text_color != GColorWhiteARGB8 && s_text_color != GColorBlackARGB8 ) ||
          ( s_bg_color != GColorWhiteARGB8 && s_bg_color != GColorBlackARGB8 )) {
        s_text_color = GColorWhiteARGB8;
        s_bg_color = GColorBlackARGB8;
      }
    #endif

    store_settings();
  } else {
    get_weather();
    strcpy(s_temp_units, strcmp(i18n_get_system_locale(), "en_US") ? "f" : "c");
  }
  
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  time_t t = time(NULL);
  update_time(localtime(&t));
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(s_main_window);
  battery_state_service_unsubscribe();
  /*#ifdef PBL_COLOR
    accel_tap_service_unsubscribe();
  #endif*/
  
  persist_write_int(KEY_GET_WEATHER_LAST_RECEIVED, s_get_weather_last_received);
    
  persist_write_string(KEY_WEATHER_TEMP_C, s_weather_temp_c_buffer);
  persist_write_string(KEY_WEATHER_TEMP_F, s_weather_temp_f_buffer);
  persist_write_string(KEY_WEATHER_CONDITIONS, s_weather_conditions_buffer);
  persist_write_string(KEY_FORECAST_0_DAY, s_forecast_0_day_buffer);
  persist_write_string(KEY_FORECAST_0_MIN_C, s_forecast_0_min_c_buffer);
  persist_write_string(KEY_FORECAST_0_MIN_F, s_forecast_0_min_f_buffer);
  persist_write_string(KEY_FORECAST_0_MAX_C, s_forecast_0_max_c_buffer);
  persist_write_string(KEY_FORECAST_0_MAX_F, s_forecast_0_max_f_buffer);
  persist_write_string(KEY_FORECAST_0_CONDITIONS, s_forecast_0_conditions_buffer);
  persist_write_string(KEY_FORECAST_1_DAY, s_forecast_1_day_buffer);
  persist_write_string(KEY_FORECAST_1_MIN_C, s_forecast_1_min_c_buffer);
  persist_write_string(KEY_FORECAST_1_MIN_F, s_forecast_1_min_f_buffer);
  persist_write_string(KEY_FORECAST_1_MAX_C, s_forecast_1_max_c_buffer);
  persist_write_string(KEY_FORECAST_1_MAX_F, s_forecast_1_max_f_buffer);
  persist_write_string(KEY_FORECAST_1_CONDITIONS, s_forecast_1_conditions_buffer);
  persist_write_string(KEY_FORECAST_2_DAY, s_forecast_2_day_buffer);
  persist_write_string(KEY_FORECAST_2_MIN_C, s_forecast_2_min_c_buffer);
  persist_write_string(KEY_FORECAST_2_MIN_F, s_forecast_2_min_f_buffer);
  persist_write_string(KEY_FORECAST_2_MAX_C, s_forecast_2_max_c_buffer);
  persist_write_string(KEY_FORECAST_2_MAX_F, s_forecast_2_max_f_buffer);
  persist_write_string(KEY_FORECAST_2_CONDITIONS, s_forecast_2_conditions_buffer);
  
  persist_write_string(KEY_TEMP_UNITS, s_temp_units);
  persist_write_int(KEY_BG_COLOR, s_bg_color);
  persist_write_int(KEY_TEXT_COLOR, s_text_color);
  persist_write_string(KEY_WEATHER_SERVICE, s_weather_service);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}