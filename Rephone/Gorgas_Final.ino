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
int encendido=0; //Bandera para verificar si esta en el rango de horas 
int reset=0; //En caso no responda el chip o el GPS
int data_incorrecta=1; //Contador de datos incorrectos
//Datos extraidos del txt perimetro 
float latcent = 0; 
float lngcent = 0; 
double radio = 0;
//Datos extraidos del txt tiempo
int inicio_min=0;
int inicio_hor=0;
int fin_min=0;
int fin_hor=0;
int frecuencia=0;
double desplazamiento = 0;
//Datos extraidos del txt id
char estado; //Avisa si la persona sale del perimetro
char id[10] = {0};

void setup() {
    attachInterrupt(GPIO_46,interruptServer,FALLING);
    //Serial.begin(9600);
    pinMode(LEDG, OUTPUT); 
    pinMode(LEDR, OUTPUT); 
    pinMode(LEDB, OUTPUT);
    pinMode(LED_B,OUTPUT);
    pinMode(LED_G,OUTPUT);
    pinMode(LED_R,OUTPUT);
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
      char aux_datos[10] = {0}; //Maximo 10 digitos
     //Extraer data de id
      LFile.Read("id.txt",readdata,100,0);
      for(int i=0; readdata[i] != ' ' && readdata[i] != '\r'  ;i++)
      {
        id[i] = readdata[i];
      }
      for(int i=0; i<sizeof(readdata);i++)
      {
        if(readdata[i] == ':')
        { 
          if(readdata[i+1] == ' ') i = i+1;
          estado = readdata[i+1];
        }
      }
      memset(readdata,0,sizeof(readdata));
    //Extraer data de tiempo
      LFile.Read("tiempo.txt",readdata,100,0);
      for(int i=0;readdata[i] != 'm'  ;i++)
      {
        if(readdata[i] == ':')
        {
          int cuenta = 0;
          if(readdata[i+1] == ' ') i=i+1;
          for(int j=i+1;readdata[j] != 'm';j++)
          {
            aux_datos[cuenta]=readdata[j];
            cuenta=cuenta+1;
          }
          break;
        }
      }
      frecuencia = atol(aux_datos);
      memset(aux_datos,0,sizeof(aux_datos));
      for(int i=0; i<100;i++)
      {
        if(readdata[i] == 'h' && readdata[i+11] == ':')
        {
         if(readdata[i+12] == ' ')i=i+1;
         aux_datos[0] = readdata[i+12];
         aux_datos[1] = readdata[i+13];
         inicio_hor = atol(aux_datos);
         aux_datos[0] = readdata[i+15];
         aux_datos[1] = readdata[i+16];
         inicio_min = atol(aux_datos);
         memset(aux_datos,0,sizeof(aux_datos));
        }
        else if(readdata[i] == 'h' && readdata[i+8] == ':')
        {
          if(readdata[i+9] == ' ')i=i+1;
         aux_datos[0] = readdata[i+9];
         aux_datos[1] = readdata[i+10];
         fin_hor = atol(aux_datos);
         aux_datos[0] = readdata[i+12];
         aux_datos[1] = readdata[i+13];
         fin_min = atol(aux_datos);
         memset(aux_datos,0,sizeof(aux_datos));
        }
        else if(readdata[i] == 'r' && readdata[i+5] == ':')
        {
          if(readdata[i+6] == ' ')i=i+1;
          int cuenta = 0;
          for(int j=i+6;readdata[j] != 'm' && readdata[j] != ' ';j++)
          {
             aux_datos[cuenta] = readdata[j];
             cuenta++;
          }
          desplazamiento = atof(aux_datos);
          memset(aux_datos,0,sizeof(aux_datos));
          break;
        }
      }
      memset(readdata,0,sizeof(readdata));
     //Extraer de perimetro
      LFile.Read("perimetro.txt",readdata,100,0);
      int ayuda = 0;
      for(int i=0;readdata[i] != 'm'  ;i++) 
      {
        if(readdata[i] == '-')
        {
          int contador=0;
          memset(aux_datos,0,sizeof(aux_datos));
          for(int j=i; readdata[j]!= ',' && readdata[j] != ' ' && readdata[j] != '\r' && readdata[j] != '\n' ;j++)
          {
            aux_datos[contador] = readdata[j];
            contador = contador + 1;
          }
          ayuda = ayuda + 1;
          if (ayuda == 1) latcent = atof(aux_datos);
          else if (ayuda == 2){ lngcent = atof(aux_datos); ayuda = 0;}
        } 
        else if(readdata[i] == 'R')
        {
          int contador=0;
          memset(aux_datos,0,sizeof(aux_datos));
          for(int j=i+7; readdata[j]!= ' ' && readdata[j] != 'm';j++)
          {
            aux_datos[contador] = readdata[j];
            contador = contador + 1;
          }
          radio = atof(aux_datos);
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
        ////////////////////////////////////////////////////////////////////////AQUI INICIA EL CAMBIO
        if(inicio_hor < fin_hor)
        {
        	if(t.hour > inicio_hor) 
	  	{
			if(t.hour < fin_hor) encendido = 1;
			else if(t.hour > fin_hor) encendido = 0;
			else
			{
				if(t.min < fin_min) encendido = 1;
				else  encendido = 0;
			}
		}
		
		else if(t.hour < inicio_hor) encendido = 0;
		else
		{
			if(t.min >= inicio_min) encendido = 1;
			else  encendido = 0;
		}
        }

	else if(inicio_hor > fin_hor)
        {
        	if(t.hour > inicio_hor) encendido = 1;
		else if(t.hour < inicio_hor)
	  	{
			if(t.hour < fin_hor) encendido = 1;
			else if(t.hour > fin_hor) encendido = 0;
			else
			{
				if(t.min < fin_min) encendido = 1;
				else  encendido = 0;
			}
		}
		else
		{
			if(t.min >= inicio_min) encendido = 1;
			else  encendido = 0;
		}
        }

	else
	{
		if(inicio_min < fin_min)
		{
			if(t.hour != inicio_hor) encendido = 0;
			else
			{
				if(t.min < fin_min && t.min >= inicio_min) encendido = 1;
				else  encendido = 0;
			}
		}
		else
		{
			if(t.hour != inicio_hor) encendido = 1;
			else
			{
				if(t.min >= fin_min && t.min < inicio_min) encendido = 0;
				else  encendido = 1;
			}
		}
	}

        ////////////////////////////////////////////////////////////////////////AQUI ACABA EL CAMBIO
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
                sprintf((char*)writedata, "Fecha:%d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f Bateria:%i Satelites:%.1f Status:%c\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,bat,sat,status);
                LFile.Write("ubicaciones.txt", writedata);
                estado = 'D';
                LFile.Create("id.txt");
                LFile.Write("id.txt", id);
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
                sprintf((char*)writedata, "Fecha:%d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f Bateria:%i Satelites:%.1f Status:%c\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,bat,sat,status);
                LFile.Write("ubicaciones.txt", writedata);
                estado = 'D';
                LFile.Create("id.txt");
                LFile.Write("id.txt", id);
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
        //Serial.println("Fecha configurada");
        LDateTime.setTime(&t);
      }
      else 
      {
        fecha=0;
        //Serial.println("Esperando fecha");
      }
    }
    delay(500);
  }
  
  if(fecha == 1)
  {                         
    Verificar_Encendido();
  }
  if(encendido == 1 && ubicacion == 1) //Dispositivo funcionando
  {
    GPS();
  } 
  while(ubicacion == 0)
  {
      Verificar_Encendido();///
      if (LGPS.check_online())
      {
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
                sprintf((char*)writedata, "Fecha:%d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f Bateria:%i Satelites:%.1f Status:%c\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,bat,sat,status);
                LFile.Write("ubicaciones.txt", writedata);
                estado = 'D';
                LFile.Create("id.txt");
                LFile.Write("id.txt", id);
                LFile.Write("id.txt","\r\nEstado: Dentro del perimetro\r\n");
                //Serial.print("Sin desfase ");
                //Serial.print(writedata);
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
                sprintf((char*)writedata, "Fecha:%d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f Bateria:%i Satelites:%.1f Status:%c\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,bat,sat,status);
                LFile.Write("ubicaciones.txt", writedata);
                estado = 'D';
                LFile.Create("id.txt");
                LFile.Write("id.txt", id);
                LFile.Write("id.txt","\r\nEstado: Dentro del perimetro\r\n");
                //Serial.print("Con desfase ");
                //Serial.print(writedata);
            }
            else 
            {
                ubicacion = 0;
                //sprintf((char*)writedata, "Configurando A : %d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f Satelites:%.1f Status:%c\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,sat,status);
                //LFile.Write("ubicaciones.txt", writedata);
                //Serial.print("Data no valida ");
                //Serial.print(writedata);
            }
          }
        }
        else 
        {
          ubicacion = 0;
          //char writedata[200] = {0};
          //sprintf((char*)writedata, "Configurando V: %d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f Satelites:%.1f Status:%c\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,sat,status);
          //LFile.Write("ubicaciones.txt", writedata);
          //Serial.print("Data no valida ");
          //Serial.print(writedata);
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
       if(encendido == 1)///
       {//
       digitalWrite(LEDB, LED_OFF);
       digitalWrite(LEDG, LED_OFF);
       digitalWrite(LEDR, LED_ON);
       delay(3000); //Encendido 3 segundos 
       digitalWrite(LEDR, LED_OFF);
       } ///
       ////
       else { 
       digitalWrite(LEDB, LED_OFF);
       digitalWrite(LEDG, LED_OFF);
       digitalWrite(LEDR, LED_ON);
       delay(1000);
       digitalWrite(LEDB, LED_OFF);
       digitalWrite(LEDG, LED_OFF);
       digitalWrite(LEDR, LED_OFF);
       delay(1000);
       digitalWrite(LEDB, LED_OFF);
       digitalWrite(LEDG, LED_OFF);
       digitalWrite(LEDR, LED_ON);
       delay(1000); 
       digitalWrite(LEDR, LED_OFF);}
       ////

     }
     else if(estado == 'D') //Persona no salió del perimetro
     { 
       if(encendido == 1) ////
       {///
       digitalWrite(LEDR, LED_OFF);
       digitalWrite(LEDG, LED_ON);
       digitalWrite(LEDB, LED_OFF);
       delay(3000); //Encendido 3 segundos 
       digitalWrite(LEDG, LED_OFF);
       }///
        ////
       else { 
       digitalWrite(LEDB, LED_OFF);
       digitalWrite(LEDR, LED_OFF);
       digitalWrite(LEDG, LED_ON);
       delay(1000);
       digitalWrite(LEDB, LED_OFF);
       digitalWrite(LEDR, LED_OFF);
       digitalWrite(LEDG, LED_OFF);
       delay(1000);
       digitalWrite(LEDB, LED_OFF);
       digitalWrite(LEDR, LED_OFF);
       digitalWrite(LEDG, LED_ON);
       delay(1000); 
       digitalWrite(LEDG, LED_OFF);}
       ////
    }
  }
  else if(fecha == 1)
  {
        if(encendido == 1) ///
        { ///
       digitalWrite(LEDG, LED_OFF);
       digitalWrite(LEDR, LED_OFF);
       digitalWrite(LEDB, LED_ON);
       delay(3000); //Encendido 3 segundos 
       digitalWrite(LEDB, LED_OFF);
        }////
       ////
       else { 
       digitalWrite(LEDR, LED_OFF);
       digitalWrite(LEDG, LED_OFF);
       digitalWrite(LEDB, LED_ON);
       delay(1000);
       digitalWrite(LEDR, LED_OFF);
       digitalWrite(LEDG, LED_OFF);
       digitalWrite(LEDB, LED_OFF);
       delay(1000);
       digitalWrite(LEDR, LED_OFF);
       digitalWrite(LEDG, LED_OFF);
       digitalWrite(LEDB, LED_ON);
       delay(1000); 
       digitalWrite(LEDB, LED_OFF);}
       ////
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
    sprintf((char*)writedata, "GPS no enciende hoy %d-%d-%i  %d:%d:%d Bateria:%i\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec,bat);
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
      sprintf((char*)writedata, "Fecha:%d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f %Bateria:%i Satelites:%.1f Status:%c\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,bat,sat,status);
       LFile.Write("ubicaciones.txt", writedata);  
       Analizar_Data_Perimetro(lat,lng);
    }
    else 
    {
      bat = LBattery.level();
      sprintf((char*)writedata, "Mala cobertura fecha:%d-%d-%i  %d:%d:%d Latitud:%f Longitud:%f Bateria:%i Satelites:%.1f Status:%c\r\n",t.day, t.mon,(int)t.year - 48,t.hour, t.min, t.sec, lat, lng,bat,sat,status);
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
     //Algoritmo de deteccion salida perimetro
     largo= abs(lat - latcent)*111120;
     ancho= abs(lng - lngcent)*111120;
     distancia = sqrt(pow(largo,2)+pow(ancho,2));
     if(distancia > radio) 
     {
      estado = 'F';
      LFile.Create("id.txt");
      LFile.Write("id.txt", id);
      LFile.Write("id.txt","\r\nEstado: Fuera del perimetro\r\n");
     }
     else if(distancia <= radio && estado != 'F')
     {
        estado = 'D';
        LFile.Create("id.txt");
        LFile.Write("id.txt", id);
        LFile.Write("id.txt","\r\nEstado: Dentro del perimetro\r\n");
     }
}

void Verificar_Encendido()
{
  	/////////////////////////////////////////////////////////////////////////////////////////////AQUI INICIA EL CAMBIO
        LDateTime.getTime(&t);
        if(inicio_hor < fin_hor)
        {
        	if(t.hour > inicio_hor) 
	  	{
			if(t.hour < fin_hor) encendido = 1;
			else if(t.hour > fin_hor) encendido = 0;
			else
			{
				if(t.min < fin_min) encendido = 1;
				else  encendido = 0;
			}
		}
		
		else if(t.hour < inicio_hor) encendido = 0;
		else
		{
			if(t.min >= inicio_min) encendido = 1;
			else  encendido = 0;
		}
        }

	else if(inicio_hor > fin_hor)
        {
        	if(t.hour > inicio_hor) encendido = 1;
		else if(t.hour < inicio_hor)
	  	{
			if(t.hour < fin_hor) encendido = 1;
			else if(t.hour > fin_hor) encendido = 0;
			else
			{
				if(t.min < fin_min) encendido = 1;
				else  encendido = 0;
			}
		}
		else
		{
			if(t.min >= inicio_min) encendido = 1;
			else  encendido = 0;
		}
        }

	else
	{
		if(inicio_min < fin_min)
		{
			if(t.hour != inicio_hor) encendido = 0;
			else
			{
				if(t.min < fin_min && t.min >= inicio_min) encendido = 1;
				else  encendido = 0;
			}
		}
		else
		{
			if(t.hour != inicio_hor) encendido = 1;
			else
			{
				if(t.min >= fin_min && t.min < inicio_min) encendido = 0;
				else  encendido = 1;
			}
		}
	}

	///////////////////////////////////////////////////////////////////AQUI ACABA EL CAMBIO
}
