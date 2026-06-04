#include "encoder.h"

volatile int8_t  encoder_delta = 0;
volatile uint8_t key_event     = 0;

static uint32_t press_duration = 0;
static uint8_t  key_pressed    = 0;

void Encoder_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    EXTI_InitStructure.EXTI_Line    = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitTypeDef  NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    SysTick_Config(SystemCoreClock / 1000);

    encoder_delta  = 0;
    key_event      = 0;
    press_duration = 0;
    key_pressed    = 0;
}

void Key_Scan_10ms(void)
{
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == Bit_RESET)
    {
        if (key_pressed == 0)
        {
            key_pressed    = 1;
            press_duration = 0;
        }
        else
        {
            press_duration += 10;
            if (press_duration >= 2000 && key_pressed == 1)
            {
                key_event  = 2;
                key_pressed = 2;
            }
        }
    }
    else
    {
        if (key_pressed == 1)
        {
            if (press_duration >= 50 && press_duration < 500)
            {
                key_event = 1;
            }
        }
        key_pressed    = 0;
        press_duration = 0;
    }
}
