//Ubicacion
#include <LGPS.h>
#include <stdio.h>
//Fecha y almacenamiento de datos
#include <LCFile.h>
#include <LDateTime.h>
#include <LBattery.h>
datetimeInfo t; //Variable para establecer la fecha
int meses[12]= {31,28,31,30,31,30,31,31,30,31,30,31};
unsigned char *utc_date_time_i = 0; //Establece la comunicacion inicial con el GPS
//Pulsadores
#define GPIO_46 E4 //Boton 
//LED
#define LEDR  15 //GPIO_50  
#define LEDG  A3 //GPIO_02
#define LEDB  5 //GPIO_19 
#define    LED_B    16 //Rephone
#define    LED_G    17 //Rephone
#define    LED_R    18 //Rephone
#define    LED_ON     1
#define    LED_OFF    0
//Var
float lat = 0; //Float para ir almacenando la latitud provieniente del GPS
float lng = 0; //Float para ir almacenando la longitud proveniente del GPS
float latprevia = 0; //Float para almacenar la ultima latitud valida
float lngprevia = 0; //Float para almacenr la ultima longitud valida
char status; //Filtro para determinar la data correcta
int bat; //Valor actual de bateria del RePhone
double sat; //Numero de satelites captados
int fecha=0; //Bandera para verificar si se configuró la fecha
int ubicacion=0; //Bandera para verificar si se adquirió una ubicacion correcta
int reset=0; //En caso no responda el chip o el GPS
int data_incorrecta=1; //Contador de datos incorrectos 
float latcent = -12.023662;   // -3.427623; 
float lngcent = -77.056754;  // -73.318614; 
double radio = 100;  //2000;
int frecuencia=1;  //6;
double desplazamiento = 1500;
//Datos extraidos del txt id
char estado; //Avisa si la persona sale del perimetro
int id;
void setup() {
    attachInterrupt(GPIO_46,interruptServer,FALLING);
    pinMode(LEDG, OUTPUT); 
    pinMode(LEDR, OUTPUT); 
    pinMode(LEDB, OUTPUT);
    pinMode(LED_B,OUTPUT);
    pinMode(LED_G,OUTPUT);
    pinMode(LED_R,OUTPUT);
    Serial.begin(9600);
    //Apago el led RGB
    digitalWrite(LEDG, LED_OFF);
    digitalWrite(LEDR, LED_OFF);
    digitalWrite(LEDB, LED_OFF);
    //Apago los leds del RePhone
    digitalWrite(LED_B,1);
    digitalWrite(LED_G,1);
    digitalWrite(LED_R,1);
    //Extraer data de los .txt
      char readdata[100] = {0};
      char auxdata[10]={0};
     //Extraer data de id
      LFile.Read("id.txt",readdata,100,0);
      for(int i=0; readdata[i] != ' ' && readdata[i] != '\r'  ;i++)
      {
        auxdata[i]=readdata[i];
      }
      id = atof(auxdata);
      memset(auxdata,0,sizeof(auxdata));
      for(int i=0; i<sizeof(readdata);i++)
      {
        if(readdata[i] == ':')
        { 
          if(readdata[i+1] == ' ') i = i+1;
          estado = readdata[i+1];
        }
      }
      memset(readdata,0,sizeof(readdata));
    digitalWrite(LEDG, LED_ON);
    delay(5000);
    digitalWrite(LEDG,LED_OFF); 
    ///Configurar Hora y fecha al arranque
    if (LGPS.check_online())
    {
      utc_date_time_i = LGPS.get_utc_date_time();
      t.year  = utc_date_time_i[0];
      if((int)t.year > 17 && (int)t.year<30) 
      {
        t.mon   = utc_date_time_i[1];
        t.day   = utc_date_time_i[2];
        t.hour  =  utc_date_time_i[3] - 5;
        if(t.hour < 0) 
        {
          t.hour = t.hour+24;
          if(t.day == 1)
          {
            t.mon=t.mon-1;
            t.day=meses[t.mon-1];
          }
          else t.day = t.day-1;
        }
        t.min   =  utc_date_time_i[4];
        t.sec   = utc_date_time_i[5];
        fecha = 1;
        LDateTime.setTime(&t);
        LDateTime.getTime(&t);
        
        lat = LGPS.get_latitude() * (-1);
        lng = LGPS.get_longitude();     
        lng = LGPS.get_longitude()*(-1);
        status = LGPS.get_status();
        sat = LGPS.get_sate_used();
        if(status == 'A' && lat != 0 && lng != 0)
        { 
          if(abs(lat)>90) lat = lat/100;
          if(abs(lng)>180) lng = lng/100; 
          double largo = 0;
          double ancho = 0;
          double distancia = 0;
          char writedata[200] = {0};
          largo= abs(lat - latcent)*111120;
          ancho= abs(lng - lngcent)*111120;
          distancia = sqrt(pow(largo,2)+pow(ancho,2));
          if(distancia < radio)//Si el primer dato esta dentro del perimetro
          {
              ubicacion = 1;
              latprevia = lat;
              lngprevia = lng;
                //Almaceno la primera data valida
                bat = LBattery.level();
                sprintf((char*)writedata, "Fecha:%d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f Bateria:%i Satelites:%.1f Status:%c ID:%i\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,bat,sat,status,id);
                LFile.Write("ubicaciones.txt", writedata);
                estado = 'D';
                sprintf((char*)writedata,"%i",id);
                LFile.Create("id.txt");
                LFile.Write("id.txt", writedata);
                LFile.Write("id.txt","\r\nEstado: Dentro del perimetro\r\n");
                delay(1000*60*frecuencia); 
          }
          else 
          {
            lng = lng + 0.667 ;
            largo= abs(lat - latcent)*111120;
            ancho= abs(lng - lngcent)*111120;
            distancia = sqrt(pow(largo,2)+pow(ancho,2));
            if(distancia < radio)//Si el primer dato es dentro del perimetro
            {
              ubicacion = 1;
              latprevia = lat;
              lngprevia = lng;
                //Almaceno la primera data valida
                 bat = LBattery.level();
                sprintf((char*)writedata, "Fecha:%d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f Bateria:%i Satelites:%.1f Status:%c ID:%i\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,bat,sat,status,id);
                LFile.Write("ubicaciones.txt", writedata);
                estado = 'D';
                sprintf((char*)writedata,"%i",id);
                LFile.Create("id.txt");
                LFile.Write("id.txt", writedata);
                LFile.Write("id.txt","\r\nEstado: Dentro del perimetro\r\n");
                delay(1000*60*frecuencia); 
            }
            else ubicacion = 0;
          }
        }
        else 
        {
          ubicacion = 0;
        }
      }
      else fecha=0;
     }
}

void loop() {
  while(fecha == 0) //Intenta hasta establecer la fecha
  {
    if (LGPS.check_online())
    {
      utc_date_time_i = LGPS.get_utc_date_time();
      t.year  = utc_date_time_i[0];
      if((int)t.year > 17 && (int)t.year < 30) 
      {
        t.mon   = utc_date_time_i[1];
        t.day   = utc_date_time_i[2];
        t.hour  =  utc_date_time_i[3] - 5;
        if(t.hour < 0) 
        {
          t.hour = t.hour+24;
          if(t.day == 1)
          {
            t.mon=t.mon-1;
            t.day=meses[t.mon-1];
          }
          else t.day = t.day-1;
        }
        t.min   =  utc_date_time_i[4];
        t.sec   = utc_date_time_i[5];
        fecha = 1;
        LDateTime.setTime(&t);
      }
      else 
      {
        fecha=0;
      }
    }
    delay(500);
  }
  
  if(ubicacion == 1) //Dispositivo funcionando
  {
    GPS();
  } 
  while(ubicacion == 0)
  {
      if (LGPS.check_online())
      {
        LDateTime.getTime(&t);
        lat = LGPS.get_latitude() * (-1);
        lng = LGPS.get_longitude();
        lng = LGPS.get_longitude()*(-1);
        status = LGPS.get_status();
        sat = LGPS.get_sate_used();
        ///
        char writedata[200] = {0};
        bat = LBattery.level();
        sprintf((char*)writedata, "Fecha:%d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f Bateria:%i Satelites:%.1f Status:%c ID:%i\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,bat,sat,status,id);
        Serial.println(writedata);    
        ///
        if(status == 'A' && lat != 0 && lng != 0)
        { 
          if(abs(lat)>90) lat = lat/100;
          if(abs(lng)>180) lng = lng/100; 
          double largo = 0;
          double ancho = 0;
          double distancia = 0;
          char writedata[200] = {0};
          largo= abs(lat - latcent)*111120;
          ancho= abs(lng - lngcent)*111120;
          distancia = sqrt(pow(largo,2)+pow(ancho,2));
          if(distancia < radio)//Si el primer dato esta dentro del perimetro
          {
              ubicacion = 1;
              latprevia = lat;
              lngprevia = lng;
                //Almaceno la primera data valida
                bat = LBattery.level();
                sprintf((char*)writedata, "Fecha:%d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f Bateria:%i Satelites:%.1f Status:%c ID:%i\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,bat,sat,status,id);
                LFile.Write("ubicaciones.txt", writedata);
                estado = 'D';
                sprintf((char*)writedata,"%i",id);
                LFile.Create("id.txt");
                LFile.Write("id.txt", writedata);
                LFile.Write("id.txt","\r\nEstado: Dentro del perimetro\r\n");
          }
          else 
          {
            lng = lng + 0.667 ;
            largo= abs(lat - latcent)*111120;
            ancho= abs(lng - lngcent)*111120;
            distancia = sqrt(pow(largo,2)+pow(ancho,2));
            if(distancia < radio)//Si el primer dato es dentro del perimetro
            {
              ubicacion = 1;
              latprevia = lat;
              lngprevia = lng;
                //Almaceno la primera data valida
                bat = LBattery.level();
                sprintf((char*)writedata, "Fecha:%d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f Bateria:%i Satelites:%.1f Status:%c ID:%i\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,bat,sat,status,id);
                LFile.Write("ubicaciones.txt", writedata);
                estado = 'D';
                
                sprintf((char*)writedata,"%i",id);
                LFile.Create("id.txt");
                LFile.Write("id.txt", writedata);
                LFile.Write("id.txt","\r\nEstado: Dentro del perimetro\r\n");
            }
            else 
            {
                ubicacion = 0;
            }
          }
        }
        else 
        {
          ubicacion = 0;
        }
      }
      else ubicacion = 0;
      delay(500);
  }
  delay(1000*60*frecuencia); 
}
void interruptServer()
{
  if(fecha == 0 && ubicacion == 0)
  {
      digitalWrite(LEDG, LED_ON);
      digitalWrite(LEDR, LED_ON);
      digitalWrite(LEDB, LED_ON);
      delay(3000); //Encendido 3 segundos 
      digitalWrite(LEDR, LED_OFF);
      digitalWrite(LEDG, LED_OFF);
      digitalWrite(LEDB, LED_OFF);
  }
  else if(fecha == 1 && ubicacion == 1) //Dispositivo funcionando
  {
     if(estado == 'F') //Persona salió del perimetro
     {
       digitalWrite(LEDB, LED_OFF);
       digitalWrite(LEDG, LED_OFF);
       digitalWrite(LEDR, LED_ON);
       delay(3000); //Encendido 3 segundos 
       digitalWrite(LEDR, LED_OFF);
     }
     else if(estado == 'D') //Persona no salió del perimetro
     { 
       digitalWrite(LEDR, LED_OFF);
       digitalWrite(LEDG, LED_ON);
       digitalWrite(LEDB, LED_OFF);
       delay(3000); //Encendido 3 segundos 
       digitalWrite(LEDG, LED_OFF);
    }
  }
  else if(fecha == 1)
  {
       digitalWrite(LEDG, LED_OFF);
       digitalWrite(LEDR, LED_OFF);
       digitalWrite(LEDB, LED_ON);
       delay(3000); //Encendido 3 segundos 
       digitalWrite(LEDB, LED_OFF);
  }
}

void GPS()
{
  char writedata[200] = {0};
  double largo = 0;
  double ancho = 0;
  double distancia = 0;
  reset = 0;
  while (LGPS.check_online() != 1) //Hasta que obtengas respuesta del GPS
  {
    reset=reset+1;
    delay(100);
    if(reset == 100)
    {  
      break;
    }
  }
  if(reset == 100) //Watchdog
  { 
    LDateTime.getTime(&t);
    bat = LBattery.level();
    sprintf((char*)writedata, "GPS no enciende hoy %d-%d-%i  %d:%d:%d Bateria:%i ID:%i\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec,bat,id);
    LFile.Write("ubicaciones.txt", writedata);
  }
  else
  {
    for(int i=0;i<10;i++)
    {
      //
      lat = LGPS.get_latitude() * (-1);
      lng = LGPS.get_longitude();
      lng = LGPS.get_longitude() * (-1);
      status = LGPS.get_status();
      sat = LGPS.get_sate_used(); 
      if(status == 'A' && lat != 0 && lng != 0)
      { 
        if(abs(lat)>90) lat = lat/100;
        if(abs(lng)>180) lng = lng/100; 
        largo= abs(lat - latprevia)*111120;
        ancho= abs(lng - lngprevia)*111120;
        distancia = sqrt(pow(largo,2)+pow(ancho,2));
        if(distancia < desplazamiento*frecuencia*data_incorrecta)//Data valida en base a la ultima data aceptada
        {
          latprevia = lat;
          lngprevia = lng;
          data_incorrecta = 1;
          break;
        }
        else
        {
          lng = lng + 0.667;
          largo= abs(lat - latprevia)*111120;
          ancho= abs(lng - lngprevia)*111120;
          distancia = sqrt(pow(largo,2)+pow(ancho,2));
          if(distancia < desplazamiento*frecuencia*data_incorrecta)//Data valida en base a la ultima data aceptada
          {
            latprevia = lat;
            lngprevia = lng;
            data_incorrecta = 1;
            break;
          }
          else
          {
            if(i == 9) data_incorrecta++;
          }
        }
      }
      else 
      {
        if(i == 9) data_incorrecta++;
      }  
      delay(500);
      //
    }
    LDateTime.getTime(&t);
    if(data_incorrecta == 1)//Si recibi un dato correcto
    { 
      bat = LBattery.level();
      sprintf((char*)writedata, "Fecha:%d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f %Bateria:%i Satelites:%.1f Status:%c ID:%i\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,bat,sat,status,id);
       LFile.Write("ubicaciones.txt", writedata);  
       Analizar_Data_Perimetro(lat,lng);
    }
    else 
    {
      bat = LBattery.level();
      sprintf((char*)writedata, "Mala cobertura fecha:%d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f Bateria:%i Satelites:%.1f Status:%c ID:%i\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,bat,sat,status,id);
      LFile.Write("ubicaciones.txt", writedata);
    }
  }
  reset=0;
}
void Analizar_Data_Perimetro(float lat,float lng)
{
     double largo = 0;
     double ancho = 0;
     double distancia = 0;
     char writedata[200] = {0};
     //Algoritmo de deteccion salida perimetro
     largo= abs(lat - latcent)*111120;
     ancho= abs(lng - lngcent)*111120;
     distancia = sqrt(pow(largo,2)+pow(ancho,2));
     if(distancia >= radio) 
     {
      estado = 'F';
      sprintf((char*)writedata,"%i",id);
      LFile.Create("id.txt");
      LFile.Write("id.txt", writedata);
      LFile.Write("id.txt","\r\nEstado: Fuera del perimetro\r\n");
     }
}

