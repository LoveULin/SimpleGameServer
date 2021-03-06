
#ifndef __ULIN_TYPE_H_
#define __ULIN_TYPE_H_

#undef LIKELY
#undef UNLIKELY

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x)   (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

#include <boost/thread/detail/singleton.hpp>
#include <boost/property_tree/ptree.hpp>

typedef boost::detail::thread::singleton<boost::property_tree::ptree> pt;

#endif

