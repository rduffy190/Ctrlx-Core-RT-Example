#include "rt_application.h"
#include "trace.h"

namespace Example{
common::scheduler::SchedEventResponse RTApplication::execute(const common::scheduler::SchedEventType& eventType,
                                                            const common::scheduler::SchedEventPhase& eventPhase,
                                                            comm::datalayer::Variant& param)
{
  if(eventType == common::scheduler::SchedEventType::SCHED_EVENT_TICK){
    m_ticks++; 
    m_ticks = m_ticks%500; 
  }
  if(m_ticks == 0 && m_inputs && m_outputs){
    TRACE_MSG("500 TICKS"); 
    u_int8_t* inData; 
    u_int8_t* outData; 
    auto result = m_inputs->beginAccess(inData,m_inputRev); 
    if(result == DL_OK){
      TickUpdate::sampleInputProcessing(inData, m_inMap);
    }
    m_inputs->endAccess(); 
    result = m_outputs->beginAccess(outData,m_outputRev);
    if(result == comm::datalayer::DlResult::DL_OK){ 
      TRACE_MSG("Writing"); 
      TickUpdate::sampleOutputProcessing(outData, m_outMap);
    }
    m_outputs->endAccess(); 
  }
    return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
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