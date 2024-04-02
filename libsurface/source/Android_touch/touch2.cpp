/**
 * 解决imgui触摸穿透问题
 */

#include "touch.h"
// C
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>

// C++
#include "draw.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_android.h"

#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>

#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <dirent.h>

using namespace std;


#define FROM_SCREEN 0x0
#define FROM_SOCKET 0x1

#define UNGRAB 0x0
#define GRAB 0x1

#define UP 0x0
#define DOWN 0x1

struct input_events {
    int type;
    vector<input_event> events;
};

struct Rectangle {
    float x;
    float y;
    float w;
    float h;
};

mutex locker;
queue<input_events> input_queue;

int screen_fd;
int screen_width;
int screen_height;
int uinput_fd;
int finger_down = 0;
int current_slot = 0;
int current_screen_slot = 0;
int finger_id[11];

Draw *draw;

static int abs_parse(int fd) {
    uint8_t *bits = NULL;
    ssize_t bits_size = 0;
    int i, j;
    int res;
    struct label *bit_labels;
    const char *bit_label;
    int count = 0;
    while (1) {
        res = ioctl(fd, EVIOCGBIT(EV_ABS, bits_size), bits);
        if (res < bits_size)
            break;
        bits_size = res + 16;
        bits = (uint8_t *) realloc(bits, bits_size * 2);
        if (bits == NULL)
            printf("failed to allocate buffer of size %d\n", (int) bits_size);
    }
    for (i = 0; i < res; i++) {
        for (j = 0; j < 8; j++)
            if (bits[i] & 1 << j) {
                char down = ' ';
                struct input_absinfo abs;
                if (ioctl(fd, EVIOCGABS(i * 8 + j), &abs) == 0) {
                    switch (i * 8 + j) {
                        case ABS_MT_SLOT:
                            // printf("ABS_MT_SLOT");
                            break;
                        case ABS_MT_TOUCH_MAJOR:
                            //   printf("ABS_MT_TOUCH_MAJOR");
                            break;
                        case ABS_MT_TOUCH_MINOR:
                            //   printf("ABS_MT_TOUCH_MINOR");
                            break;
                        case ABS_MT_POSITION_X:
                            // printf("ABS_MT_POSITION_X");
                            break;
                        case ABS_MT_POSITION_Y:
                            //printf("ABS_MT_POSITION_Y");
                            break;
                        case ABS_MT_TRACKING_ID:
                            //  printf("ABS_MT_TRACKING_ID");
                            break;
                        case ABS_MT_DISTANCE:
                            //  printf("ABS_MT_DISTANCE");
                            break;
                    }
                    /* printf(" : value %d, min %d, max %d, fuzz %d, flat %d, resolution %d\n",
                            abs.value, abs.minimum, abs.maximum, abs.fuzz, abs.flat,
                            abs.resolution);*/
                    if (i * 8 + j == ABS_MT_POSITION_X) {
                        screen_width = abs.maximum + 1;
                    } else if (i * 8 + j == ABS_MT_POSITION_Y) {
                        screen_height = abs.maximum + 1;
                    }
                }
                count++;
            }
    }
    free(bits);
    return count;
}

int open_device(const char *dev_name) {
    int fd = open(dev_name, O_RDWR);
    if (fd > 0) {
        if (abs_parse(fd) > 0) {
            return fd;
        } else {
            close(fd);
            return -1;
        }
    }
    printf("Could not open %s, %s", dev_name, strerror(errno));
    return -1;
}


