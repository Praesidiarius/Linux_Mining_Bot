//
// Created by JBinggi on 01.07.18.
//

#include <curl/curl.h>
#include <fstream>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <zconf.h>
#include <termios.h>
#include <termio.h>
#include <arpa/inet.h>

#include "helper.h"
#include "data.h"


using namespace std;

// for convenience
using json = nlohmann::json;


void vDownLoadUpdateFile(void) {
    char caCommand[400];
    vSetStatusText("Download Zip");
    // Run startcpu.sh

    sprintf(caCommand, "wget %s%s 2>/dev/null", UPDATE_PATH, UPDATE_FILE);
    string strResult = exec(caCommand);

    sprintf(caCommand, "unzip -o %s -d Update/", UPDATE_FILE);
    strResult = exec(caCommand);

    //sprintf(caCommand,"dpkg -i  Update/*");
    //strResult = exec (caCommand);

    sprintf(caCommand, "chmod +x Update/*.sh");
    strResult = exec(caCommand);

    sprintf(caCommand, "rm -f update.zip wget*");
    strResult = exec(caCommand);

    vSetStatusText("install updates");

    //sprintf(caCommand, "Update/update.sh");
    //strResult = exec(caCommand);
    vSetStatusText("Updates installed");

}


void vUpdateCgMiningBot(TagConfig *ptagConfig, char *pcVersion) {
    char caCommand[1000];
    char caCgBot[100];

    vSetStatusText("Update Miner");
    sprintf(caCgBot,"cgbot_%s.tar.gz",pcVersion);
    sprintf(caCommand, "wget %s%s 2>/dev/null", UPDATE_PATH, caCgBot);
    string strResult = exec(caCommand);

    if(strResult.empty())
        vSetDebugText("%/%s not found",UPDATE_PATH,caCgBot);
    else
        vSetDebugText("%/%s downloaded",UPDATE_PATH,caCgBot);

    sprintf(caCommand, "mv %s %s_old",ptagConfig->caHomePath,ptagConfig->caHomePath);
    strResult = exec(caCommand);

    sprintf(caCommand, "mkdir %s",ptagConfig->caHomePath);
    strResult = exec(caCommand);

    sprintf(caCommand, "tar -xzf %s -C %s",caCgBot,ptagConfig->caHomePath);
    strResult = exec(caCommand);

    sprintf(caCommand, "mv %s_old/miners %s/miners",ptagConfig->caHomePath,ptagConfig->caHomePath);
    strResult = exec(caCommand);

    sprintf(caCommand, "rm -rf %s_old",ptagConfig->caHomePath);
    strResult = exec(caCommand);
}


void vDownloadMiners(void) {
    char caCommand[400];
    vSetStatusText("Download miners.zip...");
    // Run startcpu.sh

    sprintf(caCommand, "wget %s%s 2>/dev/null", UPDATE_PATH, "miners.zip");
    string strResult = exec(caCommand);

    vSetStatusText("unzip and move");
    sprintf(caCommand, "mv miners miners_old");
    strResult = exec(caCommand);

    sprintf(caCommand, "unzip -o %s -d ./", "miners.zip");
    strResult = exec(caCommand);

    sprintf(caCommand, "rm miners.zip");
    strResult = exec(caCommand);
    vSetStatusText("new miners installed");
    //sprintf(caCommand,"dpkg -i  Update/*");
    //strResult = exec (caCommand);

    //sprintf(caCommand,"chmod +x Update/*.sh");
    //strResult = exec (caCommand);

    //sprintf(caCommand,"Update/update.sh");
    //strResult = exec (caCommand);

}


