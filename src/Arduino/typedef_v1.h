const uint8_t SW_NUM_101            = 28;
const uint8_t DIRECT_SW_NUM_101     = 15;
const uint8_t MLSW_NUM_101          = 4;
const uint8_t MATRIX_OUT_NUM_101    = 3;
const uint8_t MATRIX_IN_NUM_101     = 3;

const uint8_t SW_GPIO_101[] = {
  20, 19, 18, 17, 16, 15, 
  21, 22,     24,  
                  25, 
  6,
  7,  8,      10, 11
};

const uint8_t MATRIX_OUT_GPIO_101[] = {
  4, 1, 9
};

const uint8_t MATRIX_IN_GPIO_101[] = {
  5, 0, 23
};

const uint32_t STATUS_BITS_101[] = {
  STATUS_BIT_SW1  , 
  STATUS_BIT_SW2  , 
  STATUS_BIT_SW3  , 
  STATUS_BIT_SW4  , 
  STATUS_BIT_SW5  , 
  STATUS_BIT_SW6  , 
  STATUS_BIT_SW7  , 
  STATUS_BIT_SW8  , 
  STATUS_BIT_MLSW1, 
  STATUS_BIT_SW9  , 
  STATUS_BIT_SW10 , 
  STATUS_BIT_SW11 , 
  STATUS_BIT_SW12 , 
  STATUS_BIT_MLSW2, 
  STATUS_BIT_MLSW3, 
  STATUS_BIT_MLSW4, 
  STATUS_BIT_SW13 , 
  STATUS_BIT_SW14 , 
  STATUS_BIT_SW15 , 
  STATUS_BIT_SW16 , 
  STATUS_BIT_SW17 , 
  STATUS_BIT_SW18 , 
  STATUS_BIT_SW19 , 
  STATUS_BIT_SW20 , 
  STATUS_BIT_SW21 , 
  STATUS_BIT_SW22 , 
  STATUS_BIT_SW23 , 
  STATUS_BIT_SW24
};

const uint32_t STATUS_BITS_SW_101[]= {
  STATUS_BIT_SW1, STATUS_BIT_SW2, STATUS_BIT_SW3, STATUS_BIT_SW4, STATUS_BIT_SW5, STATUS_BIT_SW6,
  STATUS_BIT_SW7, STATUS_BIT_SW8,                 STATUS_BIT_SW9, 
                                                                  STATUS_BIT_SW13, 
  STATUS_BIT_SW15, 
  STATUS_BIT_SW20, STATUS_BIT_SW21,               STATUS_BIT_SW23, STATUS_BIT_SW24};

const uint32_t STATUS_BIT_MATRIX[MATRIX_OUT_NUM_101][MATRIX_IN_NUM_101] = {
  {STATUS_BIT_SW12, STATUS_BIT_SW17, STATUS_BIT_SW10}, 
  {STATUS_BIT_SW16, STATUS_BIT_SW18, STATUS_BIT_SW11}, 
  {STATUS_BIT_SW22, STATUS_BIT_SW19, STATUS_BIT_SW14}
};
