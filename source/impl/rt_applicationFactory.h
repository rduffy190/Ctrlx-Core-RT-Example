#pragma once
#include "comm/datalayer/datalayer.h"
#include "common/scheduler/i_scheduler3.h"
#include "rt_application.h"

namespace Example{
  class RTApplicationFactory :public common::scheduler::ICallableFactory2
  {
    public: 
      std::shared_ptr<common::scheduler::ICallable> createCallable(const comm::datalayer::Variant& param);
      common::scheduler::SchedStatus destroyCallable(const std::shared_ptr<common::scheduler::ICallable>& callable);
      common::scheduler::SchedStatus getCallableArguments(std::vector<std::string>& arguments);
      void getFactoryDescription(std::vector<std::string>& description) const;
      void getCallableConfigurations(comm::datalayer::Variant& configurations) const;
      void setDataLayer(comm::datalayer::IDataLayerFactory3* dataLayer); 
      void resetDataLayer(); 

    private: 
      std::shared_ptr<RTApplication> m_application = std::make_shared<RTApplication>(); 
      comm::datalayer::IDataLayerFactory3* m_dataLayer; 
  };
}