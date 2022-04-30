from array import ArrayType
from ast import Str
import os
import json
import re # regex
import ctypes
from typing import List

config = {"entry": "src", "exit": "dist", "autoClear":False} # stores config (includes defaults)
pathsep = "/" # cross platform support for file paths

if os.path.exists("ArrowPack-Config.json"): # config file
    with open("ArrowPack-Config.json", "rt") as file:
        contents = json.loads(file.read())
        for key,value in contents.items():
            if config[key]:
                config[key] = value

srcFiles = []

if not os.path.exists(config["entry"]):
    raise Exception("Could not find directory:"+config["entry"])


for root, dirs, files in os.walk(config["entry"]):
    for dir in dirs:
        try:
            os.mkdir(config["exit"]+"/"+dir)
        except OSError:
            pass
    for file in files:
        #print(f"Root: {root} File: {file}")
        srcFiles.append(root.replace("\\","/")+"/"+file) # using os.path makes sure that code is cross platform
        

"""
def findString(regex,slice,fileExtension=".js"):
    files = [x for x in srcFiles if x.endswith(fileExtension)]
    for file in files:
        with open(file, "rt") as f:
            contents = f.read()
            result = re.finditer(regex, contents) # use regex to search through file

        for match in result:
            
            Filepath = match.group()[slice[0]:slice[1]]
            if Filepath[0] == "\\":
                Filepath = Filepath.replace("\\", config["entry"]+pathsep, 1) # changes html root path to compatible path
            elif Filepath[0] == "/":
                Filepath = Filepath.replace("/", config["entry"]+pathsep, 1) 
            
            if not os.path.exists(Filepath):
                raise Exception(f"Could not find path: {Filepath}") # check that file exits

            with open(Filepath) as include:
                contents = contents.replace(match.group(), include.read())
        
        with open(config["exit"]+file[len(config["entry"]):],"w") as f: # complicated string stuff to get correct file path to save to
            f.write(contents)
findString("<include>.*</include>", [9,-10],".html")"""

GoWrapper = ctypes.cdll.LoadLibrary("./FileHandler.dll") # loads go file
HTMLHandler = GoWrapper.HTMLHandler # gets html handler function from Go file
HTMLHandler.argtypes = [ctypes.POINTER(ctypes.c_char_p),ctypes.c_char_p,ctypes.c_char_p] 
#HTMLHandler.restype = ctypes.c_char_p

#free = GoWrapper.free # used to free C variables when no longer needed to avoid memory leak
#free.argtypes = [ctypes.c_char_p]

def toCStringArray(strs: List[str]): # https://github.com/fluhus/snopher/blob/master/src/join/join.py
    ptr = (ctypes.c_char_p * (len(strs) + 1))()
    ptr[:-1] = [s.encode() for s in strs]
    ptr[-1] = None
    return ptr

def ConvertToCStringList(StringList):
    arrayLength = len(StringList)
    ArrayType = ctypes.c_char_p * arrayLength
    StringArray = ArrayType()
    for i,val in enumerate(StringList):
        print(val)
        StringArray[i] = ctypes.c_char_p(val.encode("utf-8"))
    return StringArray


HTMLHandler(toCStringArray([x for x in srcFiles if x.endswith(".html")]), config["entry"].encode("utf-8"), config["exit"].encode("utf-8"))

#free(success)