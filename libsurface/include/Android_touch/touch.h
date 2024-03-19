#ifndef NATIVESURFACE_TOUCH_H
#define NATIVESURFACE_TOUCH_H

void initTouch(int eventX);

void initTouch();

void closeTouch();

void touchDown(long x, long y, long finger);

void touchMove(long x, long y, long finger);

void touchUp(long finger);

#endif //NATIVESURFACE_TOUCH_H