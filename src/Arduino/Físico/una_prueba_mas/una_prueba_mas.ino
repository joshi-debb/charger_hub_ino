#include <LiquidCrystal.h>



int tiempo = 0;
int distancia = 0;
int dimension = 0;
bool esta_abierta = true;
int estado = 0;
int trig = 7;
int echo = 8;
int trig2 = 2;
int echo2 = 13;
int telefono1 = 0;
int telefono2 = 0;
int turno1 = 0;
int turno2 = 0;

const int fotoresistorPin = A0;
const int fotoresistorPin2 = A1; 



LiquidCrystal lcd(12, 11, 5, 4, 3, 6);

void setup() {
  Serial.begin(9600); 
  lcd.begin(16, 2);
  pinMode(trig,OUTPUT);
  pinMode(echo,INPUT);
  pinMode(trig2,OUTPUT);
  pinMode(echo2,INPUT);
  lcd.setCursor(0,0);
  lcd.print("hola");
}

void loop() {
  Serial.println("PASE");
  if(luz(0)){
    disparar_distancia();
  }
  if(luz(1)){
    disparar2();
  }
  Serial.println("Telefono: ");
  Serial.println(telefono2);
  delay(300);
}

void disparar_distancia(){
  //Determinamos cual sensor ultrasonico usar
    digitalWrite(trig,HIGH);
    digitalWrite(trig,LOW);
    tiempo = pulseIn(echo,HIGH);
  //Se realizan las mediciones
  //Serial.println(tiempo);  
  int distancia = tiempo/58.2;
    
  if(distancia > 20){
    mensaje2(0);
  }
  //Se verifica la presencia y tambien se determina la medicion de telefono inicial
  if(distancia < 20){
    //Serial.println("Hay dispositivo!"); 
    //Serial.println("Distancia: "); 
    //Serial.println(distancia); 
    limpiar(0);
    mensaje4(0);
    if(turno1 == 0){
      telefono1 = distancia;
      turno1 ++;
    }
  //Se verifica que sea igual al dispositivo inicial
    if(telefono1 != distancia || telefono1 == (distancia-1) || telefono1 == (distancia+1)){
     limpiar(0);
     mensaje3(0);
    }
  }else{
    Serial.println("NO Hay dispositivo!"); 
  }
  delay(200);

}

void disparar2(){
  digitalWrite(trig2,HIGH);
  digitalWrite(trig2,LOW);
  tiempo = pulseIn(echo2,HIGH);    
  int distancia = tiempo/58.2;
  dimension = 15 - distancia;
  //Se verifica la presencia y tambien se determina la medicion de telefono inicial
  if(distancia < 20){
    //Serial.println("Hay dispositivo!"); 
    //Serial.println("Distancia: "); 
    //Serial.println(distancia); 
    limpiar(1);
    mensaje4(1);
    if(turno2 == 0 && distancia != 0){
      telefono2 = distancia;
      turno2 ++;
    }
  //Se verifica que sea igual al dispositivo inicial

    
    if(telefono2 != distancia){
     limpiar(1);
     mensaje3(1);  
    }
  
  }else{
    Serial.println("NO Hay dispositivo!"); 
  }

}

void mensaje1(int pos){

  lcd.setCursor(0, pos);
  lcd.print("Puerta Abierta");
  Serial.println("Abierta");
}

void mensaje2(int pos){
  lcd.setCursor(0, pos);
  lcd.print("Vacia");
  Serial.println("Vacia");
}

void mensaje3(int pos){
  lcd.setCursor(0, pos);
  lcd.print("Hubo cambio!");
  Serial.println("Cambio");
}

void mensaje5(int pos){
  lcd.setCursor(0, pos);
  lcd.print("No cambio");
}

void mensaje4(int pos){
  lcd.setCursor(0, pos);
  lcd.print("Ocupada");
  Serial.println("Ocupada");
}

void limpiar(int pos){
  lcd.setCursor(0, pos);
  lcd.print("                ");
}

void mensaje_inicial(int pos){
  lcd.setCursor(0, pos);
  lcd.print("                ");
}

bool luz(int pos){
  int valorLuz = 0;
  if(pos == 0){
    valorLuz = analogRead(fotoresistorPin);
  }else if (pos == 1){
    valorLuz = analogRead(fotoresistorPin2);
  }
  Serial.print("Valor de luz: ");
  Serial.println(valorLuz);

  if(valorLuz > 700){
    Serial.println(" Esta cerrada");
    return true;
  }else{
     Serial.println(" Esta abierta");
     limpiar(pos);
     mensaje1(pos);
     return false;
  }


  delay(100);
}