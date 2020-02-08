#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#define RX_PIN 4 // D2
#define TX_PIN 5 // D1
#define slotNumber 3

const char* ssid = "Infostrada-2.4GHz-9454A5";
const char* password = "0525646993722559";
String protocolServer = "http";
int portServer = 80;
String urlServer = "";

String thingName = "smartparking";
String td = "";

DynamicJsonDocument p_doc(1200);
DeserializationError err;

int properties_number = 4;
int objectProperties_number = 3;
int actions_number = 3;
int events_number = 0;

// Properties
const char* property1_name = "slotsAvailable";
int property1_value = 3;
int property1_minimum = 0;
int property1_maximum = 3;
const char* property2_name = "slot1";
JsonObject property2_value;
String property2_schema = "[{\"type\":\"boolean\",\"name\":\"available\"},{\"type\":\"integer\",\"minimum\":2,\"maximum\":200,\"name\":\"threshold\"}]";
const char* property3_name = "slot2";
JsonObject property3_value;
String property3_schema = "[{\"type\":\"boolean\",\"name\":\"available\"},{\"type\":\"integer\",\"minimum\":4,\"maximum\":50,\"name\":\"threshold\"}]";
const char* property4_name = "slot3";
JsonObject property4_value;
String property4_schema = "[{\"type\":\"boolean\",\"name\":\"available\"},{\"type\":\"integer\",\"minimum\":4,\"maximum\":30,\"name\":\"threshold\"}]";


// Actions
const char* action1_name = "changeThresholdSlot1";
int action1_inputsNumber = 1;
String action1_schema[1] = {"{\"name\":\"threshold\",\"type\":\"integer\",\"minimum\":2,\"maximum\":200}"};
const char* action2_name = "changeThresholdSlot2";
int action2_inputsNumber = 1;
String action2_schema[1] = {"{\"name\":\"threshold\",\"type\":\"integer\",\"minimum\":4,\"maximum\":50}"};
const char* action3_name = "changeThresholdSlot3";
int action3_inputsNumber = 1;
String action3_schema[1] = {"{\"name\":\"threshold\",\"type\":\"integer\",\"minimum\":4,\"maximum\":30}"};


// Requests
String req1 = "/";
String req2 = "/" + thingName;
String req3 = "/" + thingName + "/all/properties";
String req4 = "/" + thingName + "/properties/" + property1_name;
String req5 = "/" + thingName + "/properties/" + property2_name;
String req6 = "/" + thingName + "/properties/" + property3_name;
String req7 = "/" + thingName + "/properties/" + property4_name;
String req8 = "/" + thingName + "/actions/" + action1_name;
String req9 = "/" + thingName + "/actions/" + action2_name;
String req10 = "/" + thingName + "/actions/" + action3_name;

ESP8266WebServer server(portServer);

IPAddress ipS;

SoftwareSerial NodeMCU(RX_PIN, TX_PIN);

String stream = "";
bool done = false;

int i, j, k;

