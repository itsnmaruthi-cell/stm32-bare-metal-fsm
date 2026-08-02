/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile uint8_t button_press=0;
typedef enum{STATE_IDLE,STATE_RUNNING,STATE_ERROR,STATE_STOP}states;
//Pointers for GPIOC(USER BUTTON)
volatile uint32_t* RCC_AHB1ENR =(volatile uint32_t*)0X40023830;
volatile uint32_t* GPIOC_MODER =(volatile uint32_t*)0X40020800;
//volatile uint32_t* GPIOC_IDR =(volatile uint32_t*)0X40020810;
volatile uint32_t* RCC_APB2ENR =(volatile uint32_t*)0X40023844;
volatile uint32_t* SYSCFG_EXTICR4 =(volatile uint32_t*)0X40013814;
volatile uint32_t* EXTI_IMR =(volatile uint32_t*)0X40013C00;
volatile uint32_t* EXTI_FTSR =(volatile uint32_t*)0X40013C0C;
volatile uint32_t* EXTI_PR =(volatile uint32_t*)0X40013C14;
//EXTI15_10 PRIORITY NUMBER IS 47. 47-32=15
volatile uint32_t* NVIC_ISER1=(volatile uint32_t*)0XE000E104;
//Pointers for GPIOA(UART TX,RX)
volatile uint32_t* GPIOA_MODER =(volatile uint32_t*)0X40020000;
volatile uint32_t* GPIOA_AFRL =(volatile uint32_t*)0X40020020;
//Pointers for USART2
volatile uint32_t* RCC_APB1ENR =(volatile uint32_t*)0X40023840;
volatile uint32_t* USART2_SR =(volatile uint32_t*)0X40004400;
volatile uint32_t* USART2_DR =(volatile uint32_t*)0X40004404;
volatile uint32_t* USART2_BRR =(volatile uint32_t*)0X40004408;
volatile uint32_t* USART2_CR1 =(volatile uint32_t*)0X4000440C;
volatile uint32_t* USART2_CR2 =(volatile uint32_t*)0X40004410;
states current_state=STATE_IDLE;
const char STATE_ONE[]="STATE_IDLE\r\n";
const char STATE_TWO[]="STATE_RUNNING\r\n";
const char STATE_THREE[]="STATE_ERROR\r\n";
const char STATE_FOUR[]="STATE_STOP\r\n";
volatile uint8_t STATE_INDEX=0;
void USART2_SendData(void);
void FSM_Update(void);
void EXTI15_10_IRQHandler(void);
#define ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))//CHECK MACROS DEFINITION PROPERLY
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
  /* USER CODE BEGIN 2 */
  *RCC_AHB1ENR|=(1U<<2);//PORTC CLOCK ENABLE
  *GPIOC_MODER&=~(3U<<26);//26,27 BITS OF MODER CLEARED AND ALSO SET TO INPUT MODE
  *RCC_AHB1ENR|=(1U<<0);//PORTA CLOCK ENABLE
  *GPIOA_MODER&=~(3U<<4);
  *GPIOA_MODER&=~(3U<<6);
  *GPIOA_MODER|=(2U<<4);//PA2 SET TO ALTERNATE FUNCTION MODE
  *GPIOA_MODER|=(2U<<6);//PA3 SET TO ALTERNATE FUNCTION MODE
  *RCC_APB2ENR|=(1U<<14);//CLOCK ENABLE FOR SYSCFG
  *RCC_APB1ENR|=(1U<<17);//CLOCK ENABLE FOR USART
  *SYSCFG_EXTICR4&=~(15U<<4);
  *SYSCFG_EXTICR4|=(2U<<4);//EXTERNAL INTERRUPTS FROM LINE13 AND PORT C
  *EXTI_FTSR|=(1U<<13);//LINE 13 SET TO FALLING EDGE TRIGGERED
  *EXTI_IMR|=(1U<<13);//LINE13 INTERRUPTS UNMASKED
  *NVIC_ISER1|=(1U<<8);//EXTI13 INTERRUPTS NOW HANDLES BY CPU.40-32=8
  *GPIOA_AFRL&=~(15U<<8);
  *GPIOA_AFRL|=(7U<<8);//PA2 SET TO TX
  *GPIOA_AFRL&=~(15U<<12);
  *GPIOA_AFRL|=(7U<<12);//PA3 SET TO RX
  *USART2_BRR=0X0683;//BAUD RATE TO BE 9600 AT 16MHz
  *USART2_CR1|=(1U<<2);//RX ENABLED
  *USART2_CR1|=(1U<<3);//TX ENABLED
  //*USART2_CR1|=(1U<<7);//ENABLING INTERRUPTS BY UART
  *USART2_CR1|=(1U<<13);//UART ENABLED
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  FSM_Update();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void EXTI15_10_IRQHandler(void){
	if(((*EXTI_PR>>13)&0X01)){
		button_press=1;
		*EXTI_PR|=(1U<<13);
	}
}
void FSM_Update(void){
	if(button_press){
		button_press=0;
		switch(current_state){
		case STATE_IDLE:
			USART2_SendData();
			current_state=STATE_RUNNING;
			break;

		case STATE_RUNNING:
			USART2_SendData();
			current_state=STATE_ERROR;
			break;
		case STATE_ERROR:
			USART2_SendData();
			current_state=STATE_STOP;
			break;
		case STATE_STOP:
			USART2_SendData();
			current_state=STATE_IDLE;
			break;
		}
	}
}
void USART2_SendData(void){

		if(current_state==STATE_IDLE){
			uint8_t length=ARRAY_LENGTH(STATE_ONE);

			for(uint8_t i=0;i<length;i++){
				if(STATE_ONE[STATE_INDEX]=='\0'){//CHECKING FOR NULL CHARACTER

					STATE_INDEX=0;
				}
				else{
					while(!((*USART2_SR>>7)&0X01)); // wait for TXE before EVERY byte
					*USART2_DR=STATE_ONE[STATE_INDEX];
					STATE_INDEX++;
				}
			}
		}
		else if(current_state==STATE_RUNNING){
			uint8_t length=ARRAY_LENGTH(STATE_TWO);

			for(uint8_t i=0;i<length;i++){
				if(STATE_TWO[STATE_INDEX]=='\0'){//CHECKING FOR NULL CHARACTER

					STATE_INDEX=0;
				}
				else{
					while(!((*USART2_SR>>7)&0X01)); // wait for TXE before EVERY byte
					*USART2_DR=STATE_TWO[STATE_INDEX];
					STATE_INDEX++;
				}
			}
		}
		else if(current_state==STATE_ERROR){
			uint8_t length=ARRAY_LENGTH(STATE_THREE);

			for(uint8_t i=0;i<length;i++){
				if(STATE_THREE[STATE_INDEX]=='\0'){//CHECKING FOR NULL CHARACTER

					STATE_INDEX=0;
				}
				else{
					while(!((*USART2_SR>>7)&0X01)); // wait for TXE before EVERY byte
					*USART2_DR=STATE_THREE[STATE_INDEX];
					STATE_INDEX++;
				}
			}
		}
		else if(current_state==STATE_STOP){
			uint8_t length=ARRAY_LENGTH(STATE_FOUR);

			for(uint8_t i=0;i<length;i++){
				if(STATE_FOUR[STATE_INDEX]=='\0'){//CHECKING FOR NULL CHARACTER

					STATE_INDEX=0;
				}
				else{
					while(!((*USART2_SR>>7)&0X01)); // wait for TXE before EVERY byte
					*USART2_DR=STATE_FOUR[STATE_INDEX];
					STATE_INDEX++;
				}
			}
		}



}
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
