/********************************************************
 * HEADER
 *********************************************************
 * SYNOPSIS
 *   ./CG_Mining_Bot ...
 *
 * DESCRIPTION
 *   This is the main program for getting and checking
 *   settings from dashboard server. Also checking miner
 *   is running and running smoothly.
 *   Restarting Rig if new settings are available
 *
 * OPTIONS:
 *   -h, --help      Print this help
 *   -v, --version   Print software version
 *
 * EXAMPLES:
 *   ./CG_Mining_Bot
 *
 * IMPLEMENTATION
 *   version         CG_Mining_Bot (https://github.com/CoinGarden) 0.1.1
 *   author          JÃ¼rg Binggeli, Nathanael Kammermann
 *   copyright       Copyright (c) https://github.com/CoinGarden
 *   license         MIT License
 *
 * HISTORY
 *   2019/04/23 : Merged rc2 to master - new config.json, better doku - and more
 *   2019/04/22 : NKammermann: AMD Support Alpha
 *   2018/07/05 : JBinggi: Improve ncurser GUI + added cpu stats
 *   2018/07/03 : JBinggi: Added ncurser GUI
 *   2018/07/03 : JBinggi: Added cpuminer api support
 *   2018/07/02 : JBinggi: Fix CPU load
 *   2018/07/02 : jBinggi: Added EWBF support
 *   2018/07/01 : jBinggi: Added hashrate payment, Bugfixes
 *   2018/06/30 : jBinggi: Added Header, Bugfixes
 *   2018/06/29 : jBinggi: initial creation
 *
 * TODOS:
 *   Add miner api support
 *   better miner error handling
 *   Add AMD support
 *   Change all C language to C++
 *
 *********************************************************
 * END_OF_HEADER
 *********************************************************/
using namespace std;


#include <iostream>
#include <unistd.h>
#include <thread>         // std::thread
#include <sys/types.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <cstring>
#include <arpa/inet.h>
#include <cinttypes>
#include <cstdio>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <curl/curl.h>
#include <fstream>
#include <iomanip>
#include <ncurses.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <errno.h>

#include "../nlohmann/json.hpp"
#include "helper.h"
#include "minerapi.h"
#include "defines.h"
#include "data.h"
#include "ngui.h"

//#define DEBUG
#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) ;
#endif


//For thread
//sudo add-apt-repository ppa:ubuntu-toolchain-r/test
//sudo apt-get update
//sudo apt-get upgrade
//sudo apt-get install libstdc++6

//start:
//sudo screen -dmS CGBot ./CG_Mining_Bot

// for convenience
using json = nlohmann::json;

//Global Variable
bool bReboot_m = false;
bool bExitProgram_m = false;
bool bHashRatePayment_m;
int iFanSpeed_m = 90;

//Global Structs
GpuInfo tagGpuInfo_m[MAX_GPUS];
CpuInfo tagCpuInfo_m[MAX_CPUS];
MinerSettings tagMinerSettings_m;
HostInfo tagHostInfo_m;
TagConfig tagConfig_m;


void vCreateExcavatorConfigJson() {
    char caCommand[500];
    vSetStatusText("Generate Excavator File");
    // Run startcpu.sh
    sprintf(caCommand, "sudo cp %s %s", EXCAVATOR_CMD_FILE_TEMPLATE, EXCAVATOR_CMD_FILE);
    string strResult = exec(caCommand);
    if (!strResult.empty())
        vSetError(0, strResult.c_str());
    // Wallet
    sprintf(caCommand, "sed -i -e \"s/##BTCUSER##/%s/g\" %s", tagMinerSettings_m.caWallet, EXCAVATOR_CMD_FILE);
    strResult = exec(caCommand);
    // Workermissing
    sprintf(caCommand, "sed -i -e \"s/##WORKER##/%s/g\" %s", tagMinerSettings_m.caHostName, EXCAVATOR_CMD_FILE);
    strResult = exec(caCommand);
    // Url
    sprintf(caCommand, "sed -i -e \"s/##NHPOOLURL##/%s/g\" %s", tagMinerSettings_m.caPool1Url, EXCAVATOR_CMD_FILE);
    strResult = exec(caCommand);
    // Port
    sprintf(caCommand, "sed -i -e \"s/##NHPOOLPORT##/%s/g\" %s", tagMinerSettings_m.caPool1Port, EXCAVATOR_CMD_FILE);
    strResult = exec(caCommand);
    // Algo
    sprintf(caCommand, "sed -i -e \"s/##ALGO##/%s/g\" %s", tagMinerSettings_m.caPool1Pass, EXCAVATOR_CMD_FILE);
    strResult = exec(caCommand);
}

