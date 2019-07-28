    //
// Created by JBinggi on 01.07.18.
//

#ifndef CG_MINING_BOT_DATA_H

#define CG_MINING_BOT_DATA_H

#include <mutex>
#include "../nlohmann/json.hpp"
#include "defines.h"

// Rig States see at phpmyAdmin table rigadmin_rig_state
typedef enum {
    eOnline = 3,
    eRestarting = 7,
    eOffline = 8,
    eError = 9,
    eStartError = 10,
    eRuntimeError = 11,
    eHardwareError = 12,
    eUpdating = 20,
} ERigState;

//GPU Data Struct
typedef struct {
    bool bOnline;
    char caName[1000];
    float fTemp;
    float fPower;
    float fPowerLimit;
    int iFanSpeed;
    char caHashRate[100];
    float fHash;
    char caHashUnit[10];
} GpuInfo;

//CPU Data Struct
typedef struct {
    bool bOnline;
    char caName[1000];
    float fTemp;
    float fPower;
    //int iFanSpeed;
    char caHashRate[100];
    float fHash;
    char caHashUnit[10];
} CpuInfo;

//Miner Settings
typedef struct {
    bool bUpdate;
    char caCurrency[10];
    char caHostName[100];

    char caPool1Pass[200];
    char caPool1Port[200];
    char caPool1Url[200];
    char caPool1User[200];
    char caWallet[200];
    int iOcEnabled;
    int iPillEnabled;
    int iOcCore;
    int iOcMemCore;
    int iOcPowerLimit;
    int iFanSpeed;
    bool bHasFan;
    char caStartGPUScript[10000];
    char caStartCPUScript[10000];

    int iSwitchbacktimer;
} MinerSettings;

typedef struct {
    std::mutex gui_mutex;
    char caHostName[100];
    char caMacAddr[30];
    char caIPAddr[30];
    char caVersion[50];
    char caGpuMinerVersion[300];
    char caCpuMinerVersion[300];

    char caCudaVersion[300];
    char caNvidiaSmiVersion[300];

    int iGpuCount;
    int iCpuCount;
    bool bDoReboot;
} HostInfo;

typedef struct {
    char caHomePath[300];
    char caScriptTemplate[300];
    char caErrorLog[300];
    int iCheckStateInterval;
    int iGetSettingsInterval;

    bool bUseDashboard;
    char caGetSettingsUrl[300];
    char caUpdateStateUrl[300];
    char caGpuManufacurer[100];
    bool bHasFan;

    bool bUseOC;
    int iOcCore;
    int iOcMemCore;
    int iPowerLimit;
    
    bool bUsePill;
    char caPillPath[300];
    bool bGenerateXorg;
    char caXorgScriptPath[300];
} TagConfig;

void vSetError(int iErrorCode, const char *format, ...);

bool bHasError();

char *pcGetLastError();

void vClearError();

void vSetStatusText(const char *format, ...);

char *pcGetStatus();

void vClearStatusText();

void vSetDebugText(const char *format, ...);

char *pcGetDebugText();

void vSetRigState(ERigState eRigState);

ERigState eGetRigState();

void vSaveMinerSettings(MinerSettings *ptagMinerSettings);

bool bParseJson(MinerSettings *ptagMinerSettings, nlohmann::json jsonfile);

int iGetMinerSettings(HostInfo *ptagHostInfo, MinerSettings *ptagMinerSettings);

bool bUpdateMinerInfo(HostInfo *ptagHostInfo, GpuInfo ptagGpuInfos[MAX_GPUS], CpuInfo ptagCpuInfos[MAX_CPUS],
                      ERigState eRigState);

int iLoadMinerSettings(MinerSettings *ptagMinerSettings, bool bCheckChanged);

int iGetCPUPowerUsage(HostInfo *pHostInfo, CpuInfo ptagCpuInfos[MAX_CPUS]);

int iReadNvideaInfo(GpuInfo ptagGpuInfos[MAX_GPUS]);

int iGetCpuInfo(CpuInfo ptagCpuInfos[MAX_CPUS]);

bool bLoadConfig(char *pcConfig, MinerSettings *ptagMinerSettings, TagConfig *ptagConfig);

#endif //CG_MINING_BOT_DATA_H
