#include <stdint.h>
#include "mcc_generated_files/mcc.h"

/*-------------------------------
-----------DRIVERS-----------
-------------------------------*/


//-------------LUMINAIRE-------------//


static uint16_t __duty(uint16_t duty){
    return (duty * 0x19f) / 100;
}

static uint16_t curr_duty = 0;

void LUM_Set(uint16_t duty_percent){
  
    //if(duty_percent != curr_duty)
    //    EPWM_LoadDutyValue(__duty(duty_percent));
    uint16_t duty = __duty(duty_percent);
    
    if(duty > curr_duty){
        for (uint16_t i = curr_duty + 1; i <= duty; i++) {
            EPWM_LoadDutyValue(i);
            __delay_ms(1);
        }
    }
    else if(duty < curr_duty){
        for (int32_t i = curr_duty - 1; i >= duty; i--) {
            //if(i == 0)
            //    EPWM_Initialize();
            //else
            EPWM_LoadDutyValue((uint16_t)i);
            __delay_ms(1);
        }
    }
    curr_duty = duty;

}
    
    

void LUM_Init(uint8_t duty_percent){
    EPWM_Initialize();
    LUM_Set(duty_percent);
}


//-------------PANEL-------------//
float adc_vcc;

void PV_Init(float vcc){
     adc_vcc = vcc;
}
       
float PV_GetVoltage(){
    float convertion = (float) ADC_GetConversion(channel_AN3);
    float divider = (float) 0xFFFF;
    float volts_per_bit = adc_vcc / divider;
    return (convertion * volts_per_bit);
}


//-------------LED-------------//

static bool curr_state;

void LED_Init(bool state){
    if(state)
        IO_RA5_SetHigh();
    else
        IO_RA5_SetLow();
    curr_state = state;
}

void LED_On(){
    IO_RA5_SetHigh();
    curr_state = true;
}

void LED_Off(){
    IO_RA5_SetLow();
    curr_state = false;
}

void LED_Toggle(){
    if(curr_state)
        LED_Off();
    else
        LED_On();

}


//-------------RADAR-------------//

bool RADAR_Detect(){
    return IO_RA1_GetValue();
}

//-------------TMR-------------//

static uint64_t __ms = 0;

void TMR_ISR(){
    __ms++;
}

void TMR_Init(){
    TMR0_SetInterruptHandler(TMR_ISR);
}

uint64_t TMR_GetElapsedMS(){
    return __ms;
}


/*-------------------------------
-----------APPLICATION-----------
-------------------------------*/

#define MAX_LEVEL 97
#define MIN_LEVEL 10
#define DIVIDER_RATIO (30.0f / (30.0f + 51.0f))
#define INSUFFICIENT_VOLTAGE 0.8f
#define SUFFICIENT_VOLTAGE 1.6f

enum {DISCHARGE, DISCHARGE_ENTRY, CHARGE, CHARGE_ENTRY};


void main(void){
    SYSTEM_Initialize();

    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    
    TMR_Init();
    LUM_Init(0);
    LED_Init(0);
    PV_Init(3.2);
    
    int state = CHARGE_ENTRY;
    float pv_voltage;
    bool presence;
    uint8_t min_level;
   
    uint64_t led_start_time_ms = TMR_GetElapsedMS();

    while(1){
        
        pv_voltage = PV_GetVoltage() / DIVIDER_RATIO;
        
        switch (state){
        case DISCHARGE_ENTRY:
            LED_Off();   
            state = DISCHARGE;
            break;
            
        case DISCHARGE:
            presence = RADAR_Detect();
                        
            LUM_Set((presence) ? MAX_LEVEL : MIN_LEVEL);
       
            if((pv_voltage) > SUFFICIENT_VOLTAGE)
                state = CHARGE_ENTRY;
            break;

        case CHARGE_ENTRY:
            LUM_Set(0);
            
            state = CHARGE;
            break;      
            
        case CHARGE :
            if(TMR_GetElapsedMS() - led_start_time_ms > 500 ){
                LED_Toggle();
                led_start_time_ms = TMR_GetElapsedMS();
            }
            
            if((pv_voltage) < INSUFFICIENT_VOLTAGE)
                state = DISCHARGE_ENTRY;
            break;
        }
    }
}
