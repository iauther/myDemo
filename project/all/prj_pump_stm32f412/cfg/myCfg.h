#ifndef __CFG_Hx__
#define __CFG_Hx__


#define BOARD_NUCLEO_F412ZG
//#define BOARD_PUMP_F412RET6



#define VERSION                     "V1.0.0"

#define BOOT_OFFSET                 (0)                 //32KB
#define COM_BAUDRATE                (115200)


#ifdef BOARD_NUCLEO_F412ZG
    //#define USE_N950
    #define USE_MS4525
    #define USE_BMP280
    #define USE_VALVE
    
    
    #define I2C1_FREQ               (100*1000)
    #define I2C1_SCL_PIN            
    #define I2C1_SDA_PIN
    
    #define I2C2_FREQ               (100*1000)
    #define I2C2_SCL_PIN            
    #define I2C2_SDA_PIN
    
    
    #define USE_EEPROM
    #define USE_AT24C16
    #define AT24CXX_A0_PIN          0
    #define AT24CXX_A1_PIN          0
    #define AT24CXX_A2_PIN          0
    
    
    #define PUMP_UART_PORT          UART_1
    #define COM_UART_PORT           UART_2
    #define DBG_UART_PORT           UART_3
    
    #define GPIO_WDG_PIN            {GPIOA, GPIO_PIN_0}
    #define GPIO_AT24CXX_PIN        {GPIOA, GPIO_PIN_1}
    #define GPIO_12V_PIN            {GPIOA, GPIO_PIN_4}
    #define GPIO_BUZZER_PIN         {GPIOA, GPIO_PIN_5}
    
    #define GPIO_VALVE_PIN          {GPIOA, GPIO_PIN_1}
    #define GPIO_PUMP_PWR_PIN       {GPIOA, GPIO_PIN_11}
    #define GPIO_PUMP_PWM_PIN       PWM_PIN_A0
    //#define PUMP_USE_UART         //the first pump control by pwm
    
    
    #define MS4525_INT_PIN          {GPIOD, GPIO_PIN_0}         
    
    #define GPIO_LEDR_PIN           {GPIOA, GPIO_PIN_7}
    #define GPIO_LEDG_PIN           {GPIOA, GPIO_PIN_15}
    #define GPIO_LEDB_PIN           {GPIOA, GPIO_PIN_12}
#elif defined BOARD_PUMP_F412RET6
    #define USE_N950
    #define USE_MS4525
    #define USE_BMP280
    #define USE_VALVE
    
    #define USE_EEPROM
    #define USE_AT24C16
    #define AT24CXX_A0_PIN          0
    #define AT24CXX_A1_PIN          0
    #define AT24CXX_A2_PIN          0
    
    #define PUMP_UART_PORT          UART_1
    #define COM_UART_PORT           UART_2
    #define DBG_UART_PORT           UART_3
    
    #define GPIO_WDG_PIN            {GPIOA, GPIO_PIN_0}
    #define GPIO_AT24CXX_PIN        {GPIOA, GPIO_PIN_1}
    #define GPIO_12V_PIN            {GPIOA, GPIO_PIN_4}     //PA4��Щ���죬����ʱ��ע��
    #define GPIO_BUZZER_PIN         {GPIOA, GPIO_PIN_5}
    
    #define GPIO_VALVE_PIN          {GPIOA, GPIO_PIN_8}
    #define GPIO_PUMP_PWR_PIN       {GPIOA, GPIO_PIN_11}
    #define GPIO_PUMP_PWM_PIN       PWM_PIN_B0  //{GPIOB, GPIO_PIN_0}
    
    #define MS4525_INT_PIN          {GPIOC, GPIO_PIN_0}
    
    #define GPIO_LEDR_PIN           {GPIOA, GPIO_PIN_7}
    #define GPIO_LEDG_PIN           {GPIOA, GPIO_PIN_15}
    #define GPIO_LEDB_PIN           {GPIOA, GPIO_PIN_12}
    
    
#else
    error "must define board!!"
#endif


#ifdef  USE_EEPROM
    #define UPGRADE_INFO_ADDR       0
    #define APP_OFFSET              (0x8000)            //32KB--->The End
#else
    #define UPGRADE_INFO_ADDR       (0x8000)
    #define APP_OFFSET              (0x10000)           //64KB--->The End
#endif


#endif
