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


// --------------------------------------------------------------------------------------------------------------------
// - public interface
// --------------------------------------------------------------------------------------------------------------------

#include "embot_app_canprotocol.h"




// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "embot.h"

#include <cstring>

#include "embot_hw.h"



// --------------------------------------------------------------------------------------------------------------------
// - all the rest
// --------------------------------------------------------------------------------------------------------------------

namespace embot { namespace app { namespace canprotocol {
    
    
    bldrCMD cmd2bldr(std::uint8_t cmd)
    {
        if(cmd <= static_cast<std::uint8_t>(bldrCMD::END))
        {
            return static_cast<bldrCMD>(cmd);
        }
        else if(cmd == static_cast<std::uint8_t>(bldrCMD::BROADCAST))
        {
            return bldrCMD::BROADCAST;
        } 
        else if(cmd == static_cast<std::uint8_t>(bldrCMD::GET_ADDITIONAL_INFO))
        {
            return bldrCMD::GET_ADDITIONAL_INFO;
        }
        else if(cmd == static_cast<std::uint8_t>(bldrCMD::SET_ADDITIONAL_INFO))
        {
            return bldrCMD::SET_ADDITIONAL_INFO;
        }        
        else if(cmd == static_cast<std::uint8_t>(bldrCMD::SETCANADDRESS))
        {
            return bldrCMD::SETCANADDRESS;
        }
        return bldrCMD::none;  
    }
    
    aspollCMD cmd2aspoll(std::uint8_t cmd)
    {
        if(cmd == static_cast<std::uint8_t>(aspollCMD::SET_BOARD_ADX))
        {
            return aspollCMD::SET_BOARD_ADX;
        }         
        else if(cmd == static_cast<std::uint8_t>(aspollCMD::GET_FIRMWARE_VERSION))
        {
            return aspollCMD::GET_FIRMWARE_VERSION;
        }
        return aspollCMD::none; 
    }
    
    mcpollCMD cmd2mcpoll(std::uint8_t cmd)
    {
        if(cmd == static_cast<std::uint8_t>(mcpollCMD::SET_BOARD_ID))
        {
            return mcpollCMD::SET_BOARD_ID;
        }
        else if(cmd == static_cast<std::uint8_t>(mcpollCMD::GET_FIRMWARE_VERSION))
        {
            return mcpollCMD::GET_FIRMWARE_VERSION;
        }
        return mcpollCMD::none;  
    }
    
    anypollCMD cmd2anypoll(std::uint8_t cmd)
    {
        if(cmd == static_cast<std::uint8_t>(anypollCMD::SETID))
        {
            return anypollCMD::SETID;
        }
        return anypollCMD::none;  
    }
    
    
    
    
    
    Clas frame2clas(const embot::hw::can::Frame &frame)
    {
        std::uint8_t t = (frame.id & 0x00000700) >> 8;
        if(6 == t)
        {
            return Clas::none;
        }
        return(static_cast<Clas>(t));
    }
    
    std::uint8_t frame2sender(const embot::hw::can::Frame &frame)
    {
        std::uint8_t t = (frame.id & 0x000000F0) >> 4;
        return(t);
    }
    
    
    bool frameisbootloader(const embot::hw::can::Frame &frame)
    {
        return (Clas::bootloader == frame2clas(frame)) ? true : (false);
    }
    
    bool frameispolling(const embot::hw::can::Frame &frame)
    {
        bool ret = false;
        switch(frame2clas(frame))
        {
            case Clas::pollingMotorControl:         ret = true;     break;
            case Clas::pollingAnalogSensor:         ret = true;     break;
            default:                                ret = false;    break;            
        }
        return ret;
    }    
    
    bool frameisperiodic(const embot::hw::can::Frame &frame)
    {
        bool ret = false;
        switch(frame2clas(frame))
        {
            case Clas::periodicMotorControl:        ret = true;     break;
            case Clas::periodicAnalogSensor:        ret = true;     break;
            case Clas::periodicInertialSensor:      ret = true;     break;
            case Clas::periodicSkin:                ret = true;     break;
            default:                                ret = false;    break;            
        }
        return ret;
    }
    
