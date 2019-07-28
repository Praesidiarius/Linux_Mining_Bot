//
// Created by JBinggi on 01.07.18.
//
#include <curl/curl.h>
#include <fstream>
#include <zconf.h>
#include "minerapi.h"
#include "../nlohmann/json.hpp"
#include "helper.h"
#include "data.h"
#include "defines.h"

using namespace std;

// for convenience
using json = nlohmann::json;
int iGpuCount_m;
/**
 * DualMiner ethdecrminer API Parser
 * @return success = true
 *         failed = false
 */
bool getApiDualMiner(GpuInfo ptagGpuInfos[MAX_GPUS]) {
    vector<string> strOutPut;
    vector<string> strGpuVal;

    // Look for miner process
    string strResult = exec("curl -v /dev/null localhost:3333 2>&1  | grep \"ETH:\" | grep \"Mh/s\" -| tail -1\0");

    if (strResult.empty()) {
        vSetError(0, "getApiDualMiner() No Process found");
        return false;
    }

    //Split count = GPU Count
    auto iGPUCount = (int) split(strResult, strOutPut, ' ');

    //printf("iGPUCount = %d\n",iGPUCount);
    int iCounter = 0;
    int iGpuCount = 0;
    for (auto itGpu = strOutPut.begin(); itGpu != strOutPut.end(); ++itGpu) {
        if (strstr(itGpu[iCounter].c_str(), "GPU")) {
            strcpy(ptagGpuInfos[iGpuCount].caHashRate, itGpu[iCounter + 1].c_str());
            strcat(ptagGpuInfos[iGpuCount].caHashRate, " MH/s");
            ptagGpuInfos[iGPUCount].fHash = (float) strtod(itGpu[iCounter + 1].c_str(), nullptr);
            strcpy(ptagGpuInfos[iGpuCount].caHashUnit, "MH/s");
            //cout << "iCounter= " << iCounter << " " << itGpu[iCounter+1] << endl;
            iGpuCount++;
        }
    }
    return true;
}

/**
 * CCMiner API Parser
 * @return
 */
bool getApiCCminer(GpuInfo ptagGpuInfos[MAX_GPUS]) {

    int iRc;
    unsigned char caRxBuffer[10000];
    vector<string> strOutPut;
    vector<string> subString;
    vector<string> subString2;


    // Call telnet for api
    iRc = iSendTelnet("127.0.0.1", 3333, "threads", caRxBuffer, sizeof(caRxBuffer) - 1);
    if (iRc != 0) {
        vSetError(0, "iSendTelnet return %d", iRc);
        return false;
    }

    string strRxBuffer((const char *) caRxBuffer);
    //auto iGPUCount = (int) split(strRxBuffer, strOutPut, '|');

    int iGpuCount = 0;

    for (auto itGpu = strOutPut.begin(); itGpu != strOutPut.end(); ++itGpu) {
        if (strstr(itGpu[0].c_str(), "GPU")) {
            split(itGpu[0], subString, ';');
            for (auto itHash = subString.begin(); itHash != subString.end(); ++itHash) {
                if (strstr(itHash[0].c_str(), "KHS")) {


                    split(*itHash, subString2, '=');
                    sprintf(ptagGpuInfos[iGpuCount].caHashRate, "%s %s", subString2.begin()[1].c_str(),
                            subString2.begin()[0].c_str());
                    ptagGpuInfos[iGpuCount].fHash = (float) strtod(subString2.begin()[1].c_str(), nullptr);
                    strcpy(ptagGpuInfos[iGpuCount].caHashUnit, subString2.begin()[0].c_str());
                    break;
                }
            }
        }
        iGpuCount++;
    }

    return true;
}

/**
 * EWBF Miner API Parser
 * @return api online
 */
bool getApiEwbfMiner(GpuInfo ptagGpuInfos[MAX_GPUS]) {
    json jsonfile;
    //char readBuffer[10000];

    string strResult = exec("curl -s '127.0.0.1:3333/getstat'");

    if (strResult.empty()) {
        vSetError(0, "getApiEwbfMiner() No Process found");
        return false;
    }

    //Parse Json
    jsonfile = json::parse(strResult.c_str());
    jsonfile = jsonfile["result"];

    int iCounter = 0;
    for (auto jgpu : jsonfile) {
        int iTest = jgpu["speed_sps"];
        sprintf(ptagGpuInfos[iCounter].caHashRate, "%d Sol/s", iTest);
        ptagGpuInfos[iCounter].fHash = (float) iTest;
        strcpy(ptagGpuInfos[iCounter].caHashUnit, "Sol/s");
        iCounter++;
    }
    return true;
}

