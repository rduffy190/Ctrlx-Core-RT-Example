#include "rt_application.h"
#include "trace.h"

namespace Example{
common::scheduler::SchedEventResponse RTApplication::execute(const common::scheduler::SchedEventType& eventType,
                                                            const common::scheduler::SchedEventPhase& eventPhase,
                                                            comm::datalayer::Variant& param)
{
  switch (eventType)
  {
  //if(eventType == common::scheduler::SchedEventType::SCHED_EVENT_TICK)
  case common::scheduler::SchedEventType::SCHED_EVENT_TICK:
  {
    u_int8_t* inData; 
    u_int8_t* outData; 
    auto result = m_inputs->beginAccess(inData, m_inputRev); 
    if(result == DL_OK)
    {
      EtherCATUpdate::AT(inData, m_inMap);
    } 
    else
    {
      TRACE_MSG("Failed to open the input data!")
    } 
    m_inputs->endAccess(); 
    result = m_outputs->beginAccess(outData, m_outputRev);
    if(result == comm::datalayer::DlResult::DL_OK)
      { 
        EtherCATUpdate::MDT(outData, m_outMap);
      }
    else
    {
      TRACE_MSG("Failed to open the output data!")
    }  
    m_outputs->endAccess(); 
    return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
  }

  case common::scheduler::SchedEventType::SCHED_EVENT_SWITCH_TO_SERVICE:
  {
    m_outputs->endAccess();
    m_inputs->endAccess(); 
    return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
  }
  }
}
void RTApplication::setDatalyer(comm::datalayer::IDataLayerFactory3* datalayerFactory){
  m_datalayer = datalayerFactory; 
  createClient(); 
  openMemory(); 
}

void RTApplication::resetDataLayer(){
  closeMemory(); 
  destroyClient(); 
  m_datalayer = nullptr; 
}

void RTApplication::createClient(){
  m_client = m_datalayer->createClient3(DL_IPC_AUTO);
}

void RTApplication::openMemory(){
  if(m_client){
    comm::datalayer::Variant dlMap; 
    auto result = m_client->readSync("fieldbuses/ethercat/master/instances/ethercatmaster/realtime_data/input/map",&dlMap); 
    auto varMap = comm::datalayer::GetMemoryMap(dlMap.getData());
    m_inputRev = varMap->revision(); 
    for(auto variables = varMap->variables()->begin(); variables!= varMap->variables()->end(); variables++){
      m_inMap[variables->name()->str()] = variables->bitoffset(); 
    }
    result = m_datalayer->openMemory(m_inputs,"fieldbuses/ethercat/master/instances/ethercatmaster/realtime_data/input"); 

    result = m_client->readSync("fieldbuses/ethercat/master/instances/ethercatmaster/realtime_data/output/map",&dlMap); 
    varMap = comm::datalayer::GetMemoryMap(dlMap.getData());
    m_outputRev = varMap->revision(); 
    for(auto variables = varMap->variables()->begin(); variables!= varMap->variables()->end(); variables++){
      m_outMap[variables->name()->str()] = variables->bitoffset(); 
    }
    result = m_datalayer->openMemory(m_outputs,"fieldbuses/ethercat/master/instances/ethercatmaster/realtime_data/output");
  }
}

void RTApplication::closeMemory(){
  if(m_inputs){
    m_datalayer->closeMemory(m_inputs); 
    m_inputs = nullptr; 
  }
  if(m_outputs){
    m_datalayer->closeMemory(m_outputs); 
    m_outputs = nullptr;
  }
}

void RTApplication::destroyClient(){
  if(m_client)
    delete m_client; 
}


}