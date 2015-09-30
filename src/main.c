#include <pebble.h>

const int WEATHER_UPDATE_MINUTES = 30;
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
  #define COLOR_BATT ((uint8_t)GColorWhiteARGB8)
  #define COLOR_BATT_LOW ((uint8_t)GColorRedARGB8)
  #define COLOR_BG ((uint8_t)GColorBlackARGB8)
  #define COLOR_TEXT ((uint8_t)GColorWhiteARGB8)
#else
  #define COLOR_BATT ((uint8_t)GColorWhiteARGB8)
  #define COLOR_BATT_LOW ((uint8_t)COLOR_BATT)
  #define COLOR_BG ((uint8_t)GColorBlackARGB8)
  #define COLOR_TEXT ((uint8_t)GColorWhiteARGB8)
#endif

enum {
  KEY_TEMP_C = 0,
  KEY_TEMP_F = 1,
  KEY_CONDITIONS = 2,
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
  KEY_CITY = 21
};

static int s_batt_percent = 0;
/*static char s_battery_buffer[] = "---";*/
static char s_time_buffer[] = "XX:XX";
static char s_date_buffer[] = "XXX XXX XX";
/*static char s_day_buffer[] = "X";
static char s_date_combined_buffer[] = "X XXX XX";*/
/*static char s_city_buffer[13];*/
static char s_weather_temp_buffer[5];
static char s_weather_conditions_buffer[8];
static char s_forecast_0_day_buffer[4];
static char s_forecast_0_min_buffer[5];
static char s_forecast_0_max_buffer[5];
static char s_forecast_0_conditions_buffer[8];
static char s_forecast_1_day_buffer[4];
static char s_forecast_1_min_buffer[5];
static char s_forecast_1_max_buffer[5];
static char s_forecast_1_conditions_buffer[8];
static char s_forecast_2_day_buffer[4];
static char s_forecast_2_min_buffer[5];
static char s_forecast_2_max_buffer[5];
static char s_forecast_2_conditions_buffer[8];

static Window *s_main_window;
static BitmapLayer *s_bitmap_layer;
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

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
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
  graphics_context_set_stroke_color(ctx, s_batt_percent > 20 ? (GColor)COLOR_BATT : (GColor)COLOR_BATT_LOW);
  graphics_context_set_fill_color(ctx, s_batt_percent > 20 ? (GColor)COLOR_BATT : (GColor)COLOR_BATT_LOW);
  graphics_draw_round_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h), 1);
  
  int batt_px;
  batt_px = (int)((float)s_batt_percent / 100.0 * (float)(bounds.size.w - 4) + 0.5);
  graphics_fill_rect(ctx, GRect(2, 2, batt_px, bounds.size.h - 4), 0, GCornersAll);
}

