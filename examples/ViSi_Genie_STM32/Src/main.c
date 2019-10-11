/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "visi_genie.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
	RX_REINITIALIZE,
	RX_READY
} RxStatus;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RX_BUF_SIZE			64
#define UPDATE_INTERVAL		10

#define CW	0
#define CCW 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
struct RxBuffer {
    uint8_t data[RX_BUF_SIZE];
    uint8_t head; // read from head
    uint8_t tail; // write from tail
    uint8_t count;
    uint8_t new;
    RxStatus status;
} rx_buffer;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Uart Functions
#include <stdarg.h>
char buffer[100];
void myprintf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  int len = strlen(buffer);
  HAL_UART_Transmit(&huart2, (uint8_t*)buffer, len, 1000);

}

RxStatus ReInitializeRX() {
	HAL_StatusTypeDef res = HAL_UART_Receive_IT(&huart1, &rx_buffer.new, 1);
	if (res != HAL_OK) {
		//myprintf("Failed to Initialize RX Interrupt\r\n");
		return RX_REINITIALIZE;
	}
	return RX_READY;
}

RxStatus InitializeRX() {
	rx_buffer.count = 0;
	rx_buffer.head = 0;
	rx_buffer.tail = 0;
	rx_buffer.new = 0;
	memset(rx_buffer.data, 0, RX_BUF_SIZE);

	HAL_StatusTypeDef res = HAL_UART_Receive_IT(&huart1, &rx_buffer.new, 1);
	if (res != HAL_OK) {
		return RX_REINITIALIZE;
	}
	return RX_READY;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	HAL_UART_Receive_IT(&huart1, &rx_buffer.new, 1);

	rx_buffer.data[rx_buffer.tail++] = rx_buffer.new;
	if (rx_buffer.tail == RX_BUF_SIZE) rx_buffer.tail = 0;

	rx_buffer.count++;

	rx_buffer.status = ReInitializeRX();
}