bool getXmrigNvidiaMiner(GpuInfo ptagGpuInfos[MAX_GPUS]) {
    json jsonfile;

    string strResult = exec("curl -s '127.0.0.1:3333'");

    if (strResult.empty()) {
        vSetError(0, "getXmrigNvidiaMiner() No Process found");
        return false;
    }

    //Parse Json
    jsonfile = json::parse(strResult.c_str());
    jsonfile = jsonfile["hashrate"]["threads"];

    int iCounter = 0;
    for (auto jgpu : jsonfile) {
        sprintf(ptagGpuInfos[iCounter].caHashRate, "%d kH/s", (int) jgpu[1]);
        ptagGpuInfos[iCounter].fHash = (float) jgpu[1];
        strcpy(ptagGpuInfos[iCounter].caHashUnit, "KH/s");
        //cout << ptagGpuInfos[iCounter].caHashRate << endl;
        iCounter++;
    }
    return true;
}

bool getExcavatorMiner(GpuInfo ptagGpuInfos[MAX_GPUS]) {
    json jsonfile;
    string strHash;
    char readBuffer[100000];

    // Look for minergetApiCpuOptMiner process
    sprintf(readBuffer, "%s",
            "curl -H \"Authorization: 1234\" http://localhost:38080/api?command=%7B%22id%22%3A1%2C%22method%22%3A%22algorithm.list%22%2C%22params%22%3A%5B%5D%7D 2>/dev/null");
    string strResult = exec(readBuffer);
    if (strResult.empty()) {
        printf("strResult emmpty\n");
        vSetError(0, "getExcavatorMiner() No Process found");
        return false;
    }
    //return 0;

    //Parse Json
    int iCounter = 0;
    for (iCounter = 0; iCounter < MAX_GPUS; iCounter++) {
        sprintf(readBuffer, "echo '%s' | jq -r '.algorithms[0] .workers['%d'] .speed[0]'", strResult.c_str(), iCounter);
        strHash = exec(readBuffer);
        if (strtol(strHash.c_str(), nullptr,0) == 0)
            break;

        sprintf(ptagGpuInfos[iCounter].caHashRate, "%d Sol/s", (int)strtol(strHash.c_str(), nullptr,0));
        ptagGpuInfos[iCounter].fHash = (float) (int)strtol(strHash.c_str(), nullptr,0);
        strcpy(ptagGpuInfos[iCounter].caHashUnit, "Sol/s");
    }
    return true;
}

bool getApiGrinMiner(GpuInfo ptagGpuInfos[MAX_GPUS]) {
    char *pcFind, *pcEnd;
    char caBuffer[1000];
    char caCommand[200];

    for(int iCounter=0;iCounter<iGpuCount_m;iCounter++)
    {
        sprintf(caCommand,R"(tail -n 100 %s | grep "Device %d" | grep -v "CPU" | tail -1)",GRIN_MINER_LOG,iCounter);
        string strResult  = exec (caCommand);

        strcpy(caBuffer,strResult.c_str());
        pcFind=strstr(caBuffer,"Graphs per second");
        if(pcFind)
        {
            // jump to Hashrate
            pcFind+=18;
            pcEnd=strchr(pcFind,'-');

            *pcEnd=0;

            sprintf(ptagGpuInfos[iCounter].caHashRate, "%f G/s", strtof(pcFind,nullptr));
            ptagGpuInfos[iCounter].fHash = (float) (int)strtol(pcFind, nullptr,0);
            strcpy(ptagGpuInfos[iCounter].caHashUnit, "G/s");

        }
        else
            //Todo: Errorhandling
            continue;
    }

    return true;
}

