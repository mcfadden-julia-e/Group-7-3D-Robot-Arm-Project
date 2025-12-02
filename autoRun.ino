#include <Servo.h>

#define NUM_SERVOS 6

// Assign pins for servos
int servoPins[NUM_SERVOS] = {4, 5, 6, 7, 8, 9};
Servo servos[NUM_SERVOS];
int currentPos[NUM_SERVOS] = {90, 70, 145, 90, 60, 145}; // start at mid-position
int targetPos[NUM_SERVOS]  = {90, 90, 90, 90, 90, 90};

int POS_HOME[6] = {90, 70, 60, 90, 60, 70};
int POS_LEFT_REACH[6] = {160, 70, 60, 90, 60, 145};
int POS_LEFT_REACH_2[6] = {160, 70, 60, 90, 60, 70};
int POS_LEFT_PICK[6] = {160, 150, 80, 100, 10, 145};
int POS_RIGHT_REACH[6] = {45, 70, 60, 90, 60, 70};
int POS_RIGHT_DROP[6] = {45, 70, 60, 90, 60, 145};
int GRIP_OPEN = 145;
int GRIP_CLOSE = 70;

bool autoRun = false;
unsigned long autoStartTime = 0;
unsigned long autoRunLimit = 60000;

//right angle position = 90 70 60 90 60 100
void setup() {
  Serial.begin(115200);
  Serial.println("6-DOF Robotic Arm Controller Ready(AutoRun)");
  
  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(currentPos[i]);
  }
  reportPositions();
}

void loop() {
  // Check if new command is available
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.equalsIgnoreCase("START")){
      autoRun = true;
      autoStartTime = millis();
      Serial.println("Auto-Run ENABLED");
    }
    else if (input.equalsIgnoreCase("STOP")){
      autoRun = false;
      Serial.println("Auto-Run Disabled");
    }
    else{
    parseCommand(input);
    moveSmooth();
    reportPositions();
    }
  }
  if(autoRun && millis() - autoStartTime >= autoRunLimit){
    autoRun = false;
    Serial.println("Auto-Run stopped. 1 minute limit reached.");
  }
  if(autoRun) {
    autoPickupSequence();
  }
}

void autoPickupSequence(){
  if(!autoRun) return;

  //pickup on left
  setTargets(POS_LEFT_REACH);
  moveSmooth();
  if(!autoRun) return;

  setTargets(POS_LEFT_PICK);
  moveSmooth();
  if(!autoRun) return;

  //grab duck
  targetPos[5] = GRIP_CLOSE;
  moveSmooth();
  if(!autoRun) return;

  //pickup
  setTargets(POS_LEFT_REACH_2);
  moveSmooth();
  if(!autoRun) return;

  //move over box
  setTargets(POS_RIGHT_REACH);
  moveSmooth();
  if (!autoRun) return;

  setTargets(POS_RIGHT_DROP);
  moveSmooth();
  if (!autoRun) return;

  //drop duck
  targetPos[5] = GRIP_OPEN;
  moveSmooth();
  if(!autoRun) return;

  /*center
  setTargets(POS_HOME);
  moveSmooth();
  delay(50);*/
}

void setTargets(int p[6]) {
  for(int i = 0; i < NUM_SERVOS; i++){
    targetPos[i] = p[i];
  }
}

void parseCommand(String input) {
  input.replace(',', ' ');  // allow comma or space separated values
  int idx = 0;
  while (input.length() > 0 && idx < NUM_SERVOS) {
    int spaceIndex = input.indexOf(' ');
    String value = (spaceIndex == -1) ? input : input.substring(0, spaceIndex);
    value.trim();
    if (value.length() > 0) {
      targetPos[idx] = constrain(value.toInt(), 0, 180);
      idx++;
    }
    if (spaceIndex == -1) break;
    input = input.substring(spaceIndex + 1);
  }
}

void moveSmooth() {
  bool moving = true;
  while (moving) {
    moving = false;
    for (int i = 0; i < NUM_SERVOS; i++) {
      if (currentPos[i] != targetPos[i]) {
        moving = true;
        if (currentPos[i] < targetPos[i]) currentPos[i]++;
        else currentPos[i]--;
        servos[i].write(currentPos[i]);
      }
    }
    delay(10); // controls smoothness (smaller = smoother but slower)
  }
}

void reportPositions() {
  Serial.print("Current positions: ");
  for (int i = 0; i < NUM_SERVOS; i++) {
    Serial.print(currentPos[i]);
    if (i < NUM_SERVOS - 1) Serial.print(' ');
  }
  Serial.println();
}

