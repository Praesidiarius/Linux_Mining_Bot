//
// Created by JBinggi on 03.07.18.
//

using namespace std;

#include <cstdio>
#include <zconf.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <thread>         // std::thread
#include <menu.h>
#include "ngui.h"
#include "defines.h"
#include "data.h"
#include "minerapi.h"
#include "helper.h"


#define STATUS_HEIGHT 4
#define SETTINGS_HEIGHT 6
#define GPU_HEIGHT 10

extern bool bExitProgram_m;

GpuInfo *patagGpuInfos_m;
CpuInfo *patagCpuInfos_m;
HostInfo *ptagHostInfo_m;
MinerSettings *ptagMinerSettings_m;

extern bool bHashRatePayment_m;
bool bForceGuiUpdate_m = false;
bool bScreenMenu_m = false;

typedef enum {
    eBlackWhite = 10,
    eBlackGreen,
    eBlackRed,
    eBlackCyan,
    eBlackYellow,
    eWhiteBlack,
    eRedYellow,
    eWhiteCyan
} EGuiColor;


typedef enum {
    eGpu,
    eCpu,
    eOther
} EMiner;

EMiner eMinerType_m;

char *pcCutWalletToLen(char *pcWallet, int iMaxLen) {
    static char caWallet[200];

    if (strlen(pcWallet) > iMaxLen) {
        strcpy(caWallet, pcWallet);
        caWallet[(iMaxLen / 2) - 3] = 0;
        strcat(caWallet, "...");
        strcat(caWallet, &pcWallet[strlen(pcWallet) - (iMaxLen / 2)]);
        return caWallet;

    }
    return pcWallet;
}

/**
 * Print host info section
 * @param winHostInfo
 */
void printHostInfo(WINDOW *winHostInfo) {
    box(winHostInfo, 0, 0);
    mvwprintw(winHostInfo, 1, 2, "Hostname: %s    IP: %s    MAC: %s ",
              ptagHostInfo_m->caHostName, ptagHostInfo_m->caIPAddr, ptagHostInfo_m->caMacAddr);

    wrefresh(winHostInfo);
}

/**
 * Prints status and error section
 * @param winStatus
 */
void printStatusErrorText(WINDOW *winStatus) {
    box(winStatus, 0, 0);
    if (eGetRigState() == eRestarting || eGetRigState() == eOffline) {
        wattron(winStatus, COLOR_PAIR((bHasError()) ? eBlackRed : eBlackYellow));
        mvwprintw(winStatus, 1, 2, "%s", "Status:");
        wattroff(winStatus, COLOR_PAIR((bHasError()) ? eBlackRed : eBlackYellow));
    } else if (eGetRigState() == eOnline) {
        wattron(winStatus, COLOR_PAIR((bHasError()) ? eBlackRed : eBlackGreen));
        mvwprintw(winStatus, 1, 2, "%s", "Status:");
        wattroff(winStatus, COLOR_PAIR((bHasError()) ? eBlackRed : eBlackGreen));
    }
    wattron(winStatus, COLOR_PAIR(eWhiteBlack));
    mvwprintw(winStatus, 1, 10, "%s", pcGetStatus());
    wattroff(winStatus, COLOR_PAIR(eWhiteBlack));

    if (bHasError()) {
        wattron(winStatus,COLOR_PAIR(eRedYellow));
        mvwprintw(winStatus, 2, 2, "%s %s", "Error :", pcGetLastError());
        wattroff(winStatus, COLOR_PAIR(eRedYellow));
    }
    wrefresh(winStatus);
}

/**
 * Prints Settings section
 * @param winSettings
 */
void printSettings(WINDOW *winSettings) {
    box(winSettings, 0, 0);

    //if(has_colors())
    //wattron(winSettings,COLOR_PAIR(eBlackWhite));

    mvwprintw(winSettings, 1, 2, "Url    : %s:%s", ptagMinerSettings_m->caPool1Url, ptagMinerSettings_m->caPool1Port);
    mvwprintw(winSettings, 2, 2, "PUser  : %s", pcCutWalletToLen(ptagMinerSettings_m->caPool1User, 65));
    mvwprintw(winSettings, 3, 2, "PPass  : %s", ptagMinerSettings_m->caPool1Pass);
    mvwprintw(winSettings, 4, 2, "OC: enabled/pill/core/mem/pl  %s/%s/%d/%d/%d",
              (ptagMinerSettings_m->iOcEnabled) ? "true" : "false",
              (ptagMinerSettings_m->iPillEnabled) ? "true" : "false",
              ptagMinerSettings_m->iOcCore,
              ptagMinerSettings_m->iOcMemCore,
              ptagMinerSettings_m->iOcPowerLimit);

    //if(has_colors())
    //wattron(winSettings,COLOR_PAIR(eBlackWhite));

    wrefresh(winSettings);
}

