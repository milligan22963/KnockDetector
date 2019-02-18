
int g_spacing = 250; // 250ms per period
int g_delta = 50; // +- 50 ms
byte g_knockArray[] = { 2, 1, 2}; //, 1, 1, 1 };
byte g_knockPosition = 0; // start at the beginning
byte g_maxKnocks;

unsigned long g_sequenceStartTime = 0;
unsigned long g_nextTime = 0;
unsigned long g_maxSequenceTime = 0;

const byte g_magneticOutput = 4;
const byte g_knockInput = 3;
const byte g_hit = 13;
const byte g_sound = 8;
bool g_sequenceStarted = false;
volatile bool g_buttonPressed = false;

void ButtonPress() {
  g_buttonPressed = true;
}

void setup() {
  pinMode(g_knockInput, INPUT_PULLUP);
  pinMode(g_magneticOutput, OUTPUT);
  pinMode(g_hit, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(g_knockInput), ButtonPress, RISING);
  
  g_maxKnocks = sizeof(g_knockArray);
  g_maxSequenceTime = 0;
  
  // max time for a sequence to be detected
  for (byte index = 0; index < g_maxKnocks; index++) {
    g_maxSequenceTime += g_knockArray[index] * g_spacing;
  }
  g_maxSequenceTime += g_spacing; // pad a little for slop

  Serial.begin(115200);

  Serial.print("Max Sequence Time: ");
  Serial.println(g_maxSequenceTime);
  Serial.print("Max Knocks: ");
  Serial.println(g_maxKnocks);
}

void InvalidSequence()
{
  g_knockPosition = 0;
  
  Serial.println("Invalid sequence");

  tone(g_sound, 261);
  delay(750);
  tone(g_sound, 151);
  delay(1250);

  noTone(g_sound);

  g_buttonPressed = false; // effectively ignore anything while making sound

  Serial.println("Ready.");
}

void loop() {
  unsigned long currentTime = millis();

  if (g_buttonPressed == true) {
    // first time in, we need to start timing
    if (g_knockPosition == 0) {
      Serial.println("Start position");
      g_sequenceStarted = true;
      g_sequenceStartTime = currentTime;
      g_nextTime = (g_knockArray[g_knockPosition] * g_spacing) + currentTime; // we we expect to be
      digitalWrite(g_hit, HIGH);
      Serial.print("Start Time: ");
      Serial.print(g_sequenceStartTime);
      Serial.print(" Next Time: ");
      Serial.println(g_nextTime);
      g_knockPosition++;
    } else if ((g_nextTime >= currentTime - g_delta) && (g_nextTime <= currentTime + g_delta)) {
      Serial.println("Match time");
      g_knockPosition++;
      if (g_knockPosition == g_maxKnocks) {
        Serial.println("Door open!!!!!!!!!!!!!!!");
        g_sequenceStarted = false;
        digitalWrite(g_magneticOutput, HIGH);
        // reset?
      } else {
        Serial.print("Next time: ");
        g_nextTime = (g_knockArray[g_knockPosition] * g_spacing) + currentTime;
        Serial.println(g_nextTime);
      }
    } else if (g_sequenceStarted == true) {
      Serial.print("Out of band: ");
      Serial.print(g_nextTime);
      Serial.print(", ");
      Serial.println(currentTime);
      InvalidSequence();
    }
    g_buttonPressed = false; // reset
  } else if ((g_knockPosition > 0) && (currentTime > g_sequenceStartTime + g_maxSequenceTime) && (g_sequenceStarted == true)) {
    Serial.print("No Press: ");
    Serial.print(currentTime);
    Serial.print(", ");
    Serial.print(g_sequenceStartTime);
    Serial.print(", ");
    Serial.println(g_maxSequenceTime);
    digitalWrite(g_hit, LOW);
    InvalidSequence();
  }
}