void setup() {
    Serial.begin(115200);
    NodeMCU.begin(9600);
    Serial.println();

    // properties data 
    DynamicJsonDocument op2_doc(226);
    JsonArray arr2;
    DynamicJsonDocument op3_doc(226);
    JsonArray arr3;
    DynamicJsonDocument op4_doc(226);
    JsonArray arr4;

    deserializeJson(op2_doc, property2_schema);
    arr2 = op2_doc.as<JsonArray>();
    property2_value = p_doc.createNestedObject();
    for(i=0; i<arr2.size(); i++) {
        String op2_name = arr2[i]["name"];
        String op2_type = arr2[i]["type"];
				if(op2_type.equals("boolean"))
            property2_value[op2_name] = true;
				if(op2_type.equals("integer") || op2_type.equals("number"))
            property2_value[op2_name] = 6;
		}

    deserializeJson(op3_doc, property3_schema);
    arr3 = op3_doc.as<JsonArray>();
    property3_value = p_doc.createNestedObject();
    for(i=0; i<arr3.size(); i++) {
        String op3_name = arr3[i]["name"];
        String op3_type = arr3[i]["type"];
				if(op3_type.equals("boolean"))
            property3_value[op3_name] = true;
				if(op3_type.equals("integer") || op3_type.equals("number"))
            property3_value[op3_name] = 6;
		}

    deserializeJson(op4_doc, property4_schema);
    arr4 = op4_doc.as<JsonArray>();
    property4_value = p_doc.createNestedObject();
    for(i=0; i<arr4.size(); i++) {
        String op4_name = arr4[i]["name"];
        String op4_type = arr4[i]["type"];
				if(op4_type.equals("boolean"))
            property4_value[op4_name] = true;
				if(op4_type.equals("integer") || op4_type.equals("number"))
            property4_value[op4_name] = 6;
		}
  
    connection(ssid, password);
    
    td = "{\"title\":\"smartparking\",\"id\":\"uri\",\"@context\":\"https://www.w3.org/2019/wot/td/v1\",\"security\":\"nosec_sc\",\"securityDefinitions\":{\"nosec_sc\":{\"scheme\":\"nosec\"}},\"forms\":[{\"href\":\""+urlServer+"/all/properties\",\"contentType\":\"application/json\",\"op\":[\"readallproperties\",\"writeallproperties\",\"readmultipleproperties\",\"writemultipleproperties\"]}],\"description\":\"a simulation of a smart parking using distance sensors\",\"properties\":{\"slotsAvailable\":{\"forms\":[{\"href\":\""+urlServer+"/properties/"+property1_name+"\",\"contentType\":\"application/json\",\"op\":[\"readproperty\"],\"htv:methodName\":\"GET\"}],\"type\":\"integer\",\"minimum\":0,\"maximum\":3,\"observable\":false,\"readOnly\":true,\"writeOnly\":false,\"description\":\"total number of parking slots available\"},\"slot1\":{\"forms\":[{\"href\":\""+urlServer+"/properties/"+property2_name+"\",\"contentType\":\"application/json\",\"op\":[\"readproperty\",\"writeproperty\"],\"htv:methodName\":\"GET\"}],\"type\":\"object\",\"properties\":{\"available\":{\"type\":\"boolean\"},\"threshold\":{\"type\":\"integer\",\"minimum\":2,\"maximum\":200}},\"observable\":false,\"readOnly\":true,\"writeOnly\":true,\"description\":\"first parking slot\",\"sensor\":{\"name\":\"HC-SR04\",\"type\":\"ultrasonic\"}},\"slot2\":{\"forms\":[{\"href\":\""+urlServer+"/properties/"+property3_name+"\",\"contentType\":\"application/json\",\"op\":[\"readproperty\",\"writeproperty\"],\"htv:methodName\":\"GET\"}],\"type\":\"object\",\"properties\":{\"available\":{\"type\":\"boolean\"},\"threshold\":{\"type\":\"integer\",\"minimum\":4,\"maximum\":50}},\"observable\":false,\"readOnly\":true,\"writeOnly\":true,\"description\":\"second parking slot\",\"sensor\":{\"name\":\"Adafruit VL53L0X\",\"type\":\"laser\"}},\"slot3\":{\"forms\":[{\"href\":\""+urlServer+"/properties/"+property4_name+"\",\"contentType\":\"application/json\",\"op\":[\"readproperty\",\"writeproperty\"],\"htv:methodName\":\"GET\"}],\"type\":\"object\",\"properties\":{\"available\":{\"type\":\"boolean\"},\"threshold\":{\"type\":\"integer\",\"minimum\":4,\"maximum\":30}},\"observable\":false,\"readOnly\":true,\"writeOnly\":true,\"description\":\"third parking slot\",\"sensor\":{\"name\":\"Sharp IR\",\"type\":\"laser\"}}},\"actions\":{\"changeThresholdSlot1\":{\"forms\":[{\"href\":\""+urlServer+"/actions/"+action1_name+"\",\"contentType\":\"application/json\",\"op\":\"invokeaction\",\"htv:methodName\":\"POST\"}],\"input\":{\"threshold\":{\"type\":\"integer\",\"minimum\":2,\"maximum\":200}},\"safe\":true,\"idempotent\":true,\"description\":\"change threshold to slot1\"},\"changeThresholdSlot2\":{\"forms\":[{\"href\":\""+urlServer+"/actions/"+action2_name+"\",\"contentType\":\"application/json\",\"op\":\"invokeaction\",\"htv:methodName\":\"POST\"}],\"input\":{\"threshold\":{\"type\":\"integer\",\"minimum\":4,\"maximum\":50}},\"safe\":true,\"idempotent\":true,\"description\":\"change threshold to slot2\"},\"changeThresholdSlot3\":{\"forms\":[{\"href\":\""+urlServer+"/actions/"+action3_name+"\",\"contentType\":\"application/json\",\"op\":\"invokeaction\",\"htv:methodName\":\"POST\"}],\"input\":{\"threshold\":{\"type\":\"integer\",\"minimum\":4,\"maximum\":30}},\"safe\":true,\"idempotent\":true,\"description\":\"change threshold to slot3\"}}}";

    done = false;
    int count = 0;
    while(!done) {
        // get parking data
        NodeMCU.print('i');
        while(NodeMCU.available() > 0) {
            char c = NodeMCU.read();
            if(c != '\n' && c != 'd') 
                stream += c;
            else if(c == 'd') {
                if(count == 7)
                   done = true;
            }
            else {
                if(!stream.equals("")) {
                    if(count == 0)
                        property1_value = stream.toInt();
                    else if(count == 1) {
                        if(stream.equals("A"))
                            property2_value["available"] = true;  
                        else
                            property2_value["available"] = false;  
                    }
                    else if(count == 2)
                        property2_value["threshold"] = stream.toInt();
                    else if(count == 3) {
                        if(stream.equals("A"))
                            property3_value["available"] = true;  
                        else
                            property3_value["available"] = false;     
                    }
                    else if(count == 4)
                        property3_value["threshold"] = stream.toInt();
                    else if(count == 5) {
                        if(stream.equals("A"))
                            property4_value["available"] = true;  
                        else
                            property4_value["available"] = false;       
                    }
                    else if(count == 6)
                        property4_value["threshold"] = stream.toInt();
                    
                    Serial.printf("%d: %s\n", count, stream.c_str());    
                    stream = "";
                    count++;
                }   
            }
        }
    }
    
    // Server requests
    server.on(req1,HTTP_GET,handleReq1);
    server.on(req2,HTTP_GET,handleReq2);
    server.on(req3,HTTP_GET,handleReq3);
    server.on(req4,HTTP_GET,handleReq4);
    server.on(req5,HTTP_GET,handleReq5);
    server.on(req6,HTTP_GET,handleReq6);
    server.on(req7,HTTP_GET,handleReq7);
    server.on(req8,HTTP_POST,handleReq8);
    server.on(req9,HTTP_POST,handleReq9);
    server.on(req10,HTTP_POST,handleReq10);
    server.on(req8,HTTP_GET,handleReq11);
    server.on(req9,HTTP_GET,handleReq12);
    server.on(req10,HTTP_GET,handleReq13);

    server.begin();

    Serial.println("Server started");
    Serial.println(urlServer);
}    