/**
 * Prints GPU info section
 * @param winGpuInfo
 */

void vPrintGPUMinerInfo(WINDOW *winGpuInfo)
{
    char const caH[10][20]={"GpuNr","GpuName","Hash","Tmp","Fan","Watt",0};
    const char caGPUFormat[100]       ={"GPU(%d) %s  %s  %.0fC   %d%%   %.2fW"};
    const char caGPUFormat_Header[100]={"GPU(%d);%s ;%s ;%.0fC  ;%d%%  S;%.2fW"};

    char caHeaderTemp[200];
    char caHeader[200];
    int iIndex;

    // clear screen and build box
    wclear(winGpuInfo);
    box(winGpuInfo, 0, 0);

    if (patagGpuInfos_m[0].caHashRate[0] == 0)
        strcpy(patagGpuInfos_m[0].caHashRate, "n/a MH/s");

    // Format Header -> beautify this code section
    sprintf(caHeaderTemp, caGPUFormat_Header, 0,
            patagGpuInfos_m[0].caName,
            patagGpuInfos_m[0].caHashRate,
            patagGpuInfos_m[0].fTemp,
            patagGpuInfos_m[0].iFanSpeed,
            20.0);
    bool bPaste = true;
    int iPasteIndex = 0;
    int iIndex2 = 0;
    for (iIndex = 0; iIndex < 80; iIndex++) {
        if (caHeaderTemp[iIndex] == ';') {
            bPaste = true;
            caHeader[iIndex] = ' ';
            continue;
        }
        if (bPaste) {
            if (caH[iPasteIndex][iIndex2] == 0) {
                caHeader[iIndex] = ' ';
                bPaste = false;
                iPasteIndex++;
                iIndex2 = 0;
                continue;
            }
            caHeader[iIndex] = caH[iPasteIndex][iIndex2];
            iIndex2++;
            continue;
        } else
            caHeader[iIndex] = ' ';

    }
    caHeader[iIndex] = 0;

    // print header
    mvwprintw(winGpuInfo, 1, 2, caHeader);

    for (iIndex = 0; iIndex < ptagHostInfo_m->iGpuCount; iIndex++) {
        if (patagGpuInfos_m[iIndex].caHashRate[0] == 0)
            strcpy(patagGpuInfos_m[iIndex].caHashRate, "n/a MH/s");

        //if(has_colors())
        //wattron(winSettings,COLOR_PAIR(eBlackWhite));
        mvwprintw(winGpuInfo, iIndex + 2, 2, caGPUFormat,
                  iIndex,
                  patagGpuInfos_m[iIndex].caName,
                  patagGpuInfos_m[iIndex].caHashRate,
                  patagGpuInfos_m[iIndex].fTemp,
                  patagGpuInfos_m[iIndex].iFanSpeed,
                  patagGpuInfos_m[iIndex].fPower);


        //if(has_colors())
        //wattron(winSettings,COLOR_PAIR(eBlackWhite));

    }
    wrefresh(winGpuInfo);

}

/**
 * Prints CPU info sectioniGetScreens
 * @param winCpuInfo
 */

