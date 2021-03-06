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


#ifndef _EMBOT_TOOLS_H_
#define _EMBOT_TOOLS_H_

// - namespace embot::tools belongs to the embot (emBEDDED RObot) libray of C++11 classes and functions which are 
// - designed to run in the icub/r1 robot's embedded environment.
// - the objects inside this namenspace are tools used to validate behaviours in the microcontrollers inside the robot
// - but can also be used inside icub-main classes which runs on the PC104 platform
// - hence particular attention was put in avoiding any call to YARP or embot::sys (RTOS) or embot::hw (HW of the micro). 
// - we also don't use in here any embot::common funtions or types to guarantee maximum portability.

#include <cstdint>
#include <vector>

namespace embot { namespace tools {
    
    class Histogram
    {
    public:
        
        struct Config
        {   // there are nsteps() intervals each containing .step values which fill the range [.min, ... , .max)
            std::uint64_t               min;        // the start value of first interval.
            std::uint64_t               max;        // the upper limit of all possible values (which is actually max-1).
            std::uint32_t               step;       // the width of the interval 
            Config() : min(0), max(0), step(0) {}
            Config(std::uint64_t mi, std::uint64_t ma, std::uint32_t st) : min(mi), max(ma), step(st) {}
            std::uint64_t range() const { return max - min; }
            std::uint32_t nsteps() const { return ( (range() + step - 1) / step); }
            bool isvalid() const { return ((0 == step) || (min >= max)) ? false : true; }
        };
        
        struct Values
        {
            std::uint64_t               total;          // cumulative number = below + sum(inside) + beyond
            std::uint64_t               below;          // number of occurrences in ( -INF, config.min )
            std::uint64_t               beyond;         // number of occurrenced in [ inside.size() * config.step, +INF )
            std::vector<std::uint64_t>  inside;         // inside[i] contains the number of occurrences in [ config.min + i*config.step, config.min + (i+1)*config.step )  
        };
            
        
        Histogram();
        ~Histogram();
    
        bool init(const Config &config);
        
        bool add(std::uint64_t value);
        
        bool reset();  
        
        const embot::tools::Histogram::Config * getconfig() const;
        const embot::tools::Histogram::Values * getvalues() const;
        
        // it generates the pdf in a vector which is long Config::nsteps()+2 item. 
        // the first position contains probability that the value is < Config::min. 
        // the last position keeps probability that the value is >= Config::max. 
        // position i-th contain probability that value belongs inside [Config:min + i*Config::step, Config:min + (i+1)*Config::step).
        bool probabilitydensityfunction(std::vector<std::uint32_t> &values, const std::uint32_t scale) const;
        bool probabilitydensityfunction(std::vector<double> &values) const;
        
    private:        
        struct Impl;
        Impl *pImpl;    
    };    
    
} } // namespace embot { namespace tools {




namespace embot { namespace tools {
    
    // the object validates a given period expressed in micro-seconds.
    class PeriodValidator
    {
    public:
        
        struct Config
        {   
            std::uint64_t                       period;                     // the period under test.
            std::uint64_t                       alertvalue;                 // it is the value beyond which we produce an alert string. it must be > period.  
            std::uint64_t                       reportinterval;             // if not zero, it keeps the value in usec between two reports
            embot::tools::Histogram::Config     histoconfig;                // if is valid(), then we produce an histogram  
            Config() : period(0), alertvalue(0), reportinterval(0) {}
            Config(std::uint64_t pe, std::uint64_t al, std::uint64_t ri, const embot::tools::Histogram::Config &hi) 
                : period(pe), alertvalue(al), reportinterval(ri), histoconfig(hi) {}
            bool isvalid() const { return ((0 == period) || (period >= alertvalue)) ? false : true; }
        };
        

        
        PeriodValidator();
        ~PeriodValidator();
    
        bool init(const Config &config);
        
        // it must be regularly called every Config.period micro-seconds.
        // it accepts the current time, returns delta time from previous call of tick(), it computes a histogram of deltas.
        // currtime_usec is the absolute time in micro-seconds (e.g., as generated by embot::sys::timeNow() or by static_cast<std::uint64_t>(1000000.0*yarp::os::Time::now()))
        bool tick(std::uint64_t currtime_usec, std::uint64_t &deltatime_usec);
        
        // it removes all that was previously added with tick().
        bool reset();  
        
        // if true it is time for the regular report because more  than Config.reportinterval micro-seconds have passed from previous report time.
        bool report() const;
        
        // if true: there is an alert because the current delta is > Config.alertvalue 
        bool alert(std::uint64_t &deltatime_usec) const;
        
        const embot::tools::Histogram * histogram() const;
               
    private:        
        struct Impl;
        Impl *pImpl;    
    };    
    
} } // namespace embot { namespace tools {






#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------