int scan_devices(int eventX) {
    const char *dirname = "/dev/input";
    char devname[4096];
    char *filename;
    DIR *dir;
    struct dirent *de;
    int fd;

    if (eventX > -1) {
        sprintf(devname, "%s/event%d", dirname, eventX);
        printf("Open custom device: %s\n", devname);
        fd = open_device(devname);
        if (fd > 0) {
            return fd;
        }
        printf("Cannot open the custom device: %d\n", eventX);
        memset(devname, 0, 4096);
    }
    dir = opendir(dirname);
    if (dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while ((de = readdir(dir))) {
        if (de->d_name[0] == '.' &&
            (de->d_name[1] == '\0' ||
             (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        fd = open_device(devname);
        if (fd > 0) {
            return fd;
        }
    }
    closedir(dir);
    return -1;
}

bool send_cmd(int devFd, __u16 type, __u16 code, __s32 value) {
    // return true;
    input_event event{
            .type = type, .code = code, .value = value};
    if (devFd != -1) {
        ssize_t writeSize = write(devFd, &event, sizeof(event));
        if (writeSize == sizeof(event)) {
            // printf("CMD: %d %d %d %d\n", devFd, type, code, value);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

string get_rand_str(int length) {
    char tmp;
    string buffer;
    random_device rd;
    default_random_engine random(rd());
    for (int i = 0; i < length; i++) {
        tmp = random() % 36;
        if (tmp < 10) {
            tmp += '0';
        } else {
            tmp -= 10;
            tmp += 'A';
        }
        buffer += tmp;
    }
    return buffer;
}

int create_uinput_device(int screenW, int screenH) {
    int uinput_dev_fd;
    struct uinput_user_dev uinp;
    uinput_dev_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinput_dev_fd == 0) {
        printf("Unable to open /dev/uinput\n");
        return -1;
    }
    memset(&uinp, 0, sizeof(uinp));
    strncpy(uinp.name, get_rand_str(8).c_str(), UINPUT_MAX_NAME_SIZE);
    uinp.id.version = 4;
    uinp.id.bustype = BUS_USB;
    uinp.absmin[ABS_MT_SLOT] = 0;
    uinp.absmax[ABS_MT_SLOT] = 10;
    uinp.absmin[ABS_MT_TOUCH_MAJOR] = 0;
    uinp.absmax[ABS_MT_TOUCH_MAJOR] = 15;
    uinp.absmin[ABS_MT_POSITION_X] = 0;
    uinp.absmax[ABS_MT_POSITION_X] = screenW;
    uinp.absmin[ABS_MT_POSITION_Y] = 0;
    uinp.absmax[ABS_MT_POSITION_Y] = screenH;
    uinp.absmin[ABS_MT_TRACKING_ID] = 0;
    uinp.absmax[ABS_MT_TRACKING_ID] = 65535;
    uinp.absmin[ABS_MT_PRESSURE] = 0;
    uinp.absmax[ABS_MT_PRESSURE] = 255;
    ioctl(uinput_dev_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinput_dev_fd, UI_SET_EVBIT, EV_REL);
    ioctl(uinput_dev_fd, UI_SET_EVBIT, EV_ABS);
    ioctl(uinput_dev_fd, UI_SET_EVBIT, EV_SYN);
    ioctl(uinput_dev_fd, UI_SET_ABSBIT, ABS_X);
    ioctl(uinput_dev_fd, UI_SET_ABSBIT, ABS_Y);
    ioctl(uinput_dev_fd, UI_SET_ABSBIT, ABS_PRESSURE);
    ioctl(uinput_dev_fd, UI_SET_ABSBIT, ABS_MT_SLOT);
    ioctl(uinput_dev_fd, UI_SET_ABSBIT, ABS_MT_TOUCH_MAJOR);
    ioctl(uinput_dev_fd, UI_SET_ABSBIT, ABS_MT_POSITION_X);
    ioctl(uinput_dev_fd, UI_SET_ABSBIT, ABS_MT_POSITION_Y);
    ioctl(uinput_dev_fd, UI_SET_ABSBIT, ABS_MT_TRACKING_ID);
    ioctl(uinput_dev_fd, UI_SET_ABSBIT, ABS_MT_PRESSURE);
    ioctl(uinput_dev_fd, UI_SET_KEYBIT, BTN_TOUCH);
    ioctl(uinput_dev_fd, UI_SET_KEYBIT, BTN_TOOL_FINGER);
    ioctl(uinput_dev_fd, UI_SET_PROPBIT, INPUT_PROP_DIRECT);
    write(uinput_dev_fd, &uinp, sizeof(uinp));
    ioctl(uinput_dev_fd, UI_DEV_CREATE);
    // printf("uinput_dev_fd: %d\n", uinput_dev_fd);
    return uinput_dev_fd;
}

vector<input_event> input_event_filter(vector<input_event> events) {
    vector<input_event> event;
    for (int i = 0; i < events.size(); i++) {
        if (events[i].type == EV_KEY || events[i].type == EV_ABS || events[i].type == EV_SYN) {
//            printf("type: %X,code: %X,value:%X\n",events[i].type ,events[i].code,events[i].value);
            event.push_back(events[i]);
        }
    }
    return event;
}

MDisplayInfo getTouchDisplyInfo1() {
    if (draw->displayInfo.orientation == 0 || draw->displayInfo.orientation == 2) {
        return {draw->displayInfo.width, draw->displayInfo.height, draw->displayInfo.orientation};;
    } else {
        return {draw->displayInfo.height, draw->displayInfo.width, draw->displayInfo.orientation};
    }
}


ImVec2 rotatePointx1(uint32_t orientation, ImVec2 mxy, ImVec2 wh) {
    if (orientation == 0) {
        return mxy;
    }
    ImVec2 xy(mxy.x, mxy.y);
    if (orientation == 3) {
        xy.x = (float) wh.y - mxy.y;
        xy.y = mxy.x;
    } else if (orientation == 2) {
        xy.x = (float) wh.x - mxy.x;
        xy.y = (float) wh.y - mxy.y;
    } else if (orientation == 1) {
        xy.x = mxy.y;
        xy.y = (float) wh.x - mxy.x;
    }
    return xy;
}

// imgui 触摸坐标
int eventX = 0, eventY = 0;
int fingerIndex = 0;

bool isPointInsideRectangles(ImVec2 point, const std::vector<Rectangle> &rectangles) {
    for (const Rectangle &rect: rectangles) {
        if (point.x >= rect.x && point.x <= rect.x + rect.w &&
            point.y >= rect.y && point.y <= rect.y + rect.h) {
            return false;
        }
    }
    return true;
}

// 函数用于迭代处理所有窗口
vector<Rectangle> getAllWindowPositionsAndSizes() {
    vector<Rectangle> arr;
    // 获取当前上下文中的窗口列表
    ImGuiContext *context = ImGui::GetCurrentContext();
    // 遍历所有窗口
    for (int i = 0; i < context->Windows.Size; ++i) {
        ImGuiWindow *window = context->Windows[i];
        if (window->WasActive && !window->Hidden) {
            ImVec2 windowPosition = window->Pos;
            ImVec2 windowSize = window->Size;
            arr.push_back({windowPosition.x,
                           windowPosition.y,
                           windowSize.x,
                           windowSize.y
                          });
        }
    }
    return arr;
}

bool canTouch = true;

void on_queue_event() {
    if (!input_queue.empty()) {
        input_events events = input_queue.front();
        input_queue.pop();
        if (events.type == FROM_SCREEN) {
            send_cmd(uinput_fd, EV_ABS, ABS_MT_SLOT, current_screen_slot);
        }
        int status = IM_MOVE;
        for (input_event e: events.events) {
            switch (e.type) {
                case EV_KEY: {
                    if (e.code == BTN_TOUCH) {
                        if (e.value == DOWN) {
                            status = IM_DOWN;
                        } else if (e.value == UP) {
                            status = IM_UP;
                        }
                    }
                    break;
                }
                case EV_ABS: {
                    if (e.code == ABS_MT_TRACKING_ID) {
                        if (e.value != -1) {
                            status = IM_DOWN;
                        } else {
                            status = IM_UP;
                            break;
                        }
                    } else if (e.code == ABS_MT_SLOT) {
                        fingerIndex = e.value;
                    } else if (fingerIndex == 0) {
                        if (e.code == ABS_MT_POSITION_X) {
                            eventX = e.value;
                        } else if (e.code == ABS_MT_POSITION_Y) {
                            eventY = e.value;
                        }
                    }
                }
            }
        }

        // 判断imgui是否可用
        if (ImGui::GetCurrentContext()) {
            MDisplayInfo mDisplayInfo = getTouchDisplyInfo1();
            ImVec2 point = rotatePointx1(mDisplayInfo.orientation, {(float) eventX, (float) eventY},
                                         {(float) screen_width, (float) screen_height});
            ImVec2 newEvent((point.x * (float) mDisplayInfo.width) / (float) screen_width,
                            (point.y * (float) mDisplayInfo.height) / (float) screen_height);
            ImGuInputEvent imGuInputEvent{};
            imGuInputEvent.fingerIndex = fingerIndex;
            imGuInputEvent.pos = newEvent;
            if (status == IM_DOWN) {           // 按下
                imGuInputEvent.type = IM_DOWN;
                // 获取所有窗口判断点击位置是否在窗口内部
                vector<Rectangle> all = getAllWindowPositionsAndSizes();
                canTouch = isPointInsideRectangles(newEvent, all);
            } else if (status == IM_MOVE) {    // 移动
                imGuInputEvent.type = IM_MOVE;
            } else {                           // 放开
                imGuInputEvent.type = IM_UP;
                canTouch = true;
            }
            ImGui_ImplAndroid_HandleInputEvent(imGuInputEvent);
        }

        if (canTouch) {
            for (input_event ev: events.events) {
                if (ev.type == EV_ABS) {
                    switch (ev.code) {
                        case ABS_MT_TRACKING_ID:
                            if (events.type == FROM_SCREEN) {
                                finger_id[current_screen_slot] = ev.value;
                            } else {
                                finger_id[current_slot] = ev.value;
                            }
                            if (ev.value == -1) {
                                finger_down--;
                                if (finger_down == 0) {
                                    send_cmd(uinput_fd, ev.type, ev.code, ev.value);
                                    send_cmd(uinput_fd, EV_KEY, BTN_TOUCH, UP);
                                    send_cmd(uinput_fd, EV_KEY, BTN_TOOL_FINGER, UP);
                                    continue;
                                }
                            } else {
                                finger_down++;
                                send_cmd(uinput_fd, ev.type, ev.code, ev.value);
                                send_cmd(uinput_fd, EV_ABS, ABS_MT_PRESSURE, 80);
                                send_cmd(uinput_fd, EV_KEY, BTN_TOUCH, DOWN);
                                send_cmd(uinput_fd, EV_KEY, BTN_TOOL_FINGER, DOWN);
                                continue;
                            }
                            break;
                        case ABS_MT_SLOT:
                            if (events.type == FROM_SCREEN) {
                                current_screen_slot = ev.value;
                            } else {
                                current_slot = ev.value;
                            }
                            break;
                        default:
                            break;
                    }
                }
                send_cmd(uinput_fd, ev.type, ev.code, ev.value);
            }
        }
    }
}


void on_input() {
    vector<input_event> events;
    input_event event{};
    while (read(screen_fd, &event, sizeof(event)) > 0) {
        events.push_back(event);
        if (event.type == EV_SYN && event.code == SYN_REPORT && event.value == 0) {
            input_events e = {
                    .type = FROM_SCREEN,
                    .events = input_event_filter(events)};
            locker.lock();
            input_queue.push(e);
            locker.unlock();
            events.clear();
            on_queue_event();
        }
    }
    printf("on_input end\n");
}


input_event make_event(__u16 type, __u16 code, __s32 value) {
    input_event ev = {
            .type = type,
            .code = code,
            .value = value};
    return ev;
}

void touchDown(long x, long y, long finger) {
    MDisplayInfo mDisplayInfo = getTouchDisplyInfo1();
    ImVec2 newEvent((x * (float) screen_width / (float) mDisplayInfo.width),
                    (y * (float) screen_height) / (float) mDisplayInfo.height);
    vector<input_event> event;
    event.push_back(make_event(EV_ABS, ABS_MT_SLOT, finger));
    event.push_back(make_event(EV_ABS, ABS_MT_TRACKING_ID, finger));
    event.push_back(make_event(EV_ABS, ABS_MT_POSITION_X, newEvent.x));
    event.push_back(make_event(EV_ABS, ABS_MT_POSITION_Y, newEvent.y));
    event.push_back(make_event(EV_SYN, 0, 0));
    input_events events = {
            .type = FROM_SOCKET,
            .events = event};
    locker.lock();
    input_queue.push(events);
    locker.unlock();
    on_queue_event();
}

void touchUp(long finger) {
    vector<input_event> event;
    finger_id[finger] = -1;
    event.push_back(make_event(EV_ABS, ABS_MT_SLOT, finger));
    event.push_back(make_event(EV_ABS, ABS_MT_TRACKING_ID, -1));
    event.push_back(make_event(EV_SYN, 0, 0));
    input_events events = {
            .type = FROM_SOCKET,
            .events = event};
    locker.lock();
    input_queue.push(events);
    locker.unlock();
    on_queue_event();
}


void touchMove(long x, long y, long finger) {
    MDisplayInfo mDisplayInfo = getTouchDisplyInfo1();
    ImVec2 newEvent((x * (float) screen_width / (float) mDisplayInfo.width),
                    (y * (float) screen_height) / (float) mDisplayInfo.height);
    vector<input_event> event;
    event.push_back(make_event(EV_ABS, ABS_MT_SLOT, finger));
    event.push_back(make_event(EV_ABS, ABS_MT_POSITION_X, newEvent.x));
    event.push_back(make_event(EV_ABS, ABS_MT_POSITION_Y, newEvent.y));
    event.push_back(make_event(EV_SYN, 0, 0));
    input_events events = {
            .type = FROM_SOCKET,
            .events = event};
    locker.lock();
    input_queue.push(events);
    locker.unlock();
    on_queue_event();
}


void *initTouchDevice(int eventX) {
    //扫描设备触摸驱动eventX号
    screen_fd = scan_devices(eventX);

    //打开设备驱动写入
    ioctl(screen_fd, EVIOCGRAB, GRAB);

    //创建虚拟触摸设备
    uinput_fd = create_uinput_device(screen_width, screen_height);

    // 系统输入事件监听
    on_input();
    //释放触摸句柄
    ioctl(screen_fd, EVIOCGRAB, UNGRAB);
    close(screen_fd);
    close(uinput_fd);
    return NULL;
}

void closeTouch() {
    screen_fd = -1;
}

/**
 * 手动指定event位置
 * @param eventX
 */
void initTouch(int eventX) {
    //触摸监听
    std::thread touchThread(initTouchDevice, eventX);
    touchThread.detach();
}


void initTouch() {
    //触摸监听
    std::thread touchThread(initTouchDevice, -1);
    touchThread.detach();
}
