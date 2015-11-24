/*
 * Copyright (C) 2012 iCub Facility - Istituto Italiano di Tecnologia
 * Author: Marco Maggiali
 * email:   marco.maggiali@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/


/* @file       hal_quad_enc.c
	@brief      This file implements the quadrature encoder interface.
	@author     marco.maggiali@iit.it, 
    @date       26/03/2013
**/

// - modules to be built: contains the HAL_USE_* macros ---------------------------------------------------------------
#include "hal_brdcfg_modules.h"

// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "stdlib.h"
#include "string.h"
#include "hal_gpio.h"
#include "hal_brdcfg.h"
#include "hl_core.h" 

// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "hal_quad_enc.h"

// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------

#define TIMx_PRE_EMPTION_PRIORITY 2
#define TIMx_SUB_PRIORITY 0
#define ICx_FILTER          (u8) 8 // 8<-> 670nsec


//#define ENCODER_PPR 28672-1// 14400-1 //(for LCORE with 900cpr disk and x4 interpolation)
#define ENCODER_PPR (256*1000)
#define ENCODER_START_VAL (32*1000)
#define ENCODER1_TIMER TIM3
#define ENCODER2_TIMER TIM2
#define ENCODER3_TIMER TIM4
#define ENCODER4_TIMER TIM5


// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------

static void s_hal_quad_enc_set_index_found(uint8_t encoder_number);

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------

static hal_bool_t index_found[4] = {hal_false,hal_false,hal_false,hal_false};

// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------

extern void hal_quad_enc_Init(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_ICInitTypeDef TIM_ICInitStructure;
  EXTI_InitTypeDef   EXTI_InitStructure;  
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  
//  // Encoder 1 unit connected to TIM2, 4X mode  
//  // ENCODER 1
{
	/* Configure 
							 PA0 as PHA 
							 PB3 as PHB 
							 PG12  as INDEX  */
	
  /* TIM2 clock source enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  
	
	/* Enable GPIOA and GPIOB, clock */
	
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE);
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB , ENABLE);

  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
	
  /* Connect TIM pins to AF1 */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2);	
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_TIM2);	
	
  /* Timer configuration in Encoder mode */
  TIM_DeInit(ENCODER1_TIMER);
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  
  TIM_TimeBaseStructure.TIM_Prescaler = 0x0;  // No prescaling 
  TIM_TimeBaseStructure.TIM_Period = ENCODER_PPR;  
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
  TIM_TimeBaseInit(ENCODER1_TIMER, &TIM_TimeBaseStructure);
 
  TIM_EncoderInterfaceConfig(ENCODER1_TIMER, TIM_EncoderMode_TI12, 
                             TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
  TIM_ICStructInit(&TIM_ICInitStructure);
  
  TIM_ICInitStructure.TIM_ICFilter = ICx_FILTER;
  TIM_ICInit(ENCODER1_TIMER, &TIM_ICInitStructure);
  
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInit(ENCODER1_TIMER, &TIM_ICInitStructure);
      
 // Clear all pending interrupts
 // TIM_ClearFlag(ENCODER1_TIMER, TIM_FLAG_Update);
 // TIM_ITConfig(ENCODER2_TIMER, TIM_IT_Update, ENABLE);
  //Reset counter
  ENCODER1_TIMER->CNT = ENCODER_START_VAL;
  
  TIM_Cmd(ENCODER1_TIMER, ENABLE);

}
  // Encoder 2 unit connected to TIM3, 4X mode  
  // ENCODER 2
{  
  /* TIM2 clock source enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  /* Enable GPIOC, clock */
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC , ENABLE);
  
  GPIO_StructInit(&GPIO_InitStructure);
  /* Configure PC.06,07 as encoder input */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  /* Connect TIM pins to AF2 */
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM3);	
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM3);	
#warning -> removed the TIM3 interrupt enable	
	
  /* Enable the TIM3 Update Interrupt */
