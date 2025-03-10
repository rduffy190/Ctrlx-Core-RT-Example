# README sdk-example-rt

This code provides the boilerplate code required for a realtime application with access to the inputs and outputs of the EtherCAT bus. 

## Setup

### Installation of the App Build Environment

__To develop ctrlX Real-Time Apps we recommend to use a ctrlX App Build Environment.__ 

How to create and start a ctrlX App Build Environment is described in [ctrlX WORKS App Build Environment](https://boschrexroth.github.io/ctrlx-automation-sdk/setup_qemu_ctrlx_works.html)

## Documentation of the ctrlX AUTOMATION RT SDK

The documentation of the ctrlX AUTOMATION RT SDK is provided as a set of HTML pages contained in the RT SDK zip archive. 

You can open this documentation in several ways:

__The easiest way: On your (Windows) host system:__

* Unzip the zip archive into a directory e.g. c:\sdk.control.rt
* Open the folder c:\sdk.control.rt\public\site
* Double click the file index.html

Your web browser on your host system will show the [start page of the RT SDK documentation](file:///C:/sdk.control.rt/public/site/index.html)

__On your App Build Environment (this includes the installation of the RT SDK):__

We recommend to use Visual Studio Code with the extensions Remote SSH, CMake, CMake Tools, C/C++ extensions and __Live Server__.

* Start your App Build Environment using ctrlX WORKS.
* Start Visual Studio Code on your host system, connect via Remote SSH to your App Build Environment.
* Drag and drop the RT SDK zip archive from your host system into the file explorer window of Visual Studio Code.
* Copy the setup.sh from these files in the same folder.
* Open an integrated terminal window in Visual Studio Code run: `./setup.sh`

__With these steps the RT SDK is already installed.__

# Usage 

## Real-Time application

This real-time sample provides the essential framework for your application to run on the scheduler while consuming and prodiving data to the EtherCAT master. 

### Implementation Details

In the rt_application.cpp the __SCHED_EVENT_TICK__ indicates that the scheduler has called the application to run and the code will call the EtherCATUpdates functions in the __User__ folder. The _MDT_ function will provide data to the drives from the application while the _AT_ funtion will bring the feedback. 

The data is provided in 1 byte sections and indexed based on the name provided in the ctlrX IO configuraiton tool and also shown on the ctrlX datalayer map under the EtherCAT fieldbus instance. 

the __std::memcpy__ copy function can be used to copy the data into the MDT message and pointers can be used to retrieve the data. 

For example, providing the command velocity in the  MDT could be done as follows

```cpp
int32_t Velocity = 300000;
std::memcpy(&outData[m_outMap["Axis1/MDT.VelocityCommand"]/8], &Velocity, 4);
 ```

This will write a velocity of 30.0000 to the drive parameter S-0-0040. 

Likewise, data can be read from the AT by defining a pointer with the appropriate size and returning the data from the AT message. For example, 

```cpp
int16_t* StatusWord;
StatusWord = (int16_t*)(&inData[m_inMap["Axis1/AT.Drive_status_word"]/8]); 
 ```

### Coding Rules for the Event Tick Handling

* Avoid "run time expensive" actions e.g. file handling, connection handling, std::cout usage,...
* If possible prepare data in the none real-time part of your code.


## Trace
The ctrlX Trace function writes logs to systemd using journald. Because this can be very slow two kinds of logging via journald are provided:

1. A direct logging to journald - this can be slow.
2. Using the Trace service - this is very fast, because messages are written into a queue which is emptied in a separate low prior thread.

Explanation:

* An app must use direct logging when the ctrlX Trace service is not yet available, e.g. in the BundleActivator.
* To avoid a watch dog error in the method __execute()__ called by the Scheduler the __Trace service has to be used__.

The helper class Trace manages these two usages automatically.

### Usage of the helper class Trace within your project

#### __CMake entries__

Add these entries to your CMakeLists.txt file, see [CMakeLists.txt](../../../../samples-cpp/scheduler/src/CMakeLists.txt) of sample [scheduler](../../../../samples-cpp/scheduler/src/BundleActivator.cpp):

libcommon_log_trace_buffered_static.a implements the direct logging, add this library:

```cmake
target_link_libraries(${ACTIVATOR_LIB}
  -Wl,--no-whole-archive ${CELIX_LIBRARIES}
  ${SDK_ROOT_DIR}/lib/common.log.trace/${TARGET_PLATFORM}/libcommon_log_trace_buffered_static.a
  systemd # required by libcommon_log_trace_buffered_static.a
)
```

Add the subdirectory to your include directories:

```cmake
target_include_directories(${ACTIVATOR_LIB}
...
  PRIVATE ${SDK_ROOT_DIR}/include/common.log.trace 
  PRIVATE ${SDK_ROOT_DIR}/src/common.log.trace 
)
```

And the source code to your project:

```cmake
add_library(${ACTIVATOR_LIB} SHARED
...
  trace/Trace.cpp
  ${SDK_ROOT_DIR}/src/common.log.trace/trace_itf_wrapper.cpp
)
```

The class Trace uses the macros provided by the trace interface wrapper classes:

[trace_itf_wrapper.h](../../../../src/common.log.trace/trace_itf_wrapper.h)
[trace_itf_wrapper.cpp](../../../../src/common.log.trace/trace_itf_wrapper.cpp)

See also the usage of the macros provided by the template classes:

[trace_itf_wrapper_template.h](../../../../src/common.log.trace/trace_itf_wrapper_template.h)
[trace_itf_wrapper_template.cpp](../../../../src/common.log.trace/trace_itf_wrapper_template.cpp)

### Trace Categories

#### __Real-Time Trace__

The above mentioned real-time trace macros

* TRACE_INFO
* TRACE_WARNING
* TRACE_ERROR

are exemplary declared and defined in the helper class:

[Trace.h](Trace.h)
[Trace.cpp](Trace.cpp)

Please feel free to adapt this classes to your needs.

__Use this category for logging in time-critical code sections.__

Log output is written into a queue, this is very fast. A thread with low priority reads the logs from there and writes them to journald.

!!! important
    __These macro category requires the Trace service! If this service is not available, no logs are made.__

__Hint:__

To get the log service use the service dependency

```cpp
common::log::trace::IRegistrationRealTime3
```

see [BundleActivator.cpp](../../../../samples-cpp/scheduler/src/BundleActivator.cpp) of sample [scheduler](../../../../samples-cpp/scheduler/src/BundleActivator.cpp)

#### __None-Real-Time Trace__

Please use for local debugging in the app build environment the wrapper macros

* LOG_INFO
* LOG_WARNING
* LOG_ERROR

implemented in the helper class:

[Logger.h](Logger.h)

This macro redirects all messages to *stdout* in the app build environment in addition to *journald*.

!!! important
    The LOG macros redirects the messages only to stdout if the environment variable __SNAP_ENV__ is defined with __DEBUG__

The trace logs will be displayed in the terminal during local debugging like the following output of dl-shm-owner example:

```bash
2024-07-31 06:27:46 [INFO] src/BundleActivator.cpp(20): ShmOwnerBundleActivator()  
2024-07-31 06:27:46 [INFO] src/BundleActivator.cpp(30): ShmOwnerBundleActivator() Creating ServiceDependency to comm::datalayer::IDataLayerFactory
-> mount: /tmp/dl-shm-owner/snap-common/nvram: mount point does not exist.
2024-07-31 06:27:48 [INFO] src/shm/ShmOwner.cpp(238): datalayerServiceStateChanged()  
2024-07-31 06:27:48 [INFO] src/shm/ShmOwner.cpp(230): datalayerServiceAdded()  
2024-07-31 06:27:48 [INFO] src/shm/ShmOwner.cpp(170): init()  
2024-07-31 06:27:48 [INFO] src/shm/ShmOwner.cpp(184): start()  
...
```

#### __Log Levels__

Each category has three log levels:

1. __INFO__: Used to log normal control flow (states, function calls, results,...)
2. __WARNING__: Used to log unusual events that do NOT affect the intended goal (function). The user should consider these events.
3. __ERROR__: Used to log unusual events that cause an abort of the control flow. The intended goal (function) was NOT achieved. The user must consider these events.

See the example [dl-shm-owner](../../../../samples-cpp/dl-shm-owner/src/shm/ShmOwner.cpp):

```cpp
    auto result = m_datalayerProviderPtr->start();
    if (comm::datalayer::STATUS_FAILED(result))
    {
      LOG_ERROR("Starting Data Layer provider failed.");
      return;
    }

    LOG_INFO("Starting Data Layer provider succeeded.");
 ```

## License

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.