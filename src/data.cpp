//
// Created by JBinggi on 01.07.18.
//
#include <curl/curl.h>
#include <iomanip>
#include <fstream>
#include <cstdarg>
#include "../nlohmann/json.hpp"
#include "helper.h"
#include "defines.h"
#include "data.h"

using namespace std;

// for convenience
using json = nlohmann::json;

//extern int iGpuCount_m;

MinerSettings tagLastMinerSettings_m;

char caErrorText_m[10000];
char caStatusText_m[10000];
char caDebugText_m[10000];
int iErrorCode_m = 0;
bool bHasError_m = false;
ERigState eRigState_m = eOffline;
extern TagConfig tagConfig_m;


void vSetDebugText(const char *format, ...) {
    va_list argptr;
    char caTime[100];
    va_start(argptr, format);
    memset(caDebugText_m, 0, sizeof(caDebugText_m));
    vsnprintf(caDebugText_m, sizeof(caDebugText_m) - 1, format, argptr);

    // Log error
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strcpy(caTime, asctime(timeinfo));
    caTime[strlen(caTime) - 1] = 0;
    std::ofstream outfile;

    outfile.open(FILE_DEBUG_LOG, std::ios_base::app);
    outfile << caTime << " - " << caDebugText_m << endl;


    va_end(argptr);

}

char *pcGetDebugText() {
    return caDebugText_m;
}


/**
 * Set ErrorText
 *
 */
void vSetError(int iErrorCode, const char *format, ...) {
    char caTime[100];
    va_list argptr;
    va_start(argptr, format);
    memset(caErrorText_m, 0, sizeof(caErrorText_m));
    vsnprintf(caErrorText_m, sizeof(caErrorText_m) - 1, format, argptr);
    iErrorCode_m = iErrorCode;
    bHasError_m = true;
    json jsonfile;

    // Log error
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strcpy(caTime, asctime(timeinfo));
    caTime[strlen(caTime) - 1] = 0;
    std::ofstream outfile;

    outfile.open(tagConfig_m.caErrorLog, std::ios_base::app);
    outfile << caTime << " - " << caErrorText_m << endl;

    vSetRigState(eRuntimeError);
    va_end(argptr);
}

bool bHasError() {
    return bHasError_m;
}

/**
 * return last error
 * @return
 */
char *pcGetLastError() {
    return caErrorText_m;
}

/**
 * Clear error
 */
void vClearError() {
    iErrorCode_m = 0;
    bHasError_m = false;
    memset(caErrorText_m, 0, sizeof(caErrorText_m));
    memset(caStatusText_m, 0, sizeof(caStatusText_m));
}

/**
 * Set Status Text
 *
 */
void vSetStatusText(const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    memset(caStatusText_m, ' ', sizeof(caStatusText_m));
    vsnprintf(caStatusText_m, sizeof(caStatusText_m) - 1, format, argptr);
    caStatusText_m[strlen(caStatusText_m)] = ' ';
    caStatusText_m[70] = 0;
    va_end(argptr);
}

/**
 * return status text
 * @return
 */
char *pcGetStatus() {
    return caStatusText_m;
}

/**
 * Clear Status Text
 */
void vClearStatusText() {
    memset(caStatusText_m, 0, sizeof(caStatusText_m));
}

/**
 * Save Miner Settings
 * @param ptagMinerSettings
 */
void vSaveMinerSettings(MinerSettings *ptagMinerSettings) {
    memcpy(&tagLastMinerSettings_m, ptagMinerSettings, sizeof(MinerSettings));
}

void vSetRigState(ERigState eRigState) {
    eRigState_m = eRigState;
}

ERigState eGetRigState() {
    return eRigState_m;
}

/**
 * Parse Json data/settings from dashboard server
 * @param jsonfile Json file
 */
