
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

#ifndef _EMBOT_HW_PGA308_H_
#define _EMBOT_HW_PGA308_H_

#include "embot_common.h"
#include "embot_hw.h"
#include "embot_hw_onewire.h"


namespace embot { namespace hw { namespace PGA308 {
     
    
    enum class Amplifier { zero = 0, one = 1, two = 2, three = 3, four = 4, five = 5, none = 32, all = 33, maxnumberof = 6};
    
    
    struct Config
    {   // each amplifier uses a separate channel of onewire communication
        embot::hw::onewire::Channel     onewirechannel;
        embot::hw::onewire::Config      onewireconfig;
        
        Config() : onewirechannel(embot::hw::onewire::Channel::zero) {}
    };
    
    
    enum class RegisterAddress { ZDAC = 0x00, GDAC = 0x01, CFG0 = 0x02, CFG1 = 0x03, CFG2 = 0x04, SFTC = 0x07 }; 
    
    
    struct ZDACregister
    {   
        std::uint16_t value;
        static const std::uint16_t Default = 0x0000;
        
        // ZDAC Register: Zero DAC Register (Fine Offset Adjust)
        // bit:         15      14      13      12      11      10      09      08      07      06      05      04      03      02      01      00
        // content:     ZD15    ZD14    ZD13    ZD12    ZD11    ZD10    ZD9     ZD8     ZD7     ZD6     ZD5     ZD4     ZD3     ZD2     ZD1     ZD0
        // teh value x contained in the register is mapped into: +VREF/2 – (x/65536) (VREF)
        // the possible ranges of the offset is [+VREF*0.5, -VREF*0.4999847]
        
        ZDACregister() : value(0) {}
            
        ZDACregister(std::uint16_t v) : value(v) {}    
        
        void reset() { value = 0; }
        
        void set(std::uint16_t zdac) { value = zdac; }
        
        void setDefault() { value = ZDACregister::Default; }
    };    

    struct GDACregister
    {   
        std::uint16_t value;
        static const std::uint16_t Default = 0x4000;
        
        // GDAC Register: Gain DAC Register
        // bit:         15      14      13      12      11      10      09      08      07      06      05      04      03      02      01      00
        // content:     GD15    GD14    GD13    GD12    GD11    GD10    GD9     GD8     GD7     GD6     GD5     GD4     GD3     GD2     GD1     GD0
        // the value x contained in the register is mapped into: 0.333333333 + x * (1.000000000 – 0.333333333) / 65536 = 0.333333333 + x * 1.0172526 * 10–5
        // value of register x is: (DesiredGain – 0.333333333) / (1.0172526 x 10–5)
        // the range of gain is [0.333333333, 0.999989824] and GDACregister::Default is 0.499999999
        
        GDACregister() : value(0) {}
            
        GDACregister(std::uint16_t v) : value(v) {} 
        
        void reset() { value = 0; }
        
        void set(std::uint16_t gdac) { value = gdac; }
        
        void setDefault() { value = GDACregister::Default; }
    }; 
    
    struct CFG0register
    {
        std::uint16_t value;
        
        static const std::uint8_t DefaultGO = 0x06;     // output gain = 6.0
        static const std::uint8_t DefaultMUX = 0x01;    // VIN1= VINPositive, VIN2= VINNegative
        static const std::uint8_t DefaultGI = 0x04;     // input gain = 16     
        static const std::uint8_t DefaultOS = 0x00;     // coarse offset = 0 [mV]
        
        // CFG0 Output Amplifier Gain Select, Front-End PGA Mux & Gain Select, Coarse Offset Adjust on Front-End PGA
        // bit:         15      14      13      12      11      10      09      08      07      06      05      04      03      02      01      00
        // content:     GO2     GO1     GO0     GI4     GI3     GI2     GI1     GI0     OS7     OS6     OS5     OS4     OS3     OS2     OS1     OS0
        
        CFG0register() : value(0) {}
            
        CFG0register(std::uint8_t GO, std::uint8_t MUX, std::uint8_t GI, std::uint8_t OS)
        {
            value = 0;
            setGO(GO); 
            setMUX(MUX);
            setGI(GI);
            setOS(OS);
        }
        
        CFG0register(std::uint16_t v) : value(v) {} 
        
        void reset() { value = 0; }
        
        void setDefault() { value = 0; setGO(CFG0register::DefaultGO); setMUX(CFG0register::DefaultMUX); setGI(CFG0register::DefaultGI); setOS(CFG0register::DefaultOS); }
        
