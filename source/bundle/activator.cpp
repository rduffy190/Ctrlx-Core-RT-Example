#include "celix/BundleActivator.h"
#include <iostream>
#include "cmp.h"
#include "../impl/Logger.h"
class ExampleActivator{
  public:
    explicit ExampleActivator(const std::shared_ptr<celix::BundleContext>& ctx){
      std::cout<<"Activator Called" << std::endl; 
      auto cmp = std::make_unique<ExampleComponent>(); 
      auto& component =  ctx->getDependencyManager()->createComponent(std::move(cmp),"ExampleComponent");
      component.setCallbacks(&ExampleComponent::init,
                             &ExampleComponent::start,
                             &ExampleComponent::stop, 
                             &ExampleComponent::deInit); 
      component.createServiceDependency<common::scheduler::IScheduler3>(ICOMMON_SCHEDULER_CPP_INTERFACE_NAME)
               .setVersionRange(ICOMMON_SCHEDULER_CPP_CONSUMER_RANGE)
               .setRequired(true)
               .setStrategy(celix::dm::DependencyUpdateStrategy::suspend)
               .setCallbacks(&ExampleComponent::setSchedularService); 
      component.createServiceDependency<comm::datalayer::IDataLayerFactory3>(IDATALAYER_FACTORY_INTERFACE_NAME3)
               .setVersionRange(IDATALAYER_CONSUMER_RANGE)
               .setRequired(true)
               .setStrategy(celix::dm::DependencyUpdateStrategy::suspend)
               .setCallbacks(&ExampleComponent::setDatalayerService); 
      LOG_INFO("OPTIONAL service common::log::trace::IRegistrationRealTime3");
      component.createServiceDependency<common::log::trace::IRegistrationRealTime3>(
                common::log::trace::CPP_INTERFACE_TRACE_REGISTRATION_REAL_TIME_3_NAME)
                .setVersionRange(common::log::trace::CPP_INTERFACE_TRACE_REGISTRATION_REAL_TIME_3_CONSUMER_RANGE)
                .setRequired(true)
                .setStrategy(DependencyUpdateStrategy::suspend)
                .setCallbacks(&ExampleComponent::traceServiceAdded, &ExampleComponent::traceServiceRemoved);
      component.build(); 
                       
    }
    ~ExampleActivator()noexcept{
      std::cout <<"Activator destroyed"<< std::endl; 
    }
};
CELIX_GEN_CXX_BUNDLE_ACTIVATOR(ExampleActivator)