bool bParseJson(MinerSettings *ptagMinerSettings, json jsonfile) {
    char pcDefualt[] = {"xxx"};
    char pcDefaultPill[] = {"0"};
    char pcDefaultCore[] = {"0"};
    char pcDefaultPowerLimit[] = {"0"};
    char pcDefaultHashRate[] = {"0"};
    char caState[1000];
    vSetStatusText("Parse Json Files");
    vSetError(0,"jsonfile = %s",jsonfile.dump().c_str());
    strcpy(caState, getStringFromJson(jsonfile["state"], pcDefualt));
    if (strstr(caState, pcDefualt)) {
        vSetError(0, "No Data from Dashboard");
        return false;
    } else if (strstr(caState, "error")) {
        strcpy(caState, getStringFromJson(jsonfile["message"], pcDefualt));
        vSetError(0, "Error from Dashboard : %s", caState);
        return false;
    }
  
    ptagMinerSettings->bHasFan=false;
    strcpy(ptagMinerSettings->caCurrency,getStringFromJson(jsonfile["message"]["currency"],pcDefualt));
    strcpy(ptagMinerSettings->caHostName,getStringFromJson(jsonfile["message"]["hostname"],pcDefualt));
    strcpy(ptagMinerSettings->caPool1Pass,getStringFromJson(jsonfile["message"]["pool1pass"],pcDefualt));
    strcpy(ptagMinerSettings->caPool1Port,getStringFromJson(jsonfile["message"]["pool1port"],pcDefualt));
    strcpy(ptagMinerSettings->caPool1Url,getStringFromJson(jsonfile["message"]["pool1url"],pcDefualt));
    strcpy(ptagMinerSettings->caPool1User,getStringFromJson(jsonfile["message"]["pool1user"],pcDefualt));
    strcpy(ptagMinerSettings->caWallet,getStringFromJson(jsonfile["message"]["wallet"],pcDefualt));

    // Quick check if space at begin or end and remove it
    if (ptagMinerSettings->caHostName[0] == ' ') {
        strcpy(ptagMinerSettings->caHostName, &ptagMinerSettings->caHostName[1]);
    }
    if (ptagMinerSettings->caHostName[strlen(ptagMinerSettings->caHostName) - 1] == ' ') {
        ptagMinerSettings->caHostName[strlen(ptagMinerSettings->caHostName) - 1] = 0;
    }
    if (ptagMinerSettings->caWallet[0] == ' ') {
        strcpy(ptagMinerSettings->caWallet, &ptagMinerSettings->caWallet[1]);
    }
    if (ptagMinerSettings->caWallet[strlen(ptagMinerSettings->caWallet) - 1] == ' ') {
        ptagMinerSettings->caWallet[strlen(ptagMinerSettings->caWallet) - 1] = 0;
    }

    ptagMinerSettings->iOcEnabled = jsonfile["message"]["oc_enabled"];

    ptagMinerSettings->iPillEnabled = (int)strtol(getStringFromJson(jsonfile["message"]["oc_pill_enabled"], pcDefaultPill), nullptr,0);
    ptagMinerSettings->iOcCore = (int)strtol(getStringFromJson(jsonfile["message"]["oc_core"], pcDefaultCore), nullptr,0);
    ptagMinerSettings->iOcMemCore = (int)strtol(getStringFromJson(jsonfile["message"]["oc_memcore"], pcDefaultCore), nullptr,0);
    ptagMinerSettings->iOcPowerLimit = (int)strtol(
            getStringFromJson(jsonfile["message"]["oc_powerlimit"], pcDefaultPowerLimit), nullptr,0);

 #if 0
    // Timer for Hashrate payment
    if (jsonfile["message"].find("switchbacktimer") != jsonfile["message"].end())
    {
        //TODO:iSwitchbacktimer is 0; dashboard error
        ptagMinerSettings->iSwitchbacktimer = 1;
    }
    else
    {
        ptagMinerSettings->iSwitchbacktimer=0;
    }
#endif
    //ptagMinerSettings->bUpdate = (bool)strtol(getStringFromJson(jsonfile["message"]["update"], pcDefaultHashRate), nullptr,0);

    // Gpu start script
    vSetError(0,"new values applied");
    string strtemp = DEFAULT_GPU_SCRIPT;
    strcpy(ptagMinerSettings->caStartGPUScript, getStringFromJson(jsonfile["message"]["startscript"], strtemp.c_str()));

    vSetError(0,"start script generated");
    if (strstr(ptagMinerSettings->caStartGPUScript, "ERROR")) {
        // if error use default mining script
        vSetError(0, "GPU startscript %s", ptagMinerSettings->caStartGPUScript);
        strcpy(ptagMinerSettings->caStartGPUScript, strtemp.c_str());
    }

    // Cpu start script
    //strtemp = DEFAULT_CPU_SCRIPT;
    //strcpy(ptagMinerSettings->caStartCPUScript,
    //       getStringFromJson(jsonfile["message"]["startcpuscript"], strtemp.c_str()));

    if (strstr(ptagMinerSettings->caStartCPUScript, "ERROR")) {
        // if error use default mining script
        vSetError(0, "CPU startscript %s", ptagMinerSettings->caStartCPUScript);
        strcpy(ptagMinerSettings->caStartCPUScript, strtemp.c_str());
    }
    
    vSetError(0,"bparsejson done");
    //bQuickParameterCheck(ptagMinerSettings);
    return true;
}