void loop() {
    // verify if slot is available or not
    while(NodeMCU.available() > 0) {
        char c = NodeMCU.read();
        if(c == '0') {
            done = false;
            while(!done) {
                while(NodeMCU.available() > 0) {
                    c = NodeMCU.read();
                    if(c != 'd')
                        stream += c;
                    else { 
                        if(stream.equals("A")) {
                            property1_value += 1;
                            property2_value["available"] = true;
                        }
                        else {
                            property1_value -= 1;
                            property2_value["available"] = false;       
                        }
                        done = true;
                        NodeMCU.print('k');
                        Serial.println(stream);
                        stream = "";    
                    }     
                }
            }
        }
        else if(c == '1') {
            done = false;
            while(!done) {
                while(NodeMCU.available() > 0) {
                    c = NodeMCU.read();
                    if(c != 'd')
                        stream += c;
                    else { 
                        if(stream.equals("A")) {
                            property1_value += 1;
                            property3_value["available"] = true;
                        }
                        else {
                            property1_value -= 1;
                            property3_value["available"] = false;       
                        }
                        done = true;
                        NodeMCU.print('k');
                        Serial.println(stream);
                        stream = "";    
                    }     
                }
            }
        }
        else if(c == '2') {
            done = false;
            while(!done) {
                while(NodeMCU.available() > 0) {
                    c = NodeMCU.read();
                    if(c != 'd')
                        stream += c;
                    else { 
                        if(stream.equals("A")) {
                            property1_value += 1;
                            property4_value["available"] = true;
                        }
                        else {
                            property1_value -= 1;
                            property4_value["available"] = false;       
                        }
                        done = true;
                        NodeMCU.print('k');
                        Serial.println(stream);
                        stream = "";    
                    }     
                }
            }
        }
    }

    // handle Requests
    server.handleClient();
}

void connection(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    
    Serial.print("\nConnecting to ");
    Serial.print(ssid);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("\nConnected");
    Serial.print("IP address: ");
    ipS = WiFi.localIP();
    Serial.println(ipS);

    urlServer = protocolServer + "://" + ipS.toString() + ":" + portServer + "/" + thingName;
}

// Request functions
void handleReq1() {
    String resp = "";

    Serial.println("\nGET Thing URL");
    resp = "[\"" + urlServer + "\"]";
    server.send(200, "application/ld+json", resp);
}

