//
// Created by jbinggeli on 01.07.18.
//

#ifndef CG_MINING_BOT_DEFINES_H
#define CG_MINING_BOT_DEFINES_H

//#define TEST
#ifdef TEST
#define NO_NCURSER
#endif

#define SOFTWARE_VERSION "0.2.0"
#define DISABLE_DASHBOARD_JSON_API

#define MAX_GPUS 12
#define MAX_CPUS 16

#define FILE_CONFIG                 "config.json"

#define START_GPU_SCRIPT            "start.sh"
#define START_GPU_SCRIPT_NS         "start_nh.sh"

#define START_CPU_SCRIPT            "startcpu.sh"

#define FILE_SETTINGS               "settings.active"
#define FILE_SETTINGS_ERROR         "settings.error"
#define FILE_XORG_TEMP              "xorg.new.temp"

#define FILE_DEBUG_LOG              "debug.log"

#define HOSTNAME_FILE               "/etc/hostname"
#define UPDATE_FILE                 "update.zip"
#define UPDATE_PATH                 "https://dashboard.coin-garden.io/"
#define DEFAULT_CPU_SCRIPT          "/home/sdgroup/sdg/miners/cpuminer-opt/cpuminer -a cryptonightv7 -o stratum+tcp://xmr-eu1.nanopool.org:14444 -u 48m2m2kwif45RmfJntQiSgj6ESbuPdmJuTWVAMEDdxtCGbMRqRmWEoUKduZ47C559AAQNqfV3bWWNV9LWwMQ8CypPBegfTp -p x"
//#define DEFAULT_CPU_SCRIPT          "/home/dbadmin/cg/miners/cpuminer-opt/cpuminer -a lyra2zoin -o stratum+tcp://zoin.netabuse.net:3000 -u freeminer999.${RIGNAME} -p x"
#define DEFAULT_GPU_SCRIPT          "/home/sdgroup/sdg/miners/dualminereth/ethdcrminer64 -epool ella-eu1.cgpools.io:7007 -ewal 0xc6ab65E731C3E5a47A17B40F2f508AAe2FcF7D7b -eworker ${RIGNAME} -epsw x -tt -100 -gser 1 -mode 1 -allpools 1 -allcoins 1"

#define FILE_CORN_ORIG              "origcron.settings"
#define FILE_CORN_TEMP              "tempcron.settings"

#define NVIDIA_SMI                  "/usr/bin/nvidia-smi"
#define NVIDIA_SET                  "/usr/bin/nvidia-settings"

#define EXCAVATOR_CMD_FILE_TEMPLATE "/opt/excavator/etc/template_command_file.json"
#define EXCAVATOR_CMD_FILE          "/opt/excavator/etc/default_command_file.json"

#define GRIN_MINER_CONFIG           "grin-miner.toml"
#define GRIN_MINER_LOG              "grin-miner.log"


#endif //CG_MINING_BOT_DEFINES_H
