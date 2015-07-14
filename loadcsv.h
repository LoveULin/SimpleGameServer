
#ifndef __ULIN_LOADCSV_H_
#define __ULIN_LOADCSV_H_

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <csv.h>
#include "type.h"

// in main.cc
extern boost::property_tree::ptree pt;

class LoadCSV : private boost::noncopyable
{
    class CSVTable
    {
    public:
        void SaveData(const std::string &data)
        {
            if (0 == m_rows) {
                // the first row is column's name
                const auto itPair(m_columnIndex.emplace(data.c_str(), m_columns));
                assert(itPair.second);
                ++m_columns;
                return;
            }
            else if (0 == m_columns){
                // the first column is always the key
                const auto itPair(m_keyIndex.emplace(data.c_str(), (m_rows - 1)));
                assert(itPair.second);
                // increase a line
                m_data.emplace_back();
                // no reserve for simplification
            }
            // save the data
            m_data[m_rows - 1].emplace_back(data);
            ++m_columns;
        }
        void EndLineData()
        {
            ++m_rows; 
            m_columns = 0;
        }
    private:
        // use the default load_factor
        std::unordered_map<const char*, std::size_t> m_keyIndex;
        std::unordered_map<const char*, std::size_t> m_columnIndex;
        std::vector<std::vector<std::string>> m_data;
        std::size_t m_rows {0};
        std::size_t m_columns {0};
    };

    static constexpr std::size_t tmpBufferSize = 8192;
public:
    LoadCSV() : m_pathPre(pt.get<std::string>("csvprefix")) {}
    void LoadAllCSV();
private:
    void LoadACSV(const std::string &name)
    {
        const auto itPair(m_data.emplace(name.substr(0, name.rfind('.')), CSVTable()));
        assert(itPair.second);

        void *userData(&itPair.first->second);
        char tmpBuffer[tmpBufferSize];
        std::ifstream ifs(m_pathPre + '/' + name);
        while (ifs.good()) {
            const std::streamsize size(ifs.readsome(tmpBuffer, sizeof(tmpBuffer)));
            if (size <= 0) {
                break;
            }
            if (static_cast<std::size_t>(size) != csv_parse(&m_p, tmpBuffer, size, 
                                                            cb_libcsv_1, cb_libcsv_2, userData)) {
                assert(false);
            }
        }
        const int ret(csv_fini(&m_p, cb_libcsv_1, cb_libcsv_2, userData));
        assert(0 == ret);
    }
    static void cb_libcsv_1(void *buf, std::size_t len, void *data)
    {
        CSVTable *theTable(static_cast<CSVTable*>(data));
        theTable->SaveData(std::string(static_cast<const char*>(buf), len));
    }
    static void cb_libcsv_2(int status, void *data)
    {
        CSVTable *theTable(static_cast<CSVTable*>(data));
        theTable->EndLineData();
    }
private:
    struct csv_parser m_p; 
    const std::string m_pathPre;
    // use the default load factor
    std::unordered_map<std::string, CSVTable> m_data;
};

typedef boost::detail::thread::singleton<LoadCSV> loadCSV;

#endif

