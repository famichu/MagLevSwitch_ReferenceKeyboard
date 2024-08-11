#include "Setting.h"
#include "MaglevSwitch_Board.h"

Setting::Setting(uint16_t softwareVersion){
    config.SoftwareVersion = softwareVersion;
    config.HardwareVersion = 0;
}

bool Setting::Initialize(){
    bool result = false;

    Load();
    if(load_succeeded == true){
        result = true;
    }

    return result;
}

Config Setting::Initialize(uint16_t hardwareVersion){
    // デフォルト値読み込み
    config.HardwareVersion = hardwareVersion;
    switch(hardwareVersion){
        case 100: {
            uint8_t keymapL1[28] = {
                HID_KEY_ESCAPE,       HID_KEY_1,  HID_KEY_2,        HID_KEY_3,        HID_KEY_4,    HID_KEY_5, 
                HID_KEY_TAB,          HID_KEY_Q,  HID_KEY_W,        HID_KEY_E,        HID_KEY_R,
                HID_KEY_CAPS_LOCK,    HID_KEY_A,  HID_KEY_S,        HID_KEY_D,        HID_KEY_F,
                HID_KEY_SHIFT_LEFT,   HID_KEY_Z,  HID_KEY_X,        HID_KEY_C,
                HID_KEY_CONTROL_LEFT, FN,         HID_KEY_GUI_LEFT, HID_KEY_ALT_LEFT, HID_KEY_SPACE, 
                127, 127, 127
            };
            uint8_t keymapL2[28] = {
                HID_KEY_ESCAPE,       HID_KEY_F1, HID_KEY_F2,       HID_KEY_F3,       HID_KEY_F4,   HID_KEY_F5, 
                HID_KEY_TAB,          HID_KEY_Q,  HID_KEY_W,        HID_KEY_E,        HID_KEY_R,
                HID_KEY_CAPS_LOCK,    HID_KEY_A,  HID_KEY_S,        HID_KEY_D,        HID_KEY_F,    
                HID_KEY_SHIFT_LEFT,   HID_KEY_Z,  HID_KEY_X,        HID_KEY_C,
                HID_KEY_CONTROL_LEFT, FN,         HID_KEY_GUI_LEFT, HID_KEY_ALT_LEFT, HID_KEY_SPACE, 
                127, 127, 127
            };
            memcpy(config.KeymapL1, keymapL1, sizeof(config.KeymapL1));
            memcpy(config.KeymapL2, keymapL2, sizeof(config.KeymapL2));
            break;
        }
        case 101: {
            uint8_t keymapL1[28] = {
                HID_KEY_ESCAPE,       HID_KEY_1,  HID_KEY_2,        HID_KEY_3,        HID_KEY_4,    HID_KEY_5, 
                HID_KEY_TAB,          HID_KEY_Q,  HID_KEY_W,        HID_KEY_E,        HID_KEY_R,    HID_KEY_T, 
                HID_KEY_CAPS_LOCK,    HID_KEY_A,  HID_KEY_S,        HID_KEY_D,        HID_KEY_F,    HID_KEY_G,
                HID_KEY_SHIFT_LEFT,   HID_KEY_Z,  HID_KEY_X,        HID_KEY_C,        HID_KEY_V,
                HID_KEY_CONTROL_LEFT, FN,         HID_KEY_GUI_LEFT, HID_KEY_ALT_LEFT, HID_KEY_SPACE
            };
            
            uint8_t keymapL2[28] = {
                HID_KEY_ESCAPE,       HID_KEY_F1, HID_KEY_F2,       HID_KEY_F3,       HID_KEY_F4,   HID_KEY_F5, 
                HID_KEY_TAB,          HID_KEY_Q,  HID_KEY_W,        HID_KEY_E,        HID_KEY_R,    HID_KEY_T, 
                HID_KEY_CAPS_LOCK,    HID_KEY_A,  HID_KEY_S,        HID_KEY_D,        HID_KEY_F,    HID_KEY_G,
                HID_KEY_SHIFT_LEFT,   HID_KEY_Z,  HID_KEY_X,        HID_KEY_C,        HID_KEY_V,
                HID_KEY_CONTROL_LEFT, FN,         HID_KEY_GUI_LEFT, HID_KEY_ALT_LEFT, HID_KEY_SPACE
            };
            memcpy(config.KeymapL1, keymapL1, sizeof(config.KeymapL1));
            memcpy(config.KeymapL2, keymapL2, sizeof(config.KeymapL2));
            break;
        }
        default:
            break;
    }
    
    for(int i = 0; i < MLSW_NUM; i++){
        config.ActuationThresholds[i] = ThresholdData((uint16_t)2750, 1400, MLSW_RANGE);
        config.ReleaseThresholds[i] = ThresholdData((uint16_t)1490, MLSW_LOWER_LIMIT, MLSW_RANGE);
    }

    return config;
}

