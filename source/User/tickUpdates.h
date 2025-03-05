#include <map> 
#include "comm/datalayer/datalayer.h"
#include "../impl/trace.h"

namespace TickUpdate{
            void sampleOutputProcessing(u_int8_t* outData, std::map<std::string,uint32_t> m_outMap);
            void sampleInputProcessing(u_int8_t* inData, std::map<std::string,uint32_t> m_inMap);
            //int m_spicks = 0;    
}
