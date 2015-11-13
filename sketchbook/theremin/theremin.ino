
int speakerPin = 12;
int photoCell = 0;

void setup() {

}

void loop() {
  int reading = analogRead(photoCell);
  int pitch = reading + 500;
  tone(speakerPin, pitch);
}
