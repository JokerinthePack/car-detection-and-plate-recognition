void setup() {
  size(200, 200);
}

void draw() { 
  // draw() must be present for mousePressed() to work
}

void mousePressed() {
  PrintWriter output=null;
output = createWriter("myfile.bat");
output.println("cd "+sketchPath("C:\\Users\\Ozan\\Documents\\Processing\\PlateRecognition\\x64\\Release"));
output.println("start  PlateRecognition.exe");
output.flush();
output.close();  
output=null;
launch(sketchPath("C:\\Users\\Ozan\\Documents\\Processing\\PlateRecognition\\x64\\Release")+"myfile.bat");
}