/**
 * Generate start.sh Script
 * @return success = true
 *         failed = false
 */
bool bGenerateStartScript() {
    FILE *fp, *fpWrite;
    char caBuffer[10000];
    char caStartScript[320];
    char caStartScriptNS[320];
    char caCommand[500];
    string strResult;

    vSetStatusText("Generate Start Scripts");
    memset(caBuffer, 0, sizeof(caBuffer));

    sprintf(caStartScriptNS, "%s/%s", tagConfig_m.caScriptTemplate, START_GPU_SCRIPT_NS);
    sprintf(caStartScript, "%s/%s", tagConfig_m.caScriptTemplate, START_GPU_SCRIPT);
    vSetDebugText(tagMinerSettings_m.caCurrency);
    // special case for excavator
    if (strcmp(tagMinerSettings_m.caCurrency, "BTC") == 0) {

        // Read start_ns.sh template
        fp = fopen(caStartScriptNS, "r");
        if (fp == nullptr) {
            vSetError(0, "%s is missing!!", caStartScriptNS);
            return false;
        }
        vCreateExcavatorConfigJson();

    }
    else if (strcmp(tagMinerSettings_m.caCurrency, "GRIN") == 0){

        // server login
        sprintf(caCommand, "sed -i \"/stratum_server_login/c\\stratum_server_login=\\\"%s/%s\\\"\" %s", tagMinerSettings_m.caWallet,tagMinerSettings_m.caHostName, GRIN_MINER_CONFIG);
        strResult = exec(caCommand);
        // startum server
        sprintf(caCommand, "sed -i \"/stratum_server_addr/c\\stratum_server_addr=\\\"%s:%s\\\"\" %s", tagMinerSettings_m.caPool1Url,tagMinerSettings_m.caPool1Port, GRIN_MINER_CONFIG);
        strResult = exec(caCommand);
        fp = fopen(caStartScript, "r");
        if (fp == nullptr) {
            vSetError(0, "%s is missing!!", caStartScript);
            return false;
        }

    }
    else {

        // Read start.sh template
        fp = fopen(caStartScript, "r");
        if (fp == nullptr) {
            vSetError(0, "%s is missing!!", caStartScript);
            return false;
        }

    }
    fread(caBuffer, sizeof(char), sizeof(caBuffer) - 1, fp);
    fclose(fp);

    if (tagMinerSettings_m.caStartGPUScript[0]) {
        char caScriptPath[400];

        // Create new start.sh script
        sprintf(caScriptPath, "%s/%s", tagConfig_m.caHomePath, START_GPU_SCRIPT);
        fpWrite = fopen(caScriptPath, "w");
        if (fpWrite == nullptr) {
            vSetError(0, "FILE <%s> could not be opened\n", caScriptPath);
            return false;
        }

        // Append startscript to template and save it
        sprintf(&caBuffer[strlen(caBuffer)], "%s\n", tagMinerSettings_m.caStartGPUScript);
        fwrite(caBuffer, sizeof(char), strlen(caBuffer), fpWrite);
        fclose(fpWrite);

    } else {
        // No startscript available
        vSetError(0, "bGenerateStartScript Start GPU Script not available\n");
        return false;
    }

#if 0
    // Read startcpu.sh template
    char caCpuTemplate[330];

    sprintf(caCpuTemplate,"%s/%s", tagConfig_m.caScriptTemplate, START_CPU_SCRIPT);
    fp = fopen(caCpuTemplate, "r");
    if (fp == nullptr) {
        vSetError(0, "%s is missing!!", caCpuTemplate);
        return false;
    }
    memset(caBuffer, 0, sizeof(caBuffer));
    fread(caBuffer, sizeof(char), sizeof(caBuffer) - 1, fp);
    fclose(fp);

    if (tagMinerSettings_m.caStartCPUScript[0]) {
        char caScriptPath[400];

        // Create new start.sh script
        sprintf(caScriptPath, "%s/%s", tagConfig_m.caHomePath, START_CPU_SCRIPT);
        fpWrite = fopen(caScriptPath, "w");
        if (fpWrite == nullptr) {
            vSetError(0, "FILE <%s> could not be opened\n", caScriptPath);
            return false;
        }

        // Append startscript to template and save it
        sprintf(&caBuffer[strlen(caBuffer)], "%s\n", tagMinerSettings_m.caStartCPUScript);
        fwrite(caBuffer, sizeof(char), strlen(caBuffer), fpWrite);
        fclose(fpWrite);
    } else {
        // No startscript available
        vSetError(0, "bGenerateStartScript Start CPU Script not available\n");
        return false;
    }
#endif
    return true;
}