void vPrintCPUMinerInfo(WINDOW *winCpuInfo)
{
    char const caH[10][20]={"CpuNr","CpuName","Hash","Tmp","Watt",0};
    const char caCPUFormat[100]       ={"CPU(%d) %s  %s  %02.0fC  %.2fW"};
    const char caCPUFormat_Header[100]={"CPU(%d);%s ;%s ;%02.0fC ;%.2fW"};

    char caHeaderTemp[200];
    char caHeader[200];
    int iIndex;

    // clear screen and build box
    wclear(winCpuInfo);
    box(winCpuInfo, 0, 0);

    if (patagCpuInfos_m[0].caHashRate[0] == 0)
        strcpy(patagCpuInfos_m[0].caHashRate, "n/a MH/s");

    // Format Header -> beautify this code section

    sprintf(caHeaderTemp,caCPUFormat_Header,0,
                                        patagCpuInfos_m[0].caName,
                                        patagCpuInfos_m[0].caHashRate,
                                        patagCpuInfos_m[0].fTemp,
                                        patagCpuInfos_m[0].fPower);
    bool bPaste=true;
    int iPasteIndex=0;
    int iIndex2=0;
    for(iIndex=0;iIndex<80;iIndex++)
    {
        if(caHeaderTemp[iIndex]==';')
        {
            bPaste=true;
            caHeader[iIndex]=' ';

            continue;
        }
        if (bPaste) {
            if (caH[iPasteIndex][iIndex2] == 0) {
                caHeader[iIndex] = ' ';
                bPaste = false;
                iPasteIndex++;
                iIndex2 = 0;
                continue;
            }
            caHeader[iIndex] = caH[iPasteIndex][iIndex2];
            iIndex2++;
            continue;
        } else
            caHeader[iIndex] = ' ';

    }
    caHeader[iIndex] = 0;

    // print header
    mvwprintw(winCpuInfo, 1, 2, caHeader);

    for (iIndex = 0; iIndex < ptagHostInfo_m->iCpuCount; iIndex++) {
        if (patagCpuInfos_m[iIndex].caHashRate[0] == 0)
            strcpy(patagCpuInfos_m[iIndex].caHashRate, "n/a MH/s");
        //if(has_colors())

            //wattron(winSettings,COLOR_PAIR(eBlackWhite));
        mvwprintw(winCpuInfo, iIndex+2, 2, caCPUFormat,

                  iIndex,
                  patagCpuInfos_m[iIndex].caName,
                  patagCpuInfos_m[iIndex].caHashRate,
                  patagCpuInfos_m[iIndex].fTemp,
                  patagCpuInfos_m[iIndex].fPower);

        //if(has_colors())
        //wattron(winSettings,COLOR_PAIR(eBlackWhite));

    }
    wrefresh(winCpuInfo);
}


ITEM **itScreen_m;
MENU *mMenu_m;

/**
 * Quit screen choise menu
 */
void vQuitMenu() {
    int i;

    unpost_menu(mMenu_m);
    free_menu(mMenu_m);

    for (i = 0; i <= 4; i++)
        free_item(itScreen_m[i]);

    free(itScreen_m);
}

/**
 * Print screen choise menu
 * @param caaSreens Array of screens
 */
void vPrintScreenChoiseMenu(char caaSreens[10][100]) {

    int iIndex;
    itScreen_m = (ITEM **) calloc(10, sizeof(ITEM *));
    for (iIndex = 0; iIndex < 10; iIndex++) {
        if (caaSreens[iIndex][0] == 0)
            break;
        itScreen_m[iIndex] = new_item(caaSreens[iIndex], "   ");
    }
    itScreen_m[iIndex] = nullptr;

    mMenu_m = new_menu(itScreen_m);
    set_menu_format(mMenu_m, 10, 1);

    post_menu(mMenu_m);

}


void vCreateMenu(WINDOW *wMenu) {
    char caMenu[10][20] = {"checkminer", "getsettings", "screen attach", "reboot", "help"};
    char caCommand[10][20] = {"c/C", "g/G", "s/S", "r/R", "h/H"};
    if (has_colors())
        // print menu
        if (has_colors())
            wattron(wMenu, COLOR_PAIR(eBlackCyan));
    mvwprintw(wMenu, 0, 3, caMenu[0]);
    mvwprintw(wMenu, 0, (int)strlen(caMenu[0]) + 6, caMenu[1]);
    mvwprintw(wMenu, 0, (int)strlen(caMenu[0]) + (int)strlen(caMenu[1]) + 9, caMenu[2]);
    mvwprintw(wMenu, 0, (int)strlen(caMenu[0]) + (int)strlen(caMenu[1]) + (int)strlen(caMenu[2]) + 12, caMenu[3]);
    mvwprintw(wMenu, 0, (int)strlen(caMenu[0]) + (int)strlen(caMenu[1]) + (int)strlen(caMenu[2]) + (int)strlen(caMenu[3]) + 15, caMenu[4]);
    if (has_colors()) {
        wattroff(wMenu, COLOR_PAIR(eBlackCyan));
        wattron(wMenu, COLOR_PAIR(eWhiteBlack));
    }
    // print shortcuts
    mvwprintw(wMenu, 0, 0, caCommand[0]);
    mvwprintw(wMenu, 0, (int)strlen(caMenu[0]) + 3, caCommand[1]);
    mvwprintw(wMenu, 0, (int)strlen(caMenu[0]) + (int)strlen(caMenu[1]) + 6, caCommand[2]);
    mvwprintw(wMenu, 0, (int)strlen(caMenu[0]) + (int)strlen(caMenu[1]) + (int)strlen(caMenu[2]) + 9, caCommand[3]);
    mvwprintw(wMenu, 0, (int)strlen(caMenu[0]) + (int)strlen(caMenu[1]) + (int)strlen(caMenu[2]) + (int)strlen(caMenu[3]) + 12,
              caCommand[4]);
    if (has_colors())
        wattroff(wMenu, COLOR_PAIR(eWhiteBlack));
    wrefresh(wMenu);

}

