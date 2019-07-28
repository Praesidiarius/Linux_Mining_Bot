//
// Created by JBinggi 03.07.18.
//

#ifndef CG_MINING_BOT_NGUI_H
#define CG_MINING_BOT_NGUI_H

#include "data.h"

void vGuiThread(HostInfo *ptagHostInfo, GpuInfo patagGpuInfos[MAX_GPUS], CpuInfo patagCpuInfos[MAX_GPUS],
                MinerSettings *ptagMinerSettings);

int iGetScreens(char caaSreens[10][100]);

#endif //CG_MINING_BOT_NGUI_H
