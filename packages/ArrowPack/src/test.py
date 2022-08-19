import ctypes

def ConvertToCStringList(StringList):
    arrayLength = len(StringList)
    ArrayType = ctypes.c_char_p * arrayLength
    StringArray = ArrayType()
    for i,val in enumerate(StringList):
        StringArray[i] = ctypes.c_char_p(val.encode("utf-8"))
    return StringArray

mod = ctypes.cdll.LoadLibrary("./src/DependencyTree.dll")

buildTree = mod.InitTree
buildTree.argtypes = [ctypes.POINTER(ctypes.c_char_p), ctypes.c_uint8]
list = ["test1","amongusbutsuss","LikeAndSubscribe","ok","live","src\\test12345.html"]
buildTree(ConvertToCStringList(list),ctypes.c_uint8(len(list)))
