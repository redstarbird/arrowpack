# This file is WIP and either does not work or is not stable currently
import os
import platform
import shutil
import sys
import requests # for testing internet connection
import subprocess
import re
import time

wsl = False


class CBuildFile:
    def __init__(self, output, filename, ExportedFunctions = None, SourceFiles = None, Modularize = True, ExportedRuntimeMethods = None):
        self.output = output
        self.filename = filename
        self.ExportedFunctions = ExportedFunctions
        self.SourceFiles = SourceFiles
        self.Modularize = Modularize
        self.ExportedRuntimeMethods = ExportedRuntimeMethods

class GoBuildFile:
    def __init__(self, output, filename):
        self.output = output
        self.filename = filename


def Build():
    options = {"go": False, "c": False}
    if len(sys.argv) >= 2:
        options[sys.argv[1].lower()] = True

    GoBuildFiles = (
        GoBuildFile("Build/FileHandler.wasm", "src/go/FileHandler/FileHandler.go")
    )
    
    CBuildFiles = (
        CBuildFile(
        "Build/CFunctions.js",
        "src/Main.c",
        ExportedFunctions=("cJSON_Delete","cJSON_IsArray","cJson_IsInvalid","cJSON_IsNumber","cJSON_IsString","cJSON_Parse"),
        SourceFiles=("./src/C/ReadFile.c", "src/C/cJSON/cJSON.c", "src/C/DependencyTree.c"),
        Modularize=True,
        ExportedRuntimeMethods=("malloc","ccall")
        ),
    )
    

    def BuildLinux():
        command = ""
        if options["c"] == False:
            for key, value in GoBuildFiles:
                    print(f"Building go file: {value} with tinygo")
                    runCommand(f"tinygo build -opt=2 -o {key} -target wasm {value}")
                    time.sleep(0.1)
        if options["go"] == False:
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
    
                #command = f"emcc -O3 --no-entry {ExportedFunctions} {value['entry']} -o {key} -s WASM=1"
                command = f"emcc -O3 --no-entry {value.filename}{SourceFiles}{Modularize}{ExportedRuntimeMethods} -o {value.output}"

                print("\n\n\n" + command + "\n\n\n")
                

                runCommand(command)

    def runCommand(command):
        if wsl:
            command = "wsl " + command
        success = os.system(command)
        if success != 0:
            raise Exception("Failed to run command",command,"Please check if dependencies are installed")

    def checkInstalled(program, autoInstall=False, required=True):
        if required and not autoInstall:
            assert(shutil.which(program)) != None
        return shutil.which(program)

    def checkInternetConnection(): # Checks if device has internet connection by sending a https request (needed when autoInstall is enabled)
            try:
                requests.head("https://www.google.com",timeout=3) # Send request to check Internet connection
            except requests.ConnectionError:
                raise Exception("Error: An internet connection is required")

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