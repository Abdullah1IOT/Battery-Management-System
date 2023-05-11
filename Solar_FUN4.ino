
#include<EEPROM.h> 
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>



//UTFTGLUE myGLCD(0,A2,A1,A3,A4,A0);

const float batt_voltVSsoc[21][2] = {{9.7,0},{10.15,5},{10.4,10},{10.5,15},{10.7,20},{10.95,25},{11.2,30},{11.3,35},{11.5,40},{11.6,45},{11.7,50},{11.8,55},{12,60},{12.2,65},{12.3,70},{12.4,75},{12.5,80},{12.55,85},{12.6,90},{12.7,95},{12.75,100}};   



#define Button 21
#define ESP_interupt 20
  
//SoftwareSerial Arduino_SoftSerial(35,37); // RX, TX

int SWState = 0;
int LastSWState = 0;

//----------Test Case 0: Normal Operation----------
//
float solar_sensor[] = {0,15,25,30,35,25,25,15,17,26,33,55,36,72,15,250,300,350,250,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33,55,36,50,250,300,350,250,250,155,170,260,33,55,36,15,25,30,35,0,0,0,0,0,0,0,0,0};
float temp_factor = 0.05;


//----------Test Case 1: Over SOC----------
 
//float solar_sensor[] = {0,15,25,30,35,25,25,15,17,26,33,55,36,72,15,0,15,25,30,35,25,25,15,17,26,33,150,250,300,350,250,250,155,170,260,33,55,36,33,55,36,50,250,300};
//float temp_factor = 0.2;

//----------Test Case 2: Over DOD----------

//float solar_sensor[] = {0,15,25,30,35,25,25,15,17,26,33,55,36,72,15,0,15,25,30,35,25,25,15,17,26,33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
//float temp_factor = 0.2;

//----------Test Case 3: Over Temperature----------

//float solar_sensor[] = {0,15,25,30,35,25,25,15,17,26,33,55,36,72,15,0,15,25,30,35,25,25,15,17,26,33,150,250,300,350,250,250,155,170,260,33,55,36,33,55,36,50,250};
//float temp_factor = 0.5;

//----------Test Case 4: Sensor Mulfunction (Solar Power < 0)----------

//float solar_sensor[] = {0,15,25,30,35,25,25,15,15,25,30,35,25,25,15,-15,-25};
//float temp_factor = 0.2;

//----------Test Case 5: Sensor Mulfunction (Solar Power > 1000)----------

//float solar_sensor[] = {0,15,25,30,35,25,25,15,15,25,30,35,25,25,15,1000,1200};
//float temp_factor = 0.2;



const float batt_VoltVSsoc[21][2] PROGMEM = {{21,0},{22,5},{22.4,10},{22.7,15},{23.05,20},{23.33,25},{23.5,30},{23.6,35},{23.8,40},{24,45},{24.1,50},{24.2,55},{24.3,60},{24.4,65},{24.5,70},{24.7,75},{24.8,80},{24.9,85},{25,90},{25.1,95},{25.5,100}};   

float batt_volt_soc[21][2]; 
float batt_curr[sizeof(solar_sensor)/sizeof(int)];
float batt_volt[sizeof(solar_sensor)/sizeof(int)];
float soc[sizeof(solar_sensor)/sizeof(int)];
float dod[sizeof(solar_sensor)/sizeof(int)];
int soh[sizeof(solar_sensor)/sizeof(int)];
int batt_temp[sizeof(solar_sensor)/sizeof(int)];
int charge_flag[sizeof(solar_sensor)/sizeof(int)];
int discharge_flag[sizeof(solar_sensor)/sizeof(int)];
int error_flag[sizeof(solar_sensor)/sizeof(int)];
boolean stop_flag = false;


float batt_cap[sizeof(solar_sensor)/sizeof(int)];
String error_message[sizeof(solar_sensor)/sizeof(int)];
float solar_power[sizeof(solar_sensor)/sizeof(int)];


float led_power = 120;
float batt_curr_lim = 36;
float t_sampling = 60;

int counter = 0;
long int t1,t2,t3,t4,t5,delta_t1,delta_t2,delta_t3;


int round5(int n) {
  return (n/5 + (n%5>2)) * 5;
}

void init_bms(){
   batt_cap[0] = 43200;
   batt_volt[0] = 12.3;
   soc[0] = 0.7;
   dod[0] = 1 - soc[0];
   batt_temp[0] = 50;
   
   }