void Setting::SetThresholds(ThresholdData* actuationThresholds, ThresholdData* releaseThresholds){
    for(int i = 0; i < MLSW_NUM; i++){
        config.ActuationThresholds[i] = actuationThresholds[i];
        config.ReleaseThresholds[i] = releaseThresholds[i];
    }
}

Config Setting::Load(){
    const uint32_t FLASH_TARGET_OFFSET = 0x1F0000;
    const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);

    uint8_t data_cnt = 5;
    uint8_t load = flash_target_contents[0];

    if(load == 1){
        load_succeeded = true;
        
        config.SoftwareVersion = uint16_t(flash_target_contents[1]) | ((uint16_t)flash_target_contents[2] << 8);
        config.HardwareVersion = uint16_t(flash_target_contents[3]) | ((uint16_t)flash_target_contents[4] << 8);
        
        uint8_t bytes[2];
        for(int i = 0; i < MLSW_NUM; i++){
            for(int j = 0; j < 2; j++){
                bytes[j] = flash_target_contents[data_cnt];
                data_cnt++;
            }
            config.ActuationThresholds[i].setAbsoluteBytes(bytes[0], bytes[1]);
            
            for(int j = 0; j < 2; j++){
                bytes[j] = flash_target_contents[data_cnt];
                data_cnt++;
            }
            config.ReleaseThresholds[i].setAbsoluteBytes(bytes[0], bytes[1]);
            
            for(int j = 0; j < 2; j++){
                bytes[j] = flash_target_contents[data_cnt];
                data_cnt++;
            }
            config.ReleaseThresholds[i].setLowerLimitBytes(bytes[0], bytes[1]);
            config.ActuationThresholds[i].setLowerLimitBytes(bytes[0], bytes[1]);
            
            for(int j = 0; j < 2; j++){
                bytes[j] = flash_target_contents[data_cnt];
                data_cnt++;
            }
            config.ReleaseThresholds[i].setRangeBytes(bytes[0], bytes[1]);
            config.ActuationThresholds[i].setRangeBytes(bytes[0], bytes[1]);
        }

        for(int i = 0; i < 28; i++){
            config.KeymapL1[i] = flash_target_contents[data_cnt];
            data_cnt++;
        }

        for(int i = 0; i < 28; i++){
            config.KeymapL2[i] = flash_target_contents[data_cnt];
            data_cnt++;
        }
    }

    return config;
}

void Setting::Save(Config config_){
    config = config_;

    const uint32_t FLASH_TARGET_OFFSET = 0x1F0000; // W25Q16JVの最終ブロック(Block31)のセクタ0の先頭アドレス = 0x1F0000
    uint8_t write_data[FLASH_PAGE_SIZE]; // // W25Q16JVの書き込み最小単位 = FLASH_PAGE_SIZE(256Byte)
    
    uint32_t ints = save_and_disable_interrupts();

    write_data[0] = 1;
    write_data[1] = config.SoftwareVersion & 0xFF;
    write_data[2] = (config.SoftwareVersion >> 8) & 0xFF;
    write_data[3] = config.HardwareVersion & 0xFF;
    write_data[4] = (config.HardwareVersion >> 8) & 0xFF;
    
    uint8_t data_cnt = 5;

    for(int i = 0; i < MLSW_NUM; i++){
        uint8_t* bytes = config.ActuationThresholds[i].getAbsolutedBytes();
        for(int j = 0; j < 2; j++){
            write_data[data_cnt] = bytes[j];
            data_cnt++;
        }
        
        bytes = config.ReleaseThresholds[i].getAbsolutedBytes();
        for(int j = 0; j < 2; j++){
            write_data[data_cnt] = bytes[j];
            data_cnt++;
        }
        
        bytes = config.ReleaseThresholds[i].getLowerLimitBytes();
        for(int j = 0; j < 2; j++){
            write_data[data_cnt] = bytes[j];
            data_cnt++;
        }
        
        bytes = config.ReleaseThresholds[i].getRangeBytes();
        for(int j = 0; j < 2; j++){
            write_data[data_cnt] = bytes[j];
            data_cnt++;
        }
    }
        
    for(int i = 0; i < 28; i++){
        write_data[data_cnt] = config.KeymapL1[i];
        data_cnt++;
    }

    for(int i = 0; i < 28; i++){
        write_data[data_cnt] = config.KeymapL2[i];
        data_cnt++;
    }
    
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, write_data, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
}

Config Setting::getConfig() const{
    return config;
}