/**
 * Generate Xorg File
 * @return success = true
 *         failed = false
 */
bool bGenerateXorg() {
    char caCommand[400];
    vSetStatusText("Generate Xorg File");
    // Run startcpu.sh
    sprintf(caCommand, "%s", tagConfig_m.caXorgScriptPath);
    string strResult = exec(caCommand);
    if (strstr(strResult.c_str(), "not found")) {
        vSetError(0, "ERROR: Cant find scrpit <%s>\n", tagConfig_m.caXorgScriptPath);
        return false;
    }

    if (!bFileExists(caCommand)) {
        vSetError(0, "ERROR: Cant generate Xorg\n");
        return false;
    }

    // Copy xorg to /etc/X11/xorg.conf
    vSetStatusText("copying %s to /etc/X11/xorg.conf", FILE_XORG_TEMP);
    sprintf(caCommand, "cp %s /etc/X11/xorg.conf", FILE_XORG_TEMP);
    strResult = exec(caCommand);
    if (!strResult.empty()) {
        vSetError(0, "ERROR: Cant copy Xorg: %s\n", strResult.c_str());
        return false;
    }

    // delete GENERATE_XORG_SCRIPT
    sprintf(caCommand, "rm -f %s", FILE_XORG_TEMP);
    strResult = exec(caCommand);
    if (!strResult.empty())    //return 0 ;

    {
        vSetError(0, "ERROR: Cant copy Xorg: %s\n", strResult.c_str());
        return false;
    }

    return true;
}


/**
 * Set OverClocking
 * @param iCoreClock Core Clock
 * @param iMemClock  Memory Core Clock
 * @param iPowerLimit  Power Limit
 * @return true=success
 *          false=error
 */
bool bSetOverClocking(HostInfo *ptagHostInfo, int iCoreClock, int iMemClock, int iPowerLimit) {
    char caCommand[200];
    int iIndex;
    string strResult;

    vSetStatusText("Set Over Clocking to Core:%d Memcore:%d Powerlimit:%d", iCoreClock, iMemClock, iPowerLimit);

    // enable persistance mode
    sprintf(caCommand, "%s -pm 1", NVIDIA_SMI);
    strResult = exec(caCommand);
    // Del Error Log
    strResult = exec("rm -f nvidia_error");

    //call nvidia-settings to enable fan control and set speed
    for (iIndex = 0; iIndex < ptagHostInfo->iGpuCount; iIndex++) {
        sprintf(caCommand,
                "DISPLAY=:0 XAUTHORITY=/var/run/lightdm/root/:0 %s -a [gpu:%d]/GPUMemoryTransferRateOffset[3]=%d  2>>nvidia_error",
                NVIDIA_SET, iIndex, iMemClock);
        exec(caCommand);

        // Check if errror
        strResult = exec("cat nvidia_error");
        if (!strResult.empty()) {
            vSetDebugText("GPUMemoryTransferRateOffset=%d  failed", iMemClock);
            return false;
        }
        sprintf(caCommand,
                "DISPLAY=:0 XAUTHORITY=/var/run/lightdm/root/:0 %s -a [gpu:%d]/GPUGraphicsClockOffset[3]=%d 2>>nvidia_error",
                NVIDIA_SET, iIndex, iCoreClock);
        exec(caCommand);

        //Check if error
        strResult = exec("cat nvidia_error");
        if (!strResult.empty()) {
            vSetDebugText("GPUGraphicsClockOffset=%d failed, xinit not running", iCoreClock);
            return false;
        }
        if(iPowerLimit<100)
            iPowerLimit=150;
        sprintf(caCommand, "DISPLAY=:0 XAUTHORITY=/var/run/lightdm/root/:0 %s -i %d -pl %d 2>>nvidia_error", NVIDIA_SMI,
                iIndex, iPowerLimit);
        exec(caCommand);

        //Check if error
        strResult = exec("cat nvidia_error");
        if (!strResult.empty()) {
            vSetDebugText("%s", caCommand);
            return false;
        }

    }
    return true;
}