void tInputHandler(WINDOW *wMenu) {
    char caScreens[10][100];
    bool bExit = false;
    int iChar = getch();
    int iRc;
    int iScreens = 0;
    static int iRebootCounter;

#if 1
    while (!bExitProgram_m) {
        ptagHostInfo_m->gui_mutex.lock();
        if (bScreenMenu_m) {
            switch (iChar) {
                case 'q':
                    bScreenMenu_m = false;
                    vQuitMenu();
                    bForceGuiUpdate_m = true;
                    clear();
                    break;
                    //After first screeen getch() delivers 066 for KEY_DOWN
                case KEY_DOWN:
                    menu_driver(mMenu_m, REQ_DOWN_ITEM);
                    break;
                case KEY_UP:
                    menu_driver(mMenu_m, REQ_UP_ITEM);
                    break;
                case 0xA: /* Return- bzw. Enter-Taste -> ASCII-Code */
                    if ((item_index(current_item(mMenu_m)) != iScreens)) {
                        vScreenAttach(caScreens[item_index(current_item(mMenu_m))]);
                        keypad(stdscr, TRUE);           /* We get F1, F2 etc..          */
                    }
                    bScreenMenu_m = false;
                    vQuitMenu();
                    bForceGuiUpdate_m = true;
                    clear();
                    break;
                default:
                    break;
            }
        } else {
            switch (iChar) {
                case 'q':
                    clear();
                    exit(0);
                case 'u':
                    bForceGuiUpdate_m = true;
                    break;
                case 'd':
                    vDownLoadUpdateFile();
                    break;
                case 'f':
                    vDownloadMiners();
                    break;
                case 'r':
                case 'R':
                    // Press 3 times to reboot
                    if (iRebootCounter > 1) {

                        clear();
                        vReboot(0);
                    } else
                        iRebootCounter++;
                    break;
                case '1':
                    eMinerType_m = eGpu;
                    bForceGuiUpdate_m = true;
                    break;
                case '2':
                    eMinerType_m = eCpu;
                    bForceGuiUpdate_m = true;
                    break;
                case 's':
                case 'S':
                    bScreenMenu_m = true;
                    iScreens = iListAllScreenSessions(caScreens);
                    vPrintScreenChoiseMenu(caScreens);
                    break;
                case 'c':
                case 'C':
                    vClearError();
                    vSetStatusText("Read Miner Infos");
                    iRc = iReadNvideaInfo(patagGpuInfos_m);
                    if (iRc < 0) {
                        //Error occurred
                        if (iRc == -1) {
                            vSetError(0,"Cant read Nvidia Infos");
                            vSetRigState(eError);
                            bUpdateMinerInfo(ptagHostInfo_m, patagGpuInfos_m, patagCpuInfos_m, eGetRigState());
                        } else {
                            vSetRigState(eError);
                            bUpdateMinerInfo(ptagHostInfo_m, patagGpuInfos_m, patagCpuInfos_m, eGetRigState());
                        }
                    }

                    iGetCpuInfo(patagCpuInfos_m);
                    iCheckAllMiners(ptagHostInfo_m, patagGpuInfos_m, patagCpuInfos_m);
                    vSetStatusText("Read Miner Infos finished");
                    if (!patagGpuInfos_m[0].bOnline) {

                        ;// bExit = true;
                    }
                    break;
                case 'g':
                case 'G':
                    vClearError();
                    vSetStatusText("Check Miner");
                    iRc = iGetMinerSettings(ptagHostInfo_m, ptagMinerSettings_m);

                    if (iRc == 1) {
                        vSetStatusText("New Settings, reboot required");
                    } else if (iRc == 0)
                        vSetStatusText("Settings up to date");
                    else {
                        vSetStatusText(nullptr, "Bad json see %s", FILE_SETTINGS_ERROR);
                    }
                    break;
                default:
                    break;
            }
        }

        fflush(stdin);
        ptagHostInfo_m->gui_mutex.unlock();

        iChar = getch();
        refresh();
    }
#endif
}


