#include "cmp.h"
void ExampleComponent::init(){
}
void ExampleComponent::start(){
  m_appFactory->setDataLayer(m_dataLayer); 
  m_schedular->registerCallableFactory(m_appFactory, "Example RT App"); 
}
void ExampleComponent::stop() {
  m_appFactory->resetDataLayer(); 
  m_schedular->unregisterCallableFactory(m_appFactory, true); 
  automation::scheduler::g_rt_trace.detachTrace(m_log); 
}
void ExampleComponent::deInit(){
      std::cout<<"deInit" << std::endl; 
}
void ExampleComponent::setSchedularService(common::scheduler::IScheduler3* schedular){
  m_schedular = schedular; 
}
void ExampleComponent::setDatalayerService(comm::datalayer::IDataLayerFactory3* dataLyaer){
  m_dataLayer = dataLyaer; 
}

void ExampleComponent::setRTTraceService(common::log::trace::IRegistrationRealTime2* log){
  std::cout<<"Got Trace"<<std::endl; 
  automation::scheduler::g_rt_trace.attachTrace(log); 
  m_log = log; 
}
