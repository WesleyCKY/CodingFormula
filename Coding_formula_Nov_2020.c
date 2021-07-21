#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

//======  change the group id here  =======

const String group_id = "IOT-A_052";

// ========================================

const String apSSID = group_id;
const IPAddress apIP(192, 168, 1, 1);
boolean settingMode;
String ssidList;
int pin_1 = 5;
int pin_2 = 4;
int pin_3 = 14;
int pin_4 = 12;
int pin_led = 2;
//int pin_test = 16;
int resetPin = 16;

DNSServer dnsServer;
ESP8266WebServer webServer(80);

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);

  Serial.print("^^^^^^^^");
  //pinMode(pin_test, INPUT);
  pinMode(pin_led, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(pin_1, OUTPUT);
  pinMode(pin_2, OUTPUT);
  pinMode(pin_3, OUTPUT);
  pinMode(pin_4, OUTPUT);

  delay(10);
  if (restoreConfig()) {
    if (checkConnection()) {
      settingMode = false;
      startWebServer();
      return;
    }
  }
  settingMode = true;
  setupMode();
}

void loop() {
  //pinMode(pin_test, INPUT);

  /*if (digitalRead(pin_test) != LOW && WiFi.status() != WL_CONNECTED) {
    analogWrite(pin_1, 0);
    analogWrite(pin_2, 1000);
    analogWrite(pin_3, 0);
    analogWrite(pin_4, 1000);
    delay(1000);
    //backward
    analogWrite(pin_1, 1000);
    analogWrite(pin_2, 0);
    analogWrite(pin_3, 1000);
    analogWrite(pin_4, 0);
    delay(1000);
    //left
    analogWrite(pin_1, 1000);
    analogWrite(pin_2, 0);
    analogWrite(pin_3, 0);
    analogWrite(pin_4, 1000);
    delay(1000);
    //right
    analogWrite(pin_1, 0);
    analogWrite(pin_2, 1000);
    analogWrite(pin_3, 1000);
    analogWrite(pin_4, 0);
    delay(1000);
    //stop
    analogWrite(pin_1, 0);
    analogWrite(pin_2, 0);
    analogWrite(pin_3, 0);
    analogWrite(pin_4, 0);
    delay(1000);
    pinMode(pin_test, OUTPUT);
    digitalWrite(pin_test, LOW);
    Serial.println("TEST ONCE ！");
  }
  delay(1000);*/
  if (WiFi.status() != WL_CONNECTED) {
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(500);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      delay(500);
    }
  if (settingMode) {
    dnsServer.processNextRequest();
  }
  webServer.handleClient();
  //hard reset
  /*if (digitalRead(resetPin) != LOW && WiFi.status() == WL_CONNECTED) {
    for (int i = 0; i < 96; ++i) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();
    Serial.println("WIFI configuration has been reset !");
    for (int i = 0; i <= 10 ; i++) {
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      delay(100);
    }
    Serial.println("Reset..");
    ESP.restart();
  }*/
}

boolean restoreConfig() {
  Serial.println("Reading EEPROM...");
  String ssid = "";
  String pass = "";
  if (EEPROM.read(0) != 0) {
    for (int i = 0; i < 32; ++i) {
      ssid += char(EEPROM.read(i));
    }
    Serial.print("SSID: ");
    Serial.println(ssid);
    for (int i = 32; i < 96; ++i) {
      pass += char(EEPROM.read(i));
    }
    Serial.print("Password: ");
    Serial.println(pass);
    WiFi.begin(ssid.c_str(), pass.c_str());
    return true;
  }
  else {
    Serial.println("Config not found.");
    return false;
  }
}