void operation_Mode(int count) {
     if (solar_power[count] > 10 && solar_power[count] <= 1000 && soc[count] <=1 && batt_temp[count] <=100) {
     charge_flag[count]= 1;
     discharge_flag[count] = 0;
     error_flag[count] = 0;
   } else if (solar_power[count] <= 10 && solar_power[count] >= 0 && dod[count] <=0.5 && batt_temp[count] <=100){
     charge_flag[count] = 0;
     discharge_flag[count] = 1;
     error_flag[count] = 0;
   } else if (solar_power[count] <= 10 && solar_power[count] >= 0 && dod[count] >0.5 && batt_temp[count] <=100){
     charge_flag[count] = 0;
     discharge_flag[count] = 0;
     error_flag[count] = 1;
   } else if (solar_power[count] > 10 && solar_power[count] < 1000 && soc[count] >1 && batt_temp[count] <=100){
     charge_flag[count] = 0;
     discharge_flag[count] = 0;
     error_flag[count] = 2;
   } else if (batt_temp[count]>100){
     charge_flag[count] = 0;
     discharge_flag[count] = 0;
     error_flag[count] = 3;
   } else if (solar_power[count] < 0){
     charge_flag[count] = 0;
     discharge_flag[count] = 0;
     error_flag[count] = 4;
   } else if (solar_power[count] > 1000){
     charge_flag[count] = 0;
     discharge_flag[count] = 0;
     error_flag[count] = 5;
   }else{
     charge_flag[count] = 0;
     discharge_flag[count] = 0;
     error_flag[count] = 6;
   }

     }

void eeprom_DataManagent(int count){
  eeprom_write_float(count,solar_sensor[count]);
  solar_power[count] = eeprom_read_float(count); 
}

void errorCheck(int count){
  int errorflag = error_flag[count];
  switch(errorflag) {
  case 0:
    error_message[count] = "Normal";
    //Serial.println("Normal Operation");
    break;
  case 1:
    error_message[count] = "DOD too high";
    Serial.println("DOD too high");
    delay(10);
    break;
  case 2:
    error_message[count] = "SOC too high";
    Serial.println("SOC too high");
    delay(10);
    break;
  case 3:
    error_message[count] = "Batt Temp too high";
    Serial.println("Batt Temp too high");
    delay(10);
    break;
  case 4:
    error_message[count] = "Negative Power";
    Serial.println("Negative Power");
    delay(10);
    break;
  case 5:
    error_message[count] = "Power Too High";
    Serial.println("Power Too High");
    delay(10);
    break;
  }
}

