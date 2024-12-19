#include "Demo.h"

const uint8_t getSwitchNum(char keyCode){
    uint8_t result = 0;

    switch(keyCode){
        case HID_KEY_ESCAPE:
            result =  1;
            break;
        case HID_KEY_1:
            result =  2;
            break;
        case HID_KEY_2:
            result =  3;
            break;
        case HID_KEY_3:
            result =  4;
            break;
        case HID_KEY_4:
            result =  5;
            break;
        case HID_KEY_5:
            result =  6;
            break;
        case HID_KEY_TAB:
            result =  7;
            break;
        case HID_KEY_Q:
            result =  8;
            break;
        // case HID_KEY_W:
        //   result =  A1;
        //   break;
        case HID_KEY_E:
            result =  9;
            break;
        case HID_KEY_R:
            result =  10;
            break;
        case HID_KEY_T:
            result =  11;
            break;
        case HID_KEY_CAPS_LOCK:
            result =  12;
            break;
        // case HID_KEY_A:
        //   result =  A2;
        //   break;
        // case HID_KEY_S:
        //   result =  A3;
        //   break;
        // case HID_KEY_D:
        //   result =  A4;
        //   break;
        case HID_KEY_F:
            result =  13;
            break;
        case HID_KEY_G:
            result =  14;
            break;
        case HID_KEY_SHIFT_LEFT:
            result =  15;
            break;
        case HID_KEY_Z:
            result =  16;
            break;
        case HID_KEY_X:
            result =  17;
            break;
        case HID_KEY_C:
            result =  18;
            break;
        case HID_KEY_V:
            result =  19;
            break;
        case HID_KEY_CONTROL_LEFT:
            result =  20;
            break;
        case FN:
            result =  21;
            break;
        case HID_KEY_GUI_LEFT:
            result =  22;
            break;
        case HID_KEY_ALT_LEFT:
            result =  23;
            break;
        case HID_KEY_SPACE:
            result =  24;
            break;
    }
    
    return result;
}