# SPIFFS_Shell Command Interface Documentation

The SPIFFSShell command interface provides a set of commands for interacting with the SPIFFS (SPI Flash File System) on an ESP8266 or ESP32 microcontroller. The SPIFFSShell class encapsulates the functionality and provides a command-line interface over the Serial port.

## Getting Started

To use the SPIFFSShell command interface, just include the header file <SPIFFS_Shell.h>. The Library automatically includes the necessary files, initializes the SPIFFS and starts the command interface.

```cpp
#include <SPIFFS_Shell.h>

void setup() {
  // Main program setup
}

void loop() {
  // Main program loop
}
```

## Available Commands

The SPIFFSShell command interface supports the following commands:

### ls \<directory\>

List the contents of a directory. The `directory` parameter is optional. If no directory is specified, the root directory will be listed.

Example usage:

```
spiffs# ls
Name                                     Type       Size
/dir1                                    <DIR>        
file1.txt                                FILE       1234
```

### cat \<file\>

Read the contents of a file and print them to the Serial port.

Example usage:

```
spiffs# cat file1.txt
This is the content of file1.txt.
```

### echo \<message\> \<file\>

Write a message to a file. If the file does not exist, it will be created. If the file already exists, the message will overwrite the existing content.

Example usage:

```
spiffs# echo "Hello, World!" file2.txt
```

### append \<message\> \<file\>

Append a message to the end of a file. If the file does not exist, it will be created.

Example usage:

```
spiffs# append "Additional content" file2.txt
```

### mv \<old\> \<new\>

Rename a file. The `old` parameter specifies the current name of the file, and the `new` parameter specifies the new name.

Example usage:

```
spiffs# mv file1.txt newname.txt
```

### rm \<file\>

Delete a file.

Example usage:

```
spiffs# rm file2.txt
```

### du

Print the total size of the SPIFFS flash memory in kilobytes.

Example usage:

```
spiffs# du
Flash memory size: 5120 KB
```

### df

Print the amount of used flash memory in kilobytes.

Example usage:

```
spiffs# df
Flash memory used: 1024 KB
```

### editor \<file\>

Enter a text editor mode for appending text to a file. The `file` parameter specifies the name of the file to edit. In the text editor mode, you can enter text that will be appended to the file. Type `%exit` to exit the text editor.

Example usage:

```
spiffs# editor file2.txt
- Text editor started. Enter text (or type '%exit' to quit, '%help' for commands):
This is some additional text.
%exit
- Text editor exited
```

### help

Show a list of available commands.

Example usage:

```
spiffs# help
Available commands:
ls <directory> - List the contents of a directory
cat <file> - Read the contents of a file
echo <message> <file> - Write a message to a file
append <message> <file> -

 Append a message to a file
mv <old> <new> - Rename a file
rm <file> - Delete a file
du - Print total flash memory size
df - Print used flash memory
editor <file> - Enter a text editor for appending text to a file (type '%exit' to quit, '%help' for commands)
help - Show available commands
```

## Notes

- The SPIFFS file system must be successfully mounted for the SPIFFSShell class to function properly. If mounting fails, an error message will be printed, and the command interface will not start.
- The SPIFFS_Shell.h library is still in the works, and so far, I've only had the chance to test it on the Heltec ESP32 board. If you happen to come across any problems, please don't hesitate to submit an issue report (or even better, a pull request if you're up for it!). Your feedback would be greatly appreciated!

## Limitations

- The SPIFFS_Shell command interface does not support files with names containing spaces.
- SPIFFS does not support folders or directory structures. All files are stored in the root directory.
- The maximum length of a SPIFFS file name, including the file extension, is 30 characters. This limitation is due to the underlying file system implementation.
- The total number of files that can be stored in SPIFFS is limited by the available space in the flash memory. The exact number depends on the size of the files and the available free space.
- The maximum file size that can be stored in SPIFFS depends on the configuration and the SPI flash size of your microcontroller. For example, on the ESP8266, the default configuration allows a maximum file size of 64KB.