        // Output Amplifier Gain Select: [GO2-GO0] -> from 000b to 111b: 2.0, 2.4, 3, 3.6, 4.0, 4.5, 6.0 disable-internal-feedback
        void setGO(std::uint8_t GO) { value |= (static_cast<std::uint8_t>(GO&0x07) << 13); }
        std::uint8_t getGO() const { return ((value >> 13) & 0x07); }
        
        // Front-End PGA Mux Select: [GI4] -> 0 is [VIN1= VINN, VIN2= VINP], 1 is [VIN1= VINP, VIN2= VINN]
        void setMUX(std::uint8_t MUX) { value |= (static_cast<std::uint8_t>(MUX&0x01) << 12); }
        std::uint8_t getMUX() const { return ((value >> 12) & 0x01); }
        
        // Front-End PGA Gain Select: [GI3-GI0] -> from 0000b to 1101b: 4 6 8 12 16 32 64 100 200 400 480 600 800 960 1200 1600
        void setGI(std::uint8_t GI) { value |= (static_cast<std::uint8_t>(GI&0x0f) << 8); }
        std::uint8_t getGI() const { return ((value >> 8) & 0x0f); }      

        // Coarse Offset Adjust on Front-End PGA: [OS7-OS0] where OS7 is sign (0 is +) and [OS6-OS0] is value. valid range is only x = [-100, +100]
        // which maps into (x/128)(Vref)(0.0256) for a global offset range of [-0.02*V_REF, +0.02*V_REF]. V_REF is in register CFG2.COSVR[1:0]
        void setOS(std::uint8_t OS) { value |= (static_cast<std::uint8_t>(OS&0xff)); }
        std::uint8_t getOS() const { return ((value) & 0xff); }    

    }; 


    struct CFG1register
    {
        std::uint16_t value;        
        static const std::uint16_t Default = 0x0000;
        
        // CFG1 Register: Configuration Register 1
        // bit:         15      14      13      12      11      10      09      08      07      06      05      04      03      02      01      00
        // content:     FLT-REF FLT-IPU OU-CFG  FLT-SEL CMP-SEL EXT-EN  INT-EN  EXT-POL INT-POL OU-EN   HL2     HL1     HL0     LL2     LL1     LL0
        
        CFG1register() { value = 0; }
        
        CFG1register(std::uint16_t v) : value(v) {} 
        
        void reset() { value = 0; }
        
        void set(std::uint16_t cfg1) { value = cfg1; }
        
        void setDefault() { value = CFG1register::Default; }
    };    

    struct CFG2register
    {
        std::uint16_t value;        
        static const std::uint16_t Default = 0x0000;
        
        // CFG2 Register: Configuration Register 2
        // bit:         15      14      13      12      11      10      09      08      07      06      05      04      03      02      01      00
        // content:     OWD     OWD-OFF DIS-OUT NOW     COSVR1  COSVR0  RESERVD DOUTSEL DOUT    SD      RESERVD RESERVD RESERVD RESERVD RESERVD RESERVD
        // COSVR:       00 -> ; 01-> ; 10 -> ; 11 -> ;
        CFG2register() { value = 0; }
        
        CFG2register(std::uint16_t v) : value(v) {} 
        
        void reset() { value = 0; }
        
        void set(std::uint16_t cfg2) { value = cfg2; }
        
        void setDefault() { value = CFG1register::Default; }
    };  
    
    
    struct SFTCregister
    {   
        std::uint16_t value;
        static const std::uint16_t Default = 0x0050;    // where ... there is the SOFTWARE LOCK MODE (Runs from RAM) and the PGA308 operates from data written into the RAM registers from the user
        
        // SFTC Register: Software Control
        // bit:         15      14      13      12      11      10      09      08      07      06      05      04      03      02      01      00
        // content:     RESERVD RESERVD RESERVD RESERVD RESERVD RESERVD RESERVD CHKSFLG OW-DLY  SW-L2   SW-L1   SW-L0   RFB0    XP5     XP4     XP3
        // brief description:
        // CHKSFLG: Register Checksum Bit (Read-only)               1 = register checksum correct
        // OW-DLY:  One-Wire Delay Bit                              1 = 8-bit delay from transmit to receive during One-Wire reads, 0 = 1-bit delay from transmit to receive during One-Wire reads
        // SWL[2:0] Software Lock Mode Control                      101b = SOFTWARE LOCK (Runs from RAM), etc....
        // XP[5:3] OTP Bank Selection for Software Lock Mode        If XP[5:3] = '000', then the PGA308 operates from data written into the RAM registers from the user.
        SFTCregister() : value(0) {}
            
