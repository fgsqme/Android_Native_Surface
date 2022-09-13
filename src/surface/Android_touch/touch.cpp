//
// Created by 泓清 on 2022/8/26.
//
#include "Android_touch/touch.h"


#define FROM_SCREEN 0x0
#define FROM_SOCKET 0x1

#define UNGRAB 0x0
#define GRAB 0x1

#define UP 0x0
#define DOWN 0x1

using namespace std;


int isa_event_device(const struct dirent *dir) {
    return strncmp("event", dir->d_name, 5) == 0;
}

std::string getTouchScreenDevice() {
    struct dirent **namelist;
    int i, ndev;
    ndev = scandir("/dev/input", &namelist, isa_event_device, alphasort);
    if (ndev <= 0) {
        return "";
    }
    for (i = 0; i < ndev; i++) {
        char fname[64];
        int fd = -1;
        unsigned long keybit[NBITS(KEY_CNT)];
        unsigned long propbit[INPUT_PROP_MAX];
        snprintf(fname, sizeof(fname), "%s/%s", "/dev/input", namelist[i]->d_name);
        fd = open(fname, O_RDONLY);
        if (fd < 0) {
            continue;
        }
        memset(keybit, 0, sizeof(keybit));
        memset(propbit, 0, sizeof(propbit));
        ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit);
        ioctl(fd, EVIOCGPROP(INPUT_PROP_MAX), propbit);
        close(fd);
        free(namelist[i]);
        if (test_bit(propbit, INPUT_PROP_DIRECT) &&
            (test_bit(keybit, BTN_TOUCH) || test_bit(keybit, BTN_TOOL_FINGER))) {
            return {fname};
        } else if (test_bit(keybit, BTN_TOUCH) || test_bit(keybit, BTN_TOOL_FINGER)) {
            return {fname};
        }
    }
    return "";
}

ImVec2 imVec2;

ImVec2 rotatePointx(uint32_t orientation, ImVec2 mxy, ImVec2 wh) {
    if (orientation == 0) {
        return mxy;
    }
    ImVec2 xy(mxy.x, mxy.y);
    if (orientation == 3) {
        xy.x = (float) wh.y - imVec2.y;
        xy.y = imVec2.x;
    } else if (orientation == 2) {
        xy.x = (float) wh.x - imVec2.x;
        xy.y = (float) wh.y - imVec2.y;
    } else if (orientation == 1) {
        xy.x = imVec2.y;
        xy.y = (float) wh.x - imVec2.x;
    }
    return xy;
}

ImVec2 getTouchScreenDimension(int fd) {
    int abs_x[6], abs_y[6] = {0};
    ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), abs_x);
    ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), abs_y);
    return {(float) abs_x[2], (float) abs_y[2]};
}


void touch_config() {
//    initVirtualTouch();

    std::string device = getTouchScreenDevice();
    // printf("touch event : %s\n",device.c_str());
    if (device.length() < 2) {
        printf("No Touch Event\n");
    }
    int touch_device_fd = open(device.c_str(), O_RDWR | O_SYNC | O_NONBLOCK);
    if (touch_device_fd < 0) {
        printf("Open dev Error\n");
    }

    // 触摸事件队列
    std::vector<input_event> events;
    int eventX = 0, eventY = 0;
    input_event event{};
    ImVec2 touch_screen_size = getTouchScreenDimension(touch_device_fd);
    bool touchFlag = true;
    while (true) {
        if (g_Initialized) {
            if (read(touch_device_fd, &event, sizeof(event)) > 0) {
                if (event.type == EV_SYN && event.code == SYN_REPORT && event.value == 0) {
                    bool flag = false;
                    int status = IM_UP;
                    for (input_event e: events) {
                        switch (e.type) {
                            case EV_KEY: {
                                if (e.code == BTN_TOUCH) {
                                    if (e.value == DOWN) {
                                        flag = true;
                                        status = IM_DOWN;
                                    } else if (e.value == UP) {
                                        flag = true;
                                        status = IM_UP;
                                        break;
                                    }
                                }
                                break;
                            }
                            case EV_ABS: {
                                if (e.code == ABS_MT_POSITION_X) {
                                    eventX = e.value;
                                } else if (e.code == ABS_MT_POSITION_Y) {
                                    eventY = e.value;
                                }
                            }
                        }
                    }

                    if (!flag) {
                        status = IM_MOVE;
                    }
                    events.clear();

                    ImVec2 point = rotatePointx(displayInfo.orientation, {(float) eventX, (float) eventY},
                                                 touch_screen_size);

                    ImVec2 newEvent((point.x * displayInfo.width) / touch_screen_size.x, (point.y * displayInfo.height)
                                                                                         / touch_screen_size.y);
                    ImGuInputEvent imGuInputEvent{};
                    imGuInputEvent.pos = newEvent;
                    if (status == IM_DOWN) {           // 按下
//                        printf("down x:%f y:%f\n", newEvent.x, newEvent.y);
                        imGuInputEvent.type = IM_DOWN;
                    } else if (status == IM_MOVE) {    // 移动
//                        printf("move x:%f y:%f\n", newEvent.x, newEvent.y);
                        imGuInputEvent.type = IM_MOVE;
                    } else {                           // 放开
//                        printf("up\n");
                        imGuInputEvent.type = IM_UP;
                    }
                    ImGui_ImplLinux_HandleInputEvent(imGuInputEvent);
                } else {
                    events.push_back(event);
                }
            }
        }
        std::this_thread::sleep_for(0.0001s);
    }

    pthread_exit(0);
}

void Init_touch_config() { // 初始化触摸设置
    std::thread touch_thread(touch_config);
    touch_thread.detach();
}