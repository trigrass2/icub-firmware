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

#include "embot_app_application_theCANparserBasic.h"



// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "embot.h"

#include <new>

#include "embot_sys_theJumper.h"
#include "embot_sys_Timer.h"

#include "embot_hw.h"
#include "embot_app_canprotocol.h"

#include "embot_app_theCANboardInfo.h"


// --------------------------------------------------------------------------------------------------------------------
// - pimpl: private implementation (see scott meyers: item 22 of effective modern c++, item 31 of effective c++
// --------------------------------------------------------------------------------------------------------------------



struct embot::app::application::theCANparserBasic::Impl
{    
    Config config;
    
    bool txframe;
    bool recognised;
    
    embot::app::canprotocol::Clas cls;
    std::uint8_t cmd;
    
    embot::app::canprotocol::versionOfCANPROTOCOL canprotocol;
    embot::app::canprotocol::versionOfFIRMWARE version;
    embot::app::canprotocol::Board board;
    std::uint8_t canaddress;
    

    
    embot::hw::can::Frame reply;
    

    Impl() 
    {   
        recognised = false;        
        txframe = false;
        cls = embot::app::canprotocol::Clas::none;
        cmd = 0;
        
        canaddress = 0;
        version.major = version.minor = version.build = 0;
        canprotocol.major = canprotocol.minor = 0;
        board = embot::app::canprotocol::Board::unknown;               
    }
    
    
    bool setcanaddress(const std::uint8_t adr, const std::uint16_t randominvalidmask)
    {
        embot::app::theCANboardInfo &canbrdinfo = embot::app::theCANboardInfo::getInstance();
        // i reinforce a reading from storage. just for safety. in here we are dealing w/ can address change and i want to be sure.
        canaddress = canbrdinfo.getCANaddress();
        
        std::uint8_t target = adr;
        
        if(0xff == adr)
        {
            // compute a new random address. use the randoinvalid mask to filter out the undesired values. for sure 0x8001.
            std::uint16_t mask = randominvalidmask;
            mask |= 0x8001;
            if(0xffff == mask)
            {   // hei, nothing is good for you.
                return false;
            }
            
            bool ok = false;
            for(std::uint16_t i=0; i<250; i++)
            {
                target = (embot::hw::sys::random()-embot::hw::sys::minrandom()) & 0xf;
               
                if(false == embot::common::bit::check(mask, target))
                {
                    ok = true;
                    break;                    
                }
            }
            
            if(!ok)
            {
                return false;
            }
        }
        
        // always check that is is not 0 or 0xf
        if((0 == target) || (0xf == target))
        {
            return false;
        }
            
        
        if(canaddress != target)
        {
            canbrdinfo.setCANaddress(target);
            canaddress = canbrdinfo.getCANaddress();
        }
        
        return (target == canaddress);
    }
    
    
    bool process(const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies);
    
    bool process_bl_broadcast_appl(const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies);  
    bool process_bl_board_appl(const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies);    
    bool process_bl_getadditionalinfo(const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies);
    bool process_bl_setadditionalinfo(const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies);
    bool process_bl_setcanaddress(const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies);
    
    bool process_getfirmwareversion(const embot::app::canprotocol::Clas cl, const std::uint8_t cm, const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies);    
    bool process_setid(const embot::app::canprotocol::Clas cl, const std::uint8_t cm, const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies);
    
        
};


