#include <iostream>
#include <fstream>
#include <filesystem>

#include "unzipper.h"
#include "extract.hpp"
#include "download.hpp"

#include <switch.h>

#define VERSION "1.1.6"
#define RELEASE_URL "https://github.com/HamletDuFromage/switch-cheats-db/releases/tag/v1.0"
#define ARCHIVE_URL "https://github.com/HamletDuFromage/switch-cheats-db/releases/download/v1.0/"



void initServices(){
    consoleInit(NULL);
    //ncmInitialize();
    nsInitialize();
    socketInitializeDefault();
}

void exitServices(){
    socketExit();
    nsExit();
    //ncmExit();
    consoleExit(NULL);
}

std::string readVersion(std::string path){
    std::fstream versionFile;
    std::string version = "0";
    if(std::filesystem::exists("/config/cheats-updater/" + path)){
        versionFile.open("/config/cheats-updater/" + path, std::fstream::in);
        versionFile >> version;
        versionFile.close();
    }
    return version;
}

void saveVersion(std::string path, std::string version){
    std::fstream newVersion;
    newVersion.open("/config/cheats-updater/" + path, std::fstream::out | std::fstream::trunc);
    newVersion << version << std::endl;
    newVersion.close();
}

bool isServiceRunning(const char *serviceName) {
  Handle handle;
  SmServiceName service_name = smEncodeName(serviceName);
  bool running = R_FAILED(smRegisterService(&handle, service_name, false, 1));

  svcCloseHandle(handle);

  if (!running)
    smUnregisterService(service_name);

  return running;
}

void run(){    

    bool sxos = !(isServiceRunning("dmnt:cht") && !(isServiceRunning("tx") && !isServiceRunning("rnx")));
    u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
    bool credits = kHeld & KEY_L;
    std::string filename;
    if(sxos){
        filename = "titles.zip";
        std::filesystem::create_directory("/sxos/titles");
    }
    else{
        filename = "contents.zip";
        std::filesystem::create_directory("/atmosphere/contents");
    }
    std::filesystem::create_directory("/config");
    std::filesystem::create_directory("/config/cheats-updater");

    std::vector<std::string> titles;
    //titles = getInstalledTitles({NcmStorageId_SdCard, NcmStorageId_BuiltInUser, NcmStorageId_GameCard});
    titles = getInstalledTitlesNs();

        int total = titles.size();
    std::cout << "Gefundene installierte Titel" << total << "" << std::endl;

    titles = excludeTitles("/config/cheats-updater/exclude.txt", titles);
    if((int) titles.size() != total)
        std::cout << "Gefundene ausgeschlossende Titel " << total - titles.size() << "" << std::endl;

    std::cout << std::endl;
	
	consoleUpdate(NULL);

    std::string ver = fetchVersion(RELEASE_URL, "1100-1110");
    std::string oldVersion = readVersion("version.dat");
    if(sxos) std::cout << "Aktuelle Cheats Revision: v" << oldVersion << ", downloade v" << ver << " fuer SXOS" <<std::endl;
    else std::cout << "Aktuelle Cheats Revision: v" << oldVersion << ", downloade v" << ver << " fuer AMS" <<std::endl;
    std::cout << std::endl;
    if(ver == oldVersion){
        std::cout << "Cheats sind Up-To-Date" << std::endl;
    }
    else if(ver == "-1"){
        std::cout << "Bitte pruefe die Internetverbindung" << std::endl;
    }
    else{
        std::string url = std::string(ARCHIVE_URL) + filename;
        if(downloadFile(url.c_str(), filename.c_str(), OFF)){
           //if(false){ 
			int upd = extractCheats(filename.c_str(), titles, sxos, credits);
            std::cout << "Erfolgreich entpackt " << upd << " Cheats-Dateien" << std::endl;
            saveVersion("version.dat", ver);
        }
        else{
            std::cout << "Konnte Cheat-Archiv nicht downloaden" << std::endl;
        }
    }
    consoleUpdate(NULL);
}

void cleanUp(){
    bool sxos = !(isServiceRunning("dmnt:cht") && !(isServiceRunning("tx") && !isServiceRunning("rnx")));
    int c = removeCheats(sxos);
    saveVersion("version.dat", "0");
    std::cout << "Geloecht " << c << " Cheat-Dateien" << std::endl;
}

int main(int argc, char* argv[])
{
    initServices();

	std::cout << "\033[31m" << "================================================================================" << "\033[0m" << std::endl;
    std::cout << "\033[1;31m" <<"Cheats Updater v" << VERSION << " by HamletDuFromage / German Mod by PhyniX fuer PSX-Tools\n" << "\033[0m" <<std::endl;
    std::cout << "\033[31m" << "================================================================================" << "\033[0m" << std::endl;
	std::cout << std::endl;
	
	std::cout << "\033[36m" << "Titel IDs in dem Verzeichnis \"/config/cheats-updater/exclude.txt\" werden ignoriert" << "\033[0m" << std::endl;
	
	std::cout << "Druecke [A] zum Downloaden/Updaten der Cheats" << std::endl;
    std::cout << "Halte   [L] und Druecke [A] um auch Cheat-Anweisungen herunterzuladen" << std::endl;
    std::cout << "Druecke [X] um alle vorhandenen Cheat-Dateien zu loeschen\n" << std::endl;
	
	
	
	std::cout << "Druecke [+] zum Beenden" << std::endl << std::endl;

    consoleUpdate(NULL);

    bool done = false;

    while (appletMainLoop())
    {
        hidScanInput();

        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_A){
            if(!done){
                run();
                done = true;
                std::cout << "\033[7;37m"<< "\nDruecke [+] zum Beenden" << "\033[0m" <<std::endl;
                consoleUpdate(NULL);

            }
        } 

        if (kDown & KEY_X){
            if(!done){
                cleanUp();
                done = true;
                std::cout << "\033[7;37m"<< "\nDruecke [+] zum Beenden" << "\033[0m" <<std::endl;
                consoleUpdate(NULL);

            }
        }

        if (kDown & KEY_PLUS)
            break; 
    }

    exitServices();
    return 0;
}
