#include "tickUpdates.h"

namespace TickUpdate{
    void sampleOutputProcessing(u_int8_t* outData, std::map<std::string,uint32_t> m_outMap)
    {
    if(outData[m_outMap["DO_16_1/Channel_1.Value"]/8] == 0)
            outData[m_outMap["DO_16_1/Channel_1.Value"]/8] = 255; 
    else
        outData[m_outMap["DO_16_1/Channel_1.Value"]/8] = 0; 
    }

    void sampleInputProcessing(u_int8_t* inData, std::map<std::string,uint32_t> m_inMap)
    {
        int16_t* ulCurrent =(int16_t*)(&inData[m_inMap["BusCoupler/UL_Supply_logic.UL_Current"]/8]); 
        TRACE_MSG("UL Current: %i",*ulCurrent); 
    }
}
