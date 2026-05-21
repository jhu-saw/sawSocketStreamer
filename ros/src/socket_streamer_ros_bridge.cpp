/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Anton Deguet
  Created on: 2026-05-21

  (C) Copyright 2026 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <iostream>

#include <cisstCommon/cmnCommandLineOptions.h>
#include <cisstCommon/cmnPath.h>
#include <cisstCommon/cmnUnits.h>
#include <cisstMultiTask/mtsManagerLocal.h>

#include <cisst_ros_bridge/cisst_ral.h>
#include <cisst_ros_crtk/mts_ros_crtk_bridge_required.h>
#include <sawSocketStreamer/mtsSocketStreamer.h>

int main(int argc, char ** argv)
{
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cerr, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);

    cisst_ral::ral ral(argc, argv, "socket_streamer_ros_bridge");
    cisst_ral::node_ptr_t rosNode = ral.node();

    cmnCommandLineOptions options;
    std::string jsonConfigFile;
    std::string componentName = "socket_streamer";
    std::string rosNamespace;
    double streamerPeriod = 10.0 * cmn_ms;
    double bridgePeriod = 0.1 * cmn_ms;

    options.AddOptionOneValue("j", "json-config",
                              "sawSocketStreamer JSON configuration file",
                              cmnCommandLineOptions::REQUIRED_OPTION, &jsonConfigFile);
    options.AddOptionOneValue("n", "name",
                              "sawSocketStreamer component name",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &componentName);
    options.AddOptionOneValue("N", "ros-namespace",
                              "ROS namespace for generated CRTK topics",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &rosNamespace);
    options.AddOptionOneValue("p", "period",
                              "sawSocketStreamer period in seconds",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &streamerPeriod);
    options.AddOptionOneValue("P", "bridge-period",
                              "CRTK ROS bridge period in seconds",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &bridgePeriod);

    if (!options.Parse(ral.stripped_arguments(), std::cerr)) {
        return -1;
    }

    if (!cmnPath::Exists(jsonConfigFile)) {
        std::cerr << "File not found: sawSocketStreamer JSON configuration file: "
                  << jsonConfigFile << std::endl;
        return -1;
    }

    std::string arguments;
    options.PrintParsedArguments(arguments);
    std::cout << "Options provided:" << std::endl << arguments;

    mtsManagerLocal * componentManager = mtsManagerLocal::GetInstance();

    mtsSocketStreamer * socketStreamer = new mtsSocketStreamer(componentName, streamerPeriod);
    componentManager->AddComponent(socketStreamer);
    socketStreamer->Configure(jsonConfigFile);

    mts_ros_crtk_bridge_required * crtkBridge
        = new mts_ros_crtk_bridge_required(componentName + "_ros_bridge",
                                           rosNode,
                                           bridgePeriod);
    componentManager->AddComponent(crtkBridge);
    crtkBridge->bridge_interface_required(socketStreamer->GetName(),
                                          "Required",
                                          rosNamespace);
    crtkBridge->Connect();

    componentManager->CreateAllAndWait(2.0 * cmn_s);
    componentManager->StartAllAndWait(2.0 * cmn_s);

    std::cout << "Hit Ctrl-c to quit" << std::endl;
    cisst_ral::spin(rosNode);

    componentManager->KillAllAndWait(2.0 * cmn_s);
    componentManager->Cleanup();

    delete crtkBridge;
    delete socketStreamer;

    cmnLogger::Kill();
    cisst_ral::shutdown();

    return 0;
}
