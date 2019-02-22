/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Peter Kazanzides, Anton Deguet
  Created on: 2013-12-02

  (C) Copyright 2013-2019 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <sawSocketStreamer/mtsSocketStreamer.h>
#include <cisstMultiTask/mtsInterfaceProvided.h>
#include <cisstMultiTask/mtsInterfaceRequired.h>
#include <cisstParameterTypes/prmPositionCartesianGet.h>

CMN_IMPLEMENT_SERVICES_DERIVED_ONEARG(mtsSocketStreamer, mtsTaskPeriodic, mtsTaskPeriodicConstructorArg);

void mtsSocketStreamer::Init(void)
{
    SocketConfigured = false;

    mtsInterfaceProvided * provided = AddInterfaceProvided("Configuration");
    if (provided) {
        provided->AddCommandWrite(&mtsSocketStreamer::SetDestination, this, "SetDestination");
    }
    mtsInterfaceRequired * required = AddInterfaceRequired("Robot");
    if (required) {
        required->AddFunction("GetPositionCartesian", GetPositionCartesian);
    }

    SetDestination("127.0.0.1:8051");
}

mtsSocketStreamer::~mtsSocketStreamer()
{
}

void mtsSocketStreamer::Configure(const std::string &ipPort)
{
    SetDestination(ipPort);
}

void mtsSocketStreamer::Startup(void)
{
}

void mtsSocketStreamer::Run(void)
{
    ProcessQueuedCommands();
    ProcessQueuedEvents();

    if (SocketConfigured) {
        // Packet format (9 doubles): buttons (clutch, coag), gripper, x, y, z, q0, qx, qy, qz
        // For the buttons: 0=None, 1=Clutch, 2=Coag, 3=Both
        double packet[9];

        prmPositionCartesianGet posCart;
        GetPositionCartesian(posCart);

        
        /*
        vct3 pos = posCart.Position().Translation();
        packet[2] = pos.X();
        packet[3] = pos.Y();
        packet[4] = pos.Z();

        std::cerr << posCart << std::endl;

        vctQuatRot3 qrot(posCart.Position().Rotation(), VCT_NORMALIZE);
        packet[5] = qrot.W();
        packet[6] = qrot.X();
        packet[7] = qrot.Y();
        packet[8] = qrot.Z();

        Socket.Send((char *)packet, sizeof(packet));
        */
        Json::Value jsonToSend;
        cmnDataJSON<prmPositionCartesianGet>::SerializeText(posCart, jsonToSend);
        Json::FastWriter fastWriter;
        std::string output = fastWriter.write(jsonToSend);
        Socket.Send(output.c_str(), output.size());
    }
}

void mtsSocketStreamer::Cleanup(void)
{
    Socket.Close();
}

void mtsSocketStreamer::SetDestination(const std::string &ipPort)
{
    size_t colon = ipPort.find(':');
    if (colon == std::string::npos) {
        CMN_LOG_CLASS_RUN_ERROR << "SetDestination: invalid address:port " << ipPort << std::endl;
    } else {
        unsigned short port;
        if ((sscanf(ipPort.c_str() + colon + 1, "%hu", &port) != 1)) {
            CMN_LOG_CLASS_RUN_ERROR << "SetDestination: invalid port " << ipPort << std::endl;

        } else {
            Socket.SetDestination(ipPort.substr(0, colon), port);
            SocketConfigured = true;
        }
    }
}