void ResetDisplay() {
	HAL_GPIO_WritePin(DISP_RST_GPIO_Port, DISP_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(DISP_RST_GPIO_Port, DISP_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(3000);
}

// Externs

void showError(uint16_t interval) {
    while (1) {
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); // here we toggle the led on and off after every successful ping interval.
		HAL_Delay(interval);
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		HAL_Delay(interval);
	}
}

unsigned long millis(void) {
	return HAL_GetTick();
}

uint16_t genieGetByteCount() {
	//myprintf("Genie Bytes Available : %i\r\n",rx_buffer.count);
	return rx_buffer.count;
}


void geniePutByte(uint8_t c) {
	HAL_UART_Transmit(&huart1, &c, 1, 10000);
	if (rx_buffer.status == RX_REINITIALIZE) {
		rx_buffer.status = ReInitializeRX();
	}
}

uint8_t genieGetByte() {
	if (rx_buffer.count == 0) return -1;
	uint8_t val = rx_buffer.data[rx_buffer.head++];
	if (rx_buffer.head == RX_BUF_SIZE) rx_buffer.head = 0;
	rx_buffer.count--;
	return val;
}

uint8_t geniePeekByte() {
	return rx_buffer.data[rx_buffer.head];
}

// Handlers
void myDebugHandler(char *str){
	myprintf("%s\r\n", str);
}



void myGenieEventHandler(void) {
	genieFrame Event;
	genieDequeueEvent(&Event);

	//If the cmd received is from a Reported Event (Events triggered from the Events tab of Workshop4 objects)
	static int slider_val = 0;

	/* If the commamd received is from a Reported Event, it will be processed here. */
	if (Event.reportObject.cmd == GENIE_REPORT_EVENT) {
	    if (Event.reportObject.object == GENIE_OBJ_SLIDER) { // If the Reported Message was from a Slider
	        if (Event.reportObject.index == 0) { // If Slider0 (Index = 0)
	            slider_val = genieGetEventData( & Event); // Receive the event data from the Slider0
	            genieWriteObject(GENIE_OBJ_LED_DIGITS, 0, slider_val); // Write Slider0 value to to LED Digits 0
	        }
	    }
	}

	/* If the commamd received is from a Reported Object, which occurs if a Read Object (genie.ReadOject) is requested in the main code, reply processed here. */
	else if (Event.reportObject.cmd == GENIE_REPORT_OBJ) {
	    if (Event.reportObject.object == GENIE_OBJ_USER_LED) { // If the Reported Message was from a User LED
	        if (Event.reportObject.index == 0) { // If UserLed0 (Index = 0)
	            bool UserLed0_val = genieGetEventData( & Event); // Receive the event data from the UserLed0
	            UserLed0_val = !UserLed0_val; // Toggle the state of the User LED Variable
	            genieWriteObject(GENIE_OBJ_USER_LED, 0, UserLed0_val); // Write UserLed0_val value back to to UserLed0
	        }
	    }
	}

	/********** This can be expanded as more objects are added that need to be captured *************
	*************************************************************************************************
	Event.reportObject.cmd is used to determine the command of that event, such as an reported event
	Event.reportObject.object is used to determine the object type, such as a Slider
	Event.reportObject.index is used to determine the index of the object, such as Slider0
	genieGetEventData(&Event) us used to save the data from the Event, into a variable.
	*************************************************************************************************/
	else if (Event.reportObject.cmd == GENIE_PING) {
	    if (Event.reportObject.object == GENIE_DISCONNECTED) {
	        /* This function runs once, when the LCD is disconnected, because it was turned off or out of sync.
	        You may use this to process necessary code. */
	    	//HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

//			uint16_t reason = genieGetEventData(&Event); // Receive the reason
//			if (reason == 1) {
//				HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
//				while (1);
//			} else {
//				while (1) {
//					HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); // here we toggle the led on and off after every successful ping interval.
//					HAL_Delay(500);
//					HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
//					HAL_Delay(500);
//				}
//			}


	    } else if (Event.reportObject.object == GENIE_READY) {
	        /* This function runs once, when the LCD is connected and synchronized.
	        You may use this to restore screen widgets, or process other code. */
	    	genieWriteObject(GENIE_OBJ_LED_DIGITS, 0, slider_val); // Restore Leddigits0
	        genieWriteObject(GENIE_OBJ_SLIDER, 0, slider_val); // Restore Slider0
	        static int recover_times = -1; // how many times did the display recover?
	        recover_times++;
//	        genieWriteStr(0, (String) GENIE_VERSION + "\n\n\tRecovered " + recover_times + " Time(s)!"); // Restore text in Strings0
	    } else if (Event.reportObject.object == GENIE_ACK) {
	        /* If a user issues a genie.Ping(interval) request and it passes,
	        this function will happen every 'interval' times chosen by the user. */
//	         HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); // here we toggle the led on and off after every successful ping interval.
//	         HAL_Delay(20);
//	         HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	    } else if (Event.reportObject.object == GENIE_NAK) {
	        /* If a user issues a genie.Ping(interval) request and it fails,
	        this function will happen every 'interval' times chosen by the user. */
	    }
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  ResetDisplay();
  InitializeRX();
  genieBegin();
  genieAttachEventHandler(myGenieEventHandler);
  genieAttachDebugger(myprintf);
  unsigned long lastUpdate = 0;

  int coolgaugeVal;
  bool gaugeRotation = CW;

  if (genieOnline()) {
      /* Set the brightness/Contrast of the Display - (Not needed but illustrates how)
      Most Displays, 1 = Display ON, 0 = Display OFF. See below for exceptions and for DIABLO16 displays.
      For uLCD-43, uLCD-220RD, uLCD-70DT, and uLCD-35DT, use 0-15 for Brightness Control, where 0 = Display OFF, though to 15 = Max Brightness ON. */
      genieWriteContrast(15);
  } else

  /* returns the current form the lcd is on */
  if (genieCurrentForm() == 0) {
      // if lcd is on form 0, do something.
  }

  genieWriteStr(0, GENIE_VERSION);
  lastUpdate = millis() - UPDATE_INTERVAL;

  uint8_t state;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  genieDoEvents();
	  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
		  lastUpdate = millis();
		  state = !(state);
		  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, state);
		  if (genieOnline()) { // check if the display is online (connected) and if on Form0
		      if (gaugeRotation == CW) {
		          coolgaugeVal++;
		          if (coolgaugeVal > 100) {
		              coolgaugeVal = 100;
		              gaugeRotation = CCW;
		          }
		      } else if (gaugeRotation == CCW) {
		          coolgaugeVal--;
		          if (coolgaugeVal < 0) {
		              coolgaugeVal = 0;
		              gaugeRotation = CW;
		          }
		      }
			genieWriteObject(GENIE_OBJ_COOL_GAUGE, 0, coolgaugeVal);

			// The results of this call will be available to myGenieEventHandler() after the display has responded
			genieReadObject(GENIE_OBJ_USER_LED, 0); // Do a manual read from the UserLEd0 object
		}
	  }
	  /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED_Pin|DISP_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DISP_RST_Pin */
  GPIO_InitStruct.Pin = DISP_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DISP_RST_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
