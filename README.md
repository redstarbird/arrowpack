# Arrowpack
A web bundling application that makes your website faster and easier to maintain
## Build instruction (WIP)
### Windows 10/11
#### WSL (Only windows compile method currently because of regex.h)

Dependencies (in WSL):
- Go >= 1.11
- gcc

**_Run all below commands in WSL_**

Compile go to shared library:
```sh
go build -buildmode=c-shared -o FileHandler.dll
```
Compile C to shared library:
1. ``` gcc -c DependencyTree.c ```
2. ``` gcc -shared -o DependencyTree.dll DependencyTree.o ```