    std::uint8_t frame2destination(const embot::hw::can::Frame &frame)
    {   // for not periodic messages . if periodic .... destination is 0xf
        if(frameisperiodic(frame))
        {
            return 0xf;
        }
        std::uint8_t t = (frame.id & 0x0000000F);
        return(t);
    }
    
    bool frameis4board(const embot::hw::can::Frame &frame, const std::uint8_t boardaddress)
    {
        if(frameisperiodic(frame))
        {   // we dont accept any periodic
            return false;
        }
        // for all others destination is in id
        std::uint8_t t = (frame.id & 0x0000000F); 
        if(0xf == t)
        {   // broadcast
            return true;
        }
        if((0xf & boardaddress) == t)
        {   // matches the address
            return true;
        }
        return false;        
    }
    
    std::uint8_t frame2cmd(const embot::hw::can::Frame &frame)
    {  
        if(frameisperiodic(frame))
        {
            return (frame.id & 0x0000000F);
        }
        if(frameisbootloader(frame))
        {
            return frame.data[0];
        }
        return (frame.data[0] & 0x7F);
    }
    
    std::uint8_t frame2datasize(const embot::hw::can::Frame &frame)
    {  
        if(0 == frame.size)
        {
            return 0;
        }
        if(frameisperiodic(frame))
        {
            return (frame.size);
        }
        return (frame.size-1);
    }
    
    std::uint8_t* frame2databuffer(embot::hw::can::Frame &frame)
    {  
        if(0 == frame.size)
        {
            return nullptr;
        }
        if(frameisperiodic(frame))
        {
            return &frame.data[0];
        }
        return &frame.data[1];
    }
    
    bool frame_set_clascmddestinationdata(embot::hw::can::Frame &frame, const Clas cls, const std::uint8_t cmd, const std::uint8_t destination, const void *data, const std::uint8_t sizeofdatainframe, const std::uint8_t mcindex, bool verify)
    {
        if(Clas::none == cls)
        {
            return false;
        }
        
        bool ret = false;
        
        // set cls ... it is always in id-0x00000700
        frame.id &= ~0x00000700;
        frame.id |= ((static_cast<std::uint32_t>(cls) & 0xf) << 8);
        
        // set cmd and destination. their location dends on clas
        switch(cls)
        {
            case Clas::pollingMotorControl:
            case Clas::pollingAnalogSensor:   
            case Clas::bootloader:
            {
                // destination is in id-0xf, cmd is in data[0]
                frame.id &= ~0x0000000F;
                frame.id |= (destination & 0xF);
                
                if(Clas::bootloader == cls)
                {
                    frame.data[0] = cmd;
                }
                else if(Clas::pollingMotorControl == cls)
                {
                    frame.data[0] = ((mcindex&0x1) << 7) | (cmd & 0x7F);
                }
                else
                {
                    frame.data[0] = cmd & 0x7F;
                }
                
                std::uint8_t s = (sizeofdatainframe>7) ? 7 : sizeofdatainframe;
                if((nullptr != data) && (s>0))
                {
                    std::memmove(&frame.data[1], data, s);
                }
                
                ret = true;
            } break;

            case Clas::periodicMotorControl:
            case Clas::periodicAnalogSensor: 
            case Clas::periodicInertialSensor:    
            case Clas::periodicSkin:                    
            {
                // destination is not present, cmd is in 0x0000000F
                frame.id &= ~0x0000000F;
                frame.id |= (cmd & 0xF);
                
                std::uint8_t s = (sizeofdatainframe>8) ? 8 : sizeofdatainframe;
                if((nullptr != data) && (s>0))
                {
                    std::memmove(&frame.data[0], data, s);
                }
                
                ret = true;

            } break;            
            
            default:    
            {
                ret =  false;
            } break;            
        }
        
        if((true == ret) && (true == verify))
        {
            // i check it           
            if(cls != frame2clas(frame))
            {
                return false;
            }           
            if(cmd != frame2cmd(frame))
            {
                return false;
            }
            if((!frameisperiodic(frame)) && (destination == frame2destination(frame)))
            {
                return false;
            }            
        }
        
        return ret;
    }
    
