
#ifndef __ULIN_LOG_H_
#define __ULIN_LOG_H_

#include <boost/thread/detail/singleton.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include "loadcsv.h"

class Log : private boost::noncopyable
{
public:
    enum class Severity : unsigned short
    {
        INFO,  // default 0
        NOTIFICATION, 
        WARNING, 
        ERROR, 
        FATAL 
    };
public:
    Log() // don't know whether will throw exception or not
    {
        (void)boost::log::add_console_log(std::cout, 
            boost::log::keywords::format = 
                pt::instance().get<std::string>("consolelogformat").c_str());
        (void)boost::log::add_file_log(
            boost::log::keywords::file_name = 
                pt::instance().get<std::string>("logfile") + "_%Y%m%d_%H%M%S.%N.log", 
            boost::log::keywords::rotation_size = 10 * 1024 * 1024, 
            boost::log::keywords::time_based_rotation = 
                boost::log::sinks::file::rotation_at_time_point(0, 0, 0), 
            boost::log::keywords::auto_flush = true, 
            boost::log::keywords::format = 
                pt::instance().get<std::string>("filelogformat").c_str());
        boost::log::add_common_attributes();
    }
    boost::log::sources::severity_logger<Severity> &GetLogger()
    {
        return m_logger;
    }
private:
    boost::log::sources::severity_logger<Severity> m_logger;
};

typedef boost::detail::thread::singleton<Log> theLog;

#define LOG_INFO \
    BOOST_LOG_SEV(theLog::instance().GetLogger(), Log::Severity::INFO)
#define LOG_NOTIFY \
    BOOST_LOG_SEV(theLog::instance().GetLogger(), Log::Severity::NOTIFICATION)
#define LOG_WARN \
    BOOST_LOG_SEV(theLog::instance().GetLogger(), Log::Severity::WARNING)
#define LOG_ERROR \
    BOOST_LOG_SEV(theLog::instance().GetLogger(), Log::Severity::ERROR)
#define LOG_FATAL \
    BOOST_LOG_SEV(theLog::instance().GetLogger(), Log::Severity::FATAL)

#endif

