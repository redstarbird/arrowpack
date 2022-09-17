package main

// #include <stdio.h>
// #include "../../DependencyTree.h"
import "C"
import (
	"bufio"
	"fmt"
	"io"
	"os"
	"regexp"
	"strings"
	"unsafe"
)

func saveFile(filePath string, data string) {
	file, err := os.Create(filePath)

	if err != nil {panic(err)}

	defer file.Close() // Close the file at end of this function

	w := bufio.NewWriter(file)

	w.WriteString(data)
	w.Flush()

}

func getFileContents(path string, errorIfMissing bool) (string) {

	// find if file exists
	var _, missingERR = os.Stat(path)

	if os.IsNotExist(missingERR) {
		if errorIfMissing == true {
			panic("file does not exist!")
		} else {
			return ""
		}
	}

	var file, err = os.OpenFile(path, os.O_RDONLY,0644);
	
	if err != nil {panic(err)} // print error

	defer file.Close()

	var text = make([]byte,1024)

	for {
		n, err := file.Read(text)
		if err != io.EOF {
			if err != nil {panic(err)} // print error
		}
		if n == 0 {
            break
        }
		text = text[:n]
	}

	if err != nil {
        panic(err) // print error
    }
	return string(text)
}

func toGoStrings(Cstr **C.char) []string { // https://github.com/fluhus/snopher/blob/master/src/join/join.py
	var result []string
	slice := unsafe.Slice(Cstr,1<<30)

	for i := 0; slice[i] != nil; i++ {
		result = append(result, C.GoString(slice[i]))
	}

	return result
}

//export HTMLHandler
func HandleFiles(Files **C.Node, entry string) { // first few lines of this function are probably not memory safe or something i dunno
	fmt.Println("Hello World!")
	srcFiles := []string{"test","GoTest"}
	/*srcFiles := toGoStrings(strs)
	fmt.Println(srcFiles)
	*/
	entryPath := "test"
	exitPath := "test"
	/*
	entryPath := C.GoString(entryPathC ) // change config variables from JS variables to go variables
	exitPath := C.GoString(exitPathC)
	//C.free(unsafe.Pointer(entryPathC)) // free no longer needed C variables to avoid memory leak
	//C.free(unsafe.Pointer(exitPathC))*/

	return
	for _, file := range srcFiles {
		var text string = getFileContents(file, true) // gets file contents

		re := regexp.MustCompile("<include src\".*\">") // regex pattern
		results := re.FindAllStringSubmatch(text,-1) // searchs for regex pattern in file contents

		for _, resultArray := range results { // loops through all <include> tags found

			for _, result := range resultArray { // regexp gives nested arrays of results for some reason so nested for loops need to be used
				
				var FilePath string = result[9:len(result)-10]
				
				if string(FilePath[0]) == "\\" {FilePath = strings.Replace(FilePath, "\\", entryPath + string(os.PathSeparator), 1)} else if string(FilePath[0]) == "/" {FilePath = strings.Replace(FilePath, "/", entryPath + string(os.PathSeparator), 1)} // changes html root path to compatible path
				
				text = strings.Replace(text, result,getFileContents(FilePath, false),-1) // gets file contents data from files found previously by regexp and puts into text variable
			}
		}
		savePath := strings.Replace(file,entryPath,exitPath,1) // replaces entry path with exit path in file path
		fmt.Println(savePath)
		// saveFile(savePath,text)
	
	}
}
/*
func main() {
	fmt.Println("Test")
	js.Global().Set("HandleFiles", js.FuncOf(HandleFiles))
}*/