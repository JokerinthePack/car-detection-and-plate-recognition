import processing.serial.*; // seri haberleşme kütüphanesi
import java.awt.event.KeyEvent; // seri porttan veri okuma kütüphanesi
import java.io.IOException;  // input/output kütüphanesi
import processing.video.*;  //video kamera kütüphanesi
Serial myPort; // seri obje tanımı
// değişken tanımlamaları
String angle="";
String distance="";
String data="";
String noObject;
float pixsDistance;
int iAngle, iDistance;
int index1=0;
int index2=0;
int karekonum=500;
boolean within;
boolean done;

Capture myCapture;

void setup() {
  
 size (640, 480); // ekran çözünürlüğü
 smooth();
 myPort = new Serial(this,"COM3", 9600); // seri haberleşmeye başlanır
 myPort.bufferUntil('.'); // Seri porttan '.' karakterine kadar değer okunur. örneğin: angle,distance.
 myCapture = new Capture(this, width,height, 30);
 myCapture.start();
}

void captureEvent(Capture myCapture) {
myCapture.read();
}


void draw() {
  
  fill(98,245,31);
  
  // hareket çizgisinin hareketleri belirlenir
  noStroke();
  fill(0,4); 
  rect(0, 0, width, height-height*0.065); 
  
  fill(98,245,31); // yeşil renk
  // radarı çizmek için fonksiyonlar çağırılır
  drawRadar(); 
  drawLine();
  drawObject();
  drawText();
  drawWarning();
}
void serialEvent (Serial myPort) { // seri porttan veri okunur
  // Seri porttan '.' karakterine kadar değer okunur ve "data" değişkenine atanır.
  data = myPort.readStringUntil('.');
  data = data.substring(0,data.length()-1);
  
  index1 = data.indexOf(","); //  ',' karakteri bulunur ve "index1" değişkenine koyulur
  angle= data.substring(0, index1); // "0" konumundan index1 in konumuna kadar veri okunur ve açı değeri olarak seri porta yollanır
  distance= data.substring(index1+1, data.length()); //  "index1" in konumundan "data" nın konumuna kadar veri okunur ve mesafe olarak seri porta yollanır
  
  // string değerleri integer a çevirilir
  iAngle = int(angle);
  iDistance = int(distance);
}
void drawRadar() {
  pushMatrix();
  translate(width/2,height-height*0.074); // başlangıç koordinatları yeni konuma taşınır
  noFill();
  strokeWeight(2);
  stroke(98,245,31);
  // yay çizgilerinin çizilme işlemleri
  arc(0,0,(width-width*0.0625),(width-width*0.0625),PI,TWO_PI);
  arc(0,0,(width-width*0.27),(width-width*0.27),PI,TWO_PI);
  arc(0,0,(width-width*0.479),(width-width*0.479),PI,TWO_PI);
  arc(0,0,(width-width*0.687),(width-width*0.687),PI,TWO_PI);
  // açı çizgileri çizilir
  line(-width/2,0,width/2,0);
  line(0,0,(-width/2)*cos(radians(30)),(-width/2)*sin(radians(30)));
  line(0,0,(-width/2)*cos(radians(60)),(-width/2)*sin(radians(60)));
  line(0,0,(-width/2)*cos(radians(90)),(-width/2)*sin(radians(90)));
  line(0,0,(-width/2)*cos(radians(120)),(-width/2)*sin(radians(120)));
  line(0,0,(-width/2)*cos(radians(150)),(-width/2)*sin(radians(150)));
  line((-width/2)*cos(radians(30)),0,width/2,0);
  popMatrix();
}
void drawObject() {
  pushMatrix();
  translate(width/2,height-height*0.074); // başlangıç koordinatları yeni konuma taşınır
  strokeWeight(9);
  stroke(255,10,10); // kırmızı renk
  pixsDistance = iDistance*((height-height*0.1666)*0.025); // sensörden alınan mesafe değeri cm den piksele çevirilir
  // range 40 cm olarak belirlenir
  if(iDistance<40){
    // açı ve mesafeye göre obje çizilir
  line(pixsDistance*cos(radians(iAngle)),-pixsDistance*sin(radians(iAngle)),(width-width*0.505)*cos(radians(iAngle)),-(width-width*0.505)*sin(radians(iAngle)));
  }
  popMatrix();
}
void drawLine() {
  pushMatrix();
  strokeWeight(9);
  stroke(30,250,60);
  translate(width/2,height-height*0.074); // başlangıç koordinatları yeni konuma taşınır
  line(0,0,(height-height*0.12)*cos(radians(iAngle)),-(height-height*0.12)*sin(radians(iAngle))); // açıya göre çizgi çekilir
  popMatrix();
}
void drawText() { // textler ekrana verilir
  
  pushMatrix(); // Referans 40 cm değerine göre menzil durumunu belirlenir
  if(iDistance>40) {
  noObject = "Menzil Dışı";
  }
  else {
  noObject = "Menzilde";
  }
  fill(0,0,0);
  noStroke();
  rect(0, height-height*0.0648, width, height);
  fill(98,245,31);
  textSize(12);
  
  //Ekranda buluan yazıların konum ve yazım ayarlamaları
  text("10cm",width-width*0.3854,height-height*0.0833);
  text("20cm",width-width*0.281,height-height*0.0833);
  text("30cm",width-width*0.177,height-height*0.0833);
  text("40cm",width-width*0.0729,height-height*0.0833);
  textSize(12);
  text("Araç: " + noObject, width-width*0.875, height-height*0.0277);
  text("Açı: " + iAngle +" °", width-width*0.48, height-height*0.0277);
  text("Mesafe: ", width-width*0.26, height-height*0.0277);
  if(iDistance<40) {
  text("        " + iDistance +" cm", width-width*0.225, height-height*0.0277);
  }
  textSize(12);
  fill(98,245,60);
  translate((width-width*0.4994)+width/2*cos(radians(30)),(height-height*0.0907)-width/2*sin(radians(30)));
  rotate(-radians(-60));
  text("30°",0,0);
  resetMatrix();
  translate((width-width*0.503)+width/2*cos(radians(60)),(height-height*0.0888)-width/2*sin(radians(60)));
  rotate(-radians(-30));
  text("60°",0,0);
  resetMatrix();
  translate((width-width*0.507)+width/2*cos(radians(90)),(height-height*0.0833)-width/2*sin(radians(90)));
  rotate(radians(0));
  text("90°",0,0);
  resetMatrix();
  translate(width-width*0.513+width/2*cos(radians(120)),(height-height*0.07129)-width/2*sin(radians(120)));
  rotate(radians(-30));
  text("120°",0,0);
  resetMatrix();
  translate((width-width*0.5104)+width/2*cos(radians(150)),(height-height*0.0574)-width/2*sin(radians(150)));
  rotate(radians(-60));
  text("150°",0,0);
  popMatrix(); 
}

// Araç 25 cm den yakınsa video kamera açılır, 10 cm den yakın ise fotoğrafı çekilir ve plaka uygulaması çalıştırılır
void drawWarning() {
  if (iDistance<20) {
    if (iDistance<10)
    within=true;
    
    else{
    within=false;
    done=false;
    }
    if( within==true && done==false ){
      done=true;
PImage cp = myCapture.get(0, 0, 640, 480);
cp.save("C:\\Users\\Ozan\\Documents\\Processing\\PlateRecognition\\x64\\Release\\CAR.png"); 
exec(new String[] {"cmd", "/c","start","/w", "C:\\Users\\Ozan\\Documents\\Processing\\PlateRecognition\\x64\\Release\\myfile.bat"});
}

image(myCapture, 0, 0);
background(myCapture);
}
}  
