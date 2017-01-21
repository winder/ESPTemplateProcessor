#ifndef ESP_TEMPLATE_PROCESSOR_H
#define ESP_TEMPLATE_PROCESSOR_H

#include <ESP8266WebServer.h>
#include <FS.h>

typedef String ProcessorCallback(const String& key);

class ESPTemplateProcessor {
  public:
    ESPTemplateProcessor(ESP8266WebServer& _server) :
      server(_server)
    {
    }

    bool send(const String& filePath, ProcessorCallback& processor)
    {
      // Open file.
      if(!SPIFFS.exists(filePath)) {
        Serial.print("Cannot process "); Serial.print(filePath); Serial.println(": Does not exist.");
        return false;
      }

      File file = SPIFFS.open(filePath, "r");
      if (!file) {
        Serial.print("Cannot process "); Serial.print(filePath); Serial.println(": Failed to open.");
        return false;
      }

      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server.sendHeader("Content-Type","text/html",true);
      server.sendHeader("Cache-Control","no-cache");
      server.send(200);
      //server.sendContent(<chunk>)

      // Process!
      static const uint16_t MAX = 100;
      String buffer;
      int bufferLen = 0;
      String keyBuffer;
      int val;
      char ch;
      while ((val = file.read()) != -1) {
        ch = char(val);
        
        // Lookup expansion.
        if (ch == '%') {
          // Clear out buffer.
          server.sendContent(buffer);
          buffer = "";
          bufferLen = 0;

          // Process substitution.
          keyBuffer = "";
          bool found = false;
          while (!found && (val = file.read()) != -1) {
            ch = char(val);
            if (ch == '%') {
              found = true;
            } else {
              keyBuffer += ch;
            }
          }
          
          // Check for bad exit.
          if (val == -1 && !found) {
            Serial.print("Cannot process "); Serial.print(filePath); Serial.println(": Unable to parse.");
            return false;
          }

          // Get substitution
          String processed = processor(keyBuffer);
          Serial.print("Lookup '"); Serial.print(keyBuffer); Serial.print("' received: "); Serial.println(processed);
          server.sendContent(processed);
        } else {
          bufferLen++;
          buffer += ch;
          if (bufferLen >= MAX) {
            server.sendContent(buffer);
            bufferLen = 0;
            buffer = "";
          }
        }
      }

      if (val == -1) {
        server.sendContent(buffer);
        server.sendContent("");
        return true;
      } else {
        Serial.print("Failed to process '"); Serial.print(filePath); Serial.println("': Didn't reach the end of the file.");
      }
    }


  private:
    ESP8266WebServer &server;
};

#endif
