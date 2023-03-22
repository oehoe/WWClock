//set clock EN
void setClockState(const int targetPos[rows], const int setColor[4]) {
  for (int a = 0; a < rows; a++) {
    for (int b = 0; b < cols; b++) {
      int y = (targetPos[a] >> (cols - 1) - b) & 1;
      if (y == 1) {
        state[cs][a][b][0] = setColor[0];
        state[cs][a][b][1] = setColor[1];
        state[cs][a][b][2] = setColor[2];
        state[cs][a][b][3] = setColor[3];
      }
    }
  }
}

void setClock() {
  if (minute > 29) hour = hour + 1;
  if (hour == 0) hour = 12;
  if (hour > 12) hour = hour - 12;
  setClockState(hours[hour - 1],colors[3]);

  if (minute < 5) {
    setClockState(five,colors[0]);
    setClockState(past,colors[1]);
  } else if (minute < 10) {
    setClockState(ten,colors[0]);
    setClockState(past,colors[1]);
  } else if (minute < 15) {
    setClockState(quarter,colors[0]);
    setClockState(past,colors[1]);
  } else if (minute < 20) {
    setClockState(twenty,colors[0]);
    setClockState(past,colors[1]);
  } else if (minute < 25) {
    setClockState(twenty,colors[0]);
    setClockState(five,colors[0]);
    setClockState(past,colors[1]);
  } else if (minute < 30) {
    setClockState(half,colors[2]);
    setClockState(past,colors[1]);
  } else if (minute < 35) {
    setClockState(twenty,colors[0]);
    setClockState(five,colors[0]);
    setClockState(to,colors[1]);
  } else if (minute < 40) {
    setClockState(twenty,colors[0]);
    setClockState(to,colors[1]);
  } else if (minute < 45) {
    setClockState(quarter,colors[0]);
    setClockState(to,colors[1]);
  } else if (minute < 50) {
    setClockState(ten,colors[0]);
    setClockState(to,colors[1]);
  } else if (minute < 55) {
    setClockState(five,colors[0]);
    setClockState(to,colors[1]);
  } else {
    setClockState(oclock,colors[4]);
  }
}