/**
 * Create Miner Info and send to dashboard
 */
bool bUpdateMinerInfo(HostInfo *ptagHostInfo, GpuInfo ptagGpuInfos[MAX_GPUS], CpuInfo ptagCpuInfos[MAX_CPUS],
                      ERigState eRigState) {
    string readBuffer;
    char caCurlPost[10000] = {0};
    char caMinerInfo[10000] = {0};
    char caCommand[4000];
    char caDate[1000];


    //Create Minerinfo
    memset(caMinerInfo, 0, sizeof(caMinerInfo));
    sprintf(caCommand, "date");
    string strResult = exec(caCommand);

    //Del Newline
    strcpy(caDate, strResult.c_str());
    caDate[strlen(caDate) - 1] = 0;

    // Old Dashborad api
    sprintf(caMinerInfo, "###### %s ############", caDate);
    for (int iIndex = 0; iIndex < ptagHostInfo->iGpuCount; iIndex++) {
        sprintf(&caMinerInfo[strlen(caMinerInfo)], "GPU%d(%s): %.0fC@%d%%-%.2fW : %s\n",
                iIndex,
                ptagGpuInfos[iIndex].caName,
                ptagGpuInfos[iIndex].fTemp,
                ptagGpuInfos[iIndex].iFanSpeed,
                ptagGpuInfos[iIndex].fPower,
                ptagGpuInfos[iIndex].caHashRate
        );
    }
    //Create curl POST string
    sprintf(caCurlPost, "%s=%s&%s=%s&%s=%d&%s=%s&%s=%s&%s=%d",
            "rig-hw-addr", ptagHostInfo->caMacAddr,
            "rig-ip", ptagHostInfo->caIPAddr,
            "state", eRigState,
            "errortext", pcGetLastError(),
            "minerinfo", caMinerInfo,
            "gpucount", ptagHostInfo->iGpuCount);



    // New Dashboard api
    int iGpu, iCpu;
    json jStats;
    jStats["hostInfo"]["mac"] = ptagHostInfo->caMacAddr;
    jStats["hostInfo"]["ip"] = ptagHostInfo->caIPAddr;
    jStats["hostInfo"]["state"] = eRigState;
    jStats["hostInfo"]["errorMsg"] = pcGetLastError();
    jStats["hostInfo"]["upTime"] = 1;

    jStats["gpuCount"] = ptagHostInfo->iGpuCount;
    for (iGpu = 0; iGpu < ptagHostInfo->iGpuCount; iGpu++) {
        jStats["gpuInfo"][iGpu]["name"] = ptagGpuInfos[iGpu].caName;
        jStats["gpuInfo"][iGpu]["power"] = ptagGpuInfos[iGpu].fPower;
        jStats["gpuInfo"][iGpu]["powerLimit"] = ptagGpuInfos[iGpu].fPowerLimit;
        jStats["gpuInfo"][iGpu]["temp"] = ptagGpuInfos[iGpu].fTemp;
        jStats["gpuInfo"][iGpu]["fan"] = ptagGpuInfos[iGpu].iFanSpeed;
        jStats["gpuInfo"][iGpu]["hash"] = ptagGpuInfos[iGpu].fHash;
        jStats["gpuInfo"][iGpu]["unit"] = ptagGpuInfos[iGpu].caHashUnit;
    }

    jStats["cpuBrand"] = 5;
    jStats["coreCount"] = 5;

    char caCore[200];
    for (iCpu = 0; iCpu < ptagHostInfo->iCpuCount; iCpu++) {
        sprintf(caCore, "Core %d", iCpu);
        jStats["cpuInfo"][iCpu]["name"] = caCore;
        jStats["cpuInfo"][iCpu]["power"] = ptagCpuInfos[iCpu].fPower;
        jStats["cpuInfo"][iCpu]["temp"] = ptagCpuInfos[iCpu].fTemp;
        jStats["cpuInfo"][iCpu]["hash"] = ptagCpuInfos[iCpu].fHash;
        jStats["cpuInfo"][iCpu]["unit"] = ptagCpuInfos[iCpu].caHashUnit;
    }

    //vSetDebugText("\n%s\n", jStats.dump().c_str());


    // Send Command over curl
#ifdef DISABLE_DASHBOARD_JSON_API
    readBuffer = strReadWriteCurl(tagConfig_m.caUpdateStateUrl, caCurlPost);
#else
    readBuffer=strReadWriteCurl(tagConfig_m.caUpdateStateUrl,(char*)jStats.dump().c_str());
#endif
    //cout << "readbuffer=" << readBuffer << endl;
    if (readBuffer.empty()) {
        vSetError(0, "ERROR: Cant send update to the Dashboard");
        return false;
    }
    //TODO: Error handling
}