static void weather_border_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, (GColor)COLOR_TEXT);
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
  
  window_set_background_color(s_main_window, (GColor)COLOR_BG);
  

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
  text_layer_set_text_color(s_date_layer, (GColor)COLOR_TEXT);
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
  text_layer_set_text_color(s_time_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_time_layer, s_font_big);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  
  s_weather_border_layer = layer_create(GRect(0, 100, 36, 68));
  layer_set_update_proc(s_weather_border_layer, weather_border_update_proc);
  layer_add_child(window_get_root_layer(window), s_weather_border_layer);
  
  s_weather_conditions_layer = text_layer_create(GRect(0, 114, 36, 25));
  text_layer_set_background_color(s_weather_conditions_layer, GColorClear);
  text_layer_set_text_color(s_weather_conditions_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_weather_conditions_layer, s_font_weather_icons);
  text_layer_set_text_alignment(s_weather_conditions_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_conditions_layer));
  
  s_weather_temp_layer = text_layer_create(GRect(0, 140, 36, 25));
  text_layer_set_background_color(s_weather_temp_layer, GColorClear);
  text_layer_set_text_color(s_weather_temp_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_weather_temp_layer, s_font_small);
  text_layer_set_text_alignment(s_weather_temp_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_temp_layer));

  
  s_forecast_0_day_layer = text_layer_create(GRect(36, 93, 36, 25));
  text_layer_set_background_color(s_forecast_0_day_layer, GColorClear);
  text_layer_set_text_color(s_forecast_0_day_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_forecast_0_day_layer, s_font_small_text);
  text_layer_set_text_alignment(s_forecast_0_day_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_0_day_layer));
  
  s_forecast_0_conditions_layer = text_layer_create(GRect(36, 114, 36, 25));
  text_layer_set_background_color(s_forecast_0_conditions_layer, GColorClear);
  text_layer_set_text_color(s_forecast_0_conditions_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_forecast_0_conditions_layer, s_font_weather_icons);
  text_layer_set_text_alignment(s_forecast_0_conditions_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_0_conditions_layer));
  
  s_forecast_0_max_layer = text_layer_create(GRect(36, 134, 36, 25));
  text_layer_set_background_color(s_forecast_0_max_layer, GColorClear);
  text_layer_set_text_color(s_forecast_0_max_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_forecast_0_max_layer, s_font_small);
  text_layer_set_text_alignment(s_forecast_0_max_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_0_max_layer));
  
  s_forecast_0_min_layer = text_layer_create(GRect(36, 149, 36, 25));
  text_layer_set_background_color(s_forecast_0_min_layer, GColorClear);
  text_layer_set_text_color(s_forecast_0_min_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_forecast_0_min_layer, s_font_small);
  text_layer_set_text_alignment(s_forecast_0_min_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_0_min_layer));

  
  s_forecast_1_day_layer = text_layer_create(GRect(72, 93, 36, 25));
  text_layer_set_background_color(s_forecast_1_day_layer, GColorClear);
  text_layer_set_text_color(s_forecast_1_day_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_forecast_1_day_layer, s_font_small_text);
  text_layer_set_text_alignment(s_forecast_1_day_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_1_day_layer));
  
  s_forecast_1_conditions_layer = text_layer_create(GRect(72, 114, 36, 25));
  text_layer_set_background_color(s_forecast_1_conditions_layer, GColorClear);
  text_layer_set_text_color(s_forecast_1_conditions_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_forecast_1_conditions_layer, s_font_weather_icons);
  text_layer_set_text_alignment(s_forecast_1_conditions_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_1_conditions_layer));
  
  s_forecast_1_max_layer = text_layer_create(GRect(72, 134, 36, 25));
  text_layer_set_background_color(s_forecast_1_max_layer, GColorClear);
  text_layer_set_text_color(s_forecast_1_max_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_forecast_1_max_layer, s_font_small);
  text_layer_set_text_alignment(s_forecast_1_max_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_1_max_layer));
  
  s_forecast_1_min_layer = text_layer_create(GRect(72, 149, 36, 25));
  text_layer_set_background_color(s_forecast_1_min_layer, GColorClear);
  text_layer_set_text_color(s_forecast_1_min_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_forecast_1_min_layer, s_font_small);
  text_layer_set_text_alignment(s_forecast_1_min_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_1_min_layer));
  

  s_forecast_2_day_layer = text_layer_create(GRect(108, 93, 36, 25));
  text_layer_set_background_color(s_forecast_2_day_layer, GColorClear);
  text_layer_set_text_color(s_forecast_2_day_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_forecast_2_day_layer, s_font_small_text);
  text_layer_set_text_alignment(s_forecast_2_day_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_2_day_layer));

  s_forecast_2_conditions_layer = text_layer_create(GRect(108, 114, 36, 25));
  text_layer_set_background_color(s_forecast_2_conditions_layer, GColorClear);
  text_layer_set_text_color(s_forecast_2_conditions_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_forecast_2_conditions_layer, s_font_weather_icons);
  text_layer_set_text_alignment(s_forecast_2_conditions_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_2_conditions_layer));
  
  s_forecast_2_max_layer = text_layer_create(GRect(108, 134, 36, 25));
  text_layer_set_background_color(s_forecast_2_max_layer, GColorClear);
  text_layer_set_text_color(s_forecast_2_max_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_forecast_2_max_layer, s_font_small);
  text_layer_set_text_alignment(s_forecast_2_max_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_2_max_layer));
  
  s_forecast_2_min_layer = text_layer_create(GRect(108, 149, 36, 25));
  text_layer_set_background_color(s_forecast_2_min_layer, GColorClear);
  text_layer_set_text_color(s_forecast_2_min_layer, (GColor)COLOR_TEXT);
  text_layer_set_font(s_forecast_2_min_layer, s_font_small);
  text_layer_set_text_alignment(s_forecast_2_min_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_2_min_layer));
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

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % WEATHER_UPDATE_MINUTES == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);
  bool us_units = strcmp(i18n_get_system_locale(), "en_US") == 0;

  while(t != NULL) {
    switch(t->key) {
      case KEY_CITY:
        /*snprintf(s_city_buffer, sizeof(s_city_buffer), "%s", t->value->cstring);
        text_layer_set_text(s_city_layer, s_city_buffer);*/
      break;
      case KEY_TEMP_C:
        if (!us_units) {
          snprintf(s_weather_temp_buffer, sizeof(s_weather_temp_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_weather_temp_layer, s_weather_temp_buffer);
        }
      break;
      case KEY_TEMP_F:
        if (us_units) {
          snprintf(s_weather_temp_buffer, sizeof(s_weather_temp_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_weather_temp_layer, s_weather_temp_buffer);
        }
      break;
      case KEY_CONDITIONS:
        snprintf(s_weather_conditions_buffer, sizeof(s_weather_conditions_buffer), "%s", t->value->cstring);
        text_layer_set_text(s_weather_conditions_layer, s_weather_conditions_buffer);
      break;
      case KEY_FORECAST_0_DAY:
        snprintf(s_forecast_0_day_buffer, sizeof(s_forecast_0_day_buffer), "%s", t->value->cstring);
        text_layer_set_text(s_forecast_0_day_layer, s_forecast_0_day_buffer);
      break;
      case KEY_FORECAST_0_MIN_C:
        if (!us_units) {
          snprintf(s_forecast_0_min_buffer, sizeof(s_forecast_0_min_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_forecast_0_min_layer, s_forecast_0_min_buffer);
        }
      break;
      case KEY_FORECAST_0_MIN_F:
        if (us_units) {
          snprintf(s_forecast_0_min_buffer, sizeof(s_forecast_0_min_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_forecast_0_min_layer, s_forecast_0_min_buffer);
        }
      break;
      case KEY_FORECAST_0_MAX_C:
        if (!us_units) {
          snprintf(s_forecast_0_max_buffer, sizeof(s_forecast_0_max_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_forecast_0_max_layer, s_forecast_0_max_buffer);
        }
      break;
      case KEY_FORECAST_0_MAX_F:
        if (us_units) {
          snprintf(s_forecast_0_max_buffer, sizeof(s_forecast_0_max_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_forecast_0_max_layer, s_forecast_0_max_buffer);
        }
      break;
      case KEY_FORECAST_0_CONDITIONS:
        snprintf(s_forecast_0_conditions_buffer, sizeof(s_forecast_0_conditions_buffer), "%s", t->value->cstring);
        text_layer_set_text(s_forecast_0_conditions_layer, s_forecast_0_conditions_buffer);
      break;
      case KEY_FORECAST_1_DAY:
        snprintf(s_forecast_1_day_buffer, sizeof(s_forecast_1_day_buffer), "%s", t->value->cstring);
        text_layer_set_text(s_forecast_1_day_layer, s_forecast_1_day_buffer);
      break;
      case KEY_FORECAST_1_MIN_C:
        if (!us_units) {
          snprintf(s_forecast_1_min_buffer, sizeof(s_forecast_1_min_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_forecast_1_min_layer, s_forecast_1_min_buffer);
        }
      break;
      case KEY_FORECAST_1_MIN_F:
        if (us_units) {
          snprintf(s_forecast_1_min_buffer, sizeof(s_forecast_1_min_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_forecast_1_min_layer, s_forecast_1_min_buffer);
        }
      break;
      case KEY_FORECAST_1_MAX_C:
        if (!us_units) {
          snprintf(s_forecast_1_max_buffer, sizeof(s_forecast_1_max_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_forecast_1_max_layer, s_forecast_1_max_buffer);
        }
      break;
      case KEY_FORECAST_1_MAX_F:
        if (us_units) {
          snprintf(s_forecast_1_max_buffer, sizeof(s_forecast_1_max_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_forecast_1_max_layer, s_forecast_1_max_buffer);
        }
      break;
      case KEY_FORECAST_1_CONDITIONS:
        snprintf(s_forecast_1_conditions_buffer, sizeof(s_forecast_1_conditions_buffer), "%s", t->value->cstring);
        text_layer_set_text(s_forecast_1_conditions_layer, s_forecast_1_conditions_buffer);
      break;
      case KEY_FORECAST_2_DAY:
        snprintf(s_forecast_2_day_buffer, sizeof(s_forecast_2_day_buffer), "%s", t->value->cstring);
        text_layer_set_text(s_forecast_2_day_layer, s_forecast_2_day_buffer);
      break;
      case KEY_FORECAST_2_MIN_C:
        if (!us_units) {
          snprintf(s_forecast_2_min_buffer, sizeof(s_forecast_2_min_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_forecast_2_min_layer, s_forecast_2_min_buffer);
        }
      break;
      case KEY_FORECAST_2_MIN_F:
        if (us_units) {
          snprintf(s_forecast_2_min_buffer, sizeof(s_forecast_2_min_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_forecast_2_min_layer, s_forecast_2_min_buffer);
        }
      break;
      case KEY_FORECAST_2_MAX_C:
        if (!us_units) {
          snprintf(s_forecast_2_max_buffer, sizeof(s_forecast_2_max_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_forecast_2_max_layer, s_forecast_2_max_buffer);
        }
      break;
      case KEY_FORECAST_2_MAX_F:
        if (us_units) {
          snprintf(s_forecast_2_max_buffer, sizeof(s_forecast_2_max_buffer), "%s", t->value->cstring);
          text_layer_set_text(s_forecast_2_max_layer, s_forecast_2_max_buffer);
        }
      break;
      case KEY_FORECAST_2_CONDITIONS:
        snprintf(s_forecast_2_conditions_buffer, sizeof(s_forecast_2_conditions_buffer), "%s", t->value->cstring);
        text_layer_set_text(s_forecast_2_conditions_layer, s_forecast_2_conditions_buffer);
      break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  update_time();
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  battery_state_service_subscribe(battery_handler);
  /*#ifdef PBL_COLOR
    accel_tap_service_subscribe(tap_handler);
  #endif*/
}

static void deinit() {
  window_destroy(s_main_window);
  battery_state_service_unsubscribe();
  /*#ifdef PBL_COLOR
    accel_tap_service_unsubscribe();
  #endif*/
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}