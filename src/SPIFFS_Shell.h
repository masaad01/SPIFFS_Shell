
#ifndef FS
#include <FS.h>
#endif
#ifndef SPIFFS
#include <SPIFFS.h>
#endif

#define SPIFFSShell_FORMAT_SPIFFS_IF_FAILED true
#define SPIFFSShell_MAX_LINE_WIDTH 200

class SPIFFSShell {
public:
  SPIFFSShell() {
    // Constructor
    begin();
  }

private:
  void static begin() {
    if(!Serial)
      Serial.begin(115200);
    
    if (!SPIFFS.begin(SPIFFSShell_FORMAT_SPIFFS_IF_FAILED)) {
      Serial.println("SPIFFS Mount Failed");
      return;
    }

    Serial.println("Welcome to SPIFFS Shell!\nType \"help\" to see available  commands");

    xTaskCreate(
        &(SPIFFSShell::taskFunction),    // Task function
        "spiffsTask",                  // Task name
        10000,                         // Stack size (bytes)
        NULL,                          // Task parameter
        1,                             // Task priority
        NULL                   // Task handle
    );

    printPrompt();
  }

  String static getAbsolutePath(String path){
    if(path.startsWith("/"))
      return path;
    if(path.startsWith("./"))
      return String("/") + path.substring(2);
    return String("/") + path;
  }

  void static taskFunction(void * arr) {
    while (1) {
      if (Serial.available()) {
        String command = Serial.readStringUntil('\n'); // Read a command from the serial port

        String cmd, arg1, arg2;
        parseCommand(command, cmd, arg1, arg2);

        executeCommand(cmd, arg1, arg2);
      }
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  }
  
  void static printPrompt(){
    Serial.print("\nspiffs# ");
  }

  void static parseCommand(const String& command, String& cmd, String& arg1, String& arg2) {
    Serial.println(command);
    int spaceIndex1 = command.indexOf(' '); // Find the index of the first space character
    int spaceIndex2 = command.indexOf(' ', spaceIndex1 + 1); // Find the index of the second space character

    if (spaceIndex1 == -1) {
      cmd = command;
    } else {
      cmd = command.substring(0, spaceIndex1); // Extract the command from the command string
      if (spaceIndex2 == -1) {
        arg1 = command.substring(spaceIndex1 + 1); // Extract the first argument from the command string
      } else {
        arg1 = command.substring(spaceIndex1 + 1, spaceIndex2); // Extract the first argument from the command string
        arg2 = command.substring(spaceIndex2 + 1); // Extract the second argument from the command string
      }
    }
    // Check for CR character
    if (command.indexOf('\r') != -1) {
      Serial.println("Warning: Carriage return (CR) character detected in input");
    }
  }

  void static executeCommand(const String& cmd, const String& arg1, const String& arg2) {
    if (cmd.equals("ls")) {
      lsCommand(arg1);
    } else if (cmd.equals("cat")) {
      catCommand(arg1);
    } else if (cmd.equals("echo")) {
      echoCommand(arg1, arg2);
    } else if (cmd.equals("append")) {
      appendCommand(arg1, arg2);
    } else if (cmd.equals("mv")) {
      mvCommand(arg1, arg2);
    } else if (cmd.equals("rm")) {
      rmCommand(arg1);
    } else if (cmd.equals("du")) {
      duCommand();
    } else if (cmd.equals("df")) {
      dfCommand();
    } else if (cmd.equals("help")) {
      helpCommand();
    } else if (cmd.equals("editor")) {
      editorCommand(arg1);
    } else {
      Serial.println("Invalid command");
    }
    printPrompt();
  }

  void static listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
    Serial.printf("%-40s %-10s %-10s\r\n", "Name", "Type", "Size");

    File root = fs.open(dirname);
    if (!root) {
      Serial.println("- Failed to open directory");
      return;
    }
    if (!root.isDirectory()) {
      Serial.println("- Not a directory");
      return;
    }

    File file = root.openNextFile();
    while (file) {
      if (file.isDirectory()) {
        Serial.printf("%-40s %-10s %-10s\r\n", file.name(), "<DIR>", "");
        if (levels) {
          listDir(fs, file.path(), levels - 1);
        }
      } else {
        Serial.printf("%-40s %-10s %-10d\r\n", file.name(), "FILE", file.size());
      }
      file = root.openNextFile();
    }
  }

