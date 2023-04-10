# This file is WIP and is not the most stable. It is also very messy code
import os
import platform
import shutil
import sys
import subprocess
import time
import re
import requests # for checking internet connection


wsl = False

def runCommand(command): # Function that is used to safely run commands
        if wsl:
            command = "wsl " + command
        success = os.system(command)
        if success != 0:
            raise Exception("Failed to run command",command,"Please check if dependencies are installed")

class CBuildFiles: # A class used to represent a C project with multiple files
    def __init__(self, output, filename, ExportedFunctions = None, SourceFiles = None, Modularize = True, ExportedRuntimeMethods = None, ForceFS = False):
        self.output = output # The filepath for the output to go
        self.filename = filename # The name of the entry/root file
        self.ExportedFunctions = ExportedFunctions # External library functions to be kept alive when compiled
        self.SourceFiles = SourceFiles # All C source files
        self.Modularize = Modularize # Whether to modularize the output Wasm
        self.ExportedRuntimeMethods = ExportedRuntimeMethods # Functions to be exported by emscripten to be used in JS code
        self.ForceFS = ForceFS # Whether to include file system tools

class GoBuildFile: # A class used to represent a go project
    def __init__(self, output, filename):
        self.output = output # The filepath for the output to go
        self.filename = filename # The name of the entry/root file

def Build():
    options = {"dev": False, "go": False}
    if len(sys.argv) > 1:
        options[sys.argv[1].lower()] = True # Applies options (only one currently)

    GoBuildFiles = (
        GoBuildFile("Build/FileHandler.wasm", "src/go/FileHandler/FileHandler.go")  # Not currently being used
    )
    
    CBuildFiles = ( # The object with all of the C config files
        CBuildFiles(
        "Build/CFunctions.js", # Where to save the output file to
        "src/Main.c", # The C entry file
        ExportedFunctions=("cJSON_Delete","cJSON_IsArray","cJson_IsInvalid","cJSON_IsNumber","cJSON_IsString","cJSON_Parse",), # Keep cJSON library functions alive
        SourceFiles=("src/C/cJSON/cJSON.c", "src/DependencyGraph/DependencyGraph.c", "./src/C/StringRelatedFunctions.c",
        "./src/Regex/RegexFunctions.c", "./src/DependencyGraph/FindDependencies.c","./src/SettingsSingleton/settingsSingleton.c", 
        "./src/C/ProblemHandler.c", "./src/C/TextColors.c", "./src/C/FileHandler.c", "./src/C/IntFunctions.c", "./src/Minifiers/HTMLMinifier.c", 
        "./src/C/FileTypesHandler.c", "./src/C/Stack.c", "./src/C/BundleFiles.c", "./src/C/ProgressBar.c", "./src/C/StringShiftHandler.c", 
        "./src/Minifiers/JSMinifier.c", "./src/Transformers/Transform.c", "./src/Plugins/Plugins.c"), # All of the C files
        Modularize=True,
        ExportedRuntimeMethods=("ccall","addFunction","lengthBytesUTF8","stringToUTF8", "UTF8ToString"), # Exports the methods used by the JS code
        ForceFS=True, # Allows file system operations
        ),
    )
    

    def BuildLinux(): # Currently the main build function
        command = ""
        if options["go"] == True: # Compiles go code (not currently used)
            for key, value in GoBuildFiles:
                    print(f"Building go file: {value} with tinygo")
                    runCommand(f"tinygo build -opt=2 -o {key} -target wasm {value}")
                    time.sleep(0.1)
        
        for value in CBuildFiles:
            ExportedFunctions = "" 
                
            if value.ExportedFunctions != None: # Puts the exported functions in the format used by the compile command
                for i,v in enumerate(value.ExportedFunctions):
                    if i == 0:
                        ExportedFunctions += "-sEXPORTED_FUNCTIONS=\"_" + v+"\""
                    else:
                        ExportedFunctions += ",\"_"+v+"\""

            SourceFiles = ""
            
            if value.SourceFiles != None: # Puts the exported functions in the format used by the compile command
                for i,v in enumerate(value.SourceFiles):
                    SourceFiles += " " + v

            Modularize = ""
            if value.Modularize == True: # Adds command line arguments required for modularization
                Modularize = "-s EXPORT_ES6=0 -s MODULARIZE -s USE_ES6_IMPORT_META=0 "

            ExportedRuntimeMethods = ""
            if value.ExportedRuntimeMethods != None: #Puts the exported runtime function in the format used by the compile command
                ExportedRuntimeMethods = "-s EXPORTED_RUNTIME_METHODS=["
                for i,v in enumerate(value.ExportedRuntimeMethods):
                    if i != 0:
                        ExportedRuntimeMethods += ","
                    ExportedRuntimeMethods += "\"" + v + "\""
                ExportedRuntimeMethods += "] "

            ForceFS = ""
            if value.ForceFS == True:
                ForceFS = "-s NODERAWFS=1" # Adds the file system access functions into the command

            optimizations = "-O3 -s ELIMINATE_DUPLICATE_FUNCTIONS=1 -s MINIFY_HTML=1 " # Sets up the optimizations
            
            Dev = ""
            if options["dev"] == True:
                print("Dev mode is enabled")
                Dev = """--profiling -sRUNTIME_DEBUG=1 -fsanitize=undefined -fsanitize=address 
                -sLLD_REPORT_UNDEFINED -g3 -sSTACK_OVERFLOW_CHECK=2 -sASSERTIONS=2 """ # Sets up the debug options
                optimizations = "" # Removes optimisations when in dev mode

            command = f"""emcc {optimizations}--no-entry -sALLOW_TABLE_GROWTH -sENVIRONMENT=node 
                {Dev}{value.filename}{SourceFiles} {Modularize}{ExportedRuntimeMethods}{ForceFS}
                -sBINARYEN=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH -o {value.output}""" # Puts the compile command together

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