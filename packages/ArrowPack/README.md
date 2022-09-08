# Arrowstack

A web bundling application that makes your website faster and easier to maintain

## Build instruction (WIP)

### Windows 10/11

#### WSL (Only windows compile method currently because of regex.h)

Dependencies (in WSL):

-   Go >= 1.11
-   gcc
-   Tinygo
-   Emscripten

**_Run all below commands in WSL_**

Compile go to shared library:

```sh
python3 Build.py
```