        SFTCregister(std::uint16_t v) : value(v) {} 
        
        void reset() { value = 0; }
        
        void set(std::uint16_t sftc) { value = sftc; }
        
        void setDefault() { value = SFTCregister::Default; }
    };    
    
            
    
    struct TransferFunctionConfig
    { 
        std::uint16_t       GD;                     // gain DAC. values are [0.333333333, 0.999989824]                     
        std::uint8_t        GI          : 4;        // front end gain. from 0000b to 1101b: {4 6 8 12 16 32 64 100 200 400 480 600 800 960 1200 1600}
        std::uint8_t        muxsign     : 1;        // the sign: 0 is +, 1 is -
        std::uint8_t        GO          : 3;        // output gain. from 000b to 111b: {2.0, 2.4, 3, 3.6, 4.0, 4.5, 6.0, disable-internal-feedback}
        std::uint8_t        Vcoarseoffset;
        std::uint16_t       Vzerodac;
        
        
        enum class Parameter { GD = 0, GI = 1, muxsign = 2, GO = 3, Vcoarseoffset = 4, Vzerodac = 5 };
        
        // the formula is:
        // Vout = ((muxsign*Vin + Vcoarseoffset)*GI + Vzerodac)*GD*GO
        // Vout = g * Vin + o
        // g = muxsign*GI*GD*GO 
        // o = (Vcoarseoffset*GI + Vzerodac)*GD*GO
        
        TransferFunctionConfig() : GD(0), GI(0), muxsign(0), GO(0), Vcoarseoffset(0), Vzerodac(0) {}
        
        void setDefault() 
        {
            GD = GDACregister::Default;            
            GI = CFG0register::DefaultGI; 
            muxsign = CFG0register::DefaultMUX;
            GO = CFG0register::DefaultGO; 
            Vcoarseoffset = CFG0register::DefaultOS; 
            Vzerodac = ZDACregister::Default; 
        }
        
        // from internal memory to the values of the registers to be written into the amplifier
        void obtain(CFG0register &cfg0, ZDACregister &zdac, GDACregister &gdac) const
        {
            cfg0.reset(); cfg0.setGO(GO); cfg0.setMUX(muxsign); cfg0.setGI(GI); cfg0.setOS(Vcoarseoffset);
            zdac.reset(); zdac.value = Vzerodac;
            gdac.reset(); gdac.value = GD;            
        }
        
        // from the registers read from the amplifier to internal memory.
        void assign(const CFG0register &cfg0, const ZDACregister &zdac, const GDACregister &gdac)
        {
            GO = cfg0.getGO();
            muxsign = cfg0.getMUX();
            GI = cfg0.getGI();
            Vcoarseoffset = cfg0.getOS();
            Vzerodac = zdac.value;
            GD = gdac.value;
        }
        
        void load(const CFG0register &cfg0)
        {
            GO = cfg0.getGO();
            muxsign = cfg0.getMUX();
            GI = cfg0.getGI();
            Vcoarseoffset = cfg0.getOS();            
        }
        
        void load(const ZDACregister &zdac)
        {
            Vzerodac = zdac.value;            
        }
        
        void load(const GDACregister &gdac)
        {
            GD = gdac.value;           
        }
    };  
    
    
    
    
    bool supported(Amplifier a);
    
    bool initialised(Amplifier a);
    
    // inits onewire, set default value of registers 
    result_t init(Amplifier a, const Config &config);

    // loads a transfer function (maybe obtained over can bus from the canloader)   
    result_t set(Amplifier a, const TransferFunctionConfig &tfconfig);
    
    // loads only one parameter of the transfer function. the selection of the proper register and formation of value is done inetrnally
    result_t set(Amplifier a, const TransferFunctionConfig::Parameter par, const std::uint16_t value);
    
    // retrieve the used transfer function (maybe to be sent over can bus to teh canloader)
    result_t get(Amplifier a, TransferFunctionConfig &tfconfig);
    
    // sets the register inside the amplifier. for value use GDACregister.value or similar 
    result_t set(Amplifier a, const RegisterAddress address, const std::uint16_t value);
    
    // retrieves the value of the specified register ... or rather, its buffered value ..
    result_t get(Amplifier a, const RegisterAddress address, std::uint16_t &value);

    
 
}}} // namespace embot { namespace hw { namespace PGA308 {
    


#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------


