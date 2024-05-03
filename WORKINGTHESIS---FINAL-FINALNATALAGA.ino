  #include <SoftwareSerial.h>
  #include <Fuzzy.h>
  #include <WiFiManager.h> 
  
  #include <Wire.h>
  #include <ThingSpeak.h>
  #include <iot_cmd.h>
  #include <WiFi.h>
  #include <Ezo_i2c.h>
  #include <Ezo_i2c_util.h>
  #include <iot_cmd.h>
  #include <Ezo_uart.h>
  #include <SoftwareSerial.h>                           
  #define rx 16                                         
  #define tx 17                                         
  SoftwareSerial myserial(rx, tx);      
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C address 0x27, 20 column2 `11 8z\ and 4 rows
                

  int s1 = 12;                                          
  int s2 = 13;                                           
  int s3 = 14;                                           
  int port = 1;                                         

  const uint8_t bufferlen = 32;                        
  char response_data[bufferlen];                        
  String inputstring = "";                              

  
  Ezo_uart Module1(myserial, "DO");
  Ezo_uart Module2(myserial, "EC");
  Ezo_uart Module3(myserial, "PH");


 
  unsigned long myChannelNumber = 1999950;
  const char * myWriteAPIKey = "MSYJ1RC639FOSNDW";
  float EC, DO, PH;
 WiFiClient  client;
  int counter = 0;


  const uint8_t module_count = 3;                      
  Ezo_uart Modules[module_count] = {                    
    Module1, Module2, Module3
  };

  void setup() {
  lcd.init(); // initialize the lcd
  lcd.backlight();

   WiFi.mode(WIFI_STA);
    Serial.begin(115200);
    WiFiManager wm;


    bool res;
    res = wm.autoConnect("PONDGUARD-CONNECT-TO-WIFI","password");

    if(!res) {
        Serial.println("Failed to connect");
    } 
    else {
        Serial.println("connected...yeey :)");
    }
    Serial.begin(9600);                                
    myserial.begin(9600);                              
    inputstring.reserve(20);                         
    pinMode(s1, OUTPUT);                              
    pinMode(s2, OUTPUT);                                
    pinMode(s3, OUTPUT);                              

    ThingSpeak.begin(client);  
    Serial.begin(9600);        

    for (uint8_t i = 0; i < module_count; i++) {        
      open_port(i + 1);                                
      Modules[i].send_cmd_no_resp("c,0");              

      delay(100);
      Modules[i].send_cmd_no_resp("*ok,0");          
  
      delay(100);
      Modules[i].flush_rx_buffer();                    
    }
  }

  void loop() {
    if (Serial.available() > 0) {                      
      inputstring = Serial.readStringUntil(13);        
      port = parse_input(inputstring);                  
      open_port(port);                                 

      if (inputstring != "") {                         
        Modules[port - 1].send_cmd(inputstring, response_data, bufferlen); 
        Serial.print(port);                         
        Serial.print("-");
        Serial.print(Modules[port - 1].get_name());   
        Serial.print(": ");
        Serial.println(response_data);                 
        response_data[0] = 0;                           
      }
      else {
        Serial.print("Port is set to ");              
        Serial.println(port);
      }
    }

    

    for (uint8_t i = 0; i < module_count; i++) {      
      open_port(i + 1);
      print_reading(Modules[i]);
      Serial.print(" ");
    }
    Serial.println();

    ThingSpeak.setField(1, EC);
    ThingSpeak.setField(2, PH);
    ThingSpeak.setField(3, DO);

    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if (x == 200) {
      Serial.println("Channel update successful.");
    }
    else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    delay(36000);



  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print("POND GUARD");
  lcd.setCursor(4,1);
  lcd.print("WATER QUALITY");
  lcd.setCursor(1,2);
  lcd.print("DO:");
  lcd.setCursor(14,2);
  lcd.print("pH:");
  lcd.setCursor(6,2);
  lcd.print("Temp:");
  lcd.setCursor(1,3);
  lcd.print(DO);
  lcd.setCursor(14,3);
  lcd.print(PH);
  lcd.setCursor(7,3);
  lcd.print(EC);
  }

  void print_reading(Ezo_uart &Module) {              

    if (Module.send_read()) {
      Serial.print(Module.get_name());                 
      Serial.print(": ");
      Serial.print(Module.get_reading());             
      Serial.print("    ");
    }
    if (Module.get_name() == "DO")
    {
      DO = Module.get_reading();
      Serial.print("Dissolve oxygen :");
      Serial.println(DO);

    }
    if (Module.get_name() == "EC")
    {
      EC = Module.get_reading();
      Serial.print("Temperature :");
      Serial.println(EC);
    }
    if (Module.get_name() == "PH")
    {
      PH = Module.get_reading();
      Serial.print("PH of water :");
      Serial.println(PH);
    }
  }


  uint8_t parse_input(String &inputstring) {                 
    int colon = inputstring.indexOf(':');               
    if ( colon > 0) {                                     
      String port_as_string = inputstring.substring(0, colon);  
      inputstring = inputstring.substring(colon + 1);    
      return port_as_string.toInt();                    
    }
    else {                                             
      return port;                                    
    }
  }

  void open_port(uint8_t _port) {                                

    if (port < 1 || module_count > 8)_port = 1;               
    uint8_t port_bits = _port - 1;

    digitalWrite(s1, bitRead(port_bits, 0));               
    digitalWrite(s2, bitRead(port_bits, 1));              
    digitalWrite(s3, bitRead(port_bits, 2));              
    delay(2);                                         
  }
