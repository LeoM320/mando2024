/**
 * @file main.cpp
 * @author MEOLANS, Leandro Tomás (meolansleandrotomas@outlook.com)
 * @brief Firmware mando para tablero de voley
 * @version 0.2
 * @date 13-12-2024
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <Arduino.h>
#define VERSION "13-12-2024-MANDO-V0.2"

typedef struct {
  const uint8_t PIN;
  const uint8_t CHR;
}s_Boton;

const s_Boton DEF_BUZ = {A0,' '}; //Buzzer
const s_Boton DEF_LED = {13,' '}; //Led
const s_Boton DEF_SPL = {12,'a'}; //Sumar puntos local
const s_Boton DEF_RPL = {11,'b'}; //Restar puntos local
const s_Boton DEF_SPV = {10,'c'}; //Sumar puntos visitante
const s_Boton DEF_RPV = { 9,'d'}; //Restar puntos visitante
const s_Boton DEF_SSL = { 8,'g'}; //Setear saque local
const s_Boton DEF_SSV = { 7,'h'}; //Setear saque visitante
const s_Boton DEF_INI = { 6,'k'}; //Continuar
const s_Boton DEF_INV = { 5,'i'}; //Invertir
const s_Boton DEF_RST = { 4,'j'}; //Reset
const s_Boton DEF_SIR = { 3,'f'}; //Sirena On
const s_Boton DEF_INT = { 2,' '}; //Interrupcion
const s_Boton DEF_SIR_OFF = { 0,'e'}; //Sirena Off

#define CST_RETARDO 700 //Tiempo base de retardo entre pulsaciones iterativas
#define CST_FILTRO 120 //Tiempo para evitar rebotes en hardware
#define CST_BEEP 20 //Duración del buzzer
#define CST_FCT_ACCEL 0.5 //Factor de aceleración
#define CST_LIM_ACCEL 50 //Retardo mínimo reducido por la aceleración

uint8_t botonPulsado=0; //Bandera de pulsación
/**
 * @brief Contador de iteraciones de pulsación.
 * Se reinicia en 0 al soltar el botón
 */
uint8_t contador=0;
uint8_t sonando=0; //Bandera de buzzer activo
float retardo=0; //Retardo aceleración
unsigned long tiempoFuturo=0; //ms hasta la proxima pulsación
/**
 * @brief ms de la pulsación anterior 
 * Se usa para aplicar un filtro entre pulsaciones.
 * El minimo tiempo umbral es CST_FILTRO
 */
unsigned long tiempoAnterior=0;
unsigned long tiempoBeep=0; //ms para control de beeps

/**
 * @brief Función de interrupción en CHANGE.
 * Como Arduino no admite 2 interrupciones en el mismo pin
 * y se necesitaba un RISING y FALLING se usa CHANGE y se
 * diferencia el caso con el nivel lógico del pin de interrupción
 */
void Pulsado(){
  if(!digitalRead(DEF_INT.PIN)){
    /*
      RISING.
      Se establece tiempoFuturo usando CST_FILTRO.
      Se enciende el led de estado y el botón se
      declara pulsado.
    */
    tiempoFuturo=millis()+CST_FILTRO;
    digitalWrite(DEF_LED.PIN,HIGH);
    botonPulsado=1;

  }else{
    /*
      FALLING.
      Reinicio de variables a estado inicial cuando se
      suelta un pulsador.
    */
    digitalWrite(DEF_LED.PIN,LOW);
    botonPulsado=0;
    contador=0;
    Serial.write(DEF_SIR_OFF.CHR);
  }
}

/**
 * @brief Función de buzzer.
 * Enciende el buzzer y lo declara sonando.
 * Se establece el tiempo para su respectivo
 * apagado.
 */
void Beep(){
  tiempoBeep=millis()+CST_BEEP;
  digitalWrite(DEF_BUZ.PIN,HIGH);
  sonando=1;
}