/**
 * Set Fan Speed
 * @param iFan Fan to set speed; -1 for all fans
 * @param iFanSpeed Fan Speed
 * @return success = true
 *         failed = false
 */
bool bSetFanSpeed(HostInfo *ptagHostInfo, int iFan, int iFanSpeed) {
    char caCommand[200];
    int iIndex;
    string strResult;

    vSetStatusText("Set Fanspeed to %d%%%", iFanSpeed);

    // enable persistance mode
    sprintf(caCommand, "%s -pm 1", NVIDIA_SMI);
    strResult = exec(caCommand);
    // del error log
    strResult = exec("rm -f nvidia_error");

    //call nvidia-settings to enable fan control and set speed
    for (iIndex = 0; iIndex < ptagHostInfo->iGpuCount; iIndex++) {
        if(iFan > 0 && iFan != iIndex)
            continue;
        sprintf(caCommand,
                "DISPLAY=:0 XAUTHORITY=/var/run/lightdm/root/:0 %s -a [gpu:%d]/GPUFanControlState=1  2>>nvidia_error",
                NVIDIA_SET, iIndex);
        exec(caCommand);

        // Check if errror
        strResult = exec("cat nvidia_error");
        if (!strResult.empty()) {
            vSetError(0, "GPUFanControlState=1  failed, xinit not running");
            return false;
        }
        sprintf(caCommand,
                "DISPLAY=:0 XAUTHORITY=/var/run/lightdm/root/:0 %s -a [fan:%d]/GPUTargetFanSpeed=%d 2>>nvidia_error",
                NVIDIA_SET, iIndex, iFanSpeed);
        exec(caCommand);

        //Check if error
        strResult = exec("cat nvidia_error");
        if (!strResult.empty()) {
            vSetError(0, "GPUTargetFanSpeed=%d failed, xinit not running", iFanSpeed);
            return false;
        }
    }
    return true;
}


/**
 * Start GPU Miner
 * @return success = true
 *         failed = false
 */
bool bStartCPUMiner() {
    char caCommand[400];

    vSetStatusText("Starting CPU Miner");
    // Run startcpu.sh
    sprintf(caCommand, "screen -dmS cpuminer %s/%s", tagConfig_m.caHomePath, START_CPU_SCRIPT);
    string strResult = exec(caCommand);

    //TODO: Error Handling

    return true;
}


/**
 * Start OhGodAnETHlargementPill
 * @return success = true
 *         failed = false
 */

bool bStartThePill() {
    char caCommand[400];
    string strResult;
    // Look for miner process
    vSetStatusText("Apply ETH Pill");
    sprintf(caCommand, "ps aux | grep -v grep | grep ethpill 2>/dev/null");
    strResult = exec(caCommand);
    if (strResult.empty()) {
        // Run the Pill
        sprintf(caCommand, "screen -dmS ethpill %s", tagConfig_m.caPillPath);
        strResult = exec(caCommand);
    }
    return true;
}

/**
 * Start Xinit Server
 */