char *pcReadCheckString(char *pcString, char *pcDefault) {
    //vSetDebugText("%s\n", pcString);
    if (pcString)
        return pcString;
    else
        return pcDefault;
}

/**
 * Get Miner Settings
 */
int iGetMinerSettings(HostInfo *ptagHostInfo, MinerSettings *ptagMinerSettings) {
    json jsonfile;
    json jStats;


    string readBuffer;
    char caCurlPost[3000];
    char pcDefualt[] = {"n/a"};
    vSetError(0,"Start igetminersettings");
    vSetStatusText("Check miner settings");
    //jStats["version"]["cgbot"] = SOFTWARE_VERSION;
    //jStats["version"]["os"] = pcReadCheckString(pcGetLinuxVersion(), pcDefualt);
    //jStats["version"]["cuda"] = pcReadCheckString((char *) pcGetCudaVersion(), pcDefualt);
    //jStats["version"]["smi"] = pcReadCheckString((char *) pcGetSmiVersion(), pcDefualt);

    //vSetDebugText("%s", jStats.dump().c_str());

    vSetError(0,"before curl post");
    //Create POST curl string
    snprintf(caCurlPost,sizeof(caCurlPost)-1, "%s=%s", "rig-hw-addr", ptagHostInfo->caMacAddr);

    vSetError(0,"GetSettings: TX=%s",caCurlPost);

#ifdef DISABLE_DASHBOARD_JSON_API
    readBuffer = strReadWriteCurl(tagConfig_m.caGetSettingsUrl, caCurlPost);
#else
    readBuffer=strReadWriteCurl(tagConfig_m.caGetSettingsUrl,(char*)jStats.dump().c_str());
#endif
    vSetError(0,"GetSettings: RX=%s",readBuffer.c_str());

    if (readBuffer.empty()) {
        vSetError(0, "ERROR: No Data form Dashboard\n");
        return -1;
    }

    // Check if json is valid
    bool bValid = json::accept(readBuffer);
    if (!bValid) {
        // write error settings to FILE_SETTINGS_ERROR
        vSetError(0, "ERROR: No Valid Json from Server, see %s\n",FILE_SETTINGS_ERROR);
        std::ofstream o(FILE_SETTINGS_ERROR);
        o << std::setw(4) << readBuffer << std::endl;
        return -1;
    }

    vSetError(0, "SUCCESS: got valid json\n");
    
    //Parse Json
    jsonfile = json::parse(readBuffer);
    if(!bParseJson(ptagMinerSettings, jsonfile)) {
	vSetError(0, "json parse error");
    	return -1;
    }

    vSetError(0, "before check if changed\n");
    if (strcmp(ptagMinerSettings->caCurrency,tagLastMinerSettings_m.caCurrency) != 0) {
	// write new settings to FILE_SETTINGS
	std::ofstream o(FILE_SETTINGS);
	o << std::setw(4) << jsonfile << std::endl;

	vSetError(0, "INFO: got new settings (%s/%s), reboot\n",ptagMinerSettings->caCurrency,tagLastMinerSettings_m.caCurrency);
	//New Settings
	vSetStatusText("New Settings reboot");
	return 1;
	} else {
		vSetError(0,"currency the same - check wallet");
		if(strcmp(ptagMinerSettings->caWallet,tagLastMinerSettings_m.caWallet) != 0) {
			// write new settings to FILE_SETTINGS
			std::ofstream o(FILE_SETTINGS);
			o << std::setw(4) << jsonfile << std::endl;
			vSetError(0, "INFO: got new settings (%s/%s), reboot\n",ptagMinerSettings->caWallet,tagLastMinerSettings_m.caWallet);
			//New Settings
			vSetStatusText("New Settings reboot");
			return 1;
		} else {
			vSetError(0,"wallet the same - check pool");
		}

	}
	
    vSetError(0, "INFO: settingsup2date\n");
    vSetStatusText("settings up2date");

    return 0;
}