//  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIMx_PRE_EMPTION_PRIORITY;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIMx_SUB_PRIORITY;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);

  /* Timer configuration in Encoder mode */
  TIM_DeInit(ENCODER2_TIMER);
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  
  TIM_TimeBaseStructure.TIM_Prescaler = 0x0;  // No prescaling 
  TIM_TimeBaseStructure.TIM_Period = ENCODER_PPR;  
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
  TIM_TimeBaseInit(ENCODER2_TIMER, &TIM_TimeBaseStructure);
 
  TIM_EncoderInterfaceConfig(ENCODER2_TIMER, TIM_EncoderMode_TI12, 
                             TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
  TIM_ICStructInit(&TIM_ICInitStructure);
  
  TIM_ICInitStructure.TIM_ICFilter = ICx_FILTER;
  TIM_ICInit(ENCODER2_TIMER, &TIM_ICInitStructure);
  
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInit(ENCODER2_TIMER, &TIM_ICInitStructure);
      
 // Clear all pending interrupts
  TIM_ClearFlag(ENCODER2_TIMER, TIM_FLAG_Update);
  TIM_ITConfig(ENCODER2_TIMER, TIM_IT_Update, ENABLE);
  //Reset counter
  ENCODER2_TIMER->CNT = ENCODER_START_VAL;
  
  TIM_Cmd(ENCODER2_TIMER, ENABLE);
}
  // Encoder 3 unit connected to TIM4, 4X mode  
  // ENCODER 3 
{
  /* TIM4 clock source enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  /* Enable GPIOD, clock */
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD , ENABLE);
  
  GPIO_StructInit(&GPIO_InitStructure);
  /* Configure PD.12,13 as encoder input */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_13  ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  /* Connect TIM pins to AF2 */
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);	
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);	

  /* Timer configuration in Encoder mode */
  TIM_DeInit(ENCODER3_TIMER);
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  
  TIM_TimeBaseStructure.TIM_Prescaler = 0x0;  // No prescaling 
  TIM_TimeBaseStructure.TIM_Period = ENCODER_PPR;  
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
  TIM_TimeBaseInit(ENCODER3_TIMER, &TIM_TimeBaseStructure);
 
  TIM_EncoderInterfaceConfig(ENCODER3_TIMER, TIM_EncoderMode_TI12, 
                             TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
  TIM_ICStructInit(&TIM_ICInitStructure);
  
  TIM_ICInitStructure.TIM_ICFilter = ICx_FILTER;
  TIM_ICInit(ENCODER3_TIMER, &TIM_ICInitStructure);
  
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInit(ENCODER3_TIMER, &TIM_ICInitStructure);
      
 // Clear all pending interrupts
 // TIM_ClearFlag(ENCODER3_TIMER, TIM_FLAG_Update);
 // TIM_ITConfig(ENCODER3_TIMER, TIM_IT_Update, ENABLE);
  //Reset counter
  ENCODER3_TIMER->CNT = ENCODER_START_VAL;
  
  TIM_Cmd(ENCODER3_TIMER, ENABLE);
}

  // Encoder 4 unit connected to TIM5, 4X mode  
  // ENCODER 4
{
  /* TIM5 clock source enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  /* Enable GPIOH, clock */
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOH , ENABLE);
  
  GPIO_StructInit(&GPIO_InitStructure);
  /* Configure PH.10,11 as encoder input */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11  ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOH, &GPIO_InitStructure);
  
  /* Connect TIM pins to AF2 */
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource10, GPIO_AF_TIM5);	
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource11, GPIO_AF_TIM5);	
	
  /* Timer configuration in Encoder mode */
  TIM_DeInit(ENCODER4_TIMER);
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  
  TIM_TimeBaseStructure.TIM_Prescaler = 0x0;  // No prescaling 
  TIM_TimeBaseStructure.TIM_Period = ENCODER_PPR;  
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
  TIM_TimeBaseInit(ENCODER4_TIMER, &TIM_TimeBaseStructure);
 
  TIM_EncoderInterfaceConfig(ENCODER4_TIMER, TIM_EncoderMode_TI12, 
                             TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
  TIM_ICStructInit(&TIM_ICInitStructure);
  
  TIM_ICInitStructure.TIM_ICFilter = ICx_FILTER;
  TIM_ICInit(ENCODER4_TIMER, &TIM_ICInitStructure);
  
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInit(ENCODER4_TIMER, &TIM_ICInitStructure);
 // Clear all pending interrupts
 // TIM_ClearFlag(ENCODER4_TIMER, TIM_FLAG_Update);
 // TIM_ITConfig(ENCODER4_TIMER, TIM_IT_Update, ENABLE);
  //Reset counter
  ENCODER4_TIMER->CNT = ENCODER_START_VAL;
  
  TIM_Cmd(ENCODER4_TIMER, ENABLE);
}	
    // Indexes  //
{
//	/* Enable GPIOG clock */
//  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOG , ENABLE); 
//	/* Enable SYSCFG clock */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
//	
//  GPIO_StructInit(&GPIO_InitStructure);
//  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_13 |GPIO_Pin_14 |GPIO_Pin_15;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//  GPIO_Init(GPIOG, &GPIO_InitStructure);
//	
//	/* Connect EXTI Line 12 to PAG12 pin */
//  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource12);
//  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource13);
//  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource14);
//  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource15);
//	  /* Configure EXTI Line0 */
//  EXTI_InitStructure.EXTI_Line = EXTI_Line12|EXTI_Line13|EXTI_Line14|EXTI_Line15;
//  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
//  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//  EXTI_Init(&EXTI_InitStructure);

//  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
//  NVIC_InitStructure.NVIC_IRQChannel =  EXTI15_10_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
//	
}	

}
extern uint32_t hal_quad_enc_getCounter(uint8_t encoder_number)
{
  uint32_t temp;
  switch (encoder_number)
  {	
  case 0:
  {
        temp = TIM_GetCounter(ENCODER1_TIMER);
  }
  break;
  case 1:
  {
        temp = TIM_GetCounter(ENCODER2_TIMER);      
  }
  break;
  case 2:
  {
        temp = TIM_GetCounter(ENCODER3_TIMER);  
  }
  break;
  case 3:
  {
        temp = TIM_GetCounter(ENCODER4_TIMER);  
  }
  break;
  default:
  {
  return 0;  
  }
  }         
  return temp; 
}