void battCurrCal(int count) {
    if (charge_flag[count]+discharge_flag[count]!=0){
      if (charge_flag[count] == 1 && discharge_flag[count]==0){
        batt_curr[count] = solar_power[count]/batt_volt[count];
          if (batt_curr[count]>batt_curr_lim){
      batt_curr[count] = batt_curr_lim;
      return;
    }
   } else if (charge_flag[count] == 0 && discharge_flag[count]==1){
        batt_curr[count] = (-1)*led_power/batt_volt[count];
        if(batt_curr[count]<-1*batt_curr_lim){
          batt_curr[count] = -1*batt_curr_lim;
        return;
        }
   }
 } else {
    errorCheck(count);
   }
 }

 void battTempCal(int count) {
    batt_temp[count+1] = batt_temp[count]+temp_factor*(abs(batt_curr[count]));
    return;
 }
 
 void socCal(int count) {
  
    soc[count+1] = soc[count]+(1/batt_cap[0])*(batt_curr[count])*t_sampling;
    int soc_batt_int = round(soc[count]*100);
    int soc_batt_int_t = round(soc[count+1]*100);
    
    soc_batt_int = round5(soc_batt_int);
    soc_batt_int_t = round5(soc_batt_int_t);

    t4 = micros();   
    for (int i =0;i<(sizeof(batt_voltVSsoc)/sizeof(int)/4);i++){
     int x1 = soc_batt_int-batt_voltVSsoc[i][1];
     int x2 = soc_batt_int_t-batt_voltVSsoc[i][1];
     if (x1 ==0){
      batt_volt[count] = batt_voltVSsoc[i][2];
      
     }
     if (x2 ==0){
      batt_volt[count+1] = batt_voltVSsoc[i][2];
     }
    
    }
    t5 = micros(); 
    dod[count] = 1-soc[count];
    dod[count+1] = 1-soc[count+1];
    float dg = count;
    float dg2 = (399-dg)/400;
    soh[count] = dg2*100;
    
  }

 void updateLED(int count){

    if (error_flag[count]==0 &&charge_flag[count] ==1 && discharge_flag[count]==0){
    digitalWrite(41,LOW); //ctrl yellow 
    digitalWrite(51,LOW); //ctrl red
    digitalWrite(45,HIGH); //ctrl green
    } else if(error_flag[count]==0 &&charge_flag[count] ==0 && discharge_flag[count]==1){
    digitalWrite(41,HIGH); //ctrl yellow 
    digitalWrite(51,LOW); //ctrl red
    digitalWrite(45,LOW); //ctrl green
    } else if(error_flag[count]!=0){
    digitalWrite(41,LOW); //ctrl yellow 
    digitalWrite(51,HIGH); //ctrl red
    digitalWrite(45,LOW); //ctrl green
    } else {
    digitalWrite(41,LOW); //ctrl yellow 
    digitalWrite(51,HIGH); //ctrl red
    digitalWrite(45,LOW); //ctrl green
    }
  }
 

 void battISR()
{
  
 // myGLCD.clrScr();
  digitalWrite(51,HIGH);
  digitalWrite(41,LOW);
  digitalWrite(45,LOW);
    
 // myGLCD.print("Operation Teminated by",CENTER, 130);
  //myGLCD.print("User's Request",CENTER, 150);
  //myGLCD.print("Exiting Operation in",CENTER, 190);
  //myGLCD.print("Second",CENTER, 230);

   //for(int timer = 5;timer>0;timer--)
   //{
    
    //myGLCD.print(String(timer),CENTER, 210);
    
    //delay(200000);
    
   //}
   //myGLCD.clrScr();
   //myGLCD.print("^ ^",CENTER, 170);
   //myGLCD.print("Good bye",CENTER, 130);
   //delay(40000);
   //myGLCD.clrScr();
   //exit(0);
    }

//void updateLCD(int count){
  
  //myGLCD.print(String(batt_curr[count]),300, 10);
  //myGLCD.print(String(batt_volt[count]), 300, 40);
  //myGLCD.print(String(solar_power[count]), 210, 70);
  //myGLCD.print(String(soc[count]*100), 300, 100);
  //myGLCD.print(String(dod[count]*100), 340, 130);
  //myGLCD.print(String(soh[count]), 300, 160);
  //myGLCD.print(String(batt_temp[count]), 350, 190);
 
  //myGLCD.print(error_message[count], CENTER, 250);
  
  //delay(100);
  //myGLCD.setColor(255,255,255);
  
//}

