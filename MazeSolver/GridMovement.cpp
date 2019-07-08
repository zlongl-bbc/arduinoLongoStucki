#include "TurnSensor.h"
#include "GridMovement.h"

uint16_t lineSensorValues[numSensors];

static void loadCustomCharacters()
{
  static const char levels[] PROGMEM = {
    0, 0, 0, 0, 0, 0, 0, 63, 63, 63, 63, 63, 63, 63
  };
  lcd.loadCustomCharacter(levels + 0, 0);  // 1 bar
  lcd.loadCustomCharacter(levels + 1, 1);  // 2 bars
  lcd.loadCustomCharacter(levels + 2, 2);  // 3 bars
  lcd.loadCustomCharacter(levels + 3, 3);  // 4 bars
  lcd.loadCustomCharacter(levels + 4, 4);  // 5 bars
  lcd.loadCustomCharacter(levels + 5, 5);  // 6 bars
  lcd.loadCustomCharacter(levels + 6, 6);  // 7 bars
}

void printBar(uint8_t height)
{
  if (height > 8) { height = 8; }
  const char barChars[] = {' ', 0, 1, 2, 3, 4, 5, 6, 255};
  lcd.print(barChars[height]);
}

uint16_t readSensors()
{
  return lineSensors.readLine(lineSensorValues);
}

bool aboveLine(uint8_t sensorIndex)
{
  return lineSensorValues[sensorIndex] > sensorThreshold;
}

bool aboveLineDark(uint8_t sensorIndex)
{
  return lineSensorValues[sensorIndex] > sensorThresholdDark;
}

bool aboveDarkSpot()
{
  return aboveLineDark(1) && aboveLineDark(2) && aboveLineDark(3);
}

static void lineSensorSetup()
{
  lcd.clear();
  lcd.print(F("Line cal"));

  delay(1000);

  turnSensorReset();

  motors.setSpeeds(-calibrationSpeed, calibrationSpeed);
  while((int32_t)turnAngle < turnAngle45 * 2)
  {
    lineSensors.calibrate();
    turnSensorUpdate();
  }

  motors.setSpeeds(calibrationSpeed, -calibrationSpeed);
  while((int32_t)turnAngle > -turnAngle45 * 2)
  {
    lineSensors.calibrate();
    turnSensorUpdate();
  }

  motors.setSpeeds(-calibrationSpeed, calibrationSpeed);
  while((int32_t)turnAngle < 0)
  {
    lineSensors.calibrate();
    turnSensorUpdate();
  }

  motors.setSpeeds(0, 0);

  lcd.clear();
  while(!buttonA.getSingleDebouncedPress())
  {
    readSensors();

    lcd.gotoXY(0, 0);
    for (uint8_t i = 0; i < numSensors; i++)
    {
      uint8_t barHeight = map(lineSensorValues[i], 0, 1000, 0, 8);
      printBar(barHeight);
    }
  }

  lcd.clear();
}

void turn(char dir)
{
  if (dir == 'S')
  {
    return;
  }

  turnSensorReset();

  uint8_t sensorIndex;

  switch(dir)
  {
  case 'B':
    motors.setSpeeds(-turnSpeed, turnSpeed);
    while((int32_t)turnAngle < turnAngle45 * 3)
    {
      turnSensorUpdate();
    }
    sensorIndex = 1;
    break;

  case 'L':
    motors.setSpeeds(-turnSpeed, turnSpeed);
    while((int32_t)turnAngle < turnAngle45)
    {
      turnSensorUpdate();
    }
    sensorIndex = 1;
    break;

  case 'R':
    motors.setSpeeds(turnSpeed, -turnSpeed);
    while((int32_t)turnAngle > -turnAngle45)
    {
      turnSensorUpdate();
    }
    sensorIndex = 3;
    break;

  default:
    return;
  }

  while(1)
  {
    readSensors();
    if (aboveLine(sensorIndex))
    {
      break;
    }
  }
}

void followSegment()
{
  while(1)
  {
    uint16_t position = readSensors();

    int16_t error = (int16_t)position - 2000;

    int16_t speedDifference = error / 4;

    int16_t leftSpeed = (int16_t)straightSpeed + speedDifference;
    int16_t rightSpeed = (int16_t)straightSpeed - speedDifference;

    leftSpeed = constrain(leftSpeed, 0, (int16_t)straightSpeed);
    rightSpeed = constrain(rightSpeed, 0, (int16_t)straightSpeed);

    motors.setSpeeds(leftSpeed, rightSpeed);

    if(!aboveLine(0) && !aboveLine(1) && !aboveLine(2) && !aboveLine(3) && !aboveLine(4))
    {
      break;
    }

    if(aboveLine(0) || aboveLine(4))
    {
      break;
    }
  }
}

void driveToIntersectionCenter()
{
  motors.setSpeeds(straightSpeed, straightSpeed);
  delay(intersectionDelay);
}

void driveToIntersectionCenter(bool * foundLeft, bool * foundStraight, bool * foundRight)
{
  *foundLeft = 0;
  *foundStraight = 0;
  *foundRight = 0;

  motors.setSpeeds(straightSpeed, straightSpeed);
  for(uint16_t i = 0; i < intersectionDelay / 2; i++)
  {
    readSensors();
    if(aboveLine(0))
    {
      *foundLeft = 1;
    }
    if(aboveLine(4))
    {
      *foundRight = 1;
    }
  }

  readSensors();

  if(aboveLine(1) || aboveLine(2) || aboveLine(3))
  {
    *foundStraight = 1;
  }
}

void gridMovementSetup()
{
  lineSensors.initFiveSensors();

  loadCustomCharacters();
  turnSensorSetup();
  lineSensorSetup();
}
