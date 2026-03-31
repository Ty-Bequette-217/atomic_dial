#if SK_DISPLAY
#include "display_task.h"
#include "semaphore_guard.h"

#include <hardware/pwm.h>

DisplayTask::DisplayTask(const uint8_t task_core) : Task{"Display", 2048, 1, task_core} {
  knob_state_queue_ = xQueueCreate(1, sizeof(PB_SmartKnobState));
  assert(knob_state_queue_ != NULL);

  mutex_ = xSemaphoreCreateMutex();
  assert(mutex_ != NULL);
  
  brightness_ = (1 << SK_BACKLIGHT_BIT_DEPTH) - 1;
}

DisplayTask::~DisplayTask() {
  vQueueDelete(knob_state_queue_);
  vSemaphoreDelete(mutex_);
}

void DisplayTask::run() {
    tft_.begin();
    tft_.invertDisplay(1);
    tft_.setRotation(SK_DISPLAY_ROTATION);
    tft_.fillScreen(TFT_DARKGREEN);

    // RP2350B PWM setup for backlight
    gpio_set_function(PIN_LCD_BACKLIGHT, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PIN_LCD_BACKLIGHT);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f);
    pwm_init(slice_num, &config, true);
    
    pwm_set_gpio_level(PIN_LCD_BACKLIGHT, brightness_);

    spr_.setColorDepth(8);

    if (spr_.createSprite(TFT_WIDTH, TFT_HEIGHT) == nullptr) {
      log("ERROR: sprite allocation failed!");
      tft_.fillScreen(TFT_RED);
    } else {
      log("Sprite created!");
      tft_.fillScreen(TFT_PURPLE);
    }
    spr_.setTextColor(0xFFFF, TFT_BLACK);
    
    PB_SmartKnobState state;

    const int RADIUS = TFT_WIDTH / 2;
    const uint16_t FILL_COLOR = spr_.color565(90, 18, 151);
    const uint16_t DOT_COLOR = spr_.color565(80, 100, 200);

    spr_.setTextDatum(CC_DATUM);
    spr_.setTextColor(TFT_WHITE);
    
    while(1) {
        if (xQueueReceive(knob_state_queue_, &state, portMAX_DELAY) == pdFALSE) {
          continue;
        }

        spr_.fillSprite(TFT_BLACK);
        spr_.setFreeFont(NULL);
        spr_.drawNumber(state.current_position, TFT_WIDTH / 2, TFT_HEIGHT / 2, 1);
        
        spr_.pushSprite(0, 0);

        {
          SemaphoreGuard lock(mutex_);
          pwm_set_gpio_level(PIN_LCD_BACKLIGHT, brightness_);
        }
        delay(5);
    }
}

QueueHandle_t DisplayTask::getKnobStateQueue() {
  return knob_state_queue_;
}

void DisplayTask::setBrightness(uint16_t brightness) {
  SemaphoreGuard lock(mutex_);
  brightness_ = brightness;
}

void DisplayTask::setLogger(Logger* logger) {
  logger_ = logger;
}

void DisplayTask::log(const char* msg) {
  if (logger_ != nullptr) {
    logger_->log(msg);
  }
}

#endif