extern void hal_quad_enc_single_init (uint8_t encoder_number)
{
#if 0
    static uint8_t initted = 0;
    
    if(1 == initted)
    {
        return;
    }
    
    hal_quad_enc_Init();
    
    initted = 1;
    
#else    
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    switch (encoder_number)
    {
        case 0:
        {
            // Encoder 1 unit connected to TIM3, 4X mode  
            // ENCODER 1 
            /* TIM3 clock source enable */
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
            /* Enable GPIOC, clock */
            RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC , ENABLE);
              
            GPIO_StructInit(&GPIO_InitStructure);
            /* Configure PC.06,07 as encoder input */
            GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
              
            /* Connect TIM pins to AF2 */
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM3);	
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM3);	

            //#warning -> removed the TIM3 interrupt enable		
            /* Enable the TIM3 Update Interrupt */
            //  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
            //  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIMx_PRE_EMPTION_PRIORITY;
            //  NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIMx_SUB_PRIORITY;
            //  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            //  NVIC_Init(&NVIC_InitStructure);

            /* Timer configuration in Encoder mode */
            TIM_DeInit(ENCODER1_TIMER);
            TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
              
            TIM_TimeBaseStructure.TIM_Prescaler = 0x0;  // No prescaling 
            TIM_TimeBaseStructure.TIM_Period = ENCODER_PPR;  
            TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
            TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
            TIM_TimeBaseInit(ENCODER1_TIMER, &TIM_TimeBaseStructure);
             
            TIM_EncoderInterfaceConfig(ENCODER1_TIMER, TIM_EncoderMode_TI12, 
                                         TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
            TIM_ICStructInit(&TIM_ICInitStructure);
              
            TIM_ICInitStructure.TIM_ICFilter = ICx_FILTER;
            TIM_ICInit(ENCODER1_TIMER, &TIM_ICInitStructure);
              
            TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
            TIM_ICInit(ENCODER1_TIMER, &TIM_ICInitStructure);
                  
            // Clear all pending interrupts
            // TIM_ClearFlag(ENCODER1_TIMER, TIM_FLAG_Update);
            // TIM_ITConfig(ENCODER2_TIMER, TIM_IT_Update, ENABLE);
            //Reset counter
            ENCODER1_TIMER->CNT = ENCODER_START_VAL;
              
            TIM_Cmd(ENCODER1_TIMER, ENABLE);
            break;
        }
        case 1:
        {
            // Encoder 2 unit connected to TIM2, 4X mode  
            // ENCODER 2
            /* Configure 
                                     PA0 as PHA 
                                     PB3 as PHB 
                                     PG12  as INDEX  */
            
            /* TIM2 clock source enable */
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
          
            
            /* Enable GPIOA and GPIOB, clock */
            
            RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE);
            RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB , ENABLE);

            GPIO_StructInit(&GPIO_InitStructure);
            GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            GPIO_Init(GPIOA, &GPIO_InitStructure);

            GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            GPIO_Init(GPIOB, &GPIO_InitStructure);  
                
            /* Connect TIM pins to AF1 */
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2);	
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_TIM2);	
                
            /* Timer configuration in Encoder mode */
            TIM_DeInit(ENCODER2_TIMER);
            TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
              
            TIM_TimeBaseStructure.TIM_Prescaler = 0x0;  // No prescaling 
            TIM_TimeBaseStructure.TIM_Period = ENCODER_PPR;  
            TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
            TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
            TIM_TimeBaseInit(ENCODER2_TIMER, &TIM_TimeBaseStructure);
             
            TIM_EncoderInterfaceConfig(ENCODER2_TIMER, TIM_EncoderMode_TI12, 
                                         TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
            TIM_ICStructInit(&TIM_ICInitStructure);
              
            TIM_ICInitStructure.TIM_ICFilter = ICx_FILTER;
            TIM_ICInit(ENCODER2_TIMER, &TIM_ICInitStructure);
              
            TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
            TIM_ICInit(ENCODER2_TIMER, &TIM_ICInitStructure);
                  
            // Clear all pending interrupts
            // TIM_ClearFlag(ENCODER2_TIMER, TIM_FLAG_Update);
            // TIM_ITConfig(ENCODER2_TIMER, TIM_IT_Update, ENABLE);
            //Reset counter
            ENCODER2_TIMER->CNT = ENCODER_START_VAL;
              
            TIM_Cmd(ENCODER2_TIMER, ENABLE);
            break;
        }
        case 2:
        {            
            // Encoder 3 unit connected to TIM4, 4X mode  
            // ENCODER 3 
            /* TIM4 clock source enable */
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
            /* Enable GPIOD, clock */
            RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD , ENABLE);
              
            GPIO_StructInit(&GPIO_InitStructure);
            /* Configure PD.12,13 as encoder input */
            GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_13  ;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
              
            /* Connect TIM pins to AF2 */
            GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);	
            GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);	

            /* Timer configuration in Encoder mode */
            TIM_DeInit(ENCODER3_TIMER);
            TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
              
            TIM_TimeBaseStructure.TIM_Prescaler = 0x0;  // No prescaling 
            TIM_TimeBaseStructure.TIM_Period = ENCODER_PPR;  
            TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
            TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
            TIM_TimeBaseInit(ENCODER3_TIMER, &TIM_TimeBaseStructure);
             
            TIM_EncoderInterfaceConfig(ENCODER3_TIMER, TIM_EncoderMode_TI12, 
                                         TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
            TIM_ICStructInit(&TIM_ICInitStructure);
              
            TIM_ICInitStructure.TIM_ICFilter = ICx_FILTER;
            TIM_ICInit(ENCODER3_TIMER, &TIM_ICInitStructure);
              
            TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
            TIM_ICInit(ENCODER3_TIMER, &TIM_ICInitStructure);
                  
             // Clear all pending interrupts
             // TIM_ClearFlag(ENCODER3_TIMER, TIM_FLAG_Update);
             // TIM_ITConfig(ENCODER3_TIMER, TIM_IT_Update, ENABLE);
             //Reset counter
             ENCODER3_TIMER->CNT = ENCODER_START_VAL;
              
             TIM_Cmd(ENCODER3_TIMER, ENABLE);
             break;
        }
        case 3:
        {
             // Encoder 4 unit connected to TIM5, 4X mode  
             // ENCODER 4
             /* TIM5 clock source enable */
             RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
             /* Enable GPIOH, clock */
             RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOH , ENABLE);
              
             GPIO_StructInit(&GPIO_InitStructure);
             /* Configure PH.10,11 as encoder input */
             GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11  ;
             GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
             GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
             GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
             GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
             GPIO_Init(GPIOH, &GPIO_InitStructure);
              
             /* Connect TIM pins to AF2 */
             GPIO_PinAFConfig(GPIOH, GPIO_PinSource10, GPIO_AF_TIM5);	
             GPIO_PinAFConfig(GPIOH, GPIO_PinSource11, GPIO_AF_TIM5);	
                
             /* Timer configuration in Encoder mode */
             TIM_DeInit(ENCODER4_TIMER);
             TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
              
             TIM_TimeBaseStructure.TIM_Prescaler = 0x0;  // No prescaling 
             TIM_TimeBaseStructure.TIM_Period = ENCODER_PPR;  
             TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
             TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
             TIM_TimeBaseInit(ENCODER4_TIMER, &TIM_TimeBaseStructure);
             
             TIM_EncoderInterfaceConfig(ENCODER4_TIMER, TIM_EncoderMode_TI12, 
                                         TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
             TIM_ICStructInit(&TIM_ICInitStructure);
              
             TIM_ICInitStructure.TIM_ICFilter = ICx_FILTER;
             TIM_ICInit(ENCODER4_TIMER, &TIM_ICInitStructure);
              
             TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
             TIM_ICInit(ENCODER4_TIMER, &TIM_ICInitStructure);
             // Clear all pending interrupts
             // TIM_ClearFlag(ENCODER4_TIMER, TIM_FLAG_Update);
             // TIM_ITConfig(ENCODER4_TIMER, TIM_IT_Update, ENABLE);
             //Reset counter
             ENCODER4_TIMER->CNT = ENCODER_START_VAL;
              
             TIM_Cmd(ENCODER4_TIMER, ENABLE);
             break;
         }
        default:
             return;
    }
