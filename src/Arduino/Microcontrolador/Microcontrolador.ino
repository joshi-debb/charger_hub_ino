// ARDUINO SECUNDARIO

void setup() {
  // COM
  Serial3.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);

  //Inicializar Puertos Sensores
  for(int j=3; j<12; j++){
    pinMode(j, INPUT);
  }
}


// Lectura
char solicitud[3];
void loop() {
  if (Serial3.available()) {
    Serial3.readBytes(solicitud, 2);
    /* switch (solicitud[0]) {
      case 'S': // Sensores
      { */
        switch (solicitud[0]) {
          case '1':
            {
              // Validacion estado compartimiento
              if (digitalRead(3) != 1 && solicitud[1] == '1'){
                // Error de compartimiento
                Serial3.print("1");
              }else if (digitalRead(3) != 0 && solicitud[1] == '0'){
                // Error de compartimiento
                Serial3.print("1");
              }else{
                // No hay error
                Serial3.print("0");
              }

              // Validacion temperatura compartimiento
              int temp_adc_val;
              int temp_val;
              temp_adc_val = analogRead(A0);
              temp_val = (temp_adc_val * 4.88)/10;

              if (solicitud[1] == '1'){
                // Esta en el rango de temperatura
                if (temp_val<36 || temp_val>43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              } else if (solicitud[1] == '0'){
                // Esta en el rango de temperatura
                if (temp_val>=43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              }
              break;
            }
          case '2':
            {
              // Validacion estado compartimiento
              if (digitalRead(4) != 1 && solicitud[1] == '1'){
                // Error de compartimiento
                Serial3.print("1");
              }else if (digitalRead(4) != 0 && solicitud[1] == '0'){
                // Error de compartimiento
                Serial3.print("1");
              }else{
                // No hay error
                Serial3.print("0");
              }

              // Validacion temperatura compartimiento
              int temp_adc_val;
              int temp_val;
              temp_adc_val = analogRead(A1);
              temp_val = (temp_adc_val * 4.88)/10;

              if (solicitud[1] == '1'){
                // Esta en el rango de temperatura
                if (temp_val<36 || temp_val>43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              } else if (solicitud[1] == '0'){
                // Esta en el rango de temperatura
                if (temp_val>=43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              }
              break;
            }
          case '3':
            {
              // Validacion estado compartimiento
              if (digitalRead(5) != 1 && solicitud[1] == '1'){
                // Error de compartimiento
                Serial3.print("1");
              }else if (digitalRead(5) != 0 && solicitud[1] == '0'){
                // Error de compartimiento
                Serial3.print("1");
              }else{
                // No hay error
                Serial3.print("0");
              }

              // Validacion temperatura compartimiento
              int temp_adc_val;
              int temp_val;
              temp_adc_val = analogRead(A2);
              temp_val = (temp_adc_val * 4.88)/10;

              if (solicitud[1] == '1'){
                // Esta en el rango de temperatura
                if (temp_val<36 || temp_val>43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              } else if (solicitud[1] == '0'){
                // Esta en el rango de temperatura
                if (temp_val>=43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              }
              break;
            }
          case '4':
            {
              // Validacion estado compartimiento
              if (digitalRead(6) != 1 && solicitud[1] == '1'){
                // Error de compartimiento
                Serial3.print("1");
              }else if (digitalRead(6) != 0 && solicitud[1] == '0'){
                // Error de compartimiento
                Serial3.print("1");
              }else{
                // No hay error
                Serial3.print("0");
              }

              // Validacion temperatura compartimiento
              int temp_adc_val;
              int temp_val;
              temp_adc_val = analogRead(A3);
              temp_val = (temp_adc_val * 4.88)/10;

              if (solicitud[1] == '1'){
                // Esta en el rango de temperatura
                if (temp_val<36 || temp_val>43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              } else if (solicitud[1] == '0'){
                // Esta en el rango de temperatura
                if (temp_val>=43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              }
              break;
            }
          case '5':
            {
              // Validacion estado compartimiento
              if (digitalRead(7) != 1 && solicitud[1] == '1'){
                // Error de compartimiento
                Serial3.print("1");
              }else if (digitalRead(7) != 0 && solicitud[1] == '0'){
                // Error de compartimiento
                Serial3.print("1");
              }else{
                // No hay error
                Serial3.print("0");
              }

              // Validacion temperatura compartimiento
              int temp_adc_val;
              int temp_val;
              temp_adc_val = analogRead(A4);
              temp_val = (temp_adc_val * 4.88)/10;

              if (solicitud[1] == '1'){
                // Esta en el rango de temperatura
                if (temp_val<36 || temp_val>43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              } else if (solicitud[1] == '0'){
                // Esta en el rango de temperatura
                if (temp_val>=43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              }
              break;
            }
          case '6':
            {
              // Validacion estado compartimiento
              if (digitalRead(8) != 1 && solicitud[1] == '1'){
                // Error de compartimiento
                Serial3.print("1");
              }else if (digitalRead(8) != 0 && solicitud[1] == '0'){
                // Error de compartimiento
                Serial3.print("1");
              }else{
                // No hay error
                Serial3.print("0");
              }

              // Validacion temperatura compartimiento
              int temp_adc_val;
              int temp_val;
              temp_adc_val = analogRead(A5);
              temp_val = (temp_adc_val * 4.88)/10;

              if (solicitud[1] == '1'){
                // Esta en el rango de temperatura
                if (temp_val<36 || temp_val>43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              } else if (solicitud[1] == '0'){
                // Esta en el rango de temperatura
                if (temp_val>=43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              }
              break;
            }
          case '7':
            {
              // Validacion estado compartimiento
              if (digitalRead(9) != 1 && solicitud[1] == '1'){
                // Error de compartimiento
                Serial3.print("1");
              }else if (digitalRead(9) != 0 && solicitud[1] == '0'){
                // Error de compartimiento
                Serial3.print("1");
              }else{
                // No hay error
                Serial3.print("0");
              }

              // Validacion temperatura compartimiento
              int temp_adc_val;
              int temp_val;
              temp_adc_val = analogRead(A6);
              temp_val = (temp_adc_val * 4.88)/10;

              if (solicitud[1] == '1'){
                // Esta en el rango de temperatura
                if (temp_val<36 || temp_val>43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              } else if (solicitud[1] == '0'){
                // Esta en el rango de temperatura
                if (temp_val>=43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              }
              break;
            }
          case '8':
            {
              // Validacion estado compartimiento
              if (digitalRead(10) != 1 && solicitud[1] == '1'){
                // Error de compartimiento
                Serial3.print("1");
              }else if (digitalRead(10) != 0 && solicitud[1] == '0'){
                // Error de compartimiento
                Serial3.print("1");
              }else{
                // No hay error
                Serial3.print("0");
              }

              // Validacion temperatura compartimiento
              int temp_adc_val;
              int temp_val;
              temp_adc_val = analogRead(A7);
              temp_val = (temp_adc_val * 4.88)/10;

              if (solicitud[1] == '1'){
                // Esta en el rango de temperatura
                if (temp_val<36 || temp_val>43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              } else if (solicitud[1] == '0'){
                // Esta en el rango de temperatura
                if (temp_val>=43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              }
              break;
            }
          case '9':
            {
              // Validacion estado compartimiento
              if (digitalRead(11) != 1 && solicitud[1] == '1'){
                // Error de compartimiento
                Serial3.print("1");
              }else if (digitalRead(11) != 0 && solicitud[1] == '0'){
                // Error de compartimiento
                Serial3.print("1");
              }else{
                // No hay error
                Serial3.print("0");
              }

              // Validacion temperatura compartimiento
              int temp_adc_val;
              int temp_val;
              temp_adc_val = analogRead(A8);
              temp_val = (temp_adc_val * 4.88)/10;

              if (solicitud[1] == '1'){
                // Esta en el rango de temperatura
                if (temp_val<36 || temp_val>43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              } else if (solicitud[1] == '0'){
                // Esta en el rango de temperatura
                if (temp_val>=43){
                  // Error temperatura compartimiento
                  Serial3.print("1");
                }else{
                  // No hay error
                  Serial3.print("0");
                }
              }
              break;
            }
        }
        //break;
      /* }
    } */
  }
  //delay(300);
}
