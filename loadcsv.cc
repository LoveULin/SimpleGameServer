
#include "loadcsv.h"

void LoadCSV::LoadAllCSV()
{
    int ret(csv_init(&m_p, CSV_STRICT | CSV_STRICT_FINI)); 
    assert(0 == ret);
    LoadACSV("lol.csv");
    csv_free(&m_p);
}
