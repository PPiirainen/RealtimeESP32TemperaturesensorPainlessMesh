//************************************************************
//Keskusnoden koodi:
// vastaanottaa lähettyksen,lähettää mesh-verkosta tuleen viestin UARTiin
// ja lähettää takaisin lähettäjälle tarkoitetun koodin.
// UARTista vastaanotetut viestit lähetetään MESH verkoon Send-Single -metodilla tietylle nodelle.
//Tämä pelkästään Mesh verkossam ei wifissä
//************************************************************

#include <Wire.h>
//#include <Adafruit_Sensor.h>

#include "namedMesh.h"
#include <HardwareSerial.h>
HardwareSerial SerialPort(2);
#include <ArduinoJson.h>
#define MESH_SSID "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555
#define LED_PIN 2

Scheduler   userScheduler;  // to control your personal task
namedMesh mesh;

String nodeName = "CenterNode";  // Name needs to be unique

void sendMessage();                                                 // Prototype so PlatformIO doesn't complain
                                                                    //**Create tasks:  to send messages and get readings.'
                                                                    //**This is the task that is send periodiacally.
Task taskSendMessage(TASK_SECOND * 60, TASK_FOREVER, &sendMessage);  // send message every 2 sec


void sendMessage() {  // send free heap size to HA. helps with troupleshooting
  DynamicJsonDocument doc(128);
  doc["node"] = nodeName;
  doc["CenterHeap"] = ESP.getFreeHeap();
  serializeJson(doc, SerialPort);
  //serializeJson(doc, SerialPort);
  SerialPort.write('\r');
  //SerialPort.write('\n');
  serializeJson(doc, Serial);
  Serial.println(" ");
  Serial.print("send the heap size");
  Serial.println(ESP.getFreeHeap());
  
}

void SendMessageMessageBack(String ToNode) {  // not really a task.On message reciece get node information and send back a confirmation about the recieved message.
  Serial.print("From the node ");
  Serial.println(ToNode);
  Serial.print("Sending acknolegment message back");
    //DynamicJsonDocument doc(64);
    // StaticJsonDocument<64> doc;
    // JSONVar jsonReadings;
    // double NodeTime = mesh.getNodeTime();
    // jsonReadings["node"] = nodeName;
    // jsonReadings["Ntime"] = NodeTime;
    // readings = JSON.stringify(jsonReadings);
    //double NodeTime = mesh.getNodeTime();
    //doc["node"] = nodeName;
    //doc["Ntime"] = NodeTime;
  String str;  // string to store the message
  //serializeJson(doc, str);
  String to = ToNode;
  mesh.sendSingle(to, str);  // we can send empty message. we will never be here if we havent got message from the node.
}


String ExtractIncomingNodeName(String message) {  // this takes the name of the node that sens the incomning message

  // DynamicJsonDocumentdoc(64);
  StaticJsonDocument<64> doc;
  deserializeJson(doc, message);
  //Serial.print("This is the Node that you are löooking for: ");
  const char* FromNode = doc["node"];

  Serial.print("This is the FromNode that you are löooking for: ");
  Serial.println(FromNode);
  //const char* FromID = myObject["node"]; // send confirmation message.

  return FromNode;
}