void vStartXinit() {
    char caCommand[200];
    string strResult;


    // Kill existing x sessions - otherwise xinit will fail
    sprintf(caCommand, "ps aux | grep -v grep | grep xinit 2>/dev/null");
    strResult = exec(caCommand);

    if (strResult.empty()) {
        vSetStatusText("Starting Xinit Server");
        sprintf(caCommand, "killall xinit 2>/dev/null");
        strResult = exec(caCommand);

        //# Run Xserver to keep Nvidia GPUs in P2 - otherwise they fall back to P8
        sleep(1);
        sprintf(caCommand, "screen -dmS xinit xinit");
        strResult = exec(caCommand);
    } else{
        vSetStatusText("Xinit Server is running");

    }
}

/**
 * Start GPU Miner
 * @return success = true
 *         failed = false
 */
bool bStartGPUMiner() {
    char caCommand[400];

#if 0
    vStartXinit();
    sleep(10);
#endif 

#if 0
    if(tagConfig_m.bHasFan)
    {
        if(!bSetFanSpeed(&tagHostInfo_m,-1,iFanSpeed_m))
            vSetError(0,"Set bSetFanSpeed() failed");
    }
#endif 

#if 0
    //TODO:
    //if(tagMinerSettings_m.iOcEnabled)
    //{
        //DEBUG_PRINT(("OC enabled\n"));
        //Set Over Clocking
        //if (!bSetOverClocking(&tagHostInfo_m, tagMinerSettings_m.iOcCore, tagMinerSettings_m.iOcMemCore,
        //                      tagMinerSettings_m.iOcPowerLimit))
        //    vSetError(0, "Set bSetOverClocking() failed");
    //}
    //sleep(10);
    //if (tagMinerSettings_m.iPillEnabled) {
        //Run the ETH Pill
    //    if (!bStartThePill()) {
    //        vSetError(0, "ETH Pill start failed\n");
    //    }
        //printf("Set\n");
    //}
    //sleep(10);
#endif
    // Start Miner
	vSetError(0, "Start GPU Miner");
    vSetStatusText("Starting GPU Miner");
    sprintf(caCommand, "screen -dmS miner %s/%s", tagConfig_m.caHomePath, START_GPU_SCRIPT);
    //printf("caCommand = %s\n",caCommand);
    string strResult = exec(caCommand);

    return true;
}


/**
 * Create Crontab to swtich back costomer
 * @param ptagMinerSettings MinerSettings
 * @return success  = true
 *         failed   = false
 */
bool bSetHashratePayment(MinerSettings *ptagMinerSettings) {
    char caCommand[400];
    char caCronJob[400];
    char caBuffer[10000];
    char caPath[601];
    FILE *fpOrig;
    FILE *fpTmp;

    vSetDebugText("bSetHashratePayment");
    sprintf(caCommand, "sudo crontab -l > %s", FILE_CORN_ORIG);

    string strResult = exec(caCommand);

    time_t now = time(nullptr);
    struct tm now_tm = *localtime(&now);
    struct tm then_tm = now_tm;

    // add seconds to the current time
    then_tm.tm_sec += ptagMinerSettings->iSwitchbacktimer;

    mktime(&then_tm);      // normalize it


    // Create new start.sh script
    memset(caBuffer, 0, sizeof(caBuffer));
    sprintf(caCronJob, "%d %d %d %d * %s/hashdone.sh", then_tm.tm_min, then_tm.tm_hour, then_tm.tm_mday,
            then_tm.tm_mon + 1, tagConfig_m.caHomePath);

    sprintf(caPath, "%s/%s", tagConfig_m.caHomePath, FILE_CORN_ORIG);
    fpOrig = fopen(caPath, "r");
    if (fpOrig == nullptr) {
        vSetError(0, "FILE <%s> could not be opened\n", caPath);
        return false;
    }

    sprintf(caPath, "%s/%s", tagConfig_m.caHomePath, FILE_CORN_TEMP);
    fpTmp = fopen(caPath, "w");
    if (fpTmp == nullptr) {
        vSetError(0, "FILE <%s> could not be opened\n", caPath);
        return false;
    }

    // Append startscript to template and save it
    fread(caBuffer, sizeof(char), sizeof(caBuffer) - 1, fpOrig);

    // Write new crontab to disk
    sprintf(&caBuffer[strlen(caBuffer)], "%s\n", caCronJob);
    fwrite(caBuffer, sizeof(char), strlen(caBuffer), fpTmp);
    fclose(fpTmp);

    sprintf(caCommand, "sudo crontab %s", caPath);
    strResult = exec(caCommand);

    return true;
}