    bool frame_set_sender(embot::hw::can::Frame &frame, std::uint8_t sender, bool verify)
    {
        frame.id &= ~0x000000F0;
        frame.id |= (static_cast<std::uint32_t>(sender & 0xF) << 4);
        if(verify)
        {
            if(sender != frame2sender(frame))
            {
                return false;
            }
        }
        return true;
    }
    
    bool frame_set_size(embot::hw::can::Frame &frame, std::uint8_t size, bool verify)
    {
        frame.size = (size > 8) ? (8) : (size);
        if(verify)
        {
            if(size != frame.size)
            {
                return false;
            }
        }
        return true;
    }
    
            
        void Message::set(const embot::hw::can::Frame &fr)
        {
            canframe = fr;
            
            candata.clas = frame2clas(canframe);
            candata.cmd = frame2cmd(canframe);
            candata.from = frame2sender(canframe);
            candata.to = frame2destination(canframe);              
            candata.datainframe = frame2databuffer(canframe);
            candata.sizeofdatainframe = frame2datasize(canframe);   

            valid = true;
        }
        
        void Message::clear()
        {
            candata.reset();
            
            std::memset(&canframe, 0, sizeof(canframe));
            
            valid = false;
        } 
        

        void Message::set(std::uint8_t fr, std::uint8_t t, Clas cl, std::uint8_t cm, const void *dat, std::uint8_t siz)
        {
            candata.clas = cl;
            candata.cmd = cm;
            candata.from = fr;
            candata.to = t;
            // we miss: candata.datainframe and candata.sizeofdatainframe .... but we compute them by loading something into the canframe and ....
            // ok, now canframe ...
            std::memset(&canframe, 0, sizeof(canframe));
            
            frame_set_sender(canframe, candata.from);
            frame_set_clascmddestinationdata(canframe, candata.clas, candata.cmd, candata.to, dat, siz);
            frame_set_size(canframe, siz);
            
            // ok, now we can compute ... candata.datainframe and candata.sizeofdatainframe
            candata.datainframe = frame2databuffer(canframe);
            candata.sizeofdatainframe = frame2datasize(canframe);
            
            valid = true;
        }
        
        bool Message::isvalid()
        {
            return valid;
        }


            
        bool Message_bldr_BROADCAST::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe); 
            
            if(static_cast<std::uint8_t>(bldrCMD::BROADCAST) != frame2cmd(inframe))
            {
                return false; 
            }

            return true;
        }  

        bool Message_bldr_BROADCAST::reply(embot::hw::can::Frame &outframe, const std::uint8_t sender, const ReplyInfo &replyinfo)
        {
            std::uint8_t dd[7] = {0};
            dd[0] = static_cast<std::uint8_t>(replyinfo.board);
            dd[1] = replyinfo.firmware.major;
            dd[2] = replyinfo.firmware.minor;
            dd[3] = replyinfo.firmware.build;
            
            std::uint8_t datalen = (Process::bootloader == replyinfo.process) ? (3) : (4);
            
            frame_set_sender(outframe, sender);
            frame_set_clascmddestinationdata(outframe, Clas::bootloader, static_cast<std::uint8_t>(bldrCMD::BROADCAST), candata.from, dd, datalen);
            frame_set_size(outframe, datalen+1);
            return true;
        }            
        
    
            
