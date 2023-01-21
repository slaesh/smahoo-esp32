#include <Arduino.h>
#include <driver/gpio.h>

#include "../Loggy.h"
#include "../app_params/app_params.h"
#include "../board.h"
#include "../mqtt/mqtt.h"
#include "iizi_wifi_mng.h"
#include "is_timer_expired.h"

Loggy ioLoggy("io");

#define OUTPUT_ONE                    (0)  // currently we only support one output..

#define OUTPUT_MODE_FLIPFLOP          (1)
#define OUTPUT_MODE_DIRECT            (2)
#define OUTPUT_MODE_DIRECT_OUTPUT_ON  (0)  // LOW will switch the output on!
#define OUTPUT_MODE_DIRECT_OUTPUT_OFF (1)

#if BOARD == BOARD_IWR_OLD_WO_INPUT

#define OUTPUT_MODE         (OUTPUT_MODE_DIRECT)

#define OUTPUT_PIN          (5)   // IO5 , PIN34 .. PCB_BOARDIO_ENOUT1
#define OUTPUT_FEEDBACK_PIN (8)   // IO8 , PIN33 .. PCB_BOARDIO_ENOUT2
#define INPUT_SWITCH_PIN    (4)   // IO4 , PIN24 .. PCB_BOARDIO_2
#define INPUT_BUTTON_PIN    (13)  // IO13, PIN20 .. PCB_BOARDIO_3

#elif BOARD == BOARD_IWR

#define OUTPUT_MODE         (OUTPUT_MODE_FLIPFLOP)

#define OUTPUT_PIN          (4)   // IO4 , PIN24 .. PCB_BOARDIO_4
#define OUTPUT_FEEDBACK_PIN (8)   // IO8 , PIN33 .. PCB_BOARDIO_1
#define INPUT_SWITCH_PIN    (7)   // IO7 , PIN32 .. PCB_BOARDIO_2
#define INPUT_BUTTON_PIN    (13)  // IO13, PIN20 .. PCB_BOARDIO_5

#else
#error "INVALID BOARD"
#endif

static uint8_t getLastKnownOutputState() {
  return digitalRead(OUTPUT_FEEDBACK_PIN);
}

typedef struct {
  bool state;
  bool justChanged;
  uint8_t signalEdgeCnt;
  uint32_t lastStateChange;
} t_s_checkInputResult;

static t_s_checkInputResult input = {
    .state           = false,
    .justChanged     = false,
    .signalEdgeCnt   = 0,
    .lastStateChange = 0,
};

void IRAM_ATTR switchStateChanged() {
  input.lastStateChange = millis();

#define MIN_EDGES_TO_BE_PRESSED (3)

  // only count if we not already have reached the threshould..
  // .. so we are not overflowing sometimes.. ;)
  if (input.signalEdgeCnt >= MIN_EDGES_TO_BE_PRESSED) return;

  // still not reached?
  if (++input.signalEdgeCnt < MIN_EDGES_TO_BE_PRESSED) return;

  // .. freshly toggled!
  input.state       = true;
  input.justChanged = true;
}

