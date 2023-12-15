#pragma once
#include "comm/datalayer/datalayer.h"
#include "common/scheduler/i_scheduler3.h"
#include <map> 

namespace Example{
  class RTApplication:public common::scheduler::ICallable
  {
    public:
      common::scheduler::SchedEventResponse execute(const common::scheduler::SchedEventType& eventType,
                                                    const common::scheduler::SchedEventPhase& eventPhase,
                                                    comm::datalayer::Variant& param); 
      void setDatalyer(comm::datalayer::IDataLayerFactory3* datalayerFactory);
      void resetDataLayer();
        
    private: 
      comm::datalayer::IDataLayerFactory3* m_datalayer;
      comm::datalayer::IClient3* m_client; 
      std::shared_ptr<comm::datalayer::IMemoryUser> m_inputs; 
      std::shared_ptr<comm::datalayer::IMemoryUser> m_outputs;  
      uint32_t m_inputRev; 
      uint32_t m_outputRev; 
      int m_ticks = 0; 
      std::map<std::string,uint32_t> m_inMap; 
      std::map<std::string,uint32_t> m_outMap; 
      void createClient(); 
      void openMemory(); 
      void closeMemory(); 
      void destroyClient(); 
  };
}