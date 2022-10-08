# This file is WIP and is not the most stable. It is also very messy code
import os
import platform
import shutil
import sys
import requests # for testing internet connection
import subprocess
import re
import time

wsl = False

def runCommand(command):
        if wsl:
            command = "wsl " + command
        success = os.system(command)
        if success != 0:
            raise Exception("Failed to run command",command,"Please check if dependencies are installed")

class CBuildFile:
    def __init__(self, output, filename, ExportedFunctions = None, SourceFiles = None, Modularize = True, ExportedRuntimeMethods = None, ForceFS = False):
        self.output = output
        self.filename = filename
        self.ExportedFunctions = ExportedFunctions
        self.SourceFiles = SourceFiles
        self.Modularize = Modularize
        self.ExportedRuntimeMethods = ExportedRuntimeMethods
        self.ForceFS = ForceFS

class GoBuildFile:
    def __init__(self, output, filename):
        self.output = output
        self.filename = filename

def BuildPCRE2(version="10.40"):
    print(f"Building PCRE2 version: {version}")
    runCommand("mkdir -p pcre2tempbuild") # Makes temp install directory

    runCommand(f"curl -L -o pcre2tempbuild/pcre2-{version}.tar.bz2 https://github.com/PCRE2Project/pcre2/releases/download/pcre2-{version}/pcre2-{version}.tar.bz2") # Downloads pcre2 tar archive from github

    runCommand(f"tar -xvjf pcre2tempbuild/pcre2-{version}.tar.bz2 -C pcre2tempbuild") # Unpacks pcre2 tar archive
    os.chdir(f"pcre2tempbuild/pcre2-{version}") # Moves to newly extracted pcre2 directory so that no temp files are created in the arrowpack base directory

    runCommand(f"emconfigure ./configure --prefix=/src/local --disable-pcre2-8 --enable-pcre2-16 --disable-jit --with-heap-limit=2000000") # compiles the library
    runCommand(f"emmake make") # also compiles the library
    runCommand(f"emmake make install") # this also compiles the library

    os.chdir("../../") # moves back to arrowpack workind dir
    runCommand(f"rm -rf pcre2tempbuild") # Deletes directory


def Build():
    options = {"go": False, "c": False, "dev": False, "pcre2":False}
    if len(sys.argv) >= 2:
        options[sys.argv[1].lower()] = True

    GoBuildFiles = (
        GoBuildFile("Build/FileHandler.wasm", "src/go/FileHandler/FileHandler.go")
    )
    
    CBuildFiles = (
        CBuildFile(
        "Build/CFunctions.js",
        "src/Main.c",
        ExportedFunctions=("cJSON_Delete","cJSON_IsArray","cJson_IsInvalid","cJSON_IsNumber","cJSON_IsString","cJSON_Parse","pcre2_compile_16","pcre2_get_error_message_16"),
        SourceFiles=("./src/C/ReadFile.c", "src/C/cJSON/cJSON.c", "src/DependencyTree/DependencyTree.c", "./src/C/StringRelatedFunctions.c",
        "./src/Regex/RegexFunctions.c", "./src/DependencyTree/FindDependencies.c",),
        Modularize=True,
        ExportedRuntimeMethods=("ccall",),
        ForceFS=True,
        ),
    )
    

    def BuildLinux():
        command = ""
        if options["go"] == True:
            for key, value in GoBuildFiles:
                    print(f"Building go file: {value} with tinygo")
                    runCommand(f"tinygo build -opt=2 -o {key} -target wasm {value}")
                    time.sleep(0.1)
        
        for value in CBuildFiles:
            ExportedFunctions = ""
                
            if value.ExportedFunctions != None:
                for i,v in enumerate(value.ExportedFunctions):
                    if i == 0:
                        ExportedFunctions += "-sEXPORTED_FUNCTIONS=\"_" + v+"\""
                    else:
                        ExportedFunctions += ",\"_"+v+"\""

            SourceFiles = ""
            
            if value.SourceFiles != None:
                for i,v in enumerate(value.SourceFiles):
                    SourceFiles += " " + v

            Modularize = ""
            if value.Modularize == True:
                Modularize = " -s EXPORT_ES6=0 -s MODULARIZE -s USE_ES6_IMPORT_META=0 "

            ExportedRuntimeMethods = ""
            if value.ExportedRuntimeMethods != None:
                ExportedRuntimeMethods = "-s EXPORTED_RUNTIME_METHODS=["
                for i,v in enumerate(value.ExportedRuntimeMethods):
                    if i != 0:
                        ExportedRuntimeMethods += ","
                    ExportedRuntimeMethods += "\"" + v + "\""
                ExportedRuntimeMethods += "] "
            ForceFS = ""
            if value.ForceFS == True:
                ForceFS = "-s NODERAWFS=1"

            Dev = ""
            if options["dev"] == True:
                Dev = "--profiling -fsanitize=undefined -Werror -sLLD_REPORT_UNDEFINED "


            # Command to compile pcre2 library: emconfigure ./configure --disable-pcre2-8 --enable-pcre2-16 --disable-jit --with-heap-limit=2000000 && emmake make && emmake install

            #command = f"emcc -O3 --no-entry {ExportedFunctions} {value['entry']} -o {key} -s WASM=1"
            command = f"emcc -O3 -g2 --no-entry {Dev}{value.filename}{SourceFiles} {Modularize}{ExportedRuntimeMethods}{ForceFS} -sASSERTIONS=2 -sBINARYEN=1 -sALLOW_MEMORY_GROWTH -I/usr/local/lib -L/usr/local/lib -lpcre2-16 -o {value.output}"

            print("\n\n\n" + command + "\n\n\n")
            
            

            runCommand(command)

    def checkInstalled(program, autoInstall=False, required=True):
        if required and not autoInstall:
            assert(shutil.which(program)) != None
        return shutil.which(program)

    def checkInternetConnection(): # Checks if device has internet connection by sending a https request (needed when autoInstall is enabled)
            try:
                requests.head("https://www.google.com",timeout=3) # Send request to check Internet connection
            except requests.ConnectionError:
                raise Exception("Error: An internet connection is required")

    pcre2 = False
    if len(sys.argv) > 1:
        pcre2 = sys.argv[1].lower() == "pcre2"

    if pcre2:
        BuildPCRE2()
    else:
        if platform.system() == "Windows":
            if checkInstalled("wsl"): # checks if WSL is installed
                WSLDefaultDistro = subprocess.check_output(["wsl", "--list", "--verbose"]).decode("utf-8")

                time.sleep(1)
                WSLDefaultDistro = re.sub(" +", " ",WSLDefaultDistro) # removes multiple consecutive strings
                print(WSLDefaultDistro)
                WSLDefaultDistro = WSLDefaultDistro.split("\n")



                for index, line in enumerate(WSLDefaultDistro):
                    if len(line) > 3:
                        if line[1] == "*":
                            WSLDefaultDistro = line[4:-27].split(" ")[0]
                print(WSLDefaultDistro)
                WSLDefaultDistro = WSLDefaultDistro.replace("\x00", "") # removes null characters from command output string

                if not "Ubuntu" in WSLDefaultDistro or "Debian" in WSLDefaultDistro:
                    print("Your WSL distro may not be supported errors may occur when installing, continuing with installation")
                global wsl
                wsl = True
                BuildLinux()

            elif platform.system() == "Darwin":
                print("Building on mac is not supported currently")
            elif platform.system() == "Linux":
                BuildLinux()
            else:
                print("Your operating system is not supported currently")


if __name__ == "__main__": # Driver Code
    Build()