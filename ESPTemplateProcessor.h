#ifndef ESP_TEMPLATE_PROCESSOR_H
#define ESP_TEMPLATE_PROCESSOR_H

#ifdef ESP8266
#define WebServer ESP8266WebServer
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif

#include <FS.h>

#ifdef ESP32
#include <SPIFFS.h>
#endif

typedef String ProcessorCallback(const String& key);

class ESPTemplateProcessor {
  public:
    ESPTemplateProcessor(WebServer& _server) :
      server(_server)
    {
    }

    bool send(const String& filePath, ProcessorCallback& processor, char bookend = '%', bool silentSerial = false)
    {
      // Open file.
      if(!SPIFFS.exists(filePath)) {
        if(!silentSerial) {
          Serial.print("Cannot process "); Serial.print(filePath); Serial.println(": Does not exist.");
        }
        return false;
      }

      File file = SPIFFS.open(filePath, "r");
      if (!file) {
        if(!silentSerial) {
          Serial.print("Cannot process "); Serial.print(filePath); Serial.println(": Failed to open.");
        }
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
        if (ch == bookend) {
          // Clear out buffer.
          server.sendContent(buffer);
          buffer = "";
          bufferLen = 0;

          // Process substitution.
          keyBuffer = "";
          bool found = false;
          while (!found && (val = file.read()) != -1) {
            ch = char(val);
            if (ch == bookend) {
              found = true;
            } else {
              keyBuffer += ch;
            }
          }
          
          // Check for bad exit.
          if (val == -1 && !found) {
            if(!silentSerial) {
              Serial.print("Cannot process "); Serial.print(filePath); Serial.println(": Unable to parse.");
            }
            return false;
          }

          // Get substitution
          String processed = processor(keyBuffer);
          if(!silentSerial) {
            Serial.print("Lookup '"); Serial.print(keyBuffer); Serial.print("' received: "); Serial.println(processed);
          }
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
        if(!silentSerial) {
          Serial.print("Failed to process '"); Serial.print(filePath); Serial.println("': Didn't reach the end of the file.");
        }
      }
    }


  private:
    WebServer &server;
};

#endif
