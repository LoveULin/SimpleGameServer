
#ifndef __ULIN_LOG_H_
#define __ULIN_LOG_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include "loadcsv.h"

// in main.cc
extern boost::property_tree::ptree pt;

class Log : private boost::noncopyable
{
public:
    Log() // don't know whether will throw exception or not
    {
        boost::log::add_file_log(boost::log::keywords::file_name = 
                                    pt.get<std::string>("logfile") + "_%N.log", 
                                 boost::log::keywords::rotation_size = 10 * 1024 * 1024, 
                                 boost::log::keywords::time_based_rotation = 
                                    boost::log::sinks::file::rotation_at_time_point(0, 0, 0), 
                                 boost::log::keywords::format = "[%TimeStamp%]: %Message%"
                                 );
        boost::log::add_common_attributes();
    } 
    template<typename T>
    Log &operator<<(const T &text)
    {
        BOOST_LOG(m_logger) << text;
        return *this;
    }
private:
    boost::log::sources::logger m_logger;
};

typedef boost::detail::thread::singleton<Log> theLog;

#endif