void receivedCallback(uint32_t from, String& msg) {
  
  String node;
  node = ExtractIncomingNodeName(msg);  // extract the node
  Serial.println(" ");
  Serial.println("The node name");
  Serial.println(node);

  int TestIfMessageFromTemnode = node.indexOf("Temp");
  if (TestIfMessageFromTemnode != -1) {  //immideadly send message back so that the node goes to sleep.
    SendMessageMessageBack(node);
    Serial.println(" ");
    Serial.println("Message from Temp-node");
    /*
   DynamicJsonDocument doc1(512);  // get the data from the incoming message.
    deserializeJson(doc1, msg);
    //double NodeTime = mesh.getNodeTime();
    const char* FromID = doc1["node"];
    double hum = doc1["hum"];
    double pres = doc1["pres"];
    double temp = doc1["temp"];
    double bat = doc1["batt"];
    // Serial.println("");
    //double RecievedTime = doc1["Ntime"];
    // Serial.print("This is the FromID that you are looking for: ");
    // Serial.println(FromID);
    // taskSendMessage(FromID);  // send confirmation message ASAP.
    // get the data from the incoming message.
    DynamicJsonDocument doc(512);  // This is our master document.
    //JsonObject node11 = doc.createNestedObject("n11");
    doc["node"] = FromID;
    doc["t"] = temp;
    doc["hm"] = hum;
    doc["p"] = pres;
    ["bt"] = bat;
  */
  } else {
    Serial.println("Message from normal node");
  }

  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  Serial.println(" ");
  Serial.print("this is the message send to the HA :");
  int stringLength = msg.length();  // message send as is.
  byte myStringByteArray[stringLength];
  msg.getBytes(myStringByteArray, stringLength + 1);
  SerialPort.write(myStringByteArray, stringLength);  // this sends the recieved message,msg to the UART port fot the MQTT client.
  //serializeJson(doc, SerialPort);

  SerialPort.write('\r');
  //serializeJson(doc, Serial);
  Serial.println(msg);
  Serial.println(" ");
  /*
    DynamicJsonDocument doc2(92); 
    String heapadress =  "CenterNode";
    doc2["node"] =  heapadress;
    double Heap = ESP.getFreeHeap(); 
    doc2["CenterHeap"] = Heap;
    doc["CenterHeap"] = ESP.getFreeHeap(); 
    //doc2["MinCenterHeap"] =ESP.esp_get_minimum_free_heap_size();
    serializeJson(doc2, Serial); 
    serializeJson(doc2, SerialPort);
    SerialPort.write('\r');
    */
    //  Serial.print("Node: ");
    //  Serial.println(FromID);
    //  Serial.print("Recieved time: ");
    //  Serial.print(RecievedTime);
    //  Serial.println("Node time");
    //  // Serial.print(NodeTime);
    //  Serial.print(humt);
    //  Serial.println(" %");
    //  Serial.print("Pressure: ");
    //  Serial.print(prest);
    //  Serial.println(" hpa");
    // taskSendMessage();
    // this is where the json message for the HA is asssabled.
    // serializeJson(doc, SerialPort); // this serializes the message and sends it tot he HA SEP32 node that listens to uart port.
    // SerialPort.write('\r');
    // Serial.println("this is the message send to the HA");
    // serializeJson(doc, Serial);
    //}
    //}
}

void setup() {
  Serial.begin(115200);
  SerialPort.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("This is the node that gets message via MESH and sends it to the UART. and vice versa");
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages
  pinMode(LED_PIN, OUTPUT);                           // FOR TESTING THE DATA FROM THE UART
  
  userScheduler.init();
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
 
  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);     //, &userScheduler
  // Bridge node, should (in most cases) be a root node. See [the
  // wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation)
  // for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root,
  // so call this on all nodes
  //mesh.setContainsRoot(true);
  mesh.setName(nodeName);  // This needs to be an unique name!


  mesh.onReceive(&receivedCallback);
  mesh.initOTAReceive(nodeName);
  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
    String nodetopo = mesh.subConnectionJson();
    Serial.println(nodetopo);
  });
 // if you want your node to accept OTA firmware, simply include this line
  // with whatever role you want your hardware to be. For instance, a
  // mesh network may have a thermometer, rain detector, and bridge. Each of
  // those may require different firmware, so different roles are preferrable.
  //
  // MAKE SURE YOUR UPLOADED OTA FIRMWARE INCLUDES OTA SUPPORT OR YOU WILL LOSE
  // THE ABILITY TO UPLOAD MORE FIRMWARE OVER OTA. YOU ALSO WANT TO MAKE SURE
  // THE ROLES ARE CORRECT
 mesh.initOTAReceive(nodeName);

}

void loop() {
  // it will run the user scheduler as well
  //taskSendMessage.execute();
  mesh.update();
  // This takes incpoming
  while (SerialPort.available()) {
    String teststr = SerialPort.readString();
    //teststr.trim();
    Serial.print("incoming message :");
    Serial.println(teststr);
    DynamicJsonDocument doc0(256);  // get the data from the incoming message.
    deserializeJson(doc0, teststr);
    String FromID = doc0["node"];
    int status = doc0["OnOff"];
    Serial.println(FromID);
    Serial.println(status);
    String str;
    DynamicJsonDocument doc1(256);
    doc1["OnOff"] = status;
    serializeJson(doc1, str);
    mesh.sendSingle(FromID, str);
    if (status == 1) {
      Serial.println("StartUp");
      digitalWrite(LED_PIN, HIGH);
    } else if (status == 0) {
      Serial.println("ShutDown");
      digitalWrite(LED_PIN, LOW);
    }
    
  }
}