        bool Message_bldr_BOARD::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);
            
            if(static_cast<std::uint8_t>(bldrCMD::BOARD) != frame2cmd(inframe))
            {
                return false; 
            }
            
            info.eepromerase = candata.datainframe[0];   

            return true;
        }                    
        
        bool Message_bldr_BOARD::reply(embot::hw::can::Frame &outframe, const std::uint8_t sender)
        {
            frame_set_sender(outframe, sender);
            frame_set_clascmddestinationdata(outframe, Clas::bootloader, static_cast<std::uint8_t>(bldrCMD::BOARD), candata.from, nullptr, 0);
            frame_set_size(outframe, 1);
            return true;
        }     
    

            
        bool Message_bldr_ADDRESS::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(static_cast<std::uint8_t>(bldrCMD::ADDRESS) != frame2cmd(inframe))
            {
                return false; 
            }
            
            info.datalen = candata.datainframe[0];
            info.address = candata.datainframe[1] | (static_cast<std::uint32_t>(candata.datainframe[2]) << 8) | (static_cast<std::uint32_t>(candata.datainframe[4]) << 16) | (static_cast<std::uint32_t>(candata.datainframe[5]) << 24);
            
            return true;         
        }                    
        
        bool Message_bldr_ADDRESS::reply()
        {
            return false;
        } 

        
        bool Message_bldr_START::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(static_cast<std::uint8_t>(bldrCMD::START) != frame2cmd(inframe))
            {
                return false; 
            }
          
            return true;         
        }                    

        bool Message_bldr_START::reply(embot::hw::can::Frame &outframe, const std::uint8_t sender, const bool ok)
        {
            frame_set_sender(outframe, sender);
            char dd[1] = {1};
            dd[0] = (true == ok) ? (1) : (0);
            frame_set_clascmddestinationdata(outframe, Clas::bootloader, static_cast<std::uint8_t>(bldrCMD::START), candata.from, dd, 1);
            frame_set_size(outframe, 2);
            return true;
        }  
        
        bool Message_bldr_DATA::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(static_cast<std::uint8_t>(bldrCMD::DATA) != frame2cmd(inframe))
            {
                return false; 
            }
            
            info.size = candata.sizeofdatainframe;
            info.data = &candata.datainframe[0];
            
            return true;         
        }   

        bool Message_bldr_DATA::reply(embot::hw::can::Frame &outframe, const std::uint8_t sender, const bool ok)
        {
            frame_set_sender(outframe, sender);
            char dd[1] = {1};
            dd[0] = (true == ok) ? (1) : (0);
            frame_set_clascmddestinationdata(outframe, Clas::bootloader, static_cast<std::uint8_t>(bldrCMD::DATA), candata.from, dd, 1);
            frame_set_size(outframe, 2);
            return true;
        }              
        

            
        bool Message_bldr_END::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(static_cast<std::uint8_t>(bldrCMD::END) != frame2cmd(inframe))
            {
                return false; 
            }
          
            return true;         
        }                    
            
        bool Message_bldr_END::reply(embot::hw::can::Frame &outframe, const std::uint8_t sender, const bool ok)
        {
            frame_set_sender(outframe, sender);
            char dd[1] = {1};
            dd[0] = (true == ok) ? (1) : (0);
            frame_set_clascmddestinationdata(outframe, Clas::bootloader, static_cast<std::uint8_t>(bldrCMD::END), candata.from, dd, 1);
            frame_set_size(outframe, 2);
            return true;
        }  
        
        
        bool Message_bldr_SETCANADDRESS::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(static_cast<std::uint8_t>(bldrCMD::SETCANADDRESS) != frame2cmd(inframe))
            {
                return false; 
            }
            
            info.address = candata.datainframe[0];
            info.randominvalidmask = 0x0000;
            if(3 == inframe.size)
            {
                info.randominvalidmask = candata.datainframe[2];
                info.randominvalidmask <<= 8;
                info.randominvalidmask |= candata.datainframe[1];
            }
          
            return true;         
        }                    
            
        bool Message_bldr_SETCANADDRESS::reply()
        {
            return false;
        }    
               
        
        bool Message_base_GET_FIRMWARE_VERSION::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(this->cmd != frame2cmd(inframe))
            {
                return false; 
            }
            
            info.requiredprotocol.major = candata.datainframe[0];
            info.requiredprotocol.minor = candata.datainframe[1];
          
            return true;         
        }                    
            
        bool Message_base_GET_FIRMWARE_VERSION::reply(embot::hw::can::Frame &outframe, const std::uint8_t sender, const ReplyInfo &replyinfo)
        {
            frame_set_sender(outframe, sender);
            char dd[7] = {0};
            dd[0] = static_cast<std::uint8_t>(replyinfo.board);
            dd[1] = replyinfo.firmware.major;
            dd[2] = replyinfo.firmware.minor;
            dd[3] = replyinfo.firmware.build;
            dd[4] = replyinfo.protocol.major;
            dd[5] = replyinfo.protocol.minor;
            dd[6] = ((replyinfo.protocol.major == info.requiredprotocol.major) && (replyinfo.protocol.minor >= info.requiredprotocol.minor) ) ? (1) : (0);;
                       
            frame_set_clascmddestinationdata(outframe, this->cls, this->cmd, candata.from, dd, 7);
            frame_set_size(outframe, 8);
            return true;
        } 


        bool Message_bldr_GET_ADDITIONAL_INFO::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(static_cast<std::uint8_t>(bldrCMD::GET_ADDITIONAL_INFO) != frame2cmd(inframe))
            {
                return false; 
            }
            
            info.thereisnothing = 0;
            
            counter = 0;
          
            return true;         
        }     

        std::uint8_t Message_bldr_GET_ADDITIONAL_INFO::numberofreplies()
        {
            return nreplies;
        }    
            
        bool Message_bldr_GET_ADDITIONAL_INFO::reply(embot::hw::can::Frame &outframe, const std::uint8_t sender, const ReplyInfo &replyinfo)
        {
            if(counter >= nreplies)
            {
                return false;
            }
            
            frame_set_sender(outframe, sender);
            char dd[7] = {0};
            dd[0] = counter;
            dd[1] = replyinfo.info32[4*counter];
            dd[2] = replyinfo.info32[4*counter+1];
            dd[3] = replyinfo.info32[4*counter+2];
            dd[4] = replyinfo.info32[4*counter+3];

                       
            frame_set_clascmddestinationdata(outframe, Clas::bootloader, static_cast<std::uint8_t>(bldrCMD::GET_ADDITIONAL_INFO), candata.from, dd, 5);
            frame_set_size(outframe, 6);
            
            counter ++;
            
            return true;
        }   
        

        bool Message_bldr_SET_ADDITIONAL_INFO::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(static_cast<std::uint8_t>(bldrCMD::SET_ADDITIONAL_INFO) != frame2cmd(inframe))
            {
                return false; 
            }
            
            std::uint8_t counter = candata.datainframe[0];
            if(counter > 7)
            {
                info.offset = 255;
                return false;
            }
            
            info.offset = 4*counter;
            info.info04[0] = candata.datainframe[1];
            info.info04[1] = candata.datainframe[2];
            info.info04[2] = candata.datainframe[3];
            info.info04[3] = candata.datainframe[4];
            
            return true;         
        }     
               
        bool Message_bldr_SET_ADDITIONAL_INFO::reply()
        {
            return false;
        }   


        
        char Message_bldr_SET_ADDITIONAL_INFO2::cumulativeinfo32[32] = {0};
        std::uint8_t Message_bldr_SET_ADDITIONAL_INFO2::receivedmask = 0;
        
        bool Message_bldr_SET_ADDITIONAL_INFO2::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(static_cast<std::uint8_t>(bldrCMD::SET_ADDITIONAL_INFO) != frame2cmd(inframe))
            {
                return false; 
            }
            
            std::uint8_t counter = candata.datainframe[0];
            if(counter > 7)
            {
                return false;
            }
            
            if(0 == counter)
            {
                info.valid = false;
                receivedmask = 0;
                std::memset(cumulativeinfo32, 0, sizeof(cumulativeinfo32));                
            }
            
            embot::common::bit::set(receivedmask, counter);
            std::memmove(&cumulativeinfo32[4*counter], &candata.datainframe[1], 4);
            
            info.valid = false;
            
            if(0xff == receivedmask)
            {
                std::memmove(info.info32, cumulativeinfo32, sizeof(info.info32));
                info.valid = true;
            }
                        
            return true;         
        }     
               
        bool Message_bldr_SET_ADDITIONAL_INFO2::reply()
        {
            return false;
        }    



        bool Message_base_SET_ID::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(this->cmd != frame2cmd(inframe))
            {
                return false; 
            }
            
            info.address = candata.datainframe[0];
          
            return true;         
        }                    
            
        bool Message_base_SET_ID::reply()
        {
            return false;
        } 


       
        bool Message_aspoll_SET_TXMODE::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(static_cast<std::uint8_t>(aspollCMD::SET_TXMODE) != frame2cmd(inframe))
            {
                return false; 
            }
            
            switch(board)
            {
                case Board::mtb:
                case Board::mtb4:
                {
                    // according to protocol, we may have 0 (icubCanProto_as_sigmode_signal) or 1 (icubCanProto_as_sigmode_dontsignal)
                    info.transmit = (0 == candata.datainframe[0]) ? true : false;
                    info.strainmode = StrainMode::none;
                } break;
                
                case Board::strain:
                case Board::strain2:
                {
                    // according to protocol, we may have 0 (eoas_strainmode_txcalibrateddatacontinuously), 1 (eoas_strainmode_acquirebutdonttx),
                    // 3 (eoas_strainmode_txuncalibrateddatacontinuously), or 4 (eoas_strainmode_txalldatacontinuously)
                    if(0 == candata.datainframe[0])    
                    {
                        info.transmit = true;                        
                        info.strainmode = StrainMode::txCalibrated;
                    }
                    else if(1 == candata.datainframe[0])       
                    {
                        info.transmit = false;
                        info.strainmode = StrainMode::acquireOnly;
                    }
                    else if(3 == candata.datainframe[0])       
                    {
                        info.transmit = true;  
                        info.strainmode = StrainMode::txUncalibrated;
                    }
                    else if(4 == candata.datainframe[0])       
                    {
                        info.transmit = true;  
                        info.strainmode = StrainMode::txAll;
                    }
                    else                            
                    {       
                        info.transmit = false;  
                        info.strainmode = StrainMode::none;  
                    }                        
                } break;   

                default:
                {
                    info.transmit = false;  
                    info.strainmode = StrainMode::none;                      
                } break;
            }
          
            return true;         
        }                    
            
        bool Message_aspoll_SET_TXMODE::reply()
        {
            return false;
        } 
        

        bool Message_aspoll_SKIN_SET_BRD_CFG::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(static_cast<std::uint8_t>(aspollCMD::SKIN_SET_BRD_CFG) != frame2cmd(inframe))
            {
                return false; 
            }
            
            switch(candata.datainframe[0])
            {
                case static_cast<std::uint8_t>(SkinType::withTemperatureCompensation):
                {
                    info.skintype = SkinType::withTemperatureCompensation;
                } break;
                
                case static_cast<std::uint8_t>(SkinType::palmFingerTip):
                {
                    info.skintype = SkinType::palmFingerTip;
                } break;
                
                case static_cast<std::uint8_t>(SkinType::withoutTempCompensation):
                {
                    info.skintype = SkinType::withoutTempCompensation;
                } break;
                
                case static_cast<std::uint8_t>(SkinType::testmodeRAW):
                {
                    info.skintype = SkinType::testmodeRAW;
                } break;
                
                default:
                {
                    info.skintype = SkinType::none;
                } break;                                
            }
            
            info.txperiod = 1000*candata.datainframe[1]; // transform from msec into usec
            info.noload = candata.datainframe[2];
          
            return true;         
        } 
        
            
        bool Message_aspoll_SKIN_SET_BRD_CFG::reply()
        {
            return false;
        }  


        bool Message_aspoll_SKIN_SET_TRIANG_CFG::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(static_cast<std::uint8_t>(aspollCMD::SKIN_SET_TRIANG_CFG) != frame2cmd(inframe))
            {
                return false; 
            }
            
            info.trgStart = candata.datainframe[0];
            info.trgEnd= candata.datainframe[1];
            info.shift = candata.datainframe[2];
            info.enabled = embot::common::bit::check(candata.datainframe[3], 0);
            info.cdcOffset = candata.datainframe[4] | static_cast<std::uint16_t>(candata.datainframe[5]) << 8;
         
            return true;         
        } 
        
            
        bool Message_aspoll_SKIN_SET_TRIANG_CFG::reply()
        {
            return false;
        }   




            
        bool Message_skper_TRG::load(const Info& inf)
        {
            info = inf;
          
            return true;
        }
            
        bool Message_skper_TRG::get(embot::hw::can::Frame &outframe0, embot::hw::can::Frame &outframe1)
        {
            std::uint8_t data08[8] = {0};
            data08[0] = 0x40;
            std::memmove(&data08[1], &info.the12s[0], 7);
            Message::set(info.canaddress, 0xf, Clas::periodicSkin, info.trianglenum, data08, 8);
            std::memmove(&outframe0, &canframe, sizeof(embot::hw::can::Frame));
            
            data08[0] = 0xC0;
            std::memmove(&data08[1], &info.the12s[7], 5);
            // now outofrange and error flags
            std::uint8_t errorflags = 0; 
            // bit ErrorInTriangleBit::noack is set if any bit inside notackmaskofthe12s is set.
            if(0 != info.notackmaskofthe12s)
            {
                embot::common::bit::set(errorflags, static_cast<std::uint8_t>(ErrorInTriangleBit::noack)); 
            }
            // bit ErrorInTriangleBit::notconnected is set if all 12 bits inside notconnectedmaskofthe12s are set.
            if(12 == embot::common::bit::countU16(info.notconnectedmaskofthe12s))
            {
                embot::common::bit::set(errorflags, static_cast<std::uint8_t>(ErrorInTriangleBit::notconnected)); 
            }            
            data08[6] = static_cast<std::uint8_t>((info.outofrangemaskofthe12s & 0x0ff0) >> 4);
            data08[7] = static_cast<std::uint8_t>((info.outofrangemaskofthe12s & 0x000f) << 4) | (errorflags & 0x0f);
            
            Message::set(info.canaddress, 0xf, Clas::periodicSkin, info.trianglenum, data08, 8);
            std::memmove(&outframe1, &canframe, sizeof(embot::hw::can::Frame));
            
            return true;
        }  


        
        std::uint8_t Message_mcper_PRINT::textIDmod4 = 0;
        
        bool Message_mcper_PRINT::load(const Info& inf)
        {
            info = inf;  
            nchars = std::strlen(info.text);
            nframes = (std::strlen(info.text) + 5) / 6;   
            framecounter = 0;  
            textIDmod4++;
            textIDmod4 %= 4;
            
            return (nframes>0) ? true : false;
        }
        
        std::uint8_t Message_mcper_PRINT::numberofframes()
        {       
            return nframes;
        }
            
        bool Message_mcper_PRINT::get(embot::hw::can::Frame &outframe)
        {
            if((0 == nframes) || (framecounter >= nframes))
            {
                return false;
            }
            
            bool lastframe = ((framecounter+1) == nframes) ? true : false;
            
            std::uint8_t charsinframe = 6;
            if(lastframe)
            {
                charsinframe = nchars - 6*framecounter; // less than 6                    
            }
            
            if(charsinframe > 6)
            {   // just because i am a paranoic
                charsinframe = 6;
            }
            
            std::uint8_t data08[8] = {0};
            data08[0] = lastframe ? (static_cast<std::uint8_t>(mcperCMD::PRINT) + 128) : (static_cast<std::uint8_t>(mcperCMD::PRINT));
            data08[1] = ((textIDmod4 << 4) & 0xF0) | (framecounter & 0x0F);
            std::memmove(&data08[2], &info.text[6*framecounter], charsinframe);
            
            // ok, increment framecounter
            framecounter ++;
            
            Message::set(info.canaddress, 0xf, Clas::periodicMotorControl, static_cast<std::uint8_t>(mcperCMD::PRINT), data08, 2+charsinframe);
            std::memmove(&outframe, &canframe, sizeof(embot::hw::can::Frame));
                        
            return true;
        }  


        bool Message_aspoll_ACC_GYRO_SETUP::load(const embot::hw::can::Frame &inframe)
        {
            Message::set(inframe);  
            
            if(static_cast<std::uint8_t>(aspollCMD::ACC_GYRO_SETUP) != frame2cmd(inframe))
            {
                return false; 
            }
            
            info.maskoftypes = candata.datainframe[0];
            info.txperiod = 1000*candata.datainframe[1]; // transform from msec into usec
          
            return true;         
        } 
        
            
        bool Message_aspoll_ACC_GYRO_SETUP::reply()
        {
            return false;
        } 


        bool Message_isper_DIGITAL_GYROSCOPE::load(const Info& inf)
        {
            info = inf;
          
            return true;
        }
            
        bool Message_isper_DIGITAL_GYROSCOPE::get(embot::hw::can::Frame &outframe)
        {
            std::uint8_t data08[8] = {0};
            data08[0] = static_cast<std::uint8_t>((info.x & 0x0f));
            data08[1] = static_cast<std::uint8_t>((info.x & 0xf0) >> 8);
            data08[2] = static_cast<std::uint8_t>((info.y & 0x0f));
            data08[3] = static_cast<std::uint8_t>((info.y & 0xf0) >> 8);  
            data08[4] = static_cast<std::uint8_t>((info.z & 0x0f));
            data08[5] = static_cast<std::uint8_t>((info.z & 0xf0) >> 8);            
            Message::set(info.canaddress, 0xf, Clas::periodicInertialSensor, static_cast<std::uint8_t>(isperCMD::DIGITAL_GYROSCOPE), data08, 6);
            std::memmove(&outframe, &canframe, sizeof(embot::hw::can::Frame));
                        
            return true;
        }  

        bool Message_isper_DIGITAL_ACCELEROMETER::load(const Info& inf)
        {
            info = inf;
          
            return true;
        }
            
        bool Message_isper_DIGITAL_ACCELEROMETER::get(embot::hw::can::Frame &outframe)
        {
            std::uint8_t data08[8] = {0};
            data08[0] = static_cast<std::uint8_t>((info.x & 0x0f));
            data08[1] = static_cast<std::uint8_t>((info.x & 0xf0) >> 8);
            data08[2] = static_cast<std::uint8_t>((info.y & 0x0f));
            data08[3] = static_cast<std::uint8_t>((info.y & 0xf0) >> 8);  
            data08[4] = static_cast<std::uint8_t>((info.z & 0x0f));
            data08[5] = static_cast<std::uint8_t>((info.z & 0xf0) >> 8);            
            Message::set(info.canaddress, 0xf, Clas::periodicInertialSensor, static_cast<std::uint8_t>(isperCMD::DIGITAL_ACCELEROMETER), data08, 6);
            std::memmove(&outframe, &canframe, sizeof(embot::hw::can::Frame));
                        
            return true;
        }          

}}} // namespace embot { namespace app { namespace canprotocol {




// - end-of-file (leave a blank line after)----------------------------------------------------------------------------