bool bKillScreen(void)
{
    char caCommand[100];

    vSetStatusText("Kill miner screen");
    strcpy(caCommand, "screen -S miner -X quit");
    string strResult = exec(caCommand);

    vSetStatusText("Kill ethpill screen");
    strcpy(caCommand, "screen -S ethpill -X quit");
    strResult = exec(caCommand);

    return true;
}


/**
 *  Thread Checkservice
 *
 *  Interval: INTERVAL_CHECKSERVICE
 *
 *  Reading gpu info, checking if mining
 *  software is running and sending result
 *  to the dashboard server
 */
void vCheckService() {
    while (!bExitProgram_m) {
        if (bReboot_m)
            return;
        tagHostInfo_m.gui_mutex.lock();

        bGetMinerVersion(&tagHostInfo_m, &tagMinerSettings_m);

        iCheckAllMiners(&tagHostInfo_m, tagGpuInfo_m, tagCpuInfo_m);
        if (!tagGpuInfo_m[0].bOnline) {
            vSetStatusText("Start GPU Miner");
            bStartGPUMiner();
        }
        //if (!tagCpuInfo_m[0].bOnline) {
        //    vSetStatusText("Start CPU Miner");
        //    bStartCPUMiner();
        //}
        /*
        // Set Fan to 100% if temp is over 80C
        int iIndex;
        for(iIndex=0;iIndex<tagHostInfo_m.iGpuCount;iIndex++)
        {
            if(tagGpuInfo_m[iIndex].fTemp>80.0)
                bSetFanSpeed(&tagHostInfo_m,iIndex,100);
            else if (tagGpuInfo_m[iIndex].fTemp < 60)
                bSetFanSpeed(&tagHostInfo_m,iIndex,80);
            else
                bSetFanSpeed(&tagHostInfo_m,iIndex,90);
        }
         */

        tagHostInfo_m.gui_mutex.unlock();

        srand((unsigned int)time(nullptr));
        //generate number between 1 and 10:
        int iRand = rand() % 10 + 1;
        sleep((unsigned int) tagConfig_m.iCheckStateInterval * 60 + iRand);
    };

}


/**
 *  Thread GetSettings
 *
 *  Interval: INTERVAL_GETSETTINGS
 *
 *  Reading current settings form the dashboard server,
 *  generate new start script if needed and performs reboots.
 *  Performs reboot.
 *
 */
void vGetSettings() {
    int iRc=0;
    while (!bExitProgram_m) {
        if (bReboot_m)
            return;
        tagHostInfo_m.gui_mutex.lock();
        vSetStatusText("GetSettings()");

        if(tagConfig_m.bUseDashboard)
            iRc = iGetMinerSettings(&tagHostInfo_m, &tagMinerSettings_m);
        else
            iRc = iLoadMinerSettings(&tagMinerSettings_m,true);

        if (iRc == 1) {
            if (!bGenerateStartScript())
                vSetError(0, "bGenerateStartScript() failed");

            //TODO: Not working properly
            // If no error occured and if hostname the same, restart miner
            //if(pcGetLastError()[0] == 0 && (strcmp(tagHostInfo_m.caHostName,tagMinerSettings_m.caHostName)==0)) {
            //    bKillScreen();
            //}
            //else
            {
                bReboot_m = true;
                vSetStatusText("Perform Reboot");


                vSetHostName(tagMinerSettings_m.caHostName);

                vSetRigState(eRestarting);
                bUpdateMinerInfo(&tagHostInfo_m, tagGpuInfo_m, tagCpuInfo_m, eGetRigState());

                if (tagMinerSettings_m.iSwitchbacktimer > 0) {
                    //Activate Hashrate payment
                    bHashRatePayment_m = true;
                }
                //if (bHashRatePayment_m)
                    //bSetHashratePayment(&tagMinerSettings_m);
                if(tagConfig_m.bGenerateXorg)
                    bGenerateXorg();
                //Do Reboot
                vReboot(0);
            }
        } else if (iRc == -1) {
            vSetStatusText("iGetMinerSettings returned -1");
            // Error occurred
            //TODO: Do something, but what?
        }

        tagHostInfo_m.gui_mutex.unlock();


        srand((unsigned int)time(nullptr));
        //generate number between 1 and 10:
        int iRand = rand() % 10 + 1;
        sleep((unsigned int) tagConfig_m.iGetSettingsInterval * 60 + iRand);
    };
}