static void io_setup() {
  pinMode(OUTPUT_FEEDBACK_PIN, INPUT_PULLUP);
  pinMode(INPUT_SWITCH_PIN, INPUT_PULLUP);
  pinMode(INPUT_BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(INPUT_SWITCH_PIN, switchStateChanged, CHANGE);

#if OUTPUT_MODE == OUTPUT_MODE_FLIPFLOP

  // be sure to set the output low before enabling it!
  // .. to avoid unwanted flip-flop toggles! ;)
  digitalWrite(OUTPUT_PIN, LOW);

#elif OUTPUT_MODE == OUTPUT_MODE_DIRECT

  // just set the last known value right away!
  digitalWrite(OUTPUT_PIN, getLastKnownOutputState());

#else

#error "INVALID SETUP"

#endif

  pinMode(OUTPUT_PIN, OUTPUT);
}

/*static*/ void io_toggleOutput(uint8_t _o) {
#if OUTPUT_MODE == OUTPUT_MODE_FLIPFLOP

  digitalWrite(OUTPUT_PIN, HIGH);
  vTaskDelay(1);
  digitalWrite(OUTPUT_PIN, LOW);

#elif OUTPUT_MODE == OUTPUT_MODE_DIRECT

  digitalWrite(OUTPUT_PIN, digitalRead(OUTPUT_PIN) ^ 1);

#else

#error INVALID OUTPUT MODE

#endif

  MQTT_NOTIFY_IO_CHANGES();
}

/**
 * @brief sets the output
 *
 * 0 = off
 * 1 = on
 */
/*static*/ void io_setOutput(uint8_t _o, uint8_t v) {
#if OUTPUT_MODE == OUTPUT_MODE_FLIPFLOP

  const auto curOutputState = digitalRead(OUTPUT_FEEDBACK_PIN);
  // cur and target are "the same"? just return..
  if (curOutputState == v) return;

  // otherwise, toggle!
  io_toggleOutput(_o);

#elif OUTPUT_MODE == OUTPUT_MODE_DIRECT

  digitalWrite(OUTPUT_PIN, v ? OUTPUT_MODE_DIRECT_OUTPUT_ON
                             : OUTPUT_MODE_DIRECT_OUTPUT_OFF);

#else

#error INVALID OUTPUT MODE

#endif

  MQTT_NOTIFY_IO_CHANGES();
}

/*static*/ uint8_t io_getInputState() {
  // return digitalRead(INPUT_SWITCH_PIN);
  return input.state;
}

/*static*/ uint8_t io_getOutputState() {
#if OUTPUT_MODE == OUTPUT_MODE_FLIPFLOP

  return digitalRead(OUTPUT_FEEDBACK_PIN);

#elif OUTPUT_MODE == OUTPUT_MODE_DIRECT

  return digitalRead(OUTPUT_PIN) ^ 1;  // invert it

#else

#error INVALID OUTPUT MODE

#endif
}

static void checkIfInputOff() {
  // already off? just return..
  if (input.signalEdgeCnt == 0) return;

// 50-60 Hz, use slowest
#define MS_PER_PULS          (1000 / 50)  // 20ms pulsewidth
#define PULSE_CNT_SWITCH_OFF (3)

  // store "lastChange" info BEFORE getting millis down below..
  uint32_t lastChange = input.lastStateChange;
  uint32_t now        = millis();

  // was non-zero before.. AND there was a "longer" time no change?
  if (is_timer_expired(now, lastChange, PULSE_CNT_SWITCH_OFF * MS_PER_PULS)) {
    // .. freshly toggled!
    ioLoggy.printf("Switch is OFF! %d :: %d :: %d => %d\n", lastChange,
                   input.lastStateChange, now, input.signalEdgeCnt);

    input.signalEdgeCnt = 0;
    input.state         = false;
    input.justChanged   = true;

    return;
  }
}

static uint32_t taskLoopStarted = 0;

static void handle_input() {
  static bool ignoreFirstInputChange = true;

  extern bool http_update_in_progress;
  if (http_update_in_progress) {
    return;
  }

  // input "on" is triggered by IRQ, "off" needs to be done "manually"..
  checkIfInputOff();

  if (!input.justChanged) return;

  uint32_t now = millis();

  if (input.state) {
    ioLoggy.printf("Switch is ON! %ld :: %d\n", input.lastStateChange, now);
  }

  // reset that flag!
  input.justChanged = false;

  if (ignoreFirstInputChange && now < (taskLoopStarted + 200)) {
    ioLoggy.println("ignore first input 'change' ..");
    ignoreFirstInputChange = false;
    return;
  }

  MQTT_NOTIFY_IO_CHANGES();

  ioLoggy.printf(">> input changed(%d)\n", input.state);

  const String inputTogglesOutputParam =
      iizi_get_parameter_value(IIZI_PARAM_INPUT_TOGGLES_OUTPUT_KEY);
  if (inputTogglesOutputParam == "yes") {
    ioLoggy.printf(">>>> toggle output!\n");
    io_toggleOutput(OUTPUT_ONE);
  }
}

static void handle_button() {
  const auto now                     = millis();
  static TickTimer open_portal_timer = TickTimer(2000, now);
  static TickTimer reset_timer       = TickTimer(20000, now);
  const auto btnState                = digitalRead(INPUT_BUTTON_PIN);

  // button not pressed?
  if (btnState == 1) {
    // button released and before it has been pressed long enough?
    if (open_portal_timer.isExpired(now)) {
      ioLoggy.println("BTN released, but was long pressed!");

      iizi_wifi_mng_open_portal();
    }

    open_portal_timer.reset(now);
    reset_timer.reset(now);

    return;
  }

  if (reset_timer.isExpired(now)) {
    ioLoggy.println("BTN still pressed.. RESET!");

    vTaskDelay(500 / portTICK_PERIOD_MS);

    ESP.restart();
  }
}

static void io_task(void *_) {
  ioLoggy.printf("io_task(): started @ core %d\n", xPortGetCoreID());

  io_setup();

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  taskLoopStarted = millis();

  for (;;) {
#if BOARD != BOARD_IWR_OLD_WO_INPUT
    handle_input();
    handle_button();
#else
    // nothing to handle here.. expand idle time!
    vTaskDelay(1000 / portTICK_PERIOD_MS);
#endif

    vTaskDelay(4 / portTICK_PERIOD_MS);

    // >>>>>> TODO REMOVE ME
    static uint32_t lastPrinted = millis();
    if (is_timer_expired(millis(), lastPrinted, 10 * 60 * 1000)) {
      ioLoggy.printf("stack-watermark(%d): %lu bytes\n", xPortGetCoreID(),
                     uxTaskGetStackHighWaterMark(NULL));

      lastPrinted = millis();
    }
    // <<<<<< TODO REMOVE ME
  }

  // should not happen.. ?!
  vTaskDelete(NULL);
}

static void io_createTask() {
  ioLoggy.addLogger(&Serial);
  ioLoggy.println("io_createTask");

  xTaskCreate(io_task, "io", 1024 * 2, NULL,
              // make this task a high-prio task!
              configMAX_PRIORITIES - 2, NULL);
}

// PUBLIC METHODS

void io_task_setup() { io_createTask(); }
