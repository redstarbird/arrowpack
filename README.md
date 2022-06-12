# Arrowpack
A web bundling application that makes your website faster and easier to maintain
## Build instruction (WIP)
### Windows 10
Compile go to shared library:
```sh
go build -buildmode=c-shared -o FileHandler.dll
```
Compile C to shared library:
1. ``` gcc -c DependencyTree.c ```
2. ``` gcc -shared -o DependencyTree.dll DependencyTree.o ```