boolean checkConnection() {
  int count = 0;
  Serial.print("Waiting for Wi-Fi connection");
  while ( count < 25 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.println("Connected!");

      HTTPClient http; //declare
      String url_base = "http://aicar.hk/api/code.php?group_id=";
      String url = url_base + group_id; //combine with group id
      http.begin(url);
      int httpCode = http.GET();
      if (httpCode > 0) { //Check the returning code
        String Command = http.getString();
        Serial.println(Command);
        do {
          char Str[Command.length() + 1];
          Command.toCharArray(Str, Command.length() + 1);
          char *token = strtok(Str, " ");
          char *Str_s[300];
          int index = 0;
          float time_f;
          String time_s;
          float offsetleft = 100;
          float offsetright = 100;
          float laspeed = 100;
          float raspeed = 100;
          float angle = 100;
          //==============while loop
          while (token != NULL) {
            Str_s[index] = token;
            index++;
            //Serial.println(token);
            token = strtok(NULL, " ");
          }
          Serial.println("********************************");
          Serial.println("For loop: Checking command and Run");
          Serial.println("********************************");
          Serial.println("");
          Serial.println("");
          for (int n = 0; n < index; n++) {
            Serial.println(Str_s[n]);
            //offset left and right
            if (strcmp(Str_s[n], "offsetleft") == 0) {
              ++n;
              time_s = Str_s[n];
              offsetleft = time_s.toFloat();
              Serial.println(offsetleft);
            }

            if (strcmp(Str_s[n], "offsetright") == 0) {
              ++n;
              time_s = Str_s[n];
              offsetright = time_s.toFloat();
              Serial.println(offsetright);
            }

            if (strcmp(Str_s[n], "laspeed") == 0) {
              ++n;
              time_s = Str_s[n];
              laspeed = time_s.toFloat();
              Serial.println(laspeed);
            }
            if (strcmp(Str_s[n], "raspeed") == 0) {
              ++n;
              time_s = Str_s[n];
              raspeed = time_s.toFloat();
              Serial.println(raspeed);
            }
            if (strcmp(Str_s[n], "forward") == 0) {
              ++n;
              time_s = Str_s[n];
              time_f = time_s.toFloat();
              Serial.println(time_f);
              forward(offsetleft, offsetright);
              delay(time_f);
              stop_now();
            }
            if (strcmp(Str_s[n], "backward") == 0) {
              n = n + 1;
              time_s = Str_s[n];
              time_f = time_s.toFloat();
              Serial.println(time_f);
              backward(offsetleft, offsetright);
              delay(time_f);
              stop_now();
            }
            if (strcmp(Str_s[n], "left") == 0) {
              n = n + 1;
              time_s = Str_s[n];
              angle = time_s.toFloat();
              Serial.println(angle);
              left(offsetleft, offsetright);
              delay(angle / laspeed * 1000); //
              stop_now();

            }
            if (strcmp(Str_s[n], "right") == 0) {
              n = n + 1;
              time_s = Str_s[n];
              angle = time_s.toFloat();
              Serial.println(angle);
              right(offsetleft, offsetright);
              delay(angle / raspeed * 1000);
              stop_now();

            }
            if (strcmp(Str_s[n], "stop") == 0) {
              stop_now();
              break;
            }
          }
        } while (0);
        return true;
      }
    }
    delay(500);
    Serial.print(".");
    count++;
  }
  Serial.println("Timed out.");
  return false;
}

