/*
 * Copyright (C) 2017 iCub Facility - Istituto Italiano di Tecnologia
 * Author:  Marco Accame
 * email:   marco.accame@iit.it
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


// - include guard ----------------------------------------------------------------------------------------------------
#ifndef _STM32HAL_BSP_H_
#define _STM32HAL_BSP_H_

#ifdef __cplusplus
extern "C" {
#endif


// - external dependencies --------------------------------------------------------------------------------------------  
    
#include "stm32hal_define.h"
#include "stm32hal_driver.h"
    
// - public #define  --------------------------------------------------------------------------------------------------
// empty-section
  

// - declaration of public user-defined types -------------------------------------------------------------------------    
// empty-section
    
    
// - declaration of extern public functions ---------------------------------------------------------------------------
 
// called by stm32hal_init() if stm32hal_config_t::initbsp is true
    
extern void stm32hal_bsp_init(void);    
    
    
// - public interface: begin  -----------------------------------------------------------------------------------------  
// it contains whatever cube-mx generates.


#if 		defined(STM32HAL_BOARD_NUCLEO64)

// this is taken from what cube-mx generates 
#include "../src/board/nucleo64/inc/stm32l4xx_hal_conf_nucleo64.h"

#include "../src/board/nucleo64/inc/gpio.h"
#include "../src/board/nucleo64/inc/main.h"
#include "../src/board/nucleo64/inc/usart.h"
	
	
#elif 	defined(STM32HAL_BOARD_MTB4)	

// this is taken from what cube-mx generates 
#include "../src/board/mtb4/inc/stm32l4xx_hal_conf_mtb4.h"

#include "../src/board/mtb4/inc/adc.h"
#include "../src/board/mtb4/inc/can.h"
#include "../src/board/mtb4/inc/dma.h"
#include "../src/board/mtb4/inc/gpio.h"
#include "../src/board/mtb4/inc/i2c.h"
#include "../src/board/mtb4/inc/rng.h"
#include "../src/board/mtb4/inc/main.h"
#include "../src/board/mtb4/inc/tim.h"
#include "../src/board/mtb4/inc/usart.h"


#include "../src/board/mtb4/inc/stm32l4xx_it.h"



#else
    #error STM32HAL: you must define a STM32HAL_BOARD_something
#endif


// - public interface: end --------------------------------------------------------------------------------------------


#ifdef __cplusplus
}       // closing brace for extern "C"
#endif 


#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------