void handleReq2() {
    Serial.println("\nGET Thing Description"); 
    server.send(200, "application/ld+json", td);
}

void handleReq3() {
    String resp = "";

    resp = request3();
    server.send(200, "application/ld+json", resp);
}

void handleReq4() {
    String resp = "";
    
    resp = request4();
    server.send(200, "application/ld+json", resp);
}

void handleReq5() {
    String resp = "";
    
    resp = request5();
    server.send(200, "application/ld+json", resp);
}

void handleReq6() {
    String resp = "";
    
    resp = request6();
    server.send(200, "application/ld+json", resp);
}

void handleReq7() {
    String resp = "";
    
    resp = request7();
    server.send(200, "application/ld+json", resp);
}

void handleReq8() {
    String resp = "";
    String body = server.arg("plain");
    
    resp = request8(body);
    server.send(200, "application/ld+json", resp);
}
void handleReq9() {
    String resp = "";
    String body = server.arg("plain");
    
    resp = request9(body);
    server.send(200, "application/ld+json", resp);
}
void handleReq10() {
    String resp = "";
    String body = server.arg("plain");
    
    resp = request10(body);
    server.send(200, "application/ld+json", resp);
}

void handleReq11() {
    char* resp = "Method Not Allowed";
    server.send(405, "text/plain", resp);
}
void handleReq12() {
    char* resp = "Method Not Allowed";
    server.send(405, "text/plain", resp);
}
void handleReq13() {
    char* resp = "Method Not Allowed";
    server.send(405, "text/plain", resp);
}

String request3() {
    DynamicJsonDocument tmp(1420);
    String resp = "";
    JsonObject obj = tmp.createNestedObject();

    Serial.println("\nGET all properties");
    obj[property1_name] = property1_value;
    obj[property2_name] = property2_value;
    obj[property3_name] = property3_value;
    obj[property4_name] = property4_value;
    serializeJson(obj, resp);

    return resp;
}

String request4() {
    String resp = "";

    Serial.printf("\nGET %s value\n", property1_name);
    resp = "{\"" + (String) property1_name + "\":" + property1_value + "}";
    
    return resp;
}

String request5() {
    String resp = "";
    String tmp = "";

    Serial.printf("\nGET %s value\n", property2_name);
    serializeJson(property2_value, tmp);
    resp = "{\"" + (String) property2_name + "\":" + tmp + "}";
    
    return resp;
}

String request6() {
    String resp = "";
    String tmp = "";

    Serial.printf("\nGET %s value\n", property3_name);
    serializeJson(property3_value, tmp);
    resp = "{\"" + (String) property3_name + "\":" + tmp + "}";
    
    return resp;
}

String request7() {
    String resp = "";
    String tmp = "";

    Serial.printf("\nGET %s value\n", property4_name);
    serializeJson(property4_value, tmp);
    resp = "{\"" + (String) property4_name + "\":" + tmp + "}";
    
    return resp;
}

