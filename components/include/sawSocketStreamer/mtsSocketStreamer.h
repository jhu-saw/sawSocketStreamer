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


#ifndef _mtsSocketStreamer_h
#define _mtsSocketStreamer_h

#include <cisstOSAbstraction/osaSocket.h>
#include <cisstMultiTask/mtsTaskPeriodic.h>

#include <cisstMultiTask/mtsForwardDeclarations.h>
class prmEventButton;

// Always include last!
#include <sawSocketStreamer/sawSocketStreamerExport.h>

class CISST_EXPORT mtsSocketStreamer: public mtsTaskPeriodic
{
    CMN_DECLARE_SERVICES(CMN_DYNAMIC_CREATION_ONEARG, CMN_LOG_ALLOW_DEFAULT);

 public:
    /*! Constructor
        \param name Name of the component
        \param period Period in seconds
    */
    inline mtsSocketStreamer(const std::string & name, double period):
        mtsTaskPeriodic(name, period),
        Socket(osaSocket::UDP)
        {
            Init();
        }

    inline mtsSocketStreamer(const mtsTaskPeriodicConstructorArg & arg):
        mtsTaskPeriodic(arg),
        Socket(osaSocket::UDP)
        {
            Init();
        }

    /*! Destructor */
    virtual ~mtsSocketStreamer();

    /*! Configure: set IP address and port number
        \param ipPort IP address and port number, separated by ':'
    */
    void Configure(const std::string & filename);
    void Startup(void);
    void Run(void);
    void Cleanup(void);

 protected:

    void Init(void);
    void SetDestination(const std::string &ipPort);

    mtsInterfaceRequired * InterfaceRequired;
    typedef struct {
        mtsFunctionRead Function;
        mtsGenericObject * Data;
    } DataStruct;
    typedef std::map<std::string, DataStruct> DataMapType;
    DataMapType DataMap;

    osaSocket Socket;
    bool SocketConfigured;

    Json::FastWriter FastWriter;

    mtsFunctionRead GetPositionCartesian;

};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsSocketStreamer)

#endif // _mtsSocketStreamer_h
