#!/usr/bin/env python
#
# Copyright 2015 EPFL. All rights reserved.


"""Interpreter detector calibration."""


__author__ = "stefan.bucur@epfl.ch (Stefan Bucur)"


from chef import symbex


class CalibrationOp(object):
    START = 0
    CHECKPOINT = 1
    END = 2


def _sub_calibrate():
    x = 0
    y = 1
    symbex.calibrate(CalibrationOp.CHECKPOINT, 0)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)
    x += y
    y += y
    x *= 2
    y /= 2
    symbex.calibrate(CalibrationOp.CHECKPOINT, 5)


def perform_calibration():
    symbex.calibrate(CalibrationOp.START)
    _sub_calibrate()
    symbex.calibrate(CalibrationOp.END)
