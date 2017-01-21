# ESPTemplateProcessor

This library is designed to work with the [ESP8266 Arduino SDK](https://github.com/esp8266/Arduino) and [SPIFFS](https://github.com/esp8266/Arduino/blob/master/doc/filesystem.md).

# Usage

1. Create an HTML template. Substitutions are indicated by a keyword surrounded with `%` symbols, like `%KEY_HERE%`. Here is an example:
    ```html
    <html>
      <title>%TITLE%</title>
      <body>%BODY%</body>
    </html>
    ```

2. Upload your templates to SPIFFS. There is are instruction to do so [HERE](https://github.com/esp8266/Arduino/blob/master/doc/filesystem.md#uploading-files-to-file-system).

3. Include the library, call `SPIFFS.begin()` and setup a handler.
    ```C++
    #include "ESPTemplateProcessor.h"

    ...

    void setup(void){
      SPIFFS.begin();

      ...

      server.on("/", handleRoot);
    }
    ```

4. Create a callback method used to fill in substitutions. The callback is defined `typedef String ProcessorCallback(const String& key);`:
    ```C++
    String indexProcessor(const String& key) {
      Serial.println(String("KEY IS ") + key);
      if (key == "TITLE") return "Hello World!";
      else if (key == "BODY") return "It works!";

      return "oops";
    }
    ```

5. Now in your handler use the processor to tie everything together:
    ```C++
    void handleRoot() {
      if (ESPTemplateProcessor(server).send(String("/index.html"), indexProcessor)) {
        Serial.println("SUCCESS");
      } else {
        Serial.println("FAIL");
        server.send(200, "text/plain", "page not found.");
      }
    }
    ```

# Example

In the example directory is a demo showing full functionality.