/**
 * Program start
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char *argv[]) {
    char caCommand[500];
    char caConfigPath[300];

    int pid_file = open("/var/run/whatever.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if(rc) {
        if(EWOULDBLOCK == errno) {
            cout << "CGbot is already running" << endl; // another instance is running
            return 1;
        }
    }
	vSetError(0, "Bot not running - start");
    vSetDebugText("CG_Mining_Bot V_%s start",SOFTWARE_VERSION);
    if(argc==2)
        strcpy(caConfigPath,argv[1]);
    else
        strcpy(caConfigPath,FILE_CONFIG);
	vSetError(0, "Bot not running - before load config");
    // Load Config
    if (!bLoadConfig(caConfigPath, &tagMinerSettings_m, &tagConfig_m)) {
        printf("<%s> not found\n", caConfigPath);
        return false;
    }
    //vUpdateCgMiningBot(&tagConfig_m,(char*)"0.1.11");
    //return 0;
    // check if miners exists
    sprintf(caCommand,"%s/%s",tagConfig_m.caHomePath,"miners");
    if (!bFileExists(caCommand))
    {
        vDownloadMiners();
        //vDownLoadUpdateFile()
    }
    // Get Host Infos
    if (!bGetHostInfo(&tagHostInfo_m)) {
        vSetError(0, "bGetHostInfo() ERROR:\n");
    }
    // Start ncursers gui
#ifndef NO_NCURSER
    thread guithread(vGuiThread, &tagHostInfo_m, tagGpuInfo_m, tagCpuInfo_m,
                     &tagMinerSettings_m);     // spawn new thread
#endif
    // Load Settings from file
    if(iLoadMinerSettings(&tagMinerSettings_m,false)<0) {
        if(!tagConfig_m.bUseDashboard) {
            vSetError(0,"<%s> not found",FILE_SETTINGS);
        }
    }
    if(!strcmp(tagConfig_m.caGpuManufacurer,"AMD")) {
        // TODO: Hardcoded for now - count result of lspci
		tagHostInfo_m.iGpuCount = iReadNvideaInfo(tagGpuInfo_m);
    } else {
        tagHostInfo_m.iGpuCount=iReadNvideaInfo(tagGpuInfo_m);
    }

    if (tagHostInfo_m.iGpuCount < 0) {
        // Error occurred
        if (tagHostInfo_m.iGpuCount == -1) {
            vSetRigState(eError);
            bUpdateMinerInfo(&tagHostInfo_m, tagGpuInfo_m, tagCpuInfo_m, eGetRigState());
        } else {
            vSetRigState(eError);
            bUpdateMinerInfo(&tagHostInfo_m, tagGpuInfo_m, tagCpuInfo_m, eGetRigState());
            vReboot(1);
            sleep(50);
            return 0;
        }
    }

    // CPU disabled for now
    //tagHostInfo_m.iCpuCount=iGetCpuInfo(tagCpuInfo_m);
    //iGetCPUPowerUsage(&tagHostInfo_m,tagCpuInfo_m);
    //sleep(1);

    // Set permissionys
    vSetStatusText(("Set sttart script Permission"));
    sprintf(caCommand, "chmod +x %s/*.sh", tagConfig_m.caHomePath);
    string strResult = exec(caCommand);

    // Start Getsettings thread
    vSetStatusText("Start GetSettings Thread");
    thread getSettings(vGetSettings);  // spawn new thread
    sleep(5);

    // Start checkservice thread
    vSetStatusText("Start CheckService Thread");
    thread checkService(vCheckService);     // spawn new thread


    DEBUG_PRINT(("All Thread started!!\n"));
    // synchronize threads:
    getSettings.join();                // pauses until first finishes
    checkService.join();               // pauses until second finishes
#ifndef NO_NCURSER
    guithread.join();
#endif
    return 0;
}

