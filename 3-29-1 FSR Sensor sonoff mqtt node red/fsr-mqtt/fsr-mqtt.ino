#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
Ticker ticker;

// 아래의 6개설정은 사용자 환경에 맞게 수정하세요.
const char* ssid = "i2r"; // 와이파이 AP, 또는 스마트폰의 핫스판 이름
const char* password = "00000000";  // 와이파이 AP, 또는 스마트폰의 핫스판 이름
const char* mqtt_server = "192.168.0.200"; //브로커 주소
const char* outTopic = "/kdi/outTopic"; // 이름이 중복되지 않게 설정 기록
const char* inTopic = "/kdi/inTopic"; // 이름이 중복되지 않게 설정 기록
const char* clientName = "";  // setup 함수에서 자동생성
char mac[20];  //mac address
String sMac;

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];

int fsrPin = A0;     // the FSR and 10K pulldown are connected to a0
int fsrReading;     // the analog reading from the FSR resistor divider

void tick()
{
  if(!client.connected())
    return;
  fsrReading = analogRead(fsrPin);  
  snprintf (msg, 100, "{\"mac\":\"%s\",\"fsr\":%d}", mac,fsrReading);
  client.publish(outTopic, msg);
  Serial.println(msg);
}

void setup() {
  Serial.begin(9600);
  setup_wifi();

  //이름 자동으로 생성
  byte hardwareMAC[12];  //WIFI MAC address
  WiFi.macAddress(hardwareMAC);
  sprintf(mac,"%02x%02x%02x%02x%02x%02x",hardwareMAC[5], hardwareMAC[4], hardwareMAC[3], hardwareMAC[2], hardwareMAC[1], hardwareMAC[0]);  
  clientName=&mac[0];
  sMac=mac;
  
  ticker.attach(2, tick);  //0.1 초도 가능
  //ticker.detach();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// 통신에서 문자가 들어오면 이 함수의 payload 배열에 저장된다.
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// mqtt 통신에 지속적으로 접속한다.
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientName)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, "Reconnected");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
//  Serial.println("Loop...");

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
