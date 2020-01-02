// Servo kütüphanesi
#include <Servo.h> 
// Ultrasonik sensörün trigger ve echo pinleri
const int trigPin = 10;
const int echoPin = 9;
//buzzer ve led pinleri
const int buzzer1 = 5;
const int buzzer2 = 4;
const int ledPin3 = 13;
const int ledPin2 = 7;
const int ledPin1 = 6;

// duration ve distance değişkenleri
long duration;
int distance;
int buzzerState1 = LOW;             
int buzzerState2 = LOW;


unsigned long previousMillis1 = 0;        // buzzerların en son ne zaman güncellendiği kaydedilir
unsigned long previousMillis2 = 0;        


// buzzer zaman aralıkları:
long onTime1 = 1000;           
long offTime1 = 500;           
long onTime2 = 500;           
long offTime2 = 250;

Servo myServo; // servo kontrolü için bir servo objesi yaratıldı
void setup() {
  pinMode(trigPin, OUTPUT); // trigPin çıkış olarak tanımlandı
  pinMode(echoPin, INPUT); // echoPin giriş olarak tanımlandı
  pinMode(ledPin1, OUTPUT); // led pini çıkış olarak tanımlandı
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(buzzer1, OUTPUT);
  pinMode(buzzer2, OUTPUT);
  Serial.begin(9600);
  myServo.attach(12); // Servonun bağlı olduğu pin
}
void loop() {
  // servo motoru 60 - 120 derecelik açılar arası döndür
  for(int i=60;i<=120;i++){  
  myServo.write(i);
  delay(20);
  distance = calculateDistance();// her derece için ultrasonik sensörün ölçtüğü mesafeyi hesaplayan fonksiyon
  
  Serial.print(i); // mevcut açıyı serial porta yolla
  Serial.print(","); // processing de değerler arasına konulacak karakter
  Serial.print(distance); // mesafe değerini serial porta yolla
  Serial.print("."); // processing de değerler arasına konulacak karakter

  
  // Sesli uyarılar
  if (distance<10)
  buzzer10();

  else if (distance<20)
  buzzer20();

  else
  buzzersOff();
  
  

  //Işıklı Uyarılar
  if (distance<10)
  redLight();

  else if (distance<20)
  greenLight();

  else if (distance<30)
  blueLight();

  else
  lightsOff();
  }
  
  // aynı işlemleri geri dönerken de yap
  for(int i=120;i>60;i--){  
  myServo.write(i);
  delay(20);
  distance = calculateDistance();
  Serial.print(i);
  Serial.print(",");
  Serial.print(distance);
  Serial.print(".");

  // Sesli uyarılar
  if (distance<10)
  buzzer10();

  else if (distance<20)
  buzzer20();

  else
  buzzersOff();

  //Işıklı Uyarılar
  if (distance<10)
  redLight();

  else if (distance<20)
  greenLight();

  else if (distance<30)
  blueLight();

  else
  lightsOff();

  
  
  }
}



// ultrasonik sensörün ölçtüğü mesafeyi hesaplayan fonksiyon
int calculateDistance(){ 
  
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
  // trigPin i 10 ms için HIGH yap
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH); // echoPin den değer oku,ses dalgasının yol alma süresini ms e çevir
  distance= duration*0.034/2;
  return distance;
}

  //Işıklı uyarı fonksiyonları
  int redLight(){
  
    digitalWrite (ledPin1, HIGH);
    digitalWrite (ledPin2, LOW);
    digitalWrite (ledPin3, LOW);
  }

  int greenLight(){
    
    digitalWrite (ledPin2, HIGH);
    digitalWrite (ledPin1, LOW);
    digitalWrite (ledPin3, LOW);
  }
  
  int blueLight(){

    digitalWrite (ledPin3, HIGH);
    digitalWrite (ledPin1, LOW);
    digitalWrite (ledPin2, LOW);
   }

  int lightsOff(){
    
  digitalWrite (ledPin1, LOW);
  digitalWrite (ledPin2, LOW);
  digitalWrite (ledPin3, LOW);
  }   
    
  
  //Sesli uyarı fonksiyonları
  int buzzer20(){
    unsigned long currentMillis = millis();
    analogWrite(buzzer1,130);

  if ((buzzerState2 = HIGH) && (currentMillis - previousMillis2 >= onTime2)) {
    buzzerState2 = LOW;
    previousMillis2 = currentMillis;
    digitalWrite(buzzer2, buzzerState2);
 }  
    
    
    else if ((buzzerState2 = LOW) && (currentMillis - previousMillis2 >= offTime2)) {
      buzzerState2 = HIGH;
      previousMillis2 = currentMillis;
      digitalWrite(buzzer2, buzzerState2);
  }
  }

  int buzzer10(){
  unsigned long currentMillis = millis();
  analogWrite(buzzer2,255);
  if ((buzzerState1 = HIGH) && (currentMillis - previousMillis1 >= onTime1)) {
    buzzerState1 = LOW;
    previousMillis1 = currentMillis;
    digitalWrite(buzzer1, buzzerState1);
 } 
    
    
  else if ((buzzerState1 = LOW) && (currentMillis - previousMillis1 >= offTime1)) {
      buzzerState1 = HIGH;
      previousMillis1 = currentMillis;
      digitalWrite(buzzer1, buzzerState1);
  } 
    }

 int buzzersOff(){

    analogWrite(buzzer1,255);
    analogWrite(buzzer2,255);
  }   