  void static readFile(fs::FS &fs, const char *path, bool verbose = false) {
    if(verbose) Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if (!file || file.isDirectory()) {
      Serial.println("- Failed to open file for reading");
      return;
    }

    if(verbose) Serial.println("- Read from file:");
    while (file.available()) {
      char c = '\0';
      String str = "";
      while(c != '\n' && file.available()){
        c = file.read();
        str+= c;
        if(str.length() > SPIFFSShell_MAX_LINE_WIDTH){
          str += "\n";
          break;
        }
      }
      Serial.print(str);
    }
    file.close();
  }

  void static writeFile(fs::FS &fs, const char *path, const char *message, bool verbose = false) {
    if(verbose) Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
      Serial.println("- Failed to open file for writing");
      return;
    }
    if (file.print(message)) {
      if(verbose) Serial.println("- File written");
    } else {
      Serial.println("- Write failed");
    }
    file.close();
  }

  void static appendFile(fs::FS &fs, const char *path, const char *message, bool verbose = false) {
    if(verbose) Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
      Serial.println("- Failed to open file for appending");
      return;
    }
    if (file.print(message)) {
      if(verbose) Serial.println("- Message appended");
    } else {
      Serial.println("- Append failed");
    }
    file.close();
  }

  void static renameFile(fs::FS &fs, const char *path1, const char *path2) {
    Serial.printf("Renaming file %s to %s\r\n", path1, path2);
    if (fs.rename(path1, path2)) {
      Serial.println("- File renamed");
    } else {
      Serial.println("- Rename failed");
    }
  }

  void static deleteFile(fs::FS &fs, const char *path) {
    Serial.printf("Deleting file: %s\r\n", path);
    if (fs.remove(path)) {
      Serial.println("- File deleted");
    } else {
      Serial.println("- Delete failed");
    }
  }

  void static printFlashSize() {
    uint64_t size = SPIFFS.totalBytes() / 1024;
    Serial.printf("Flash memory size: %llu KB\n", size);
  }

  void static printFlashUsed() {
    uint64_t used = SPIFFS.usedBytes() / 1024;
    Serial.printf("Flash memory used: %llu KB\n", used);
  }

  void static lsCommand(const String& dirname) {
    listDir(SPIFFS, getAbsolutePath(dirname).c_str(), 0);
  }

  void static catCommand(const String& path) {
    readFile(SPIFFS, getAbsolutePath(path).c_str());
  }

  void static echoCommand(const String& message, const String& path) {
    writeFile(SPIFFS, getAbsolutePath(path).c_str(), message.c_str());
  }

  void static appendCommand(const String& message, const String& path) {
    appendFile(SPIFFS, getAbsolutePath(path).c_str(), message.c_str());
  }

  void static mvCommand(const String& path1, const String& path2) {
    renameFile(SPIFFS, path1.c_str(), path2.c_str());
  }

  void static rmCommand(const String& path) {
    deleteFile(SPIFFS, getAbsolutePath(path).c_str());
  }

  void static duCommand() {
    printFlashSize();
  }

  void static dfCommand() {
    printFlashUsed();
  }
  
  void static editorCommand(const String& path) {
    Serial.printf("Entering text editor for file: %s\r\n", path.c_str());

    Serial.println("- Text editor started. Enter text (or type '%exit' to quit, '%help' for commands):");
    readFile(SPIFFS, getAbsolutePath(path).c_str());

    while (true) {
      while (!Serial.available()) {
        // Wait for user input
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }

      String input = Serial.readStringUntil('\n');

      if (input.equals("%exit")) {
        Serial.println("- Text editor exited");
        break;
      } else if (input.equals("%help")) {
        Serial.println("--- Available commands:");
        Serial.println("--- %exit - Exit the text editor");
        Serial.println("--- %help - Show available commands");
      } else {
        input += "\n";
        Serial.print(input); // Echo the input
        appendFile(SPIFFS, getAbsolutePath(path).c_str(), input.c_str());
      }
    }
  }

  void static helpCommand() {
    // Print help information
    Serial.println("Available commands:");
    Serial.println("ls <directory> - List the contents of a directory");
    Serial.println("cat <file> - Read the contents of a file");
    Serial.println("echo <message> <file> - Write a message to a file");
    Serial.println("append <message> <file> - Append a message to a file");
    Serial.println("mv <old> <new> - Rename a file");
    Serial.println("rm <file> - Delete a file");
    Serial.println("du - Print total flash memory size");
    Serial.println("df - Print used flash memory");
    Serial.println("editor <file> - Enter a text editor for appending text to a file (type '%exit' to quit, '%help' for commands)");
    Serial.println("help - Show available commands");
  }
}SPIFFSShell;