bool getApiBMiner(GpuInfo ptagGpuInfos[MAX_GPUS])
{
    json jsonfile;

    string strResult = exec("curl -s '127.0.0.1:1880/api/status'");

    if (strResult.empty()) {
        vSetError(0, "getApiBnMiner() Api offline");
        return false;
    }

    //Parse Json
    //cout << strResult << "\n\n";
    jsonfile = json::parse(strResult.c_str());
    jsonfile = jsonfile["miners"];

    char cGpu[] = "0";
    int iCounter = 0;

    for (iCounter = 0; iCounter < iGpuCount_m; iCounter++) {

        sprintf(ptagGpuInfos[iCounter].caHashRate, "%f G/s", (float) jsonfile[cGpu]["solver"]["solution_rate"]);
        cGpu[0]++;
        //cout << iCounter << " " << ptagGpuInfos[iCounter].caHashRate << "\n";
        ///cout << cGpu << "\n";
    }
    return true;

}

/**
 * cpuminer-opt API Parser
 * @param ptagCpunfos
 * @return api online
 */
bool getApiCpuOptMiner(CpuInfo ptagCpunfos[MAX_CPUS]) {
    vector<string> strOutPut;
    vector<string> strOutPut2;
    vector<string> strOutPut3;
    vector<string> strGpuVal;

    // Read api
    string strResult = exec("curl -v /dev/null localhost:4048/threads 2>&1");

    if (strResult.empty()) {
        vSetError(0, "getApiCpuOptMiner() No Process found");
        return false;
    }
    //CPU=0;H/s=82.56|CPU=1;H/s=68.47|CPU=2;H/s=68.13|CPU=3;H/s=67.33|

    //Split count = GPU Count
    //auto iGPUCount = (int) split(strResult, strOutPut, '|');

    int iCounter = 0;
    int iGpuCount = 0;
    for (auto itCPU = strOutPut.begin(); itCPU != strOutPut.end(); ++itCPU) {
        if (strstr(itCPU[iCounter].c_str(), "CPU")) {
            split(itCPU[iCounter], strOutPut2, ';');
            split(strOutPut2.begin()[1], strOutPut3, '=');
            //Save
            strcpy(ptagCpunfos[iGpuCount].caHashRate, strOutPut3.begin()[1].c_str()); // Hashrate
            strcat(ptagCpunfos[iGpuCount].caHashRate, strOutPut3.begin()[0].c_str()); // Unit
            ptagCpunfos[iCounter].fHash = (float) strtod(strOutPut3.begin()[1].c_str(),nullptr);
            strcpy(ptagCpunfos[iCounter].caHashUnit, strOutPut3.begin()[0].c_str());

            iGpuCount++;
        }
    }
    return true;
}

/**
 * Check if Miner Software is running
 * @param eMiner mining software
 * @param pcMinerGrep ps aux grep command
 * @return success = 1
 *         failed  = 0
 */
bool bCheckMiner(GpuInfo ptagGpuInfo[MAX_CPUS], CpuInfo ptagCpuInfo[MAX_CPUS], eMiners eMiner, char *pcMinerGrep) {
    vector<string> strOutPut;
    vector<string> strGpuVal;
    bool bApiFound = false;
    char caCommand[200];
    //sleep(1);
    vSetStatusText("Check miner online: %s", pcMinerGrep);
    // Look for miner process
    sprintf(caCommand, "ps aux | grep -v grep | grep  %s 2>/dev/null", pcMinerGrep);
    string strResult = exec(caCommand);

    // empty--> miner not running
    if (strResult.empty()) {
        vSetStatusText("%s offline", pcMinerGrep);
        return false;
    }

    // Check mining software api to get hashrate
    switch (eMiner) {
        case eClayMoreDual:
            bApiFound = getApiDualMiner(ptagGpuInfo);
            break;
        case eZenemyMiner:
        case eCCminer:
            bApiFound = getApiCCminer(ptagGpuInfo);
            break;
        case eEwbfMiner:
            bApiFound = getApiEwbfMiner(ptagGpuInfo);
            break;
        case eXmrRig:
            //bApiFound = getXmrigNvidiaMiner(ptagGpuInfo);
            break;
        case eExcavator:
            //bApiFound=getExcavatorMiner(ptagGpuInfo);
            break;
        case eGrinMiner:
            //bApiFound = getApiGrinMiner(ptagGpuInfo);
            break;
        case eBMiner:
            //bApiFound = getApiBMiner(ptagGpuInfo);
            break;
        //case eCpuMiner:
            //bApiFound = getApiCpuOptMiner(ptagCpuInfo);
            //break;
        default:
            vSetError(0, "Unknown Miner enum: eMiner %d", eMiner);
    }

    // API not deliver expected result
    if (!bApiFound)
        vSetError(0, "Miner: %s: API offline", pcMinerGrep);

    vSetStatusText("Miner %s online", pcMinerGrep);
    sleep(1);
    return true;
}