/**
 * @brief Envío de caracter por puerto serie.
 * Cuando se dan las condiciones de pulsación en el
 * pin de interrupción se verifica el estado de los botones.
 * Se enviará un caracter y se emitirá un pitido si fuera pertinente.
 * Si es de uso repetitivo se enviarán sucesivamente en retardos
 * que reducirán según contador en términos de una aceleración.
 * Los repetitivos son aquellos en los que no se verifique que contador
 * sea 0 (primera pulsación).
 * @param contador 
 */
void EnviarPulsacion(uint8_t contador){
       if (digitalRead(DEF_SPL.PIN)) { Serial.write(DEF_SPL.CHR); Beep(); }
  else if (digitalRead(DEF_RPL.PIN)) { Serial.write(DEF_RPL.CHR); Beep(); }
  else if (digitalRead(DEF_SPV.PIN)) { Serial.write(DEF_SPV.CHR); Beep(); }
  else if (digitalRead(DEF_RPV.PIN)) { Serial.write(DEF_RPV.CHR); Beep(); }
  
  if(!contador){
      if (digitalRead(DEF_SSL.PIN)) { Serial.write(DEF_SSL.CHR); Beep(); }
    else if (digitalRead(DEF_SSV.PIN)) { Serial.write(DEF_SSV.CHR); Beep(); }
    else if (digitalRead(DEF_INI.PIN)) { Serial.write(DEF_INI.CHR); Beep(); }
    else if (digitalRead(DEF_INV.PIN)) { Serial.write(DEF_INV.CHR); Beep(); }
    else if (digitalRead(DEF_RST.PIN)) { Serial.write(DEF_RST.CHR); Beep(); }
    else if (digitalRead(DEF_SIR.PIN)) { Serial.write(DEF_SIR.CHR); Beep(); }
  }
}

/**
 * @brief Función de aceleración.
 * Para valores de contador se disminuirá el retardo
 * siguiendo una curva hiperbólica. r/(x+1)^a
 * @param contador
 * @return float 
 */
float Retardo(uint8_t contador){
  retardo = CST_RETARDO/pow(contador*1.0+1,CST_FCT_ACCEL);
  if(retardo>=CST_LIM_ACCEL){
    return retardo;
  }else{
    return CST_LIM_ACCEL;
  }
}

void setup() {
  pinMode(DEF_BUZ.PIN,OUTPUT);
  pinMode(DEF_LED.PIN,OUTPUT);
  pinMode(DEF_SPL.PIN,INPUT);
  pinMode(DEF_RPL.PIN,INPUT);
  pinMode(DEF_SPV.PIN,INPUT);
  pinMode(DEF_RPV.PIN,INPUT);
  pinMode(DEF_SSL.PIN,INPUT);
  pinMode(DEF_SSV.PIN,INPUT);
  pinMode(DEF_INI.PIN,INPUT);
  pinMode(DEF_INV.PIN,INPUT);
  pinMode(DEF_RST.PIN,INPUT);
  pinMode(DEF_SIR.PIN,INPUT);
  pinMode(DEF_INT.PIN,INPUT);
  attachInterrupt(digitalPinToInterrupt(DEF_INT.PIN), Pulsado, CHANGE);
  Serial.begin(9600);
  Serial.println(VERSION);
}

void loop(){
  if(millis()>=tiempoFuturo&&botonPulsado){
    /*
      BOTÓN PULSADO.
      ¿Será legítimo?
    */
    if(!(millis()-tiempoAnterior<=CST_FILTRO)){
      /*
        Es legítimo.
      */
      EnviarPulsacion(contador);
      /*
        Limite al contador por tratarse de un uint8_t.
      */
      if(contador<=254){
        contador++;
      }
      tiempoAnterior=tiempoFuturo;
      tiempoFuturo=millis()+Retardo(contador);
    }
  }
  /**
   * Se apaga el buzzer tras el tiempo especificado.
   */
  if(millis()>=tiempoBeep&&sonando){
      digitalWrite(DEF_BUZ.PIN,LOW);
      sonando=0;
  }
}