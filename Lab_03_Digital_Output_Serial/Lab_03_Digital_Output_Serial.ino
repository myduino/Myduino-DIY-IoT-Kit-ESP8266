int i = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print("Time: ");
  Serial.print(i);
  Serial.println(" sec.");
  
  delay(1000);
  i = i + 1;
}