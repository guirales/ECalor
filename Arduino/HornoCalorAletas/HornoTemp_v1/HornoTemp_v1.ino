#include <NexText.h>
#include <Nextion.h>
#include <max6675.h>
#include <math.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <SD.h>
//General
int n, estado, Dt;
String nombreA;
int ti, tf, Dtf,Dtaux;


// Sensores infrarojos
Adafruit_MLX90614 mlx1 = Adafruit_MLX90614(0x50);
Adafruit_MLX90614 mlx2 = Adafruit_MLX90614(0x51);
Adafruit_MLX90614 mlx3 = Adafruit_MLX90614(0x52);
Adafruit_MLX90614 mlx4 = Adafruit_MLX90614(0x53);
float st1,st2,st3,st4;
//Nextion
SoftwareSerial nextion(17, 16);
uint32_t sm;
int tamN;
bool bandera1;
// SD
File logFile;
int pinSD, reintento;
String EntradaN,nombreArch;
char SalidaN[50];
bool bandera,banderaA;
//Esclavo
float TempA, TempE, Pot, ImA;
int est;

//INICIO
void setup() {
  //
  Dt = 1000;
  //
  Serial.begin(9600);
  Wire.begin();
  nexInit();
  mlx1.begin();
  mlx2.begin();
  mlx3.begin();
  mlx4.begin();
  sendCommand("page 0");//inicializa pantalla
  //SD
  pinSD = 7;
  reintento = 2;
  bandera = true;
  pinMode(pinSD, OUTPUT);//define pin selector de SD
  IniSD(pinSD);
  //Esclavo
  TempE = 20;
  Pot = 0;
  est = 1;//0 para potencia 1 para temperatura
  bandera=true;

}

void loop() {

  ti = millis();
  //tf = ti;
  Dtf = millis() - ti;
  
  while (Dtf <= Dt*1000) {
    lectoSensores();
    Esclavo();
    LectoEscrituraNextion();
    separador();
    if(Dt!=Dtaux) n=0;
    //Guardada en SD
    if(estado==1 ){
      if(bandera){
      n=0;
      bandera=false;
      }
      if(sm==1){
        
        GuardarSD();
        
        }
      
      }else{
        bandera=true;
        banderaA=true;
        }
      
    Serial.println("tam:" + String(tamN) + " :" + String(estado) + "," + nombreA + "," + String(Dt) + ", " + String(TempE));
    Serial.println("Viene de nextion: " + String(SalidaN));
    Serial.println("enviado a nextion: " + String(EntradaN));

    //Serial.println("Enviando: " + String(est) + ", " + String(pr1) + String(pr2) + String(pr3) + ", " + String(T1) + String(T2) + String(T3) + "." + String(T4) + "\n");
    //Serial.println("Reibido: " + String(Im1) + String(Im2) + String(Im3) + ". " + String(Im4) + ", " + String(TE1) + String(TE2) + String(TE3) + ". " + String(TE4) + "\n");
    
    Serial.println("Recibido esclavo TempA: " + String(TempA)+"ImA: " + String(ImA));
    Serial.println("Enviado esclavo est: " + String(est)+"potP: " + String(Pot)+"TempE: " + String(TempE));
    Serial.println("Dtf: " + String(Dtf));
    Dtaux=Dt;
    Dtf=millis()-ti;

  }
  n += 1;
}


/////////////////////////////////////////////////////////////
//FUNCIONES
//Memoria SD
void IniSD(int pinSD)
{

  Serial.println("Memoria:::" + String(SD.begin(pinSD)));
  while (reintento > 0) {
    delay(100);
    Serial.println("Memoria;;;: " + String(SD.begin(pinSD)));
    if (SD.begin(pinSD)) {
      // Serial.println("Memoria encontrada: " + String(SD.begin(7)));

      reintento = 0;
      sm = 1;
      delay(50);

    } else {
      if (bandera) {
        sendCommand("page 2");
        Serial.print("Se envia page 2");
        bandera = false;
      } else {
        Serial.println("Memoria no encontrada");
        sendCommand("get Error.sm2.val");
        recvRetNumber(&sm);
        Serial.println("valor sm2: " + String(sm) + "reintento: " + String(reintento));
        if (sm == 0) {
          reintento = 0;
        }
        delay(50);
      }
    }


  }
  sendCommand("page 1");
}