char *pcGetLinuxVersion(void) {
    char caCommand[200];
    char *pcVersion;
    strcpy(caCommand, "lsb_release -d");
    string strResult = exec(caCommand);

    pcVersion = strrchr((char *) strResult.c_str(), '\t');

    if (pcVersion) {
        pcVersion++; // del leading tabstop
        if (pcVersion[strlen(pcVersion) - 1] == '\n')
            pcVersion[strlen(pcVersion) - 1] = 0; // Cut newline at end
    }
    return pcVersion;
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

const char *pcGetCudaVersion(void) {
    char caCommand[200];
    strcpy(caCommand, "cat /usr/local/cuda/version.txt");
    static string strResult = exec(caCommand);

    rtrim(strResult);
    return strResult.c_str();
}

const char *pcGetSmiVersion(void) {
    char caCommand[200];
    static vector<string> strOutPut;

    strcpy(caCommand, "nvidia-smi | grep SMI");
    string strResult = exec(caCommand);
    if (strResult.empty()) {
        return " ";
    }
    const char *pcTemp;
    pcTemp = strstr(strResult.c_str(), "SMI");
    //cout << pcTemp<<endl;
    split(pcTemp, strOutPut, ' ');
    return (strOutPut.begin()[1].c_str());
}


/**
 * Check if file exists
 * @param name filename
 * @return found = true
 *         not found = false
 */
bool bFileExists(const std::string &name) {
    ifstream f(name.c_str());
    return f.good();
}

void vSetHostName(char *pcHostname)
{
    char caCommand[200];
    sprintf(caCommand, "echo \"%s\" > %s",pcHostname,HOSTNAME_FILE);
    string strResult = exec(caCommand);
}


/**
 * Get char* from Json Item
 * @param jsonItem json item
 * @param pcDefualt return this if Item is empty
 * @return char* Item
 */
char *getStringFromJson(json jsonItem, const char *pcDefualt) {
    static char caTemp[10000] = {""};

    memset(caTemp, 0, sizeof(caTemp));
    if (jsonItem.empty()) {
        strcpy(caTemp, pcDefualt);
        return caTemp;
    }
    string tmp = jsonItem;
    strcpy(caTemp, tmp.c_str());
    return caTemp;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((string *) userp)->append((char *) contents, size * nmemb);
    return size * nmemb;
}


/**
 * Execute Command in Terminal
 * @param cmd Command
 * @return screen output
 */
string exec(const char *cmd) {
    array<char, 128> buffer = {0};
    string result;
    shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

/**
 * Split String
 * @param txt Input string
 * @param strs Output sting
 * @param ch determiter
 * @return split cout
 */
size_t split(const string &txt, vector<string> &strs, char ch) {
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while (pos != string::npos) {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;

        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    strs.push_back(txt.substr(initialPos, min(pos, txt.size()) - initialPos + 1));

    return strs.size();
}

string strReadWriteCurl(string strUrl, char *pcPostString) {
    CURL *curl;
    CURLcode res;
    string strReadBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pcPostString);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strReadBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK)
            vSetError(res,"curl- %s\nUrl=%s\nPOST=%s\n", curl_easy_strerror(res),strUrl.c_str(),pcPostString);
    } else {
        vSetError(0,"strReadWriteCurl(): curl_easy_init failed\n");
    }

     //cout << strReadBuffer << endl;
    return strReadBuffer;
}

/*
 * Read Ip and MAC form Host
 */
bool bGetHostInfo(HostInfo *ptagHostInfo) {
    struct ifaddrs *ifAddrStruct = nullptr;
    struct ifaddrs *ifa = nullptr;
    void *tmpAddrPtr = nullptr;
    struct ifreq req;
    uint8_t *puiMac = nullptr;

    getifaddrs(&ifAddrStruct);

    int32_t sd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sd < 0) {
        /// free memory allocated by getifaddrs
        freeifaddrs(ifAddrStruct);
        return false;
    }

    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;

        /// print MAC address
        strcpy(req.ifr_name, ifa->ifa_name);
        if (ioctl(sd, SIOCGIFHWADDR, &req) != -1)
            puiMac = (uint8_t *) req.ifr_ifru.ifru_hwaddr.sa_data;

        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

            //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);

            //Save Ip & MAC
            if (!strstr(addressBuffer, "127.0.0")) {
                strcpy(ptagHostInfo->caIPAddr, addressBuffer);
                sprintf(ptagHostInfo->caMacAddr, "%02X:%02X:%02X:%02X:%02X:%02X", puiMac[0], puiMac[1], puiMac[2],
                        puiMac[3], puiMac[4], puiMac[5]);
            }

        }
            // check it is IP6
        else if (ifa->ifa_addr->sa_family == AF_INET6) {
            // is a valid IP6 Address
            tmpAddrPtr = &((struct sockaddr_in6 *) ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);

            //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
        }
    }
    char caCommand[200];

    //strcpy(ptagHostInfo->caCudaVersion, pcGetCudaVersion());
    //strcpy(ptagHostInfo->caNvidiaSmiVersion, pcGetSmiVersion());

    sprintf(caCommand, "echo $HOSTNAME");
    string strResult = exec(caCommand);
    gethostname(ptagHostInfo->caHostName, 99);
    if (ifAddrStruct != nullptr)
        freeifaddrs(ifAddrStruct);
    return true;
}


/**
 * Get a List of screen active screen sessions
 * @param caaSreens list of screen names
 * @return screen session count
 */