/**
 * Check if all miners are running
 * @param ptagHostInfo
 * @param ptagGpuInfo
 * @param ptagCpuInfo
 * @return return
 */
int iCheckAllMiners(HostInfo *ptagHostInfo, GpuInfo ptagGpuInfo[MAX_CPUS], CpuInfo ptagCpuInfo[MAX_CPUS]) {
    int iOnline = 0;

    // Read nvidia Info
    if(ptagHostInfo->iGpuCount != iReadNvideaInfo(ptagGpuInfo))
    {
        // GPU lost
        vSetDebugText("GPU lost!!!");
    }

    //if(ptagHostInfo->iCpuCount != iGetCpuInfo(ptagCpuInfo))
    //{
        // CPU lost
    //    vSetDebugText("CPU lost!!!");
    //}

    iGetCPUPowerUsage(ptagHostInfo, ptagCpuInfo);
    iGpuCount_m=ptagHostInfo->iGpuCount;
    // count running miners
    iOnline += bCheckMiner(ptagGpuInfo, ptagCpuInfo, eClayMoreDual, DUALMINER);
    iOnline += bCheckMiner(ptagGpuInfo, ptagCpuInfo, eCCminer, CCMINER);
    iOnline += bCheckMiner(ptagGpuInfo, ptagCpuInfo, eEwbfMiner, EWBFMINER);
    //iOnline += bCheckMiner(ptagGpuInfo, ptagCpuInfo, eXmrRig, XMRIG_NVIDIA);
    //iOnline += bCheckMiner(ptagGpuInfo, ptagCpuInfo, eZenemyMiner, ZENEMY);
    //iOnline += bCheckMiner(ptagGpuInfo, ptagCpuInfo, eGrinMiner, GRINMINER);
    //iOnline += bCheckMiner(ptagGpuInfo, ptagCpuInfo, eBMiner, BMINER);
    //iOnline += iCheckMiner(ptagGpuInfo, ptagCpuInfo, eExcavator, EXCAVATOR);

    if (iOnline == 0) {
        vSetRigState(eRestarting);
        ptagGpuInfo[0].bOnline = false;
    } else if (iOnline > 1) {
        vSetError(0, "More then 1 miner detected\n");
    } else {
        vSetStatusText("GPU Miner Running");
        ptagGpuInfo[0].bOnline = true;
        vSetRigState(eOnline);
    }
    //ptagCpuInfo[0].bOnline=bCheckMiner(ptagGpuInfo, ptagCpuInfo, eCpuMiner, CPUMINER);

    bUpdateMinerInfo(ptagHostInfo, ptagGpuInfo, ptagCpuInfo, eGetRigState());
}

bool bGetMinerVersion(HostInfo *ptagHostInfo, MinerSettings *ptagMinerSettings) {

    char caMiner[10000];
    vector<string> strOutPut;
    string strResult;
    string version;
    char *pcResult;

#if 0
    strncpy(caMiner, ptagMinerSettings->caStartGPUScript,sizeof(caMiner)-1);
    pcResult=strchr(caMiner, ' ');
    if(pcResult)
        *pcResult=0;

    if (!bFileExists(caMiner)) {
        vSetError(0, "ERROR: Miner %s not found\n", caMiner);
        return false;
    }
    if (strstr(caMiner, "excavator")) {
        return false;
    } else if (strstr(caMiner, "ccminer")) {
        strcat(caMiner, " --version");
        strResult = exec(caMiner);
        if (strResult.empty()) {
            vSetError(0, "%s no result\n", caMiner);
            return false;
        }
        if(split(strResult, strOutPut, '\n')>=1){
            version = strOutPut.begin()[0];
            strcpy(ptagHostInfo->caGpuMinerVersion, version.c_str());
        }
    }
#endif
    //std::ofstream outfile;

    //outfile.open("test", std::ios_base::app);
    //outfile << "Minerversion:" << version << endl;
}



