# This file is WIP and either does not work or is not stable currently
import os
import platform
import shutil
import sys
import requests # test
import subprocess
import re
import time

wsl = False

def main():
    def runCommand(command):
        if wsl:
            command = "wsl " + command

    def checkInstalled(program, autoInstall=False, required=True):
        if required and not autoInstall:
            assert(shutil.which(program)) != None

        return shutil.which(program)

    def checkInternetConnection(): # Checks if device has internet connection by sending a https request
            try:
                requests.head("https://www.google.com",timeout=3) # Send request to check Internet connection
            except requests.ConnectionError:
                raise Exception("Error: An internet connection is required")

    checkInternetConnection()

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
            


    elif platform.system() == "Darwin":
        print("Building on mac is not supported currently")
    elif platform.system() == "Linux":
        pass
    else:
        print("Your operating system is not supported currently")


if __name__ == "__main__": # Driver Code
    main()