int iListAllScreenSessions(char caaSreens[10][100]) {
    char caCommand[200];
    vector<string> strOutPut;
    // Run oc_all.sh process to set over clocking
    sprintf(caCommand, "screen -r");
    std::string strResult = exec(caCommand);

    if (strResult.empty()) {
        return 0;
    }
    split(strResult, strOutPut, '\n');

    //printf("iGPUCount = %d\n",iGPUCount);
    int iCounter = 0;
    int iGpuCount = 0;
    for (vector<string>::iterator it = strOutPut.begin(); it != strOutPut.end(); ++it) {
        string screen = *it;

        if (strstr(screen.c_str(), "Detached")) {
            const char *pcPos;
            char *pcPos2;

            //cout << screen<< endl;
            pcPos = strchr(screen.c_str(), '.');
            pcPos2 = (char *) strchr(pcPos, '\t');
            *pcPos2 = 0;
            strcpy(caaSreens[iCounter], pcPos + 1);
            //printf("caaSreens[iCounter]= %s\n",caaSreens[iCounter]);
            iCounter++;
        }
    }
    return iCounter;
}

/**
 * Attach to screen session<
 * Info: blocking until detach screen session
 * @param pcScreen
 */
void vScreenAttach(char *pcScreen) {
    char caCommand[200];

    vector<string> strOutPut;
    sprintf(caCommand, "screen -r %s", pcScreen);
    std::string strResult = exec(caCommand);
}

/**
 * Reboot
 * @param iMin
 */
void vReboot(int iMin) {
    char caBuffer[100];
    sprintf(caBuffer, "/sbin/shutdown -r +%d", iMin);
    string strResult = exec(caBuffer);
    return;
}


#define DO 0xfd
#define WONT 0xfc
#define WILL 0xfb
#define DONT 0xfe
#define CMD 0xff
#define CMD_ECHO 1
#define CMD_WINDOW_SIZE 31

void negotiate(int sock, unsigned char *buf, int len) {
    int i;

    if (buf[1] == DO && buf[2] == CMD_WINDOW_SIZE) {
        unsigned char tmp1[10] = {255, 251, 31};
        if (send(sock, tmp1, 3, 0) < 0)
            exit(1);

        unsigned char tmp2[10] = {255, 250, 31, 0, 80, 0, 24, 255, 240};
        if (send(sock, tmp2, 9, 0) < 0)
            exit(1);
        return;
    }

    for (i = 0; i < len; i++) {
        if (buf[i] == DO)
            buf[i] = WONT;
        else if (buf[i] == WILL)
            buf[i] = DO;
    }

    if (send(sock, buf, len, 0) < 0)
        exit(1);
}

static struct termios tin;

static void terminal_set(void) {
    // save terminal configuration
    tcgetattr(STDIN_FILENO, &tin);

    static struct termios tlocal;
    memcpy(&tlocal, &tin, sizeof(tin));
    cfmakeraw(&tlocal);
    tcsetattr(STDIN_FILENO, TCSANOW, &tlocal);
}

static void terminal_reset(void) {
    // restore terminal upon exit
    tcsetattr(STDIN_FILENO, TCSANOW, &tin);
}

#define BUFLEN 20

int iSendTelnet(const char *pcHost, int iPort, const char *pcMessage, unsigned char *pcResponse, int iSize) {
    int sock;
    struct sockaddr_in server;
    unsigned char buf[BUFLEN + 1];
    int len;
    int iError = 0;

    memset(pcResponse, 0, sizeof(pcResponse));
    //Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        vSetError(1, "Could not create socket. Error");
        return -1;
    }

    server.sin_addr.s_addr = inet_addr(pcHost);
    server.sin_family = AF_INET;
    server.sin_port = htons(iPort);

    //Connect to remote server
    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        vSetError(1, "connect failed. Error");
        return -2;
    }
    // set terminal
    terminal_set();
    atexit(terminal_reset);

    struct timeval ts;
    ts.tv_sec = 1; // 1 second
    ts.tv_usec = 0;

    // select setup
    fd_set fds;
    FD_ZERO(&fds);
    if (sock != 0)
        FD_SET(sock, &fds);
    FD_SET(0, &fds);

    if (send(sock, pcMessage, strlen(pcMessage), 0) < 0)
        return -3;
    while (1) {
        // wait for data
        int nready = select(sock + 1, &fds, (fd_set *) 0, (fd_set *) 0, &ts);
        if (nready < 0) {
            return -6;
        } else if (nready == 0) {
            ts.tv_sec = 1; // 1 second
            ts.tv_usec = 0;
        } else if (sock != 0 && FD_ISSET(sock, &fds)) {
            // start by reading byteS
            len = recv(sock, pcResponse, iSize, 0);
            if (len < 0) {
                iError = -5;
                break;
            }
                // wait for response
            else if (len == 0) {
                break;
            }
            negotiate(sock, pcResponse, iSize);
        }
    }
    terminal_reset();
    close(sock);
    return 0;
}