//Envio y recupercion info pantalla
void LectoEscrituraNextion(void)
{
  memset(SalidaN, 0, sizeof(SalidaN));
  sendCommand("get Medida.Salida.txt");
  recvRetString(SalidaN, 50);
  //
  delay(80);
  EntradaN = "Medida.Entrada.txt=\"" + String(Dt * n) + "," + String(n) +","+ String(TempA)+","+String(ImA)+",40.1,55.2,60.3,70.1; \"";
  sendCommand(EntradaN.c_str());

}

void separador() {

  int i, n;
  String Taux;
  n = 1;
  Taux = "";
  tamN = String(SalidaN).length();
  for (i = 0; i <= tamN; i++) {

    if (SalidaN[i] == ',') {
      //Serial.println(String(n));
      if (n == 1) {
        estado = Taux.toInt();
        Taux = "";
      }
      else if (n == 2)
      { nombreA = Taux;
        Taux = "";
      }
      else if (n == 3) {
        Dt = Taux.toInt();
        Taux = "";
      }
      n += 1;

    } else {
      Taux += SalidaN[i];

    }

  }
  TempE = Taux.toInt();
}

void Esclavo() {
  int pr1, pr2, pr3, T1, T2, T3, T4;
  int Im1, Im2, Im3, Im4, TE1, TE2, TE3, TE4;

  Wire.beginTransmission(2);

  pr1 = int(Pot / 100) % 10;
  pr2 = int(Pot / 10) % 10;
  pr3 = int(Pot) % 10;

  T1 = int(TempE / 100) % 10;
  T2 = int(TempE / 10) % 10;
  T3 = int(TempE ) % 10;
  T4 = int(TempE * 10) % 10;
  Serial.println("EN FUNCION:"+String(est));
  Wire.write(est);//0 para potencia 1 para temperatura
  Wire.write(pr1);
  Wire.write(pr2);
  Wire.write(pr3);
  Wire.write(T1);
  Wire.write(T2);
  Wire.write(T3);
  Wire.write(T4);
  Wire.endTransmission();

  delay(50);
  Wire.requestFrom(2, 8);
  Im1 = int(Wire.read());
  Im2 = int(Wire.read());
  Im3 = int(Wire.read());
  Im4 = int(Wire.read());
  TE1 = int(Wire.read());
  TE2 = int(Wire.read());
  TE3 = int(Wire.read());
  TE4 = int(Wire.read());
  delay(50);
  ImA=Im1*100+Im2*10+Im3+Im4*0.1;
  TempA=TE1*100+TE2*10+TE3+TE4*0.1;

}
//Guardar SD
void GuardarSD() {
  String lineaArch;
    nombreArch=nombreA+".CVS";
    logFile = SD.open(nombreArch, FILE_WRITE);
    //Serial.println(logFile);

    if (logFile) {
      // Serial.println("==========INICIA ESCRITURA========");
      if (banderaA) {
        lineaArch = "Tiempo(s),Temp. Esperada(°C),Temp. Actual(°C),Pot. Actual(W),Temp S1(°C),Temp S2(°C),Radiometro(W/m2),Luxometro(Lux)";
        logFile.println(lineaArch);
        logFile.close();
        banderaA = false;
      } else {

        lineaArch = String(Dt*n) + "," + String(TempE) + "," + String(TempA) + "," + String(ImA) + "," + String(st1) + "," + String(st2) + "," + String(st3) + "," + String(st4);
        logFile.println(lineaArch);
        logFile.close();
        sendCommand("g1.txt=\"\"");
      }
      // Serial.println(lineaArch);
    } else {
      // Serial.println("==========No hay memoria========");
      sendCommand("g1.txt=\" No se ha podido guardar los datos compruebe que hay memoria o que no está dañada\"");
    }



}
void lectoSensores(){
  
  st1 = mlx1.readObjectTempC();
  st2 = mlx2.readObjectTempC();
  st3 = mlx3.readObjectTempC();
  st4 = mlx4.readObjectTempC();
  
  
  }
