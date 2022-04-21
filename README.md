# sawSocketStreamer

This SAW component allows to stream data from most cisst/SAW components with little or no new code.  It compiles on Windows, Linux and likely MacOS.  It has been tested with:
  * Linux
  * Streaming `prmStateJoint` data from sawIntuitiveResearchKit and sawIntuitiveDaVinci

It current supports UDP sockets and the data is serialized using JSON
format.  It is used to stream data from the different da Vinci robots
(at JHU) to HoloLens displays.  On the HoloLens side, one can use the
open source package [dvrk-xr](https://github.com/jhu-dvrk/dvrk-xr).

Based on user requests, we could add:
  * TCP support
  * Different serialization formats (plain text, binary...)

# Links
 * License: http://github.com/jhu-cisst/cisst/blob/master/license.txt
 * JHU-LCSR software: http://jhu-lcsr.github.io/software/

# Dependencies
 * cisst libraries: https://github.com/jhu-cisst/cisst

# Using the component

## Adding the component

One can create and add the `mtsSocketStreamer` component manually in
your `main` C function but we strongly recommend using the
`cisstMultiTask` manager ability to load a configuration file to add
and connect components.  The dVRK main programs provide this options.  See https://github.com/jhu-dvrk/sawIntuitiveResearchKit/blob/master/applications/mainQtConsoleJSON.cpp.

There is first a command line option to specify one or more configuration files for the component manager:
```cpp
    cmnCommandLineOptions options;
    std::list<std::string> managerConfig;
    options.AddOptionMultipleValues("m", "component-manager",
                                    "JSON files to configure component manager",
                                    cmnCommandLineOptions::OPTIONAL_OPTION, &managerConfig);
```

Then one has to use the configuration files to configure the component manager:
```cpp
    mtsManagerLocal * componentManager = mtsManagerLocal::GetInstance();
    if (!componentManager->ConfigureJSON(managerConfig)) {
        CMN_LOG_INIT_ERROR << "Configure: failed to configure component manager, check cisstLog for error messages" << std::endl;
        return -1;
    }
```

The configuration files for the component manager will look like (more examples can be found at https://github.com/jhu-dvrk/sawIntuitiveResearchKit/tree/master/share/socket-streamer):
```json
/* -*- Mode: Javascript; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
{
    "components":
    [
        {
            "shared-library": "sawSocketStreamer",
            "class-name": "mtsSocketStreamer",
            "constructor-arg": {
                "Name": "streamerMTML",
                "Period": 0.01
            },
            "configure-parameter": "streamerMTML.json"
        }
        ,
        {
            "shared-library": "sawSocketStreamer",
            "class-name": "mtsSocketStreamer",
            "constructor-arg": {
                "Name": "streamerMTMR",
                "Period": 0.01
            },
            "configure-parameter": "streamerMTMR.json"
        }
    ]
    ,
    "connections":
    [
        {
            "required": {
                "component": "streamerMTML",
                "interface": "Required"
            }
            ,
            "provided": {
                "component": "MTML",
                "interface": "Robot"
            }
        }
        ,
        {
            "required": {
                "component": "streamerMTMR",
                "interface": "Required"
            }
            ,
            "provided": {
                "component": "MTMR",
                "interface": "Robot"
            }
        }
    ]
}

```

## Component configuration file

Each component created need a configuration file that specifies which read command to use to retrieve the data as well as the data type.  For example:
```json
/* -*- Mode: Javascript; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
{
    "ip": "10.194.86.119",
    "port": "48054",
    "data": [
        {
            "name": "measured_js",
            "type": "prmStateJoint"
        }
    ]
}
```

## Testing the streamer

To test the streamer, you can use the `nc` tool on Linux.   The main options are `l` to listen and `u` for UDP protocol.  Then you need to add the IP address and port.   With the example above, try:
```sh
nc -lu 10.194.86.119 48054
```
At that point you should see a continous stream of text in JSON format.
