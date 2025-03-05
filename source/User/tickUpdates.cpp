#include "tickUpdates.h"

long int m_ticks = 0;    

namespace TickUpdate{
    void sampleOutputProcessing(u_int8_t* outData, std::map<std::string,uint32_t> m_outMap)
    {
    m_ticks++; 
    //m_ticks = m_ticks%500; 
    if(0 == m_ticks%500){
        TRACE_MSG("500 TICKS"); 
        if(outData[m_outMap["DO_16_1/Channel_1.Value"]/8] == 0)
            outData[m_outMap["DO_16_1/Channel_1.Value"]/8] = 255; 
        else
            outData[m_outMap["DO_16_1/Channel_1.Value"]/8] = 0; 
        }
    //Enable the only drive    
    if (m_ticks > 2000) 
    {
        outData[m_outMap["Axis1/MDT.Master_control_word"]] = 57600;
    }
    if (m_ticks > 5000) 
    {
        outData[m_outMap["Axis1/MDT.VelocityCommand"]] = 30000;
    }
    }

    void sampleInputProcessing(u_int8_t* inData, std::map<std::string,uint32_t> m_inMap)
    {
        TRACE_MSG("Writing"); 
        int16_t* ulCurrent =(int16_t*)(&inData[m_inMap["BusCoupler/UL_Supply_logic.UL_Current"]/8]); 
        TRACE_MSG("UL Current: %i",*ulCurrent); 
    }
}