void startWebServer() {
  if (settingMode) {
    Serial.print("Starting Web Server at ");
    Serial.println(WiFi.softAPIP());

    webServer.on("/settings", []() {
      String s = "<h1>Wi-Fi Settings</h1><p>Please enter your password by selecting the SSID.</p>";
      s += "<form method=\"get\" action=\"setap\"><label>SSID: </label><select name=\"ssid\">";
      s += ssidList;
      s += "</select><br>Password: <input name=\"pass\" length=64 type=\"password\"><input type=\"submit\"></form>";
      s += "<br>";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    });

    webServer.on("/apcode", []() {
      String s = "<h1>Welcome to Formula Coding Competition</h1>";
      s += "<form method=\"get\" action=\"setcommand\">";
      s += "Command:<br><textarea rows=\'10' cols=\'50' name=\"APcommand\"></textarea><br><input type=\"submit\" value=\"Submit\"></form>";
      webServer.send(200, "text/html", makePage("Welcome", s));
    });

    webServer.on("/setcommand", []() {
      String x = webServer.arg("APcommand");
      Serial.println(x);
      do {
        char Str[x.length() + 1];
        x.toCharArray(Str, x.length() + 1);
        char *token = strtok(Str, " ");
        char *Str_s[300];
        int index = 0;
        float time_f;
        String time_s;
        float offsetleft = 100;
        float offsetright = 100;
        float laspeed = 100;
        float raspeed = 100;
        float angle = 100;
        //==============while loop
        while (token != NULL) {
          Str_s[index] = token;
          index++;
          //Serial.println(token);
          token = strtok(NULL, " ");
        }

        for (int n = 0; n < index; n++) {
          Serial.println(Str_s[n]);
          //offset left and right
          if (strcmp(Str_s[n], "offsetleft") == 0) {
            ++n;
            time_s = Str_s[n];
            offsetleft = time_s.toFloat();
            Serial.println(offsetleft);
          }

          if (strcmp(Str_s[n], "offsetright") == 0) {
            ++n;
            time_s = Str_s[n];
            offsetright = time_s.toFloat();
            Serial.println(offsetright);
          }

          if (strcmp(Str_s[n], "laspeed") == 0) {
            ++n;
            time_s = Str_s[n];
            laspeed = time_s.toFloat();
            Serial.println(laspeed);
          }
          if (strcmp(Str_s[n], "raspeed") == 0) {
            ++n;
            time_s = Str_s[n];
            raspeed = time_s.toFloat();
            Serial.println(raspeed);
          }
          if (strcmp(Str_s[n], "forward") == 0) {
            ++n;
            time_s = Str_s[n];
            time_f = time_s.toFloat();
            Serial.println(time_f);
            forward(offsetleft, offsetright);
            delay(time_f);
            stop_now();
          }
          if (strcmp(Str_s[n], "backward") == 0) {
            n = n + 1;
            time_s = Str_s[n];
            time_f = time_s.toFloat();
            Serial.println(time_f);
            backward(offsetleft, offsetright);
            delay(time_f);
            stop_now();
            delay(500);
          }
          if (strcmp(Str_s[n], "left") == 0) {
            n = n + 1;
            time_s = Str_s[n];
            angle = time_s.toFloat();
            Serial.println(angle);
            left(offsetleft, offsetright);
            delay(angle / laspeed * 1000); //
            stop_now();
            delay(500);
          }
          if (strcmp(Str_s[n], "right") == 0) {
            n = n + 1;
            time_s = Str_s[n];
            angle = time_s.toFloat();
            Serial.println(angle);
            right(offsetleft, offsetright);
            delay(angle / raspeed * 1000);
            stop_now();
            delay(500);

          }
          if (strcmp(Str_s[n], "stop") == 0) {
            stop_now();
            break;
          }
        }
      } while (0);

      String s = "<a href=\"/\"><button class=\"button button2\"> Go Back </button></a>";
      //end convertion and perform motion
      webServer.send(200, "text/html", makePage("Apcoding", s));
    });

    webServer.on("/setap", []() {
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      String ssid = urlDecode(webServer.arg("ssid"));
      Serial.print("SSID: ");
      Serial.println(ssid);
      String pass = urlDecode(webServer.arg("pass"));
      Serial.print("Password: ");
      Serial.println(pass);
      Serial.println("Writing SSID to EEPROM...");
      for (int i = 0; i < ssid.length(); ++i) {
        EEPROM.write(i, ssid[i]);
      }
      Serial.println("Writing Password to EEPROM...");
      for (int i = 0; i < pass.length(); ++i) {
        EEPROM.write(32 + i, pass[i]);
      }
      EEPROM.commit();
      Serial.println("Write EEPROM done!");
      String s = "<h1>Setup complete.</h1><p>device will be connected to \"";
      s += ssid;
      s += "\" after the restart.";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
      ESP.restart();
    });

    webServer.on("/Reset", []() {
      String s = "<h1>STA mode</h1><p><a href=\"/reset\"><button class=\"button\">Reset Wi-Fi Settings</button></a></p>";
      webServer.send(200, "text/html", makePage("STA mode", s));
    });

    webServer.on("/reset", []() {
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
      String s = "<h1>Wi-Fi was reset.</h1><p>Please press device reset button.</p>";
      webServer.send(200, "text/html", makePage("reset Wi-Fi Settings", s));
    });

    webServer.onNotFound([]() {
      String s = "<h1><strong>Welcome to Formula Coding Competition</strong></h1><h2>You may choose one of the following options:</h2><p><a href=\"/settings\"><button class=\"button\">Wi-Fi Setting</button></a></p>";
      s += "<p><a href=\"/apcode\"><button class=\"button button2\">Access Point</button></a></p>";
      s += "<p><a href=\"/Reset\"><button class=\"button button\">Wi-Fi Restore</button></a></p>";
      webServer.send(200, "text/html", makePage("AP mode", s));
    });
  }
  else {
    Serial.print("Starting Web Server at ");
    Serial.println(WiFi.localIP());

    webServer.on("/", []() {
      String s = "<h1>STA mode</h1><p><a href=\"/reset\"><button class=\"button\">Reset Wi-Fi Settings</button></a></p>";
      webServer.send(200, "text/html", makePage("STA mode", s));
    });

    webServer.on("/reset", []() {
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
      String s = "<h1>Wi-Fi was reset.</h1><p>Please press device reset button.</p>";
      webServer.send(200, "text/html", makePage("reset Wi-Fi Settings", s));
    });
  }
  webServer.begin();
}