#endif    
}

extern void hal_quad_enc_init_indexes_flags(void)
{
  // Indexes  //
  /* Enable GPIOG clock */
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOG , ENABLE); 
  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  // Init GPIO associated to quad_enc indexes
  GPIO_InitTypeDef GPIO_InitStructure;	
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_13 |GPIO_Pin_14 |GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
	
  /* Connect EXTI Line 12 to PAG12 pin */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource12);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource13);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource14);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource15);
    
  /* Configure EXTI Line0 */
  EXTI_InitTypeDef   EXTI_InitStructure;
  EXTI_InitStructure.EXTI_Line = EXTI_Line12|EXTI_Line13|EXTI_Line14|EXTI_Line15;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel =  EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

extern hal_bool_t hal_quad_is_index_found(uint8_t encoder_number)
{
    if (encoder_number > 3)
        return hal_false;
    
    if (index_found[encoder_number] == hal_true)
    {
        //reset flag
        index_found[encoder_number] = hal_false;
        return hal_true;
    }
    else
    {
        return hal_false;
    }
}

extern void hal_quad_enc_reset_counter(uint8_t encoder_number)
{
    switch (encoder_number)
    {
        case 0:
            TIM_SetCounter(ENCODER1_TIMER,0x0);
            break;
        case 1:
            TIM_SetCounter(ENCODER2_TIMER,0x0);
            break;
        case 2:
            TIM_SetCounter(ENCODER3_TIMER,0x0);
            break;
        case 3:
            TIM_SetCounter(ENCODER4_TIMER,0x0);
            break;
        default:
            break;
    }
}

/**
  * @brief  This function handles External lines 15 to 10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
    //index on quad enc 0
    if(EXTI_GetITStatus(EXTI_Line13) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line13);
        s_hal_quad_enc_set_index_found(0);
    }
    //index on quad enc 1
    if(EXTI_GetITStatus(EXTI_Line12) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line12);
        s_hal_quad_enc_set_index_found(1);
    }
	//index on quad enc 2
    if(EXTI_GetITStatus(EXTI_Line14) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line14);
        s_hal_quad_enc_set_index_found(2);
    }
    //index on quad enc 3
    if(EXTI_GetITStatus(EXTI_Line15) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line15);
        s_hal_quad_enc_set_index_found(3);
    }
}

// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------

static void s_hal_quad_enc_set_index_found(uint8_t encoder_number)
{
    if (encoder_number > 3)
        return;
    index_found[encoder_number] = hal_true;
}

