// Clock.h
#pragma once

class Clock {
public:
    virtual void setClock(int hours, int minutes) = 0;
    virtual void setWord(const int x, const bool clear) = 0;
};