/**
 *
 * @param ptagMinerSettings
 * @return
 */
int iLoadMinerSettings(MinerSettings *ptagMinerSettings, bool bCheckChanged) {
    char caBuffer[10000] = {0};
    string strBuffer;
    // Read settings from ssd and store it
    if (bFileExists(FILE_SETTINGS)) {
        vSetStatusText("Read settings <%s>", FILE_SETTINGS);
        std::ifstream file(FILE_SETTINGS);

        // Open settings and read all into buffer
        if (file.is_open()) {
            while (getline(file, strBuffer))
                strcat(caBuffer, strBuffer.c_str());
            file.close();
        }

        // Check if json is valid
        bool bValid = json::accept(caBuffer);
        if (!bValid) {
            printf("%s not valid json\n", FILE_SETTINGS);
            return -1;
        }
        json jsonfile;
        // Parse json file
        jsonfile = json::parse(caBuffer);
        bParseJson(ptagMinerSettings, jsonfile);
        // Save to last settings
        vSaveMinerSettings(ptagMinerSettings);

    } else {
        if(bCheckChanged)
        {
            vSetError(0,"Cant detect new Settings, file is missing");
            return -1;
        }
        // grap from server
        //vSetError(0,"Settings at <%s> not found",FILE_SETTINGS);
        ;
    }
    if(bCheckChanged)
    {
        // test if new Settings are available
        if (memcmp(&tagLastMinerSettings_m, ptagMinerSettings, sizeof(MinerSettings)) != 0) {
            //Neu Settings
            vSetStatusText("New Settings detected");
            return 1;
        }
    }
    return true;
}


/**
 * Parse Config.json file
 * @param ptagMinerSettings Miner Settings
 * @param ptagConfgetVersionsig Config Data
 * @param jsonfile pointer to json file
 * @return
 */
bool bParseConfigJson(MinerSettings *ptagMinerSettings, TagConfig *ptagConfig, json jsonfile) {
    char pcDefualt[] = {"n/a"};


    vSetStatusText("Reading config.json");
    strcpy(ptagConfig->caHomePath, getStringFromJson(jsonfile["homepath"], pcDefualt));
    strcpy(ptagConfig->caScriptTemplate, getStringFromJson(jsonfile["scriptTemplate"], pcDefualt));
    sprintf(ptagConfig->caErrorLog, "%s/%s", ptagConfig->caHomePath,
            getStringFromJson(jsonfile["errorlog"], pcDefualt));

    ptagConfig->iCheckStateInterval = jsonfile["checkStateInterval"];
    ptagConfig->iGetSettingsInterval = jsonfile["getSettingsInterval"];
    ptagConfig->bUseDashboard = jsonfile["dashboard"]["enabled"];

    try {
        ptagConfig->bHasFan= jsonfile["hasFan"];
    }
    catch (int e) {
        ptagConfig->bHasFan=false;
    }
    strcpy(ptagConfig->caGpuManufacurer, getStringFromJson(jsonfile["GpuManufacurer"], pcDefualt));

    strcpy(ptagConfig->caGetSettingsUrl, getStringFromJson(jsonfile["dashboard"]["getSettings"], pcDefualt));
    strcpy(ptagConfig->caUpdateStateUrl, getStringFromJson(jsonfile["dashboard"]["updateState"], pcDefualt));
    try
    {
        //jsonfile["standalone"]["enabled"];
        ptagConfig->bUseOC = jsonfile["standalone"]["useOC"];
        ptagConfig->iOcCore = jsonfile["standalone"]["OCcore"];
        ptagConfig->iOcMemCore = jsonfile["standalone"]["OCmemcore"];
        ptagConfig->iPowerLimit = jsonfile["standalone"]["powerlimit"];

    }
    catch (int e)
    {
        vSetError(0,"An exception occurred. Exception Nr %d",e);
    }

    ptagConfig->bUsePill = jsonfile["standalone"]["useThePill"];
    strcpy(ptagConfig->caPillPath, getStringFromJson(jsonfile["standalone"]["thePillPath"], pcDefualt));

    ptagConfig->bGenerateXorg = jsonfile["generateXorg"]["enabled"];
    sprintf(ptagConfig->caXorgScriptPath, "%s/%s", ptagConfig->caHomePath,
            getStringFromJson(jsonfile["generateXorg"]["script"], pcDefualt));

    return true;
}