void setupMode() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  delay(100);
  Serial.println("");
  for (int i = 0; i < n; ++i) {
    ssidList += "<option value=\"";
    ssidList += WiFi.SSID(i);
    ssidList += "\">";
    ssidList += WiFi.SSID(i);
    ssidList += "</option>";
  }
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  char password[20];
  sprintf(password, "%08X", ESP.getChipId());

  WiFi.softAP(apSSID, password);
  dnsServer.start(53, "*", apIP);
  startWebServer();
  Serial.print("Starting Access Point at \"");
  Serial.print(apSSID);
  Serial.print("\"");
  Serial.println("The Chip id is:");
  Serial.println(password);
  Serial.println("^^^^^^^^");
}

String makePage(String title, String contents) {

  String s = "<html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  s += "<style>html{font-family: Helvetica; display:inline-block;margin: 0px auto;text-align: center;}h1{color: #0F3376; padding: 2vh;}";
  s += ".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;}";
  s += "text-decoration: none; font-size: 40px; margin: 2px; cursor: pointer;}";
  s += ".button2 {background-color: #77878A;}";
  s += "</style>";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += "<p>";
  s += "<svg width=\"24\" height=\"24\" xmlns=\"http://www.w3.org/2000/svg\" fill-rule=\"evenodd\" clip-rule=\"evenodd\"><path d=\"M22 10h-1v-2h-11v13h5v1.617c0 .524.121 1.058.502 1.383h-5.002c-.398 0-.779-.158-1.061-.439-.281-.282-.439-.663-.439-1.061v-15c0-.398.158-.779.439-1.061.282-.281.663-.439 1.061-.439h10c.398 0 .779.158 1.061.439.281.282.439.663.439 1.061v2.5zm2 2.25c0-.69-.56-1.25-1.25-1.25h-5.5c-.69 0-1.25.56-1.25 1.25v10.5c0 .69.56 1.25 1.25 1.25h5.5c.69 0 1.25-.56 1.25-1.25v-10.5zm-15.407 11.75h-6.593l2.599-3h3.401v1.804c0 .579.337 1.09.593 1.196zm11.407-1c-.553 0-1-.448-1-1s.447-1 1-1c.552 0 .999.448.999 1s-.447 1-.999 1zm3-3v-6.024h-6v6.024h6zm-2-15h-2v-3h-17v15h6v2h-8v-19h21v5zm-.5 7h-1c-.276 0-.5.224-.5.5s.224.5.5.5h1c.275 0 .5-.224.5-.5s-.225-.5-.5-.5z\"/></svg>";
  return s;
}

String urlDecode(String input) {
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}

float forward(float offl, float offr) {
  analogWrite(pin_1, 0);
  analogWrite(pin_2, 10 * offl);
  analogWrite(pin_3, 0);
  analogWrite(pin_4, 10 * offr);

}
float backward(float offl, float offr) {
  analogWrite(pin_1, 10 * offl);
  analogWrite(pin_2, 0);
  analogWrite(pin_3, 10 * offr);
  analogWrite(pin_4, 0);
}
float stop_now() {
  analogWrite(pin_1, 0);
  analogWrite(pin_2, 0);
  analogWrite(pin_3, 0);
  analogWrite(pin_4, 0);
}

float left(float offl, float offr) { //float laspeed
  analogWrite(pin_1, 10 * offl);
  analogWrite(pin_2, 0);
  analogWrite(pin_3, 0);
  analogWrite(pin_4, 10 * offr);
}

float right(float offl, float offr) { //float raspeed
  analogWrite(pin_1, 0);
  analogWrite(pin_2, 10 * offl);
  analogWrite(pin_3, 10 * offr);
  analogWrite(pin_4, 0);
}