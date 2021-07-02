/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f1xx.h"
#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// settings and variables
/////////////////////////////////////////////////////////////////////////////////////////////////////////

// easy = the same sequence, but longer and longer
// not easy = different sequences on every stage
bool easy = true;

// really_hard = different durations, 0.5 s or 1 s
bool really_hard = false;

// length of first sequence
int firstLength = 3;
			
TIM_HandleTypeDef tim2;
bool matrixButtons[16] = {false};
bool startButton;
bool play = false;
bool show = false;
bool read = false;
bool startRead = false;
bool error = false;
bool won = false;
int stage = 0;
int randomLeds[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int randomDurations[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
int repeatDurations[10];
int time = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// toggle/reset led functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void toggleMatrixLed(int n)
{
	if (n == 3)
	{
		HAL_GPIO_TogglePin(GPIOA, 1 << 0);
	}
	else if (n == 4)
	{
		HAL_GPIO_TogglePin(GPIOA, 1 << 1);
	}
	else if(n == 14)
	{
		HAL_GPIO_TogglePin(GPIOA, 1 << 11);
	}
	else if(n == 15)
	{
		HAL_GPIO_TogglePin(GPIOA, 1 << 12);
	}
	else
	{
		HAL_GPIO_TogglePin(GPIOC, 1 << n);
	}
}

void resetMatrixLeds()
{
	HAL_GPIO_WritePin(GPIOC, 1 << 0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, 1 << 1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, 1 << 2, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, 1 << 0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, 1 << 1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, 1 << 5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, 1 << 6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, 1 << 7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, 1 << 8, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, 1 << 9, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, 1 << 10, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, 1 << 11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, 1 << 12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, 1 << 13, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, 1 << 11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, 1 << 12, GPIO_PIN_RESET);
}

void toggleStageLed(int n)
{
	if(n <= 4)
	{
		HAL_GPIO_TogglePin(GPIOA, 1 << (4+n));
	}
	else if (n==5)
	{

		HAL_GPIO_TogglePin(GPIOA, 1 << 10);
	}
}

void resetStageLeds()
{
	for(int i = 4; i <= 8; i++)
	{
		HAL_GPIO_WritePin(GPIOA, 1 << i, GPIO_PIN_RESET);
	}
	HAL_GPIO_WritePin(GPIOA, 1 << 10, GPIO_PIN_RESET);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// end animations
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void loseAnimation()
{
	resetMatrixLeds();
	toggleMatrixLed(6);
	toggleMatrixLed(10);
	toggleMatrixLed(3);
	toggleMatrixLed(15);
	if(time%2)
	{
		toggleMatrixLed(0);
		toggleMatrixLed(8);
	}
	else
	{
		toggleMatrixLed(4);
		toggleMatrixLed(12);
	}
	time++;
}

void wonAnimation()
{
	resetMatrixLeds();
	for(int i = 0; i < time%17; i++)
	{
		if(i < 4)
		{
			toggleMatrixLed(i);
		}
		else if(i < 8)
		{
			toggleMatrixLed(11-i);
		}
		else if(i < 12)
		{
			toggleMatrixLed(i);
		}
		else if(i < 16)
		{
			toggleMatrixLed(27-i);
		}
	}
	time++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// scenario functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void resetButtonsFlags();

// random single i-th led that wasn't before
void randomSingleLed(int i)
{
	randomLeds[i] = rand()%15;

	// random durations if really_hard
	if(really_hard)
	{
		randomDurations[i] = rand()%2 + 1;
	}

	bool repeated = false;
	for(int j = 0; j < i; j++)
	{
		if(randomLeds[i] == randomLeds[j])
		{
			repeated = true;
			break;
		}
	}
	while(repeated)
	{
		randomLeds[i] = rand()%15;
		repeated = false;
		for(int j = 0; j < i; j++)
		{
			if(randomLeds[i] == randomLeds[j])
			{
				repeated = true;
				break;
			}
		}
	}
}

void randomSequence(int n)
{
	if(n == 0)
	{
		for(int i = 0; i < firstLength; i++)
		{
			randomSingleLed(i);
		}
	}
	// only one new step
	if(easy)
	{
		randomSingleLed(n+firstLength-1);
	}
	// all new steps
	else
	{
		for(int i = 0; i < 10; i++)
		{
			randomSingleLed(i);
		}
	}

	for(int i = 0; i < 10; i++)
	{
		repeatDurations[i] = 0;
	}
}

void firstStage()
{
	startButton = false;
	play = true;
	show = true;
	error = false;
	read = false;
	won = false;
	startRead = false;
	stage = 0;
	resetStageLeds();
	resetMatrixLeds();
	resetButtonsFlags();
	randomSequence(stage);
}

void nextStage()
{
	stage++;

	if(stage == 7)
	{
		play = false;
		won = true;
	}
	if(stage >= 1)
	{
		toggleStageLed(stage-1);
	}

	randomSequence(stage);
	resetMatrixLeds();
	read = false;
	startRead = false;
	show = true;
	resetButtonsFlags();
}

void showSequence()
{
	for(int i = 0; i < stage+firstLength; i++)
	{
		if(repeatDurations[i] == randomDurations[i])
		{
			if(i == stage+firstLength-1)
			{
				toggleMatrixLed(randomLeds[i]);
				read = true;
			}
			continue;
		}
		else
		{
			if(repeatDurations[i] == 0)
			{
				toggleMatrixLed(randomLeds[i]);
				if(i > 0)
				{
					toggleMatrixLed(randomLeds[i-1]);
				}
			}
			repeatDurations[i] += 1;
			repeatDurations[i+1] = 0;
			break;
		}
	}
	if(read)
	{
		show = false;
		startRead = false;
		for(int i = 0; i < 10; i++)
		{
			repeatDurations[i] = 0;
		}
		return;
	}
}

void readSequence()
{

	// wait for any button
	if(!startRead)
	{
		for(int i = 0; i < 16; i++)
		{
			if(matrixButtons[i] && i == randomLeds[0]) {
				startRead = true;
				repeatDurations[0] = 1;
				matrixButtons[i] = false;
				toggleMatrixLed(i);
			}
			else if(matrixButtons[i])
			{
				error = true;
				matrixButtons[i] = false;
			}

		}
	}
	else
	{
		for(int i = 0; i < stage+firstLength; i++)
		{
			if(repeatDurations[i] == randomDurations[i])
			{
				if(i == stage+firstLength-1)
				{
					nextStage();
					break;
				}
				continue;
			}
			else
			{
				if(repeatDurations[i] == 0)
				{
					// right time to press button
					bool any_pressed = false;
					for(int j = 0; j < 16; j++)
					{
						if(matrixButtons[j] && j == randomLeds[i])
						{
							// good
							toggleMatrixLed(j);
							any_pressed = true;
						}
						else if(matrixButtons[j])  {
							// wrong
							error = true;
						}
					}
					if(!any_pressed)
					{
						error = true;
					}
				}
				else
				{
					// wrong time to press button
					for(int j = 0; j < 16; j++)
					{
						if(matrixButtons[j])
						{
							error = true;
						}
					}
				}
				resetButtonsFlags();
				repeatDurations[i] += 1;
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// detect pressed buttons
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_0)
	{
		matrixButtons[0] = true;
		startButton = true;
	}
	else if(GPIO_Pin == GPIO_PIN_1)
	{
		matrixButtons[1] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_2)
	{
		matrixButtons[2] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_3)
	{
		matrixButtons[3] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_4)
	{
		matrixButtons[4] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_5)
	{
		matrixButtons[5] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_6)
	{
		matrixButtons[6] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_7)
	{
		matrixButtons[7] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_8)
	{
		matrixButtons[8] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_9)
	{
		matrixButtons[9] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_10)
	{
		matrixButtons[10] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_11)
	{
		matrixButtons[11] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_12)
	{
		matrixButtons[12] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_13)
	{
		matrixButtons[13] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_14)
	{
		matrixButtons[14] = true;
	}
	else if(GPIO_Pin == GPIO_PIN_15)
	{
		matrixButtons[15] = true;
	}
}

void resetButtonsFlags()
{
	for(int i = 0; i < 16; i++)
	{
		matrixButtons[i] = false;
	}
	startButton = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// effects of pressed buttons in every time step
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void TIM2_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&tim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(startButton && (!play || error))
	{
		firstStage();
	}
	if(play && !error)
	{
		if(show)
		{
			showSequence();
		}
		else if(read)
		{
			readSequence();
		}
	}
	if(error)
	{
		loseAnimation();
	}
	if(won)
	{
		wonAnimation();
	}
}

int main(void)
{
	SystemCoreClock = 8000000;	// 8MHz
	HAL_Init();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// timer, every 0.75 s
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	__HAL_RCC_TIM2_CLK_ENABLE();

	tim2.Instance = TIM2;
	tim2.Init.Period = 1000 - 1;
	tim2.Init.Prescaler = 6000 - 1;
	tim2.Init.ClockDivision = 0;
	tim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim2.Init.RepetitionCounter = 0;
	tim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&tim2);

	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	HAL_TIM_Base_Start_IT(&tim2);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// leds and buttons
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	//matrix leds
	GPIO_InitTypeDef gpio_matrix_leds;
	gpio_matrix_leds.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_11 | GPIO_PIN_12;
	gpio_matrix_leds.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_matrix_leds.Pull = GPIO_NOPULL;
	gpio_matrix_leds.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &gpio_matrix_leds);

	gpio_matrix_leds.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
	gpio_matrix_leds.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_matrix_leds.Pull = GPIO_NOPULL;
	gpio_matrix_leds.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &gpio_matrix_leds);

	//matrix buttons
	GPIO_InitTypeDef gpio_matrix_button;
	gpio_matrix_button.Pin = GPIO_PIN_3 | GPIO_PIN_4;
	gpio_matrix_button.Mode = GPIO_MODE_IT_RISING;
	gpio_matrix_button.Pull = GPIO_PULLUP;
	gpio_matrix_button.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &gpio_matrix_button);

	gpio_matrix_button.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	gpio_matrix_button.Mode = GPIO_MODE_IT_RISING;
	gpio_matrix_button.Pull = GPIO_PULLUP;
	gpio_matrix_button.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &gpio_matrix_button);

	//stage leds
	GPIO_InitTypeDef gpio_stage_leds;
	gpio_stage_leds.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_10;
	gpio_stage_leds.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_stage_leds.Pull = GPIO_NOPULL;
	gpio_stage_leds.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &gpio_stage_leds);

	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// start game
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	firstStage();

	while(1)
	{
	}
}