void updateESP8266(int count){
  
  String volt = (String)(batt_volt[count]);
  String curr = (String)(batt_curr[count]);
  String pv_power = (String)(solar_power[count]);
  String soc_t = (String)(soc[count]*100);
  String dod_t = (String)(dod[count]*100);
  String soh_t = (String)(soh[count]);
  String temp = (String)(batt_temp[count]);
  
  Serial1.println(volt+"a"+curr+"b"+pv_power+"c"+soc_t+"d"+dod_t+"e"+soh_t+"f"+temp+"t"+error_message[count]+"s");
  delay(100);
 }

 void stopOperationISR(int count) {
  if (error_flag[count]!=0){
    Serial.println("Solar power = "+(String)(solar_power[count])+" W");   
    Serial.println("SOC = "+(String)(soc[count]*100)+" %"+" "+"Battery Voltage = "+(String)(batt_volt[count])+" V");  
    Serial.println("DOD = "+(String)(dod[counter]*100)+" %"+" "+"SOH = "+(String)(soh[counter])+" "+"Battery Temperature = "+(String)(batt_temp[counter])+" degree");
    Serial.println("error flag = "+(String)(error_flag[counter])+"   "+"charge flag = "+(String)(charge_flag[counter])+"   "+"discharging flag = "+discharge_flag[counter]);
    Serial.println("Fetching Solar Time = "+(String)(delta_t1)+" ms");
    Serial.println("Fetching Battery Voltage Time = "+(String)(delta_t3)+" us");
    Serial.println("Total Fetching Time = "+(String)((delta_t3/1000)+delta_t1)+" ms");
    Serial.println("Execution Time = "+(String)(delta_t2)+" ms");
    Serial.println("Total Execution Time = "+(String)(delta_t2+delta_t3/1000+delta_t1)+" ms");
    Serial.println("----------------------------------------------------------------------------------");
    Serial.println();
    delay(200);
    exit(0);
  }else{
    
  }
 }

 void setup() {
     pinMode(LED_BUILTIN, OUTPUT);
  pinMode(53,OUTPUT);
  digitalWrite(53,HIGH); ///Vcc red
  pinMode(49,OUTPUT);


  digitalWrite(49,HIGH); // Vcc button
  pinMode(43,OUTPUT);
  digitalWrite(43,HIGH); // Vcc yellow
 
  pinMode(47,OUTPUT);
  digitalWrite(47,HIGH); // Vcc green

  
  pinMode(Button, INPUT);
  digitalWrite(Button,HIGH);
  attachInterrupt(digitalPinToInterrupt(Button), battISR,LOW); 

  pinMode(ESP_interupt, INPUT);
  digitalWrite(ESP_interupt,HIGH);
  attachInterrupt(digitalPinToInterrupt(ESP_interupt), battISR,LOW); 
  
  
  //myGLCD.InitLCD();
  //myGLCD.setFont(BigFont);
  Serial.begin(115200);
   Serial1.begin(115200);
  //Arduino_SoftSerial.begin(115200);
  // myGLCD.print("Battery Current = ", LEFT, 10);
 // myGLCD.print("A", 390, 10);

  //myGLCD.print("Battery Voltage = ", LEFT, 40);
  //myGLCD.print("V", 390, 40);

  //myGLCD.print("PV Power = ", LEFT, 70);
 // myGLCD.print("W", 320, 70);

  //myGLCD.print("State of Charge = ", LEFT, 100);
  //myGLCD.print("%", 390, 100);

  //myGLCD.print("Depth of Discharge = ", LEFT, 130);
  //myGLCD.print("%", 430, 130);

 // myGLCD.print("State of Health = ", LEFT, 160);
  //myGLCD.print("%", 370, 160);

  //myGLCD.print("Battery Temperature = ", LEFT, 190);
 // myGLCD.print("C", 400, 190);

 // myGLCD.print("Battery Status: ", LEFT, 220); 
     
}

void loop() {
  init_bms();

  if (Serial1.available()) {  // check if there is incoming serial data
    String inputString = Serial1.readStringUntil('\n');  // read the string until a newline character is received
    Serial.println(inputString);  // print the received string to the serial monitor
  }



  if(counter<sizeof(solar_sensor)/sizeof(int))
  {
     t1 = millis();
     eeprom_DataManagent(counter);
     t2 = millis();
     operation_Mode(counter);
     errorCheck(counter);
     battCurrCal(counter);
     battTempCal(counter);
     socCal(counter);
     updateESP8266(counter);
     //updateLCD(counter);
     updateLED(counter);     
     stopOperationISR(counter);    
     delay(1500); 
     t3 = millis();
     delta_t1 = t2-t1;
     delta_t2 = t3-t1;
     delta_t3 = t5-t4;
  //     digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
 // delay(1000);                      // wait for a second
  // digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
 // delay(1000);  
     
    
     Serial.println("Solar power = "+(String)(solar_power[counter])+" W"+" "+"Battery Voltage = "+(String)(batt_volt[counter])+" V");  
      Serial.println("SOC = "+(String)(soc[counter]*100)+" %"+" "+"Battery Current = "+(String)(batt_curr[counter]));  
    Serial.println("DOD = "+(String)(dod[counter]*100)+" %"+" "+"SOH = "+(String)(soh[counter])+" "+"Battery Temperature = "+(String)(batt_temp[counter])+" degree");
      Serial.println("error flag = "+(String)(error_flag[counter])+"   "+"charge flag = "+(String)(charge_flag[counter])+"   "+"discharging flag = "+discharge_flag[counter]);
      Serial.println("Fetching Solar Time = "+(String)(delta_t1)+" ms");
      Serial.println("Fetching Battery Voltage Time = "+(String)(delta_t3)+" us");
      Serial.println("Total Fetching Time = "+(String)(delta_t3/1000+delta_t1)+" ms");
      Serial.println("Execution Time = "+(String)(delta_t2)+" ms");
     Serial.println("Total Execution Time = "+(String)(delta_t2+delta_t3/1000+delta_t1)+" ms");
      Serial.println("----------------------------------------------------------------------------------");
      Serial.println();
                                                                                                                                                      counter++;
  } else{
       counter = 0;
       exit(0);
  }
 
}
