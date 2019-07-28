//
// Created by jBinggi on 01.07.18.
//

#ifndef CG_MINING_BOT_MINERAPI_H
#define CG_MINING_BOT_MINERAPI_H

#include "data.h"
#include "defines.h"

#define DUALMINER       (char*)"/ethdcrminer"
#define CCMINER         (char*)"/ccminer"
#define EWBFMINER       (char*)"/ewbfzcash/miner"
#define EWBFBTCZMINER   (char*)"/ewbfbtcz/miner"
#define XMRMINER        (char*)"/xmrminer"
#define BTCMINER        (char*)"/excavator"
#define CPUMINER        (char*)"/cpuminer"
#define ZENEMY          (char*)"/z-enemy"
#define XMRIG_NVIDIA    (char*)"/xmrig-nvidia"
#define GRINMINER       (char*)"/grin-miner"
#define BMINER          (char*)"/bminer"

typedef enum {
    eClayMoreDual = 0,
    eCCminer,
    eEwbfMiner,
    eXmrRig,
    eExcavator,
    eXmrMiner,
    eBtcMiner,
    eDstmMiner,
    eZenemyMiner,
    eEggMiner,
    eGrinMiner,
    eCpuMiner,
    eBMiner
} eMiners;

bool getApiDualMiner(GpuInfo ptagGpuInfos[MAX_GPUS]);
bool getApiCCminer(GpuInfo ptagGpuInfos[MAX_GPUS]);
bool getApiEwbfMiner(GpuInfo ptagGpuInfos[MAX_GPUS]);
bool getXmrigNvidiaMiner(GpuInfo ptagGpuInfos[MAX_GPUS]);
bool getExcavatorMiner(GpuInfo ptagGpuInfos[MAX_GPUS]);
bool getApiCpuOptMiner(CpuInfo ptagCpuInfos[MAX_CPUS]);
bool getApiGrinMiner(GpuInfo ptagGpuInfos[MAX_GPUS]);
bool getApiBMiner(GpuInfo ptagGpuInfos[MAX_GPUS]);

bool bCheckMiner(GpuInfo ptagGpuInfo[MAX_CPUS], CpuInfo ptagCpuInfo[MAX_CPUS], eMiners eMiner, char *pcMinerGrep);

int iCheckAllMiners(HostInfo *ptagHostInfo, GpuInfo ptagGpuInfo[MAX_CPUS], CpuInfo ptagCpuInfo[MAX_CPUS]);

bool bGetMinerVersion(HostInfo *ptagHostInfo, MinerSettings *ptagMinerSettings);


#endif //CG_MINING_BOT_MINERAPI_H
