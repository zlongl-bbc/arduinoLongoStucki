#include <Wire.h>
#include <Zumo32U4.h>
#include "GridMovement.h"

Zumo32U4LCD lcd;
Zumo32U4Buzzer buzzer;
Zumo32U4ButtonA buttonA;
Zumo32U4Motors motors;
Zumo32U4LineSensors lineSensors;
L3G gyro;

char path[100];
uint8_t pathLength = 0;

void setup()
{

  buzzer.playFromProgramSpace(PSTR("!>g32>>c32"));

  gridMovementSetup();
  mazeSolve();
}

void loop()
{
  buttonA.waitForButton();
  mazeFollowPath();
}

char selectTurn(bool foundLeft, bool foundStraight, bool foundRight)
{
  if(foundLeft) { return 'L'; }
  else if(foundStraight) { return 'S'; }
  else if(foundRight) { return 'R'; }
  else { return 'B'; }
}

void mazeSolve()
{
  pathLength = 0;

  buzzer.playFromProgramSpace(PSTR("!L16 cdegreg4"));

  delay(1000);

  while(1)
  {
    followSegment();

    bool foundLeft, foundStraight, foundRight;
    driveToIntersectionCenter(&foundLeft, &foundStraight, &foundRight);

    if(aboveDarkSpot())
    {
      break;
    }

    // Choose a direction to turn.
    char dir = selectTurn(foundLeft, foundStraight, foundRight);

    if (pathLength >= sizeof(path))
    {
      lcd.clear();
      lcd.print(F("pathfull"));
      while(1)
      {
        ledRed(1);
      }
    }

    path[pathLength] = dir;
    pathLength++;

    simplifyPath();

    displayPath();

    if (pathLength == 2 && path[0] == 'B' && path[1] == 'B')
    {
      buzzer.playFromProgramSpace(PSTR("!<b4"));
    }

    turn(dir);
  }

  motors.setSpeeds(0, 0);
  buzzer.playFromProgramSpace(PSTR("!>>a32"));
  lcd.clear();
  lcd.print("DONE!");
}

void mazeFollowPath()
{
  buzzer.playFromProgramSpace(PSTR("!>c32"));

  delay(1000);

  for(uint16_t i = 0; i < pathLength; i++)
  {
    followSegment();
    driveToIntersectionCenter();

    turn(path[i]);
  }

  followSegment();

  motors.setSpeeds(0, 0);
  buzzer.playFromProgramSpace(PSTR("!>>a32"));
}

void simplifyPath()
{
  if(pathLength < 3 || path[pathLength - 2] != 'B')
  {
    return;
  }

  int16_t totalAngle = 0;

  for(uint8_t i = 1; i <= 3; i++)
  {
    switch(path[pathLength - i])
    {
    case 'L':
      totalAngle += 90;
      break;
    case 'R':
      totalAngle -= 90;
      break;
    case 'B':
      totalAngle += 180;
      break;
    }
  }

  totalAngle = totalAngle % 360;

  switch(totalAngle)
  {
  case 0:
    path[pathLength - 3] = 'S';
    break;
  case 90:
    path[pathLength - 3] = 'L';
    break;
  case 180:
    path[pathLength - 3] = 'B';
    break;
  case 270:
    path[pathLength - 3] = 'R';
    break;
  }

  pathLength -= 2;
}

void displayPath()
{
  path[pathLength] = 0;

  lcd.clear();
  lcd.print(path);
  if(pathLength > 8)
  {
    lcd.gotoXY(0, 1);
    lcd.print(path + 8);
  }
}
