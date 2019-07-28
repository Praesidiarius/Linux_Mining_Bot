//
// Created by JBinggi on 01.07.18.
//

#ifndef CG_MINING_BOT_HELPER_H
#define CG_MINING_BOT_HELPER_H

#include <iostream>
#include "../nlohmann/json.hpp"
#include "data.h"

void vDownLoadUpdateFile(void);

void vUpdateCgMiningBot(TagConfig *ptagConfig, char *pcVersion);

void vDownloadMiners(void);

char *getStringFromJson(nlohmann::json jsonItem, const char *pcDefualt);

std::string exec(const char *cmd);

size_t split(const std::string &txt, std::vector<std::string> &strs, char ch);

std::string strReadWriteCurl(std::string, char *pcPostString);

bool bFileExists(const std::string &name);

void vSetHostName(char *pcHostname);

bool bGetHostInfo(HostInfo *ptagHostInfo);

int iListAllScreenSessions(char caaSreens[10][100]);

void vScreenAttach(char *pcScreen);

char *pcGetLinuxVersion(void);

const char *pcGetCudaVersion(void);

const char *pcGetSmiVersion(void);

void vReboot(int iSec);

int iSendTelnet(const char *pcHost, int iPort, const char *pcMessage, unsigned char *pcResponse, int iSize);

#endif //CG_MINING_BOT_HELPER_H
