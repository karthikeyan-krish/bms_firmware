#include "main.h"

#include <FreeRTOS.h>
#include <pw_assert/check.h>
#include <pw_chrono/system_clock.h>
#include <pw_chrono/system_timer.h>
#include <pw_log/log.h>
#include <pw_sys_io_stm32cube/init.h>
#include <pw_system/work_queue.h>
#include <pw_thread/detached_thread.h>
#include <pw_thread/sleep.h>
#include <task.h>

#include "active_object.hpp"
#include "gpio.h"

#if defined(BLD_APP_SLOT_BUILD)
#include "bld_confirm.h"
#endif

namespace {
using namespace std::chrono_literals;

enum class ThreadPriority : UBaseType_t {
  kWorkQueue = tskIDLE_PRIORITY + 1,
  kNumPriorities,
};

static_assert(static_cast<UBaseType_t>(ThreadPriority::kNumPriorities) <=
              configMAX_PRIORITIES);

constexpr size_t kWorkQueueThreadWords = 512;

pw::sync::Mutex fsm_mutex_;

}  // namespace

// Functions needed when configGENERATE_RUN_TIME_STATS is on.
extern "C" void configureTimerForRunTimeStats(void) {}
extern "C" unsigned long getRunTimeCounterValue(void) { return uwTick; }

// System Clock Configuration: 80MHz
extern "C" void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};

  /** Configure the main internal regulator output voltage
   */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
    Error_Handler();
  }
}

namespace {
auto wq = pw::system::GetWorkQueue;
static void StartWorkQueueThread() {
  pw::thread::DetachedThread(
      pw::thread::freertos::Options()
          .set_name("WorkQueueThread")
          .set_priority(static_cast<UBaseType_t>(ThreadPriority::kWorkQueue))
          .set_stack_size(kWorkQueueThreadWords),
      wq());
}

static void ButtonPressed() {
  // button press from message queue
}

}  // namespace

extern "C" int main(void) {
  HAL_Init();

  SystemClock_Config();
  pw_sys_io_Init();

  bsp_init();

#if defined(BLD_APP_SLOT_BUILD)
  bld_confirm_running_image();
#endif

  StartWorkQueueThread();

  vTaskStartScheduler();

  while (1) {
  }
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == BUTTON_EXTI13_Pin) {
    pw::system::GetWorkQueue().CheckPushWork(ButtonPressed);
  }
}
extern "C" PW_NO_RETURN void Error_Handler(void) { PW_CRASH("Error"); }
