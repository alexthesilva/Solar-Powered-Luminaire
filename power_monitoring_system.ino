#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

#define DELAY_MS 1000

String separator = ";";

typedef struct {
  float avg_shuntvoltage_bat;
  float avg_busvoltage_bat;
  float avg_current_bat;
  float avg_power_bat;
  float avg_loadvoltage_bat;
  
  float shuntvoltage_bat;
  float busvoltage_bat;
  float current_bat;
  float power_bat;
  float loadvoltage_bat;

  float shuntvoltage_panel;
  float busvoltage_panel;
  float current_panel;
  float power_panel;
  float loadvoltage_panel;

  float temp;  
  float lux;
} Measurement_t;

Adafruit_INA219 ina219_panel(0x40);                                                                                                                                                                                                                                                                                                                                                        
Adafruit_INA219 ina219_bat(0x41);

Adafruit_BMP280 bmp;
BH1750 lightMeter;

Measurement_t measurements;


void get_measurements(){
  measurements.temp  =(long int) bmp.readTemperature();  
    measurements.lux = (long int) lightMeter.readLightLevel();


      measurements.avg_shuntvoltage_bat = 0;
      measurements.avg_busvoltage_bat = 0;
      measurements.avg_current_bat = 0;
      measurements.avg_power_bat = 0;
      measurements.avg_loadvoltage_bat = 0;

    for(int i = 0; i < 200; i++){
      measurements.avg_shuntvoltage_bat += ina219_bat.getShuntVoltage_mV();
      measurements.avg_busvoltage_bat += ina219_bat.getBusVoltage_V() * 1000;
      measurements.avg_current_bat += ina219_bat.getCurrent_mA();
      measurements.avg_power_bat += ina219_bat.getPower_mW();
      measurements.avg_loadvoltage_bat += measurements.busvoltage_bat + measurements.shuntvoltage_bat;
      delayMicroseconds(1);
    }

    measurements.shuntvoltage_bat = measurements.avg_shuntvoltage_bat / 200;
    measurements.busvoltage_bat = measurements.avg_busvoltage_bat / 200;
    measurements.current_bat = measurements.avg_current_bat / 200;
    measurements.power_bat = measurements.avg_power_bat / 200;
    measurements.loadvoltage_bat = measurements.avg_loadvoltage_bat / 200;

    measurements.shuntvoltage_panel = ina219_panel.getShuntVoltage_mV();
    measurements.busvoltage_panel = ina219_panel.getBusVoltage_V() * 1000;
    measurements.current_panel = ina219_panel.getCurrent_mA();
    measurements.power_panel = ina219_panel.getPower_mW();
    measurements.loadvoltage_panel = measurements.busvoltage_panel + measurements.shuntvoltage_panel;
}


void log_write(){ 
      Serial.print(String(measurements.loadvoltage_panel) + separator);
      Serial.print(String(measurements.current_panel) + separator);
      Serial.print(String(measurements.power_panel) + separator);
      Serial.print(String(measurements.loadvoltage_bat) + separator);
      Serial.print(String(measurements.current_bat) + separator);
      Serial.print(String(measurements.power_bat) + separator);      
    
      Serial.print(String(measurements.lux) + separator);
      Serial.print(String(measurements.temp));
      Serial.println("");
}

void setup() {

  Serial.begin(115200);
  
  ina219_bat.begin();
  ina219_panel.begin();

  lightMeter.begin();
  lightMeter.setMTreg(32);
  unsigned status = bmp.begin(0x76);
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* xz<Standby time. */  
}



enum {HEADER, GET_MEASUREMENTS, DISPLAY_WRITE, LOG_WRITE, DELAY};

int state = HEADER;
void loop() {

  switch(state){
    case HEADER:        
        Serial.print("V_PNL [mV]" + separator + "I_PNL [mA]" + separator + "P_PNL [mW]" + separator);
        Serial.print("V_BAT [mV]" + separator + "I_BAT [mA]" + separator + "P_BAT [mW]" + separator);
        Serial.print("LUM [Lux]" + separator + "TMP [C]");
        Serial.println("");

        state = GET_MEASUREMENTS;
      break;
    
    case GET_MEASUREMENTS:
        get_measurements();
        state = LOG_WRITE;
      break;

    case LOG_WRITE:
      log_write();
      state = DELAY;
      break;
    
    case DELAY:      
      delay(DELAY_MS);
      state = GET_MEASUREMENTS;
      break;
    
    default:
      state = GET_MEASUREMENTS;
      break;
  }

}




  
 



  

  
  