/**
 *  Thread vGuiThread0
 *
 *  Interval: INTERVAL_GETSETTINGS
 *
 *  Reading current settings form the dashboard server,
 *  generate new start script if needed and performs reboots.
 *  Performs reboot.
 *
 */
void vGuiThread(HostInfo *ptagHostInfo, GpuInfo patagGpuInfos[MAX_GPUS], CpuInfo patagCpuInfos[MAX_GPUS],
                MinerSettings *ptagMinerSettings) {
    char caBuffer[100];
    WINDOW *winStatus;
    WINDOW *winSettings;
    WINDOW *winGpuInfo;
    WINDOW *winHostInfo;
    WINDOW *winMenu;
    struct winsize screenSize;
    eMinerType_m = eGpu;

    patagGpuInfos_m = patagGpuInfos;
    patagCpuInfos_m = patagCpuInfos;
    ptagHostInfo_m = ptagHostInfo;
    ptagMinerSettings_m = ptagMinerSettings;

    GpuInfo tagLastGpuInfos_m[MAX_GPUS];
    CpuInfo tagLastCpuInfos_m[MAX_CPUS];
    MinerSettings tagLastMinerSettings_m;

    memcpy(tagLastCpuInfos_m, patagCpuInfos, sizeof(CpuInfo) * MAX_GPUS);
    memcpy(tagLastGpuInfos_m, patagGpuInfos, sizeof(GpuInfo) * MAX_GPUS);
    memcpy(&tagLastMinerSettings_m, ptagMinerSettings, sizeof(MinerSettings));

    // init ncurers
    initscr();
    clear();
    keypad(stdscr, TRUE);           /* We get F1, F2 etc..          */
    noecho();
    cbreak();    /* Line buffering disabled. pass on everything */

    // get screen size
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &screenSize);

    // generate Windows
	if (screenSize.ws_col < 10)
		screenSize.ws_col = 80;

    winHostInfo  = newwin(3, screenSize.ws_col, 1, 0);
    winStatus  = newwin(STATUS_HEIGHT, screenSize.ws_col, 3, 0);
    winSettings= newwin(SETTINGS_HEIGHT, screenSize.ws_col, STATUS_HEIGHT+2, 0);
    winGpuInfo = newwin(9+3, screenSize.ws_col, SETTINGS_HEIGHT+STATUS_HEIGHT+1, 0);
    winMenu    = newwin(13, screenSize.ws_col, 23 , 0);


    keypad(winStatus, TRUE);

    refresh();

    if (has_colors()) {
        start_color();            /* Start color 			*/
        init_pair(eBlackWhite, COLOR_BLACK, COLOR_WHITE);
        init_pair(eBlackGreen, COLOR_BLACK, COLOR_GREEN);
        init_pair(eBlackRed, COLOR_BLACK, COLOR_RED);
        init_pair(eBlackCyan, COLOR_BLACK, COLOR_CYAN);
        init_pair(eBlackYellow, COLOR_BLACK, COLOR_YELLOW);
        init_pair(eWhiteBlack, COLOR_WHITE, COLOR_BLACK);
        init_pair(eRedYellow, COLOR_RED, COLOR_YELLOW);
        init_pair(eWhiteCyan, COLOR_WHITE, COLOR_CYAN);
    }

    thread InputHandler(tInputHandler, winMenu);
    sleep(1);

    vCreateMenu(winMenu);
    wrefresh(winStatus);
    printHostInfo(winHostInfo);

    char caStatus[1000];
    char caError[1000];
    strcpy(caStatus, pcGetStatus());
    strcpy(caError, pcGetLastError());

    int iCounter = 0;
    while (!bExitProgram_m) {
        if (bScreenMenu_m) {
            // screen menu open
            sleep(1);
            continue;
        }

        ++iCounter;
        if(!(iCounter%3))
        {
            //Update Time an Welcome info

            time_t now = time(nullptr);
            struct tm now_tm = *localtime(&now);

            // print time stamp
            memset(caBuffer, 0, sizeof(caBuffer));
            sprintf(caBuffer, "Update: %02d:%02d:%02d", now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);
            if (has_colors())
                attron(COLOR_PAIR(eBlackWhite));
            mvprintw(0, 63, "%s", caBuffer);
            if (has_colors())
                attroff(COLOR_PAIR(eBlackWhite));

            // print GPU status
            if (has_colors())
                attron(COLOR_PAIR((tagLastGpuInfos_m[0].bOnline) ? eBlackGreen : eBlackRed));
            mvprintw(0, 0, "%s", "GPU");
            if (has_colors())
                attroff(COLOR_PAIR((tagLastGpuInfos_m[0].bOnline) ? eBlackGreen : eBlackRed));

            // print CPU status
            if (has_colors())
                attron(COLOR_PAIR((tagLastCpuInfos_m[0].bOnline) ? eBlackGreen : eBlackRed));
            mvprintw(0, 4, "%s", "CPU");
            if (has_colors())
                attroff(COLOR_PAIR((tagLastCpuInfos_m[0].bOnline) ? eBlackGreen : eBlackRed));

            // print if hashrate payment
            if(bHashRatePayment_m) {
                if (has_colors())
                    attron(COLOR_PAIR(eBlackGreen));
                mvprintw(0, 4, "%s", "PAY");
                if (has_colors())
                    attroff(COLOR_PAIR(eBlackGreen));
            }

            // print version
            sprintf(caBuffer, "Welcome to CG Mining Bot V%s", SOFTWARE_VERSION);
            if (has_colors())
                attron(COLOR_PAIR(eBlackWhite));
            mvprintw(0,20, "%s", caBuffer);
            if(has_colors())

                attroff(COLOR_PAIR(eBlackWhite));

            if (memcmp(tagLastCpuInfos_m, patagCpuInfos, sizeof(CpuInfo) * MAX_CPUS) != 0) {
                memcpy(tagLastCpuInfos_m, patagCpuInfos, sizeof(CpuInfo) * MAX_CPUS);
            }
            if (memcmp(tagLastGpuInfos_m, patagGpuInfos, sizeof(GpuInfo) * MAX_GPUS) != 0) {
                memcpy(tagLastGpuInfos_m, patagGpuInfos, sizeof(GpuInfo) * MAX_GPUS);
            }

            refresh();
        }
        if (eMinerType_m == eCpu) {
            if (memcmp(tagLastCpuInfos_m, patagCpuInfos, sizeof(CpuInfo) * MAX_CPUS) != 0) {
                memcpy(tagLastCpuInfos_m, patagCpuInfos, sizeof(CpuInfo) * MAX_CPUS);
                vPrintCPUMinerInfo(winGpuInfo);
                refresh();
            }
        }
        if (eMinerType_m == eGpu) {
            if (memcmp(tagLastGpuInfos_m, patagGpuInfos, sizeof(GpuInfo) * MAX_GPUS) != 0) {
                memcpy(tagLastGpuInfos_m, patagGpuInfos, sizeof(GpuInfo) * MAX_GPUS);
                vPrintGPUMinerInfo(winGpuInfo);
                refresh();
            }
        }
        if (memcmp(&tagLastMinerSettings_m, ptagMinerSettings, sizeof(MinerSettings)) != 0) {
            memcpy(&tagLastMinerSettings_m, ptagMinerSettings, sizeof(MinerSettings));
            printSettings(winSettings);
            refresh();
        }
        if ((strcmp(caStatus, pcGetStatus())!=0) || (strcmp(caError, pcGetLastError()))!=0) {
            strcpy(caStatus, pcGetStatus());
            strcpy(caError, pcGetLastError());
            printStatusErrorText(winStatus);
            refresh();
        }
        usleep(200);
        if(bForceGuiUpdate_m)
        {
            wclear(winStatus);
            wclear(winSettings);
            wclear(winMenu);
            wclear(winHostInfo);
            bForceGuiUpdate_m=false;
            if(eMinerType_m==eCpu)
                vPrintCPUMinerInfo(winGpuInfo);
            if (eMinerType_m == eGpu)
                vPrintGPUMinerInfo(winGpuInfo);

            printSettings(winSettings);
            printStatusErrorText(winStatus);
            printHostInfo(winHostInfo);
            vCreateMenu(winMenu);
            refresh();

        }
        //scr_dump("/home/dbadmin/cg/dump.screen");
    }
    clrtoeol();
    refresh();
    endwin();
}
