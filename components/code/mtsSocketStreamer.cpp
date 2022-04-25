/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Peter Kazanzides, Anton Deguet
  Created on: 2013-12-02

  (C) Copyright 2013-2022 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <sawSocketStreamer/mtsSocketStreamer.h>
#include <cisstCommon/cmnUnits.h>
#include <cisstCommon/cmnPath.h>
#include <cisstMultiTask/mtsInterfaceProvided.h>
#include <cisstMultiTask/mtsInterfaceRequired.h>
#include <cisstParameterTypes/prmPositionCartesianGet.h>

CMN_IMPLEMENT_SERVICES_DERIVED_ONEARG(mtsSocketStreamer, mtsTaskPeriodic, mtsTaskPeriodicConstructorArg);

void mtsSocketStreamer::EventWriteStruct::Callback(const mtsGenericObject & payload)
{
    if (Streamer && Streamer->mSocketConfigured) {
        Json::Value jsonToSend;
        payload.SerializeTextJSON(jsonToSend[Name]);
        std::string output = Streamer->mJSONWriter.write(jsonToSend);
        Streamer->mSocket.Send(output.c_str(), output.size());
    }
}

void mtsSocketStreamer::Init(void)
{
    mSocketConfigured = false;

    mtsInterfaceProvided * provided = AddInterfaceProvided("Configuration");
    if (provided) {
        provided->AddCommandWrite(&mtsSocketStreamer::SetDestination, this, "SetDestination");
    }
    mInterfaceRequired = AddInterfaceRequired("Required");
}

mtsSocketStreamer::~mtsSocketStreamer()
{
}

template <class _mapType>
void mtsSocketStreamer::ConfigureCommands(const Json::Value & jsonArray,
                                          _mapType & functionsMap,
                                          const std::string & commandsType,
                                          const std::string & filename)
{
    for (unsigned int index = 0; index < jsonArray.size(); ++index) {
        Json::Value jsonData = jsonArray[index];
        // look for name and type
        std::string name, type;
        name = jsonData["name"].asString();
        type = jsonData["type"].asString();
        if (name.empty() || type.empty()) {
            CMN_LOG_CLASS_INIT_ERROR << "Configure: all elements of \"" << commandsType
                                     << "\" must contain a \"name\" and \"type\" in file "
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
        auto & function = functionsMap[name];
        mInterfaceRequired->AddFunction(name, function.Function);
        function.Data = object;
    }
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

    // look for read commands
    Json::Value jsonReadCommandArray = jsonConfig["data"];
    if (jsonReadCommandArray.size() != 0) {
        CMN_LOG_CLASS_INIT_WARNING << "Configure: \"data\" is deprecated, use \"read-commands\" instead.  Please edit "
                                   << filename << std::endl;
    } else {
         jsonReadCommandArray = jsonConfig["read-commands"];
    }
    ConfigureCommands(jsonReadCommandArray, mReadFunctions, "read-commands", filename);

    // look for write commands
    Json::Value jsonWriteCommandArray = jsonConfig["write-commands"];
    ConfigureCommands(jsonWriteCommandArray, mWriteFunctions, "write-commands", filename);

     // look for write events
    Json::Value jsonWriteEventArray = jsonConfig["write-events"];
    for (const auto & jsonEvent : jsonWriteEventArray) {
        const std::string name = jsonEvent.asString();
        auto & event = mWriteEvents[name];
        event.Name = name;
        event.Streamer = this;
        mInterfaceRequired->AddEventHandlerWriteGeneric(&EventWriteStruct::Callback, &event, name);
    }
}

void mtsSocketStreamer::Startup(void)
{
    if (!mSocketConfigured) {
        CMN_LOG_CLASS_INIT_ERROR << "Startup: port not configured for " << this->GetName() << std::endl;
    }
}

void mtsSocketStreamer::Run(void)
{
    ProcessQueuedCommands();
    ProcessQueuedEvents();

    if (mSocketConfigured) {
        // sending data
        for (auto & readFunction: mReadFunctions) {
            mtsExecutionResult result;
            result = readFunction.second.Function(*(readFunction.second.Data));
            if (result.IsOK()) {
                Json::Value jsonToSend;
                readFunction.second.Data->SerializeTextJSON(jsonToSend[readFunction.first]);
                std::string output = mJSONWriter.write(jsonToSend);
                mSocket.Send(output.c_str(), output.size());
            } else {
                CMN_LOG_CLASS_RUN_ERROR << "Run: component " << this->GetName() << " run into error "
                                        << result << " while calling " << readFunction.first << std::endl;
            }
        }
        // receiving data
        char buffer[1024];
        int bytesRead = 0;
        Json::Reader jsonReader;
        Json::Value jsonReceived;
        do {
            bytesRead = mSocket.Receive(buffer, sizeof(buffer));
            if (bytesRead > 0) {
                buffer[bytesRead] = 0;
                // trying to parse the content as JSON
                if (!jsonReader.parse(buffer, jsonReceived)) {
                    CMN_LOG_CLASS_RUN_ERROR << "Run: failed to parse buffer: " << buffer << std::endl
                                            << "Error(s):" << std::endl
                                            << jsonReader.getFormattedErrorMessages()
                                            << std::endl;
                }
                // get the name of command from JSON
                const auto end = jsonReceived.end();
                for (auto received = jsonReceived.begin();
                     received != end;
                     ++received) {
                    auto commandName = received.key().asString();
                    auto writeFunction = mWriteFunctions.find(commandName);
                    if (writeFunction != mWriteFunctions.end()) {
                        bool gotException = false;
                        try {
                            writeFunction->second.Data->DeSerializeTextJSON(*received);
                        } catch (const std::exception & e) {
                            gotException = true;
                            CMN_LOG_CLASS_RUN_ERROR << "Run: failed to deserialize payload: " << *received
                                                    << std::endl << " for command \"" << commandName
                                                    << "\": " << e.what() << std::endl;
                        } catch (...) {
                            gotException = true;
                            CMN_LOG_CLASS_RUN_ERROR << "Run: failed to deserialize payload: " << *received
                                                    << std::endl << " for command \""
                                                    << commandName << "\", unkown exception" << std::endl;
                        }
                        if (!gotException) {
                            writeFunction->second.Function(*(writeFunction->second.Data));
                        }
                    } else {
                        CMN_LOG_CLASS_RUN_ERROR << "Run: write command \"" << commandName
                                                << "\" not found.  Make sure you added it to your configuration file"
                                                << std::endl;
                    }
                }
            }
        } while (bytesRead != 0);

    }
}

void mtsSocketStreamer::Cleanup(void)
{
    mSocket.Close();
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
            mSocket.SetDestination(ipPort.substr(0, colon), port);
            mSocketConfigured = true;
        }
    }
}
