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
    InterfaceRequired = AddInterfaceRequired("Required");
}

mtsSocketStreamer::~mtsSocketStreamer()
{
}

void mtsSocketStreamer::Configure(const std::string & filename)
{
    if (!cmnPath::Exists(filename)) {
        CMN_LOG_CLASS_INIT_ERROR << "Configure: " << filename
                                 << " not found!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // open json file
    std::ifstream jsonStream;
    jsonStream.open(filename.c_str());
    Json::Value jsonConfig;
    Json::Reader jsonReader;
    if (!jsonReader.parse(jsonStream, jsonConfig)) {
        CMN_LOG_CLASS_INIT_ERROR << "Configure: failed to parse configuration" << std::endl
                                 << "File: " << filename << std::endl << "Error(s):" << std::endl
                                 << jsonReader.getFormattedErrorMessages();
        exit(EXIT_FAILURE);
    }

    // look for IP and port
    bool ipPortFound = true;
    std::string ipPort;
    Json::Value jsonValue = jsonConfig["ip"];
    if (jsonValue.empty()) {
        ipPortFound = false;
    } else {
        ipPort = jsonValue.asString();
        ipPort.append(":");
        // now look for port
        jsonValue = jsonConfig["port"];
        if (jsonValue.empty()) {
            ipPortFound = false;
        } else {
            ipPort.append(jsonValue.asString());
        }
    }
    if (ipPortFound) {
        SetDestination(ipPort);
    }

    // look for data
    Json::Value jsonDataArray = jsonConfig["data"];
    for (unsigned int index = 0; index < jsonDataArray.size(); ++index) {
        Json::Value jsonData = jsonDataArray[index];
        // look for name and type
        std::string name, type;
        name = jsonData["name"].asString();
        type = jsonData["type"].asString();
        if (name.empty() || type.empty()) {
            CMN_LOG_CLASS_INIT_ERROR << "Configure: all data fields must contain a \"name\" and \"type\" in file "
                                     << filename << std::endl;
            exit(EXIT_FAILURE);
        }
        // make sure we can create a placeholder for the data type
        cmnGenericObject * baseObject;
        baseObject = cmnClassRegister::Create(type);
        if (!baseObject) {
            CMN_LOG_CLASS_INIT_ERROR << "Configure: unable to create an object of type \""
                                     << type << "\"" << std::endl;
            exit(EXIT_FAILURE);
        }
        mtsGenericObject * object = dynamic_cast<mtsGenericObject *>(baseObject);
        if (!object) {
            CMN_LOG_CLASS_INIT_ERROR << "Configure: object of type \""
                                     << type << "\" is not derived from mtsGenericObject" << std::endl;
            exit(EXIT_FAILURE);
        }
        
        // now, create a placeholder for the data
        DataStruct & data = DataMap[name];
        InterfaceRequired->AddFunction(name, data.Function);
        data.Data = object;
    }
}

void mtsSocketStreamer::Startup(void)
{
    if (!SocketConfigured) {
        CMN_LOG_CLASS_INIT_ERROR << "Startup: port not configured for " << this->GetName() << std::endl;
    }
}

void mtsSocketStreamer::Run(void)
{
    ProcessQueuedCommands();
    ProcessQueuedEvents();

    if (SocketConfigured) {
        Json::Value jsonToSend;
        DataMapType::iterator iter;
        const DataMapType::iterator end = DataMap.end();
        for (iter = DataMap.begin();
             iter != end;
             ++iter) {
            mtsExecutionResult result;
            result = iter->second.Function(*(iter->second.Data));
            if (result.IsOK()) {
                iter->second.Data->SerializeTextJSON(jsonToSend[iter->first]);
            } else {
                CMN_LOG_CLASS_RUN_ERROR << "Run: component " << this->GetName() << " run into error "
                                        << result << " while calling " << iter->first << std::endl;
            }
        }
        std::string output = FastWriter.write(jsonToSend);
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