/**
 * Read config.json file
 * @param ptagMinerSettings Miner Settings
 * @param ptagConfig Config Data
 * @return true = success
 *         false = failed
 **/
bool bLoadConfig(char *pcConfig, MinerSettings *ptagMinerSettings, TagConfig *ptagConfig) {
    char caBuffer[10000] = {0};
    string strBuffer;

    // Read settings from ssd and store it
    if (bFileExists(pcConfig)) {
        vSetStatusText("Read settings <%s>", pcConfig);
        std::ifstream file(pcConfig);
        // Open settings and read all into buffer
        if (file.is_open()) {
            while (getline(file, strBuffer)) {
                // filter json comments, only oneliner
                if (!strstr(strBuffer.c_str(), "*/"))
                    strcat(caBuffer, strBuffer.c_str());
            }
            file.close();
        } else {
            return false;
        }

        // Check if json is valid
        bool bValid = json::accept(caBuffer);
        if (!bValid) {
            printf("%s not valid json\n", pcConfig);
            return false;
        }
        json jsonfile;
        // Parse json file
        jsonfile = json::parse(caBuffer);
        try
        {
            bParseConfigJson(ptagMinerSettings, ptagConfig, jsonfile);
        }
        catch (int e)
        {
            vSetDebugText("An exception occurred. Exception Nr %d",e);
            cout << "An exception occurred. Exception Nr. " << e << '\n';
        }
        // Save to last settings
        //vSaveMinerSettings(ptagMinerSettings);
    } else {
        // grap from server
        //vSetError(0,"Settings at <%s> not found",FILE_SETTINGS);
        return false;
    }
    return true;
}


/**
 * Read nvidia-smi and load it to tagGpuInfo_m
 * @see tagGpuInfo_m
 * @return negavie = error
 *         positiv = gpucount
 */