bool embot::app::application::theCANparserBasic::Impl::process(const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies)
{
    txframe = false;
    recognised = false;
    
    if(false == embot::app::canprotocol::frameis4board(frame, canaddress))
    {
        recognised = false;
        return recognised;
    }
        
    // now get cls and cmd
    cls = embot::app::canprotocol::frame2clas(frame);
    cmd = embot::app::canprotocol::frame2cmd(frame);
    
//    replies.clear(); i dont want to clear because we may have others inside which i dont want to lose
    
    // the basic can handle only some messages ...
    
    switch(cls)
    {
        case embot::app::canprotocol::Clas::bootloader:
        {
            // only bldrCMD::BROADCAST, bldrCMD::BOARD, bldrCMD::SETCANADDRESS, bldrCMD::GET_ADDITIONAL_INFO, bldrCMD::SET_ADDITIONAL_INFO 

            if(static_cast<std::uint8_t>(embot::app::canprotocol::bldrCMD::BOARD) == cmd)
            {
                txframe = process_bl_board_appl(frame, replies);   
                recognised = true;
                // then restart ...
                embot::hw::sys::reset();
            }
            else if(static_cast<std::uint8_t>(embot::app::canprotocol::bldrCMD::BROADCAST) == cmd)
            {
                txframe = process_bl_broadcast_appl(frame, replies);   
                recognised = true;                
            }
            else if(static_cast<std::uint8_t>(embot::app::canprotocol::bldrCMD::SETCANADDRESS) == cmd)
            {
                txframe = process_bl_setcanaddress(frame, replies);  
                recognised = true;                
            } 
            else if(static_cast<std::uint8_t>(embot::app::canprotocol::bldrCMD::GET_ADDITIONAL_INFO) == cmd)
            {
                txframe = process_bl_getadditionalinfo(frame, replies);   
                recognised = true;                
            } 
            else if(static_cast<std::uint8_t>(embot::app::canprotocol::bldrCMD::SET_ADDITIONAL_INFO) == cmd)
            {
                txframe = process_bl_setadditionalinfo(frame, replies);
                recognised = true;                
            }                     
             
        } break;
        

        case embot::app::canprotocol::Clas::pollingAnalogSensor:
        {
            // only embot::app::canprotocol::aspollCMD::SET_BOARD_ADX, GET_FIRMWARE_VERSION, ??
            if(static_cast<std::uint8_t>(embot::app::canprotocol::aspollCMD::SET_BOARD_ADX) == cmd)
            {
                txframe = process_setid(cls, cmd, frame, replies);
                recognised = true;
            }
            else if(static_cast<std::uint8_t>(embot::app::canprotocol::aspollCMD::GET_FIRMWARE_VERSION) == cmd)
            {
                txframe = process_getfirmwareversion(cls, cmd, frame, replies);
                recognised = true;
            }
 
        } break;

        case embot::app::canprotocol::Clas::pollingMotorControl:
        {
            // only embot::app::canprotocol::mcpollCMD::SET_BOARD_ID, GET_FIRMWARE_VERSION, ??
            if(static_cast<std::uint8_t>(embot::app::canprotocol::mcpollCMD::SET_BOARD_ID) == cmd)
            {
                txframe = process_setid(cls, cmd, frame, replies);
                recognised = true;
            }
            else if(static_cast<std::uint8_t>(embot::app::canprotocol::mcpollCMD::GET_FIRMWARE_VERSION) == cmd)
            {
                txframe = process_getfirmwareversion(cls, cmd, frame, replies);
                recognised = true;
            }
 
        } break;
        
        default:
        {
            txframe = false;
            recognised = false;
        } break;
    }    
    
    
    return recognised;
}



bool embot::app::application::theCANparserBasic::Impl::process_bl_broadcast_appl(const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies)
{
    embot::app::canprotocol::Message_bldr_BROADCAST msg;
    msg.load(frame);
    
    embot::app::theCANboardInfo &canbrdinfo = embot::app::theCANboardInfo::getInstance();
    embot::app::theCANboardInfo::StoredInfo strd = {0};
    canbrdinfo.get(strd);
    
    embot::app::canprotocol::Message_bldr_BROADCAST::ReplyInfo replyinfo;
    replyinfo.board = static_cast<embot::app::canprotocol::Board>(strd.boardtype);
    replyinfo.process = embot::app::canprotocol::Process::application;
    replyinfo.firmware.major = strd.applicationVmajor;
    replyinfo.firmware.minor = strd.applicationVminor;
    replyinfo.firmware.build = strd.applicationVbuild;
        
    if(true == msg.reply(reply, canaddress, replyinfo))
    {
        replies.push_back(reply);
        return true;
    }        
    
    return false;
}


bool embot::app::application::theCANparserBasic::Impl::process_bl_board_appl(const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies)
{
    embot::app::canprotocol::Message_bldr_BOARD msg;
    msg.load(frame);
    
    // i dont get any info... i just must restart. 
    embot::hw::sys::reset();  
    
    return false;       
}


