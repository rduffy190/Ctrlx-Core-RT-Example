# Helper Class to use Trace

## Introduction

The ctrlX Trace function writes logs to systemd using journald. Because this can be very slow two kinds of logging via journald are provided:

1. A direct logging to journald - this can be slow.
2. Using the Trace service - this is very fast, because messages are written into a queue which is emptied in a separate low prior thread.

Explanation:

* An app must use direct logging when the ctrlX Trace service is not yet available, e.g. in the BundleActivator.
* To avoid a watch dog error in the method __execute()__ called by the Scheduler the __Trace service has to be used__. 

The helper class Trace manages these two usages automatically.

## Usage of the helper class Trace within your project

### CMakeLists.txt entries

Add these entries to your CMakeLists.txt file:

libcommon_log_trace_buffered_static.a implements the direct logging, add this library:

	target_link_libraries(${ACTIVATOR_LIB}
	  -Wl,--no-whole-archive ${CELIX_LIBRARIES}
	  ${SDK_ROOT_DIR}/lib/common.log.trace/${TARGET_PLATFORM}/libcommon_log_trace_buffered_static.a
	  systemd # required by libcommon_log_trace_buffered_static.a
	)

Add the subdirectory to your include directories:

    target_include_directories(${ACTIVATOR_LIB}
    ...
    PRIVATE ${SDK_ROOT_DIR}/src/trace
    )

And the source code to your project:

    add_library(${ACTIVATOR_LIB} SHARED
    ...
    ${SDK_ROOT_DIR}/src/trace/Trace.cpp
    )


The class Trace provides two categories of trace macros, both are writing their outputs to journald - but in in different ways.

## Trace Categories

### Real-Time Trace

The above mentioned class provides the macro category __RT_TRACE_...__.

__Use this category for logging in time-critical code sections.__

Log output is written into a queue, this is very fast. A thread with low priority reads the logs from there and writes them to journald.

!!! important
    __These macro category requires the Trace service! If this service is not available, no logs are made.__

__Hint:__

To get the log service use the service dependency ´common::log::trace::IRegistrationRealTime3´. See /home/samples-cpp/di-do/src/BundleActivator.cpp.

### None-Real-Time Trace

Use the category __TRACE_...__ for logging in NONE time-critical code sections.

__The advantage of this category is, that it is independend of the log service, but it is slow and can even block the code execution (for a short time).__

In the sample project dl-shm-owner only this category is used because there are no time-critical code sections.

## Trace Levels 

Each category has three trace levels:

1. __MSG__ (Info): Used to log normal control flow (states, function calls, results,...)
2. __WARN__ Used to log unusual events that do NOT affect the intended goal (function). The user should consider these events.
3. __ERR__: Used to log unusual events that cause an abort of the control flow. The intended goal (function) was NOT achieved. The user must consider these events.

See example dl-shm-owner, src/shm/ShmOwnerComponent.cpp:

    auto result = m_datalayerProviderPtr->start();
    if (comm::datalayer::STATUS_FAILED(result))
    {
      TRACE_ERR("Starting Data Layer provider failed.");
      return;
    }

    TRACE_MSG("Starting Data Layer provider succeeded.");
  
