
#include <iostream>
#include "celix/BundleActivator.h"
#include "trace/trace.h"
#include "realtime_template/realtime_template.h"

namespace sdk_rt
{
    class BundleActivator
    {
    public:
        explicit BundleActivator(const std::shared_ptr<DependencyManager> &mng)
        {
            auto trace = Trace::getInstance();
            trace->setSnapName("sdk-rt-di-do");
            trace->setTraceName("sdk.rt.di-do");
            trace->setEntity("sdk.rt.di-do");

            TRACE_INFO(" ");

            auto &realTimeTemplate =
                mng->createComponent<RealTimeTemplate>()
                    .setCallbacks(&RealTimeTemplate::init, &RealTimeTemplate::start, &RealTimeTemplate::stop, &RealTimeTemplate::deinit);

            // Important hint:
            // To work with the EtherCAT Master set next lines under comment.
            // See also RealTimeTemplateCallableFactory::createCallable

            TRACE_INFO("REQUIRED service common::scheduler::IScheduler3");
            realTimeTemplate.createServiceDependency<common::scheduler::IScheduler3>(COMMON_SCHED_CPP_INTERFACE_NAME)
                .setVersionRange(COMMON_SCHED_CPP_INTERFACE_DEFAULT_CONSUMER_RANGE)
                .setRequired(true)
                .setStrategy(DependencyUpdateStrategy::suspend)
                .setCallbacks(&RealTimeTemplate::schedulerServiceStateChanged)
                .setCallbacks(&RealTimeTemplate::schedulerServiceAdded, &RealTimeTemplate::schedulerServiceRemoved);

            TRACE_INFO("REQUIRED service comm::datalayer::IDataLayerFactory");
            realTimeTemplate.createServiceDependency<comm::datalayer::IDataLayerFactory>()
                .setVersionRange(IDATALAYER_CONSUMER_RANGE)
                .setRequired(true)
                .setStrategy(DependencyUpdateStrategy::suspend)
                .setName(IDATALAYER_FACTORY_INTERFACE_NAME)
                .setCallbacks(&RealTimeTemplate::datalayerServiceStateChanged)
                .setCallbacks(&RealTimeTemplate::datalayerServiceAdded, &RealTimeTemplate::datalayerServiceRemoved);

            TRACE_INFO("OPTIONAL service common::log::trace::IRegistrationRealTime3");
            realTimeTemplate.createServiceDependency<common::log::trace::IRegistrationRealTime3>(common::log::trace::CPP_INTERFACE_TRACE_REGISTRATION_REAL_TIME_3_NAME)
                .setVersionRange(common::log::trace::CPP_INTERFACE_TRACE_REGISTRATION_REAL_TIME_3_CONSUMER_RANGE)
                .setRequired(true)
                .setStrategy(DependencyUpdateStrategy::suspend)
                .setCallbacks(&RealTimeTemplate::traceServiceStateChanged)
                .setCallbacks(&RealTimeTemplate::traceServiceAdded, &RealTimeTemplate::traceServiceRemoved);
            realTimeTemplate.build(); 
        }
    };

    CELIX_GEN_CXX_BUNDLE_ACTIVATOR(BundleActivator)
}