bool embot::app::application::theCANparserBasic::Impl::process_bl_getadditionalinfo(const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies)
{
    embot::app::canprotocol::Message_bldr_GET_ADDITIONAL_INFO msg;
    msg.load(frame);
    
    embot::app::theCANboardInfo &canbrdinfo = embot::app::theCANboardInfo::getInstance();
    embot::app::theCANboardInfo::StoredInfo strd = {0};
    canbrdinfo.get(strd);
    
    embot::app::canprotocol::Message_bldr_GET_ADDITIONAL_INFO::ReplyInfo replyinfo;
    std::memmove(replyinfo.info32, strd.info32, sizeof(replyinfo.info32)); 
   
    std::uint8_t nreplies = msg.numberofreplies();
    for(std::uint8_t i=0; i<nreplies; i++)
    {
        if(true == msg.reply(reply, canaddress, replyinfo))
        {
            replies.push_back(reply);
        }
    }
    return true;
        
}


bool embot::app::application::theCANparserBasic::Impl::process_bl_setadditionalinfo(const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies)
{
    embot::app::canprotocol::Message_bldr_SET_ADDITIONAL_INFO2 msg;
    msg.load(frame);
    
    if(true == msg.info.valid)
    {   // we have received all the 8 messages in order (important is that the one with data[1] = 0 is the first)
        embot::app::theCANboardInfo &canbrdinfo = embot::app::theCANboardInfo::getInstance();
        embot::app::theCANboardInfo::StoredInfo strd = {0};
        canbrdinfo.get(strd);    
        std::memmove(strd.info32, msg.info.info32, sizeof(strd.info32));
        canbrdinfo.set(strd);
    }
    return false;        
}


bool embot::app::application::theCANparserBasic::Impl::process_setid(const embot::app::canprotocol::Clas cl, const std::uint8_t cm, const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies)
{
    embot::app::canprotocol::Message_base_SET_ID msg(cl, cm);
    msg.load(frame);
      
    setcanaddress(msg.info.address, 0x0000);
            
    return msg.reply();        
}

bool embot::app::application::theCANparserBasic::Impl::process_getfirmwareversion(const embot::app::canprotocol::Clas cl, const std::uint8_t cm, const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies)
{
    
    embot::app::canprotocol::Message_base_GET_FIRMWARE_VERSION msg(cl, cm);
    msg.load(frame);
      
    embot::app::canprotocol::Message_base_GET_FIRMWARE_VERSION::ReplyInfo replyinfo;
    
    replyinfo.board = board;
    replyinfo.firmware = version;
    replyinfo.protocol = canprotocol;
    
    if(true == msg.reply(reply, canaddress, replyinfo))
    {            
        replies.push_back(reply);
        return true;
    }
    
    return false;
}

bool embot::app::application::theCANparserBasic::Impl::process_bl_setcanaddress(const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies)
{
    embot::app::canprotocol::Message_bldr_SETCANADDRESS msg;
    msg.load(frame);
      
    
    setcanaddress(msg.info.address, msg.info.randominvalidmask);
            
    return msg.reply();        
}



// --------------------------------------------------------------------------------------------------------------------
// - the class
// --------------------------------------------------------------------------------------------------------------------



embot::app::application::theCANparserBasic::theCANparserBasic()
: pImpl(new Impl)
{       
    embot::sys::theJumper& thejumper = embot::sys::theJumper::getInstance();
}

   
        
bool embot::app::application::theCANparserBasic::initialise(Config &config)
{
    pImpl->config = config;

    
    embot::app::theCANboardInfo &canbrdinfo = embot::app::theCANboardInfo::getInstance();
    
    // retrieve version of bootloader + address
    embot::app::theCANboardInfo::StoredInfo storedinfo;
    if(true == canbrdinfo.get(storedinfo))
    {
        pImpl->canaddress = storedinfo.canaddress;
        pImpl->version.major = storedinfo.applicationVmajor;
        pImpl->version.minor = storedinfo.applicationVminor;
        pImpl->version.build = storedinfo.applicationVbuild;
        pImpl->canprotocol.major = storedinfo.protocolVmajor;
        pImpl->canprotocol.minor = storedinfo.protocolVminor;
        pImpl->board = static_cast<embot::app::canprotocol::Board>(storedinfo.boardtype);        
    }
      
    
    return true;
}
  


bool embot::app::application::theCANparserBasic::process(const embot::hw::can::Frame &frame, std::vector<embot::hw::can::Frame> &replies)
{    
    return pImpl->process(frame, replies);
}





// - end-of-file (leave a blank line after)----------------------------------------------------------------------------


