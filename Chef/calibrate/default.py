#!/usr/bin/env python
#

from chef import symbex

def calibrate():
    x = 0
    y = 1
    symbex.calibrate(1, 0)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)
    x = x + y
    y = x + y
    x = x * 2
    y = y / 2
    symbex.calibrate(1, 5)


def main():
    symbex.calibrate(0)
    calibrate()
    symbex.calibrate(2)


if __name__ == "__main__":
    main()