String request8(String body) {
    DynamicJsonDocument resp_doc(200);
    String resp = "";

    Serial.printf("\nPOST invokeaction %s\n", action1_name);
    Serial.printf("Body received: %s\n", body.c_str());
    
    err = deserializeJson(resp_doc, body);
    if(err) {
        Serial.printf("deserializeJson() failed with code %s", err.c_str());
        resp = err.c_str();
        return resp;
    }
    else {
        if(resp_doc["threshold"].isNull())
            resp = "InvalidInput";
        else {
            bool validInput = true;
            String value = "";

            String action1_input[1] = {};    
            int action1_input1_value = 0;

            i = 0;
            while(validInput and i<action1_inputsNumber) {
                switch(i) {
                    case 0: {
                        value = "";
                        serializeJson(resp_doc["threshold"], value);
                        action1_input[0] = value;
                        validInput = handleInputType(value,action1_schema[0]);
                    }
                    break;

                }
                i++;
            }    

            if(validInput) {

                action1_input1_value = action1_input[0].toInt();

                changeThresholdSlot1(action1_input1_value); 
                resp = "";
            }
            else
                resp = "InvalidInput";
        }
    }
    return resp;
}
String request9(String body) {
    DynamicJsonDocument resp_doc(200);
    String resp = "";

    Serial.printf("\nPOST invokeaction %s\n", action2_name);
    Serial.printf("Body received: %s\n", body.c_str());
    
    err = deserializeJson(resp_doc, body);
    if(err) {
        Serial.printf("deserializeJson() failed with code %s", err.c_str());
        resp = err.c_str();
        return resp;
    }
    else {
        if(resp_doc["threshold"].isNull())
            resp = "InvalidInput";
        else {
            bool validInput = true;
            String value = "";

            String action2_input[1] = {};    
            int action2_input1_value = 0;

            i = 0;
            while(validInput and i<action2_inputsNumber) {
                switch(i) {
                    case 0: {
                        value = "";
                        serializeJson(resp_doc["threshold"], value);
                        action2_input[0] = value;
                        validInput = handleInputType(value,action2_schema[0]);
                    }
                    break;

                }
                i++;
            }    

            if(validInput) {

                action2_input1_value = action2_input[0].toInt();

                changeThresholdSlot2(action2_input1_value); 
                resp = "";
            }
            else
                resp = "InvalidInput";
        }
    }
    return resp;
}
String request10(String body) {
    DynamicJsonDocument resp_doc(200);
    String resp = "";

    Serial.printf("\nPOST invokeaction %s\n", action3_name);
    Serial.printf("Body received: %s\n", body.c_str());
    
    err = deserializeJson(resp_doc, body);
    if(err) {
        Serial.printf("deserializeJson() failed with code %s", err.c_str());
        resp = err.c_str();
        return resp;
    }
    else {
        if(resp_doc["threshold"].isNull())
            resp = "InvalidInput";
        else {
            bool validInput = true;
            String value = "";

            String action3_input[1] = {};    
            int action3_input1_value = 0;

            i = 0;
            while(validInput and i<action3_inputsNumber) {
                switch(i) {
                    case 0: {
                        value = "";
                        serializeJson(resp_doc["threshold"], value);
                        action3_input[0] = value;
                        validInput = handleInputType(value,action3_schema[0]);
                    }
                    break;

                }
                i++;
            }    

            if(validInput) {

                action3_input1_value = action3_input[0].toInt();

                changeThresholdSlot3(action3_input1_value); 
                resp = "";
            }
            else
                resp = "InvalidInput";
        }
    }
    return resp;
}

// handle Input Types
bool handleInputType(String value, String schema) {   
	DynamicJsonDocument schema_doc(200);
    bool validInput = true;

    deserializeJson(schema_doc, schema);
    JsonObject obj_schema = schema_doc.as<JsonObject>();
    String type = obj_schema["type"];
    
    if(value[0] == '"')
        value.remove(0);
    if(value[value.length()-1] == '"')    
        value.remove(value.length()-1);
    
		if(type.equals("integer") || type.equals("number")) {
        int dot_count = 0;
        i = 0;
        while(validInput && i<value.length()) {
            if(!isDigit(value[i])) 
                validInput = false;
            else if(value[i] == '.')
                if(i == 0 || i == value.length()-1 || dot_count > 1)
                    validInput = false;
                else 
                    dot_count++;    
            i++;          
        } 
        if(validInput) {
            double input = value.toDouble(); 
            if(!obj_schema["minimum"].isNull()) 
                if(input < obj_schema["minimum"])
                    validInput = false;
        
            if(!obj_schema["maximum"].isNull())  
                if(input > obj_schema["maximum"])
                    validInput = false;     
          
        }
    }
    return validInput;
}

// Action functions
void changeThresholdSlot1(int threshold) {
    property2_value["threshold"] = threshold;

    // notify Arduino
    int n = 0;
    done = false;
    while(!done && n<30) {
        NodeMCU.print('0');
        NodeMCU.print(threshold);
        NodeMCU.print('d');
        delay(2000);
        while(NodeMCU.available() > 0) {
            char c = NodeMCU.read();
            if(c == 'k') 
                done = true;  
        }
        n++;
    }
 
}

void changeThresholdSlot2(int threshold) {
    property3_value["threshold"] = threshold;

    // notify Arduino
    int n = 0;
    done = false;
    while(!done && n<30) {
        NodeMCU.print('1');
        NodeMCU.print(threshold);
        NodeMCU.print('d');
        delay(2000);
        while(NodeMCU.available() > 0) {
            char c = NodeMCU.read();
            if(c == 'k')
                done = true;  
        }
        n++;
    }
}

void changeThresholdSlot3(int threshold) {
  property4_value["threshold"] = threshold;

    // notify Arduino
    int n = 0;
    done = false;
    while(!done && n<30) {
        NodeMCU.print('2');
        NodeMCU.print(threshold);
        NodeMCU.print('d');
        delay(2000);
        while(NodeMCU.available() > 0) {
            char c = NodeMCU.read();
            if(c == 'k')
                done = true;  
        }
        n++;
    }
}