int iReadNvideaInfo(GpuInfo ptagGpuInfos[MAX_GPUS]) {
    vector<string> strOutPut;
    vector<string> strGpuVal;
    //execute nvidia-smi on terminal
  
    string strNvidiaResult="";
    if(!strcmp(tagConfig_m.caGpuManufacurer,"AMD")) {
        strNvidiaResult= exec("lspci -v | grep VGA");
    } else {
        strNvidiaResult= exec("nvidia-smi --query-gpu=temperature.gpu,power.draw,fan.speed,gpu_name --format=csv,noheader,nounits");
    }
    //cout << strNvidiaResult << endl;
    //TODO: better Error Handling

    size_t found = strNvidiaResult.find("Not Supported");
    if (found != string::npos) {
        vSetError(0, "Nvidea Command not supported");
        return -1;
    }
    found = strNvidiaResult.find("ERR");
    if (found != string::npos) {
        vSetError(0, "Error detected");
        return -2;
    }
    found = strNvidiaResult.find("Reboot the system");
    if (found != string::npos) {
        vSetError(0, "Nvidia smi error: %s", strNvidiaResult.c_str());
        return -3;
    }

    //Split count = GPU Count

    auto iGPUCount = (int)split(strNvidiaResult, strOutPut, '\n' );
    int iCounter=0;
    for (vector<string>::iterator itGpu = strOutPut.begin(); itGpu != strOutPut.end(); ++itGpu)
    {
        if(!strcmp(tagConfig_m.caGpuManufacurer,"AMD")) {
            //Split in Substrings
            auto iValueCount=split(*itGpu, strGpuVal, '[' );

            // expected format + ignore last substring
            if(iValueCount<4)
                continue;

            vector<string>::iterator itGpuCount=strGpuVal.begin();
            // TODO: Values hardcoded for now - find a way to parse them
            ptagGpuInfos[iCounter].fTemp = 40;
            ptagGpuInfos[iCounter].fPower = 80;
            ptagGpuInfos[iCounter].iFanSpeed = 90;
            strncpy(ptagGpuInfos[iCounter].caName,itGpuCount[2].c_str(),999);
			char *pcEnd = strchr(ptagGpuInfos[iCounter].caName, ']');
			*pcEnd = 0;
		} else {
            //Split in Substrings
            auto iValueCount=split(*itGpu, strGpuVal, ',' );

            // expected format + ignore last substring
            if(iValueCount<4)
                continue;

            vector<string>::iterator itGpuCount=strGpuVal.begin();
            ptagGpuInfos[iCounter].fTemp = strtof((itGpuCount[0]).c_str(),0);
            ptagGpuInfos[iCounter].fPower = strtof((itGpuCount[1]).c_str(),0);
            ptagGpuInfos[iCounter].iFanSpeed = (int) strtol((itGpuCount[2]).c_str(),0,10);
            strcpy(ptagGpuInfos[iCounter].caName,itGpuCount[3].c_str());
        }
        
        iCounter++;
    }
    return iCounter;
}

int iGetCpuInfo(CpuInfo ptagCpuInfos[MAX_CPUS]) {
    vector<string> strOutPut;
    vector<string> strCpuVal;
    int iCounter = 0;

    // Read Model name
    string strResult = exec("cat /proc/cpuinfo | grep \"model name\"");
    split(strResult, strOutPut, '\n');
    for ( auto itCpu = strOutPut.begin(); itCpu != strOutPut.end(); ++itCpu) {
        //Split in Substrings
        split(*itCpu, strCpuVal, ':');
        auto itGpuCount = strCpuVal.begin();

        // Parse CPU name
        strcpy(ptagCpuInfos[iCounter].caName, itGpuCount[1].c_str());
        iCounter++;
    }

    // Get CPU temp
    strResult = exec("cat /sys/class/thermal/thermal_zone*/temp");
    split(strResult, strOutPut, '\n');
    iCounter = 0;
    for ( auto itCpu = strOutPut.begin(); itCpu != strOutPut.end(); ++itCpu) {
        // Parse CPU name
        ptagCpuInfos[iCounter].fTemp = (float)(strtod(itCpu[0].c_str(), nullptr) / 1000.f);

        iCounter++;
    }
    return iCounter - 1;
}

int iGetCPUPowerUsage(HostInfo *pHostInfo, CpuInfo ptagCpuInfos[MAX_CPUS]) {
    static long iCurrentJoule = 0;
    static long iLastJoule = 0;
    static time_t tNow;
    static time_t tLast;

    if (iLastJoule == 0) {
        // frist round
        tLast = time(nullptr); // read current time in sec
        string strResult = exec("cat /sys/class/powercap/intel-rapl/intel-rapl:0/energy_uj");
        iLastJoule = (int)strtol(strResult.c_str(), nullptr,0) / 1000000; // mili Joule to Joule
        return 0;
    } else {
        tNow = time(nullptr); // read current time in sec
        string strResult = exec("cat /sys/class/powercap/intel-rapl/intel-rapl:0/energy_uj");
        iCurrentJoule = strtol(strResult.c_str(), nullptr,0) / 1000000; // mili Joule to Joule

        double fWattUsage = ((double) iCurrentJoule - (double) iLastJoule) / (tNow - tLast);
        int iIndex;
        for (iIndex = 0; iIndex < pHostInfo->iCpuCount; iIndex++)
            ptagCpuInfos[iIndex].fPower = (float) fWattUsage / 4.0f;
        tLast = tNow;
        iLastJoule = iCurrentJoule;
        return 1;
    }
}


bool bGetVersions()
{

}

