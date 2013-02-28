/*
 * Copyright (C) 2012 iCub Facility - Istituto Italiano di Tecnologia
 * Author:  Valentina Gaggero, Marco Accame
 * email:   valentina.gaggero@iit.it, marco.accame@iit.it
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
#ifndef _HAL_BASE_HID_H_
#define _HAL_BASE_HID_H_


/* @file       hal_base_hid.h
    @brief      This header file implements hidden interface to hal.
    @author     marco.accame@iit.it
    @date       09/12/2011
 **/


// - external dependencies --------------------------------------------------------------------------------------------



// - declaration of extern public interface ---------------------------------------------------------------------------
 
#include "hal_base.h"


// - #define used with hidden structs ---------------------------------------------------------------------------------
// empty-section

// - definition of hidden structs -------------------------------------------------------------------------------------

typedef struct
{
    uint8_t     dummy;
} hal_base_hid_brdcfg_t;

// - declaration of extern hidden variables ---------------------------------------------------------------------------
// empty-section


// - declaration of extern hidden functions ---------------------------------------------------------------------------


extern hal_result_t hal_base_hid_static_memory_init(void);

extern hal_bool_t hal_base_hid_initted_is(void);



/** @fn         extern uint32_t hal_base_memory_getsize(const hal_base_cfg_t *cfg, uint32_t *size04aligned)
    @brief      Gets the size of the 4-aligned memory required by the hal in order to work according to a given
                configuration. 
    @param      cfg             The target configuration. 
    @param      size04aligned   The number of bytes of teh 4-aligned RAM memory which is required. (if not NULL)
    @return     The number of bytes of the 4bytes-aligned RAM which is required

 **/
extern uint32_t hal_base_hid_memory_getsize(const hal_base_cfg_t *cfg, uint32_t *size04aligned);


/** @fn         extern hal_result_t hal_base_initialise(const hal_base_cfg_t *cfg, uint32_t *data04aligned)
    @brief      Initialise the hal to work for a given configuration and with a given external memory.
    @param      cfg             The target configuration. 
    @param      data04aligned   The 4bytes-aligned RAM which is required, or NULL if none is required.
    @return     hal_res_OK or hal_res_NOK_generic 
 **/
extern hal_result_t hal_base_hid_initialise(const hal_base_cfg_t *cfg, uint32_t *data04aligned); 


// - definition of extern hidden inline functions ---------------------------------------------------------------------
// empty-section


#endif  // include guard

// - end-of-file (leave a blank line after)----------------------------------------------------------------------------




