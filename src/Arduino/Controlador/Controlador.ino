#include <LiquidCrystal.h>
#include <LedControl.h>
#include "Abcdario.h"

#include <EEPROM.h>

String Cadena = ""; // Variable que guarda la cadena de caracteres que se va a mostrar en el LCD
String msg;         // Mensaje que se recibe por bluetooth

// tiempo de inactividad

bool inactivo = false;
bool bloqueo_login = false;
unsigned long TiempoSesion = 0;
unsigned long TiempoInactivo = 0;
int ContInactivo = 0;
int CuantoSegundosT = 0;  
bool es_Interrupcion = false;

// esta variable guarda el espacio en memoria del usuario logueado
int usuario_logueado;
// Pines para los unicos botones
int btnAceptar = 13;
int btnCancelar = 12;

// Variable que controla todos los estados del programa
int estado = 0;
bool detenerkeypadpaso = false;
bool verificarname = false;
int estado_usuario = 0;
int estado_login = 0;

int estado_eleccion = 0;

int estado_menu_usuario = 0;

// VARIABLES GLOBALES PARA EL INGRESO DE DATOS DE USUARIO A LA EPRROM

String usu;
String nu;
String con;

//--------------------------////
const int pines_de_Filas[4] = {22, 23, 24, 25};
const int pines_de_columnas[3] = {26, 27, 28};

char tecla; // Variable para obtener los valores del teclado
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
ABDC abcd;

// este struct ocupa 32 bytes de memoria
struct Usuario
{
  char nombre[13];
  char telefono[9];
  char password[13];
};

// este struct ocupa 17 bytes de memoria total 1683 bytes
struct Logs
{
  int identificador;
  char descripcion[15];
};

// Inicia con cero ya que el administrador no se cuenta como usuario
int cantidad_de_usuarios = EEPROM.read(0);

// EEPROM.get(0, cantidad_de_usuarios);
// contador de logs
int cantidad_de_logs = EEPROM.read(2);

// contador de memoria
int offset_usuarios = 1722;
int offset_compartimentos = 4;
int offset_logs = 22;

// agregar contador de logs al inicio de la eprom

// MANEJO DE LA MEMORIA EEPROM
// cantidad de usuarios 0-1
// cantidad de logs 2-3
// Compartimientos 4-21
// Logs 22-1721
// Usuarios 1722 - en adelante

void setup()
{
  Serial.begin(9600);  // Iniciar el puerto Serial 0 -> Para el monitor serial
  Serial1.begin(9600); // Iniciar el puerto Serial 1 -> Para el bluetooth
  Serial3.begin(9600); // COMUNICACIÓN Serial con el Arduino secundario
  pinMode(btnAceptar, INPUT);
  pinMode(btnCancelar, INPUT);
  // para la matriz driver
  lcd.begin(16, 4);
  // for para el teclado
  for (int i = 0; i < 4; i++)
  {
    pinMode(pines_de_Filas[i], OUTPUT);
    digitalWrite(pines_de_Filas[i], HIGH);
  }
  for (int i = 0; i < 3; i++)
  {
    pinMode(pines_de_columnas[i], INPUT_PULLUP);
  }
  IniciarLedControl();
  LedCompartimientos();

  TiempoSesion = millis();

  // clear_eeprom();
}
long int t0 = 0;
void loop()
{
  control_de_estados_lcd();
}

// ============ Sensores =========================
char consultarCompartimiento(int valor)
{
  // 0, compartimiento vacio
  // 1, compartimiento ocupado
  int compartimiento;
  int cont_cell = 0;

  for (int i = offset_compartimentos; i <= 21; i += 2)
  {
    cont_cell++;
    if (cont_cell == valor)
    {
      EEPROM.get(i, compartimiento);
      if (compartimiento == 0)
      {
        return '0';
      }
      else
      {
        return '1';
      }
    }
  }
}

bool consultarCompartimientoVacio()
{
  for (int i = 1; i < 10; i++)
  {
    byte compartimiento;
    // compartimiento = EEPROM.read(i); // error - corregir
    if (compartimiento == 255)
    {
      return true;
    }
  }
  return false;
}

char coinidirchar(int x)
{
  char res;
  switch (x)
  {
  case 1:
  {
    res = '1';
    break;
  }
  case 2:
  {
    res = '2';
    break;
  }
  case 3:
  {
    res = '3';
    break;
  }
  case 4:
  {
    res = '4';
    break;
  }
  case 5:
  {
    res = '5';
    break;
  }
  case 6:
  {
    res = '6';
    break;
  }
  case 7:
  {
    res = '7';
    break;
  }
  case 8:
  {
    res = '8';
    break;
  }
  case 9:
  {
    res = '9';
    break;
  }
  }
  return res;
}

char respuesta[2];
char envio[2];

void monitoreoCompartimientos()
{
  // Byte 1: Numero Sensor
  // Byte 2: Estado en memoria

  // Consultar compartimientos
  for (int k = 1; k < 10; k++)
  {
    // Cargar buffer de envio
    char b3 = consultarCompartimiento(k);
    envio[0] = coinidirchar(k);
    // envio[1] = '0'; // aqui va el valor del EEPROM
    envio[1] = b3; // aqui va el valor del EEPROM
    // Enviar Info
    // Serial3.print("21");
    Serial.println(envio[0]);
    Serial3.print(envio);
    // Recibir Info
    Serial3.readBytes(respuesta, 2);

    // Reportar error de compartimiento en el log
    if (String(respuesta[0]) == "1")
    {
      Serial.println("entro aqui incidente");
      agregar_logs(1, "Incidente");

      //
    }
    // Reportar error de temperatura en el log
    if (String(respuesta[1]) == "1")
    {
      Serial.println("entro aqui temperatura");
      agregar_logs(1, "Error Temp");

      //
    }
  }
}

// ============================================

void control_de_estados_lcd()
{
  check_timer();
  if (estado == 0)
  {
    mostrar_celdas();
    SecuenciaInicial();

    estado = 1;
    lcd.clear();
  }
  else if (estado == 1)
  {
    mostrar_celdas();
    MenuPrincipal();
  }
  else if (estado == 2)
  {
    login();
  }
  else if (estado == 3)
  {
    // REGISTRO DE USUARIOS
    if (estado_usuario == 0)
    {
      if (!metodoEntrada())
      {
        verificarConexion();
        usu = recibirApp("R", "N", "Nombre");

        if (verificarUsuario(usu))
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Nombre de");
          lcd.setCursor(0, 1);
          lcd.print("Usuario en uso");
          delay(300);
          lcd.clear();
          estado_usuario = 0;
        }
        else
        {
          if (CheckNombre(usu))
          {

            delay(100);
            lcd.clear();
            estado_usuario = 1;
          }
        }
      }
      else
      {
        lcd.clear();
        Cadena = "";
        while (true)
        {
          lcd.setCursor(4, 0);
          lcd.print("REGISTRO ");
          verificarname = false;
          lcd.setCursor(0, 1);
          lcd.print("Usuario: ");
          ConcatenarLetras();
          lcd.setCursor(0, 2);
          lcd.print(Cadena);
          if (digitalRead(btnAceptar) == HIGH)
          {
            usu = Cadena;            
            if (verificarUsuario(usu))
            {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Nombre de");
              lcd.setCursor(0, 1);
              lcd.print("Usuario en uso");
              delay(300);
              lcd.clear();
              estado_usuario = 0;
              delay(100);
              break;
            }
            else
            {
              if (CheckNombre(usu))
              {
                delay(100);
                usu = Cadena;
                Cadena = "";
                lcd.clear();
                estado_usuario = 1;
                delay(100);
                break;
              }
            }
          }
          else if (digitalRead(btnCancelar) == HIGH)
          {
            // limpiar solo la cadena
            Cadena = "";
            lcd.clear();
          }
        }
      }
    }
    else if (estado_usuario == 1)
    {
      if (!metodoEntrada())
      {
        Serial.println("APP - Estado usuario 1");
        verificarConexion();
        nu = recibirApp("R", "T", "Telefono");

        if (CheckNumber(nu))
        {
          estado_usuario = 2;
          // break;
        }
      }
      else
      {
        lcd.clear();
        Cadena = "";
        while (true)
        {
          lcd.setCursor(0, 1);
          lcd.print("Telefono: ");
          ConcatenarLetras();
          lcd.setCursor(0, 2);
          lcd.print(Cadena);
          if (digitalRead(btnAceptar) == HIGH)
          {
            if (CheckNumber(Cadena))
            {
              delay(100);
              Serial.println(Cadena);
              nu = Cadena;
              Cadena = "";
              lcd.clear();
              estado_usuario = 2;
              delay(100);
              break;
            }
          }
          else if (digitalRead(btnCancelar) == HIGH)
          {
            // limpiar solo la cadena
            Cadena = "";
            lcd.clear();
          }
        }
      }
    }
    else if (estado_usuario == 2)
    {
      if (!metodoEntrada())
      {
        verificarConexion();
        con = recibirApp("R", "C", "Contrasenia");
        if (CheckPassword(con))
        {
          estado_usuario = 3;
          // break;
        }
      }
      else
      {
        lcd.clear();
        Cadena = "";
        while (true)
        {
          lcd.setCursor(0, 1);
          lcd.print("Contrasenia: ");
          ConcatenarLetras();
          lcd.setCursor(0, 2);
          lcd.print(Cadena);
          if (digitalRead(btnAceptar) == HIGH)
          {
            if (CheckPassword(Cadena))
            {
              delay(100);
              con = Cadena;
              Cadena = "";
              Serial.println(Cadena);
              lcd.clear();
              estado_usuario = 3;
              delay(100);
              break;
            }
          }
          else if (digitalRead(btnCancelar) == HIGH)
          {
            // limpiar solo la cadena
            Cadena = "";
            lcd.clear();
          }
        }
      }
    }
    else if (estado_usuario == 3)
    {
      Serial.println(usu);
      Serial.println(nu);
      Serial.println(con);
      String userEncript;
      String nuEncript;
      String conEncript;
      userEncript = EncryptDecrypt(EncryptDecrypt(usu, 3), 5);
      nuEncript = EncryptDecrypt(EncryptDecrypt(nu, 3), 5);
      conEncript = EncryptDecrypt(EncryptDecrypt(con, 3), 5);
      Serial.println(userEncript);
      Serial.println(nuEncript);
      Serial.println(conEncript);
      estado = 1;
      lcd.clear();
      Usuario us1;
      userEncript.toCharArray(us1.nombre, sizeof(us1.nombre));
      nuEncript.toCharArray(us1.telefono, sizeof(us1.telefono));
      conEncript.toCharArray(us1.password, sizeof(us1.password));

      agregar_usuario(us1);
    }
  }
  else if (estado == 4)
  {
    menu_admin();
  }
  else if (estado == 5)
  {
    menu_usuario();
  }
  else if (estado == 6)
  {
    if (es_Interrupcion)
    {
      Interrupcion();
    }else{
      estado = 7;
    }
  }
  else if(estado == 7){
    if(bloqueo_login){
      usuario_logueado = 0;
      CuantoSegundosT = 0;
      lcd.clear();
      estado = 1;
    }else{
      cerrar_sesion();
    }
  }
}

void SecuenciaInicial()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kemel");
  lcd.setCursor(6, 1);
  lcd.print("202006373");
  lcd.setCursor(0, 2);
  lcd.print("Cristian");
  lcd.setCursor(6, 3);
  lcd.print("202010893");
  delay(300);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Josue");
  lcd.setCursor(6, 1);
  lcd.print("202006353");
  lcd.setCursor(0, 2);
  lcd.print("Mariano");
  lcd.setCursor(6, 3);
  lcd.print("202010770");
  delay(300);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Jonatan");
  lcd.setCursor(6, 1);
  lcd.print("202000424");
  lcd.setCursor(0, 2);
  lcd.print("Rodrigo");
  lcd.setCursor(6, 3);
  lcd.print("201906053");
  delay(300);
}
void MenuPrincipal()
{

  if (digitalRead(btnAceptar))
  {
    monitoreoCompartimientos();
    delay(300);
  }
  lcd.setCursor(0, 0);
  lcd.print("1. LOGIN");
  lcd.setCursor(0, 1);
  lcd.print("2. REGISTRO");
  tecla = LeerKeypad();
  String palabra = "";
  if (tecla != ' ')
  {
    if (tecla == '1')
    {
      estado = 2;
      lcd.clear();
      detenerkeypadpaso = true;
      estado_login = 0;
      tecla = "";
      CuantoSegundosT = 0;
    }
    else if (tecla == '2')
    {
      estado_usuario = 0;
      estado = 3;
      lcd.clear();
      detenerkeypadpaso = true;
      CuantoSegundosT = 0;
    }
    else
    {
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("OPCION ");
      lcd.setCursor(3, 2);
      lcd.print("INCORRECTA");
      delay(300);
      lcd.clear();
      CuantoSegundosT = 0;
    }
  }
}
char LeerKeypad()
{
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(pines_de_Filas[i], LOW);
    for (int j = 0; j < 3; j++)
    {
      if (digitalRead(pines_de_columnas[j]) == LOW)
      {
        char matriz_de_teclas[4][3] = {
            {'1', '2', '3'},
            {'4', '5', '6'},
            {'7', '8', '9'},
            {'*', '0', '#'}};
        return matriz_de_teclas[i][j];
      }
    }
    digitalWrite(pines_de_Filas[i], HIGH);
  }
  return ' ';
}

int Iterador = 0;
void ConcatenarLetras()
{
  char tecla2;
  if (Iterador == 0)
  {
    abcd.ViewMatriz(Iterador);
  }
  tecla2 = LeerKeypad();

  if (tecla2 != ' ')
  {
    CuantoSegundosT = 0;
    Serial.print("Inactividad Nula: ");
    Serial.println(CuantoSegundosT);
    delay(145);
    if (tecla2 == '*')
    {
      if (Iterador == 0)
      {
        Iterador = 31;
      }
      else
      {
        Iterador--;
        abcd.ViewMatriz(Iterador);
      }
    }
    else if (tecla2 == '#')
    {
      if (Iterador == 31)
      {
        Iterador = 0;
      }
      else
      {
        Iterador++;
        abcd.ViewMatriz(Iterador);
      }
    }
    else if (tecla2 == '0')
    {
      Cadena += abcd.getLetra(Iterador);
    }
    else if (tecla2 != '*' || tecla2 != '#' || tecla2 != '0')
    {
      if (detenerkeypadpaso)
      {
        detenerkeypadpaso = false;
      }
      else
      {
        Cadena += tecla2;
      }
    }
  }
}

String upper(String string)
{
  String aux;
  for (int i = 0; i < string.length(); i++)
  {
    aux += toupper(string[i]);
  }
  return aux;
}

void leer_usuarios()
{
  Serial.println("Usuarios: ");
  Usuario usuario;
  for (int i = offset_usuarios; i < EEPROM.length() - 1; i += sizeof(usuario))
  {
    EEPROM.get(i, usuario);
    Serial.print("Nombre: ");
    Serial.println(usuario.nombre);
    Serial.print("Numero: ");
    Serial.println(usuario.telefono);
    Serial.print("Password: ");
    Serial.println(usuario.password);
  }
}

String nom;
String pass;
String tel;

bool verificarUsuario(String usuario)
{
  // ver si el usuario ya existe en la eeprom
  if (usuario == EncryptDecrypt(EncryptDecrypt("GBKOH,525565", 3), 5))
  {
    return true;
  }
  for (int i = offset_usuarios; i < EEPROM.length() - 1; i += sizeof(Usuario))
  {
    Usuario usuario_leido;
    EEPROM.get(i, usuario_leido);
    int siguiente_direccion = offset_usuarios;
    for (int i = 0; i < cantidad_de_usuarios; i++)
    {
      EEPROM.get(siguiente_direccion, usuario_leido);

      if (EncryptDecrypt(EncryptDecrypt(usuario_leido.nombre, 3), 5) == usuario)
      {
        return true;
      }
      siguiente_direccion += sizeof(Usuario);
    }
  }
  return false;
}

void agregar_usuario(Usuario usuario)
{
  nom = usuario.nombre;
  pass = usuario.password;
  tel = usuario.telefono;

  for (int i = offset_usuarios; i < EEPROM.length() - 1; i += sizeof(Usuario))
  {
    Usuario usuario_leido;
    EEPROM.get(i, usuario_leido);
    int siguiente_direccion = offset_usuarios;
    bool encontrado = false;
    for (int i = 0; i < cantidad_de_usuarios; i++)
    {

      EEPROM.get(siguiente_direccion, usuario_leido);

      if (strcmp(usuario_leido.nombre, usuario.nombre) == 0)
      {
        encontrado = true;
        break;
      }
      siguiente_direccion += sizeof(Usuario);
    }
    if (encontrado == true)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Nombre");
      lcd.setCursor(0, 1);
      lcd.print("Repetido");
      delay(1000);
      lcd.clear();
      return;
    }
    else
    {
      int siguiente_direccion = offset_usuarios;
      for (int i = 0; i < cantidad_de_usuarios; i++)
      {
        siguiente_direccion += sizeof(Usuario);
      }

      lcd.print("Usuario Agregado");
      delay(200);
      lcd.clear();
      EEPROM.put(siguiente_direccion, usuario);
      cantidad_de_usuarios++;
      EEPROM.put(0, cantidad_de_usuarios);
      return;
    }
  }
}

// este metodo busca un usuario en la eeprom por su nombre
void buscar_usuario_dir(int direccion)
{
  Usuario usuario_leido;

  EEPROM.get(direccion, usuario_leido);
  Serial.print("Nombre: ");
  Serial.println(usuario_leido.nombre);
  Serial.print("Numero: ");
  Serial.println(usuario_leido.telefono);
  Serial.print("Password: ");
  Serial.println(usuario_leido.password);
}

// Este metodo recorre la eeprom buscando un compartimiento vacio
void ingreso_celulares(int direccion)
{
  int compartimiento = 0;
  int cont_cell = 0;
  String pass;
  bool aceptado = false;

  for (int i = offset_compartimentos; i <= 21; i += 2)
  {
    EEPROM.get(i, compartimiento);
    cont_cell++;

    if (compartimiento == 0)
    {
      Usuario usuario_leido;
      EEPROM.get(direccion, usuario_leido);
      pass = usuario_leido.password;
      // aqui
      int cont = 2;
      if (!metodoEntrada())
      {
        verificarConexion();
        while (cont > 0)
        {
          con = recibirApp("V", "C", "Contrasenia");

          if (EncryptDecrypt(EncryptDecrypt(con, 3), 5) == pass)
          {

            compartimiento = 1;

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Ingresando");
            lcd.setCursor(0, 1);
            lcd.print("En Pos: ");
            lcd.setCursor(8, 1);
            lcd.print(cont_cell);
            delay(700);

            lcd.clear();
            lcd.setCursor(2, 0);
            lcd.print("Verificando");
            lcd.setCursor(2, 1);
            lcd.print("Presencia y ");
            lcd.setCursor(2, 2);
            lcd.print("Temperatura");
            lcd.setCursor(2, 3);
            lcd.print("Optima");

            char enviar[2];
            char response[2];
            enviar[0] = coinidirchar(cont_cell);
            enviar[1] = '1';
            while (true)
            {
              delay(25);
              Serial3.print(enviar);
              // Recibir Info
              if (Serial3.available() == 2)
              {
                Serial3.readBytes(response, 2);
                limpiarBuffer2();
              }
              Serial.print("respuesta sensor 0: ");
              Serial.println(response[0]);
              Serial.print("respuesta sensor 1: ");
              Serial.println(response[1]);
              // Ya ingreso el telefono
              if (String(response[0]) == "0" && String(response[1]) == "0")
              {
                estado = 5;
                lcd.clear();
                break;
              }
            }

            lcd.setCursor(3, 1);
            lcd.print("Ingresado");
            delay(600);
            lcd.clear();
            agregar_logs(1, "Ingreso Cel");
            EEPROM.put(i, direccion);
            aceptado = true;
            delay(500);
            return;
          }
          else
          {
            cont = cont - 1;
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print("Contrasenia ");
            lcd.setCursor(0, 2);
            lcd.print("Incorrecta");
            delay(300);
            lcd.clear();
            agregar_logs(1, "Error Ingreso");
          }
        }
        if (aceptado == false)
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Intento fallido");
          delay(1000);
          lcd.clear();
          lcd.setCursor(2, 1);
          lcd.print("Sistema");
          lcd.setCursor(2, 2);
          lcd.print("Bloqueado");
          // estado = 1;
          es_Interrupcion = true;
          bloqueo_login = false;
          estado = 6;
          return;
        }
      }
      else
      {
        lcd.clear();
        Cadena = "";
        while (cont > 0)
        {
          lcd.setCursor(0, 1);
          lcd.print("Contrasenia: ");
          ConcatenarLetras();
          lcd.setCursor(0, 2);
          lcd.print(Cadena);
          if (digitalRead(btnAceptar) == HIGH)
          {
            delay(300);
            String aux;
            aux = EncryptDecrypt(EncryptDecrypt(pass, 3), 5);
            Serial.print("Cadena: ");
            Serial.println(Cadena);
            Serial.print("pass: ");
            Serial.println(aux);

            if (Cadena == aux)
            {

              compartimiento = 1;

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Ingresando");
              lcd.setCursor(0, 1);
              lcd.print("En Pos: ");
              lcd.setCursor(8, 1);
              lcd.print(cont_cell);
              delay(700);
              lcd.clear();
              lcd.setCursor(2, 0);
              lcd.print("Verificando");
              lcd.setCursor(2, 1);
              lcd.print("Presencia y ");
              lcd.setCursor(2, 2);
              lcd.print("Temperatura");
              lcd.setCursor(2, 3);
              lcd.print("Optima");
              
              char enviar[2];
              char response[2];
              enviar[0] = coinidirchar(cont_cell);
              enviar[1] = '1';
              while (true)
              {
                delay(25);
                Serial3.print(enviar);
                // Recibir Info
                if (Serial3.available() == 2)
                {
                  Serial3.readBytes(response, 2);
                  limpiarBuffer2();
                }
                Serial.print("respuesta sensor 0: ");
                Serial.println(response[0]);
                Serial.print("respuesta sensor 1: ");
                Serial.println(response[1]);
                // Ya ingreso el telefono
                if (String(response[0]) == "0" && String(response[1]) == "0")
                {
                  estado = 5;
                  lcd.clear();
                  break;
                }
              }

              lcd.setCursor(3, 1);
              lcd.print("Ingresado");
              delay(600);
              lcd.clear();
              agregar_logs(1, "Ingreso Cel");
              EEPROM.put(i, direccion);
              aceptado = true;
              delay(500);
              return;
            }
            else
            {
              cont = cont - 1;
              lcd.clear();
              lcd.setCursor(0, 1);
              lcd.print("Contrasenia ");
              lcd.setCursor(0, 2);
              lcd.print("Incorrecta");
              delay(300);
              lcd.clear();
              agregar_logs(1, "Error Ingreso");
            }
          }

          if (digitalRead(btnCancelar) == HIGH)
          {
            delay(300);
            Cadena = "";
            lcd.clear();
          }
        }
        if (aceptado == false)
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Intento fallido");
          delay(1000);
          lcd.clear();
          lcd.setCursor(2, 1);
          lcd.print("Sistema");
          lcd.setCursor(2, 2);
          lcd.print("Bloqueado");
          // estado = 1;
          es_Interrupcion = true;
          bloqueo_login = false;
          estado = 6;
          return;
        }
      }
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("No hay espacio");
  lcd.setCursor(0, 1);
  lcd.print("Sistema lleno");
  delay(1000);
  lcd.clear();
  estado = 5;
  return;
}

void retiro_celulares(int direccion, int celda)
{
  int compartimiento;
  int cont_cell = 0;
  String pass;

  Cadena = "";

  if (verificar_almacenado(usuario_logueado) == true)
  {

    for (int i = offset_compartimentos; i <= 21; i += 2)
    {
      EEPROM.get(i, compartimiento);
      cont_cell++;
      if (compartimiento == direccion && celda == cont_cell)
      {
        Usuario usuario_leido;
        EEPROM.get(direccion, usuario_leido);
        pass = usuario_leido.password;

        bool aceptado = false;
        int cont = 2;

        if (!metodoEntrada())
        {
          verificarConexion();
          while (cont > 0)
          {
            con = recibirApp("V", "C", "Contrasenia");

            if (EncryptDecrypt(EncryptDecrypt(con, 3), 5) == pass)
            {
              compartimiento = 1;

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Retirando");
              lcd.setCursor(0, 1);
              lcd.print("De Pos: ");
              lcd.setCursor(8, 1);
              lcd.print(cont_cell);

              char enviar[2];
              char response[2];
              enviar[0] = coinidirchar(cont_cell);
              enviar[1] = '0';
              while (true)
              {
                Serial3.print(enviar);
                // Recibir Info
                Serial3.readBytes(response, 2);
                Serial.print("respuesta sensor 0");
                Serial.println(response[0]);
                Serial.print("respuesta sensor 1: ");
                Serial.println(response[1]);
                // Ya ingreso el telefono
                if (String(response[0]) == "0" && String(response[1]) == "0")
                {
                  estado = 5;
                  lcd.clear();
                  break;
                }
              }

              agregar_logs(1, "Retiro Cel");
              EEPROM.put(i, 0);
              aceptado = true;
              delay(500);
              return;
            }
            else
            {
              cont = cont - 1;
              lcd.clear();
              lcd.setCursor(0, 1);
              lcd.print("Contrasenia ");
              lcd.setCursor(0, 2);
              lcd.print("Incorrecta");
              delay(300);
              lcd.clear();
              agregar_logs(1, "Error Retiro");
            }
          }
          if (aceptado == false)
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Intento Fallido");
            delay(1000);
            lcd.clear();
            lcd.setCursor(2, 1);
            lcd.print("Sistema");
            lcd.setCursor(2, 2);
            lcd.print("Bloqueado");

            // estado = 1;
            es_Interrupcion = true;
            bloqueo_login = false;
            estado = 6;
            return;
          }
        }
        else
        {
          lcd.clear();
          Cadena = "";
          while (cont > 0)
          {
            lcd.setCursor(0, 1);
            lcd.print("Contrasenia: ");
            ConcatenarLetras();
            lcd.setCursor(0, 2);
            lcd.print(Cadena);
            if (digitalRead(btnAceptar) == HIGH)
            {
              String aux;
              aux = EncryptDecrypt(EncryptDecrypt(pass, 3), 5);

              if (Cadena == aux)
              {
                compartimiento = 1;

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Retirando");
                lcd.setCursor(0, 1);
                lcd.print("De Pos: ");
                lcd.setCursor(8, 1);
                lcd.print(cont_cell);

                char enviar[2];
                char response[2];
                enviar[0] = coinidirchar(cont_cell);
                enviar[1] = '0';
                while (true)
                {
                  Serial3.print(enviar);
                  // Recibir Info
                  Serial3.readBytes(response, 2);
                  Serial.print("respuesta sensor 0");
                  Serial.println(response[0]);
                  Serial.print("respuesta sensor 1: ");
                  Serial.println(response[1]);
                  // Ya ingreso el telefono
                  if (String(response[0]) == "0" && String(response[1]) == "0")
                  {
                    estado = 5;
                    lcd.clear();
                    break;
                  }
                }

                agregar_logs(1, "Retiro Cel");
                EEPROM.put(i, 0);
                aceptado = true;
                delay(500);
                return;
              }
              else
              {
                cont = cont - 1;
                lcd.clear();
                lcd.setCursor(0, 1);
                lcd.print("Contrasenia ");
                lcd.setCursor(0, 2);
                lcd.print("Incorrecta");
                delay(300);
                lcd.clear();
                agregar_logs(1, "Error Retiro");
              }
            }
            if (digitalRead(btnCancelar) == HIGH)
            {
              Cadena = "";
              lcd.clear();
            }
          }
          if (aceptado == false)
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Intento Fallido");
            delay(1000);
            lcd.clear();
            lcd.setCursor(2, 1);
            lcd.print("Sistema");
            lcd.setCursor(2, 2);
            lcd.print("Bloqueado");

            // estado = 1;
            es_Interrupcion = true;
            bloqueo_login = false;
            estado = 6;
            return;
          }
        }
      }
    }
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Intento fallido");
    lcd.setCursor(0, 1);
    lcd.print("No tiene celular");
    lcd.setCursor(0, 2);
    lcd.print("almacenado");
    delay(1000);
    lcd.clear();
    estado = 5;
    return;
  }
  lcd.clear();
  mostrar_celdas_usuario(usuario_logueado);
  lcd.setCursor(0, 2);
  lcd.print("Seleccione una");
  lcd.setCursor(0, 3);
  lcd.print("Casilla valida");
  delay(1000);
}

// Este metodo recorre la eeprom buscando un compartimiento vacio
void mostrar_celdas()
{
  int compartimiento;
  int cont_cell = 0;

  for (int i = offset_compartimentos; i <= 21; i += 2)
  {
    EEPROM.get(i, compartimiento);
    cont_cell++;
    if (compartimiento != 0)
    {
      AgregarCelular(cont_cell);
    }
    else
    {
      QuitarCelular(cont_cell);
    }
  }
  LedCompartimientos();
}

// Este metodo recorre la eeprom buscando un compartimiento vacio
void mostrar_celdas_usuario(int direccion)
{
  int compartimiento;
  int cont_cell = 0;
  lcd.clear();
  int cursor = 0;

  for (int i = offset_compartimentos; i <= 21; i += 2)
  {
    EEPROM.get(i, compartimiento);
    cont_cell++;
    if (compartimiento == direccion)
    {
      lcd.setCursor(0, 0);
      lcd.print("Celulares: ");
      lcd.setCursor(cursor, 1);
      lcd.print(cont_cell);
      lcd.setCursor(cursor + 1, 1);
      lcd.print(", ");
    }
    cursor++;
  }
}

void menu_admin()
{
  lcd.setCursor(0, 0);
  lcd.print("1. VER LOGS");
  lcd.setCursor(0, 1);
  lcd.print("2. ESTADOS");
  lcd.setCursor(0, 2);
  lcd.print("3. CERRAR SESION");
  tecla = LeerKeypad();
  if (tecla != ' ')
  {
    if (tecla == '1')
    {
      lcd.clear();
      mostrar_logs();
      estado = 4;
      lcd.clear();

      detenerkeypadpaso = true;

      tecla = "";
      return;
    }
    else if (tecla == '2')
    {
      estado_usuario = 0;
      estado = 3;
      lcd.clear();
      mostrar_estados();
      delay(500);

      estado = 4;
      lcd.clear();
      detenerkeypadpaso = true;
    }
    else if (tecla == '3')
    {
      estado_login = 0;
      estado = 1;
      lcd.clear();
      lcd.print("CERRANDO SESION");
      agregar_logs(1, "Sesion Cerrada");
      delay(300);
      lcd.clear();
      detenerkeypadpaso = true;
    }
    else
    {
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("OPCION ");
      lcd.setCursor(3, 2);
      lcd.print("INCORRECTA");
      delay(300);
      estado = 4;
      lcd.clear();
    }
  }
}

void menu_usuario()
{
  mostrar_celdas();
  lcd.setCursor(0, 0);
  lcd.print("1. Ingresar Celu");
  lcd.setCursor(0, 1);
  lcd.print("2. Retirar Celul");
  lcd.setCursor(0, 2);
  lcd.print("3. C. Sesion");
  lcd.setCursor(0, 3);
  lcd.print("4. E. Cuenta");
  tecla = LeerKeypad();
  String palabra = "";
  if (tecla != ' ')
  {
    if (tecla == '1')
    {
      estado = 2;
      lcd.clear();
      detenerkeypadpaso = true;
      estado_login = 0;
      tecla = "";
      ingreso_celulares(usuario_logueado);
      estado == 5;
    }
    else if (tecla == '2')
    {

      estado_usuario = 0;
      detenerkeypadpaso = true;
      mostrar_celdas_usuario(usuario_logueado);
      delay(1000);
      detenerkeypadpaso = false;

      int posicion_borrar;

      mostrar_celdas();
      delay(1000);
      lcd.clear();
      Cadena = "";
      while (true)
      {
        lcd.setCursor(0, 1);
        lcd.print("Ingresar pos: ");
        ConcatenarLetras();
        lcd.setCursor(0, 2);
        lcd.print(Cadena);
        if (digitalRead(btnAceptar) == HIGH)
        {
          posicion_borrar = Cadena.toInt();
          delay(300);
          break;
        }
        if (digitalRead(btnCancelar) == HIGH)
        {
          Cadena = "";
          lcd.clear();
        }
      }
      retiro_celulares(usuario_logueado, posicion_borrar);
      mostrar_celdas();
      estado == 5;
    }
    else if (tecla == '3')
    {
      cerrar_sesion();
    }
    else if (tecla == '4')
    {
      // eliminar usuario
      Serial.println("Eliminar usuario");
      eliminar_cuenta(usuario_logueado);
    }

    else
    {
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("OPCION ");
      lcd.setCursor(3, 2);
      lcd.print("INCORRECTA");
      delay(300);
      lcd.clear();
    }
  }
}

void cerrar_sesion()
{
  usuario_logueado = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cerrando sesion");
  agregar_logs(1, "Sesion Cerrada");
  delay(1000);
  lcd.clear();
  estado = 1;
  CuantoSegundosT = 0;
}

String EncryptDecrypt(String text, char key)
{
  String DateEncrypt = text;
  for (size_t i = 0; i < DateEncrypt.length(); i++)
  {
    DateEncrypt[i] ^= key;
  }
  return DateEncrypt;
}

void check_timer()
{
  unsigned long tiempoActual = millis();

  // Verifica si ha transcurrido un segundo (1000 milisegundos)
  if ((tiempoActual - TiempoSesion) >= 1000)
  {
    CuantoSegundosT++;
    TiempoSesion = tiempoActual;
    Serial.print("Segundo Inactivo: ");
    Serial.println(CuantoSegundosT);
  }

  if (CuantoSegundosT == 300)
  {
    Serial.print("Cerrando por Inactividad");
    estado = 6;
  }
}

void Interrupcion()
{
  unsigned long tiempoActual = millis();
  if ((tiempoActual - TiempoInactivo) >= 1000)
  {
    ContInactivo++;
    TiempoInactivo = tiempoActual;
    Serial.print("Interrupcion: ");
    Serial.println(ContInactivo);
  }

  if (ContInactivo == 10)
  {
    Serial.print("Fin de la interrupcion");
    ContInactivo = 0;
    es_Interrupcion = false;
  }
}

void mostrar_logs()
{
  Logs log;
  int contador = 0;
  bool esperarBoton = true;
  String aux;
  String des;
  for (int i = offset_logs; i < EEPROM.length() - 1; i += sizeof(log) * 4)
  {
    EEPROM.get(i, log);

    des = log.descripcion;
    aux = EncryptDecrypt(EncryptDecrypt(des, 3), 5);
    Serial.println(aux);
    if (aux == 0)
    {
    }
    else
    {
      lcd.setCursor(0, 0);

      // crear string con el identificador y la descripcino
      String logString = String(log.identificador) + " " + String(aux);
      lcd.print(logString);
      Serial.println(log.identificador);
    }

    EEPROM.get(i + sizeof(log), log);

    des = log.descripcion;
    aux = EncryptDecrypt(EncryptDecrypt(des, 3), 5);
    Serial.println(aux);
    if (aux == 0)
    {
    }
    else
    {
      lcd.setCursor(0, 1);
      String logString = String(log.identificador) + " " + String(aux);
      lcd.print(logString);
      Serial.println(log.identificador);
    }

    EEPROM.get(i + sizeof(log) * 2, log);

    des = log.descripcion;
    aux = EncryptDecrypt(EncryptDecrypt(des, 3), 5);
    Serial.println(aux);
    if (aux == 0)
    {
    }
    else
    {
      lcd.setCursor(0, 2);
      String logString = String(log.identificador) + " " + String(aux);
      lcd.print(logString);
      Serial.println(log.identificador);
    }
    EEPROM.get(i + sizeof(log) * 3, log);

    des = log.descripcion;
    aux = EncryptDecrypt(EncryptDecrypt(des, 3), 5);
    Serial.println(aux);
    if (aux == 0)
    {
    }
    else
    {
      lcd.setCursor(0, 3);
      String logString = String(log.identificador) + " " + String(aux);
      lcd.print(logString);
      Serial.println(log.identificador);
    }

    // Esperar la entrada del botón
    esperarBoton = true;
    while (esperarBoton)
    {
      if (digitalRead(btnAceptar) == HIGH)
      {

        esperarBoton = false;
        lcd.clear();
        lcd.print("Pagina Sig.");

        delay(300);
        lcd.clear();
      }
      else if (digitalRead(btnCancelar) == HIGH)
      {
        // Aquí puedes agregar el código para manejar la cancelación
        // Regresar al menu admin
        lcd.clear();
        lcd.setCursor(2, 2);
        lcd.print("Saliendo...");
        delay(200);
        return;
      }
    }
  }
}

// ESTA NO ESTA
void agregar_logs(int identificador, String descripcion)
{
  // convertir string a char
  Serial.println("cuantos logs hay ahorita");
  Serial.println(cantidad_de_logs);
  if (cantidad_de_logs == 99)
  {
    cantidad_de_logs = 0;
    offset_logs = 22;
  }
  Logs log;

  String des = "";
  des = EncryptDecrypt(EncryptDecrypt(descripcion, 3), 5);
  log.identificador = cantidad_de_logs;
  des.toCharArray(log.descripcion, sizeof(log.descripcion));

  for (int i = offset_logs; i < EEPROM.length() - 1; i += sizeof(Logs))
  {

    Logs log_leido;
    EEPROM.get(i, log_leido);
    int siguiente_direccion = offset_logs;
    for (int i = 0; i < cantidad_de_logs; i++)
    {
      siguiente_direccion += sizeof(Logs);
    }
    Serial.println("se va guardar un log");
    delay(100);
    // borrar lo que se encuentra en esta posicion
    // EEPROM.put(siguiente_direccion, 0); //
    EEPROM.put(siguiente_direccion, log);
    cantidad_de_logs++;
    EEPROM.put(2, cantidad_de_logs);
    return;
  }
}
// ESTO NO ESTA

void leer_logs()
{
  Logs log;
  for (int i = offset_logs; i < EEPROM.length() - 1; i += sizeof(log))
  {

    EEPROM.get(i, log);
    Serial.print("Identificador: ");
    Serial.println(log.identificador);
    Serial.print("Descripcion: ");
    Serial.println(log.descripcion);
  }
}

void eliminar_cuenta(int direccion)
{

  int siguiente_direccion = offset_usuarios;
  bool encontrado = false;

  for (int i = offset_usuarios; i < EEPROM.length() - 1; i += sizeof(Usuario))
  {
    Usuario usuario_leido;
    Usuario usuario_a_eliminar;
    EEPROM.get(siguiente_direccion, usuario_leido);
    EEPROM.get(direccion, usuario_a_eliminar);

    String nombre = usuario_a_eliminar.nombre;
    String password = usuario_a_eliminar.password;

    nom = usuario_leido.nombre;
    pass = usuario_leido.password;

    if (nom == nombre && pass == password)
    {
      encontrado = true;
      Serial.print("Se encontro el usuario");
      break;
    }
    siguiente_direccion += sizeof(Usuario);
  }

  if (verificar_almacenado(usuario_logueado) == false)
  {
    if (encontrado == true)
    {
      Serial.println("Se va a eliminar la cuenta");
      lcd.clear();
      lcd.setCursor(2, 1);
      lcd.print("Eliminacion");
      lcd.setCursor(2, 2);
      lcd.print("Cuenta");
      lcd.setCursor(2, 3);
      lcd.print(usu);
      agregar_logs(1, "Eliminar cuenta");

      Serial.println(siguiente_direccion);
      
      Usuario usuario_siguiente;
      EEPROM.get(siguiente_direccion + sizeof(Usuario), usuario_siguiente);
      EEPROM.put(siguiente_direccion, usuario_siguiente);
      siguiente_direccion += sizeof(Usuario);
      for (int j = siguiente_direccion; j < sizeof(Usuario) + siguiente_direccion - 1; j++)
      {
        EEPROM.put(j, 0);
      }

      usuario_logueado = 0;
      lcd.clear();
      estado = 1;
      estado_login == 0;
      cantidad_de_usuarios--;
      EEPROM.put(0, cantidad_de_usuarios);
      return;
    }
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No se puede");
    lcd.setCursor(0, 1);
    lcd.print("Eliminar cuenta");
    lcd.setCursor(0, 2);
    lcd.print("Celulares");
    lcd.setCursor(0, 3);
    lcd.print("Almacenados!!");
    delay(1000);
    mostrar_celdas_usuario(usuario_logueado);
    delay(1000);
    lcd.clear();
    estado = 5;
    return;
  }
}

void clear_eeprom()
{
  for (int i = 0; i < EEPROM.length(); i++)
    EEPROM.write(i, 0);
}

bool verificar_almacenado(int direccion)
{
  int compartimiento;
  for (int i = offset_compartimentos; i <= 21; i += 2)
  {
    EEPROM.get(i, compartimiento);
    if (compartimiento == direccion)
    {
      return true;
    }
  }
  return false;
}

/*Pregunta al usuario el método de entrada.Retorna:
  true -> Por teclado
  false -> Por app*/
bool metodoEntrada()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Elija el metodo");
  lcd.setCursor(0, 1);
  lcd.print("de entrada:");
  lcd.setCursor(0, 2);
  lcd.print("1.Teclado 2.App");
  Cadena = "";
  while (true)
  {
    ConcatenarLetras();
    Serial.println(Cadena);
    lcd.setCursor(0, 3);
    lcd.print(Cadena);
    if (digitalRead(btnAceptar))
    {
      delay(200);
      if (Cadena == "1")
      {
        Serial.println("Entrada por Teclado");
        return true;
      }
      else if (Cadena == "2")
      {
        Serial.println("Entrada por App");
        return false;
      }
      else
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Opcion Invalida");
        lcd.setCursor(0, 1);
        lcd.print("Elija de nuevo");
        Cadena = "";
        delay(600);
        lcd.clear();
      }
      lcd.setCursor(0, 0);
      lcd.print("Elija el metodo");
      lcd.setCursor(0, 1);
      lcd.print("de entrada:");
      lcd.setCursor(0, 1);
      lcd.print("1.Teclado 2.App");
    }
    else if (digitalRead(btnCancelar))
    {
      // limpiar solo la cadena
      Cadena = "";
      lcd.setCursor(0, 3);
      lcd.print("                ");
    }
  }
}

void login()
{
  if (estado_login == 0)
  {
    if (!metodoEntrada())
    {
      Serial.println("APP");
      verificarConexion();
      usu = recibirApp("L", "N", "Nombre");
      if(verificarUsuario(usu)){
        estado_login = 1;
      }else{
        estado_login = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Usuario no");
        lcd.setCursor(0, 1);
        lcd.print("encontrado");
        delay(1000);
        lcd.clear();
      }
    }
    else
    {
      lcd.clear();
      Cadena = "";
      detenerkeypadpaso = true;
      while (true)
      {
        detenerkeypadpaso = false;
        lcd.setCursor(0, 1);
        lcd.print("Usuario: ");
        lcd.setCursor(0, 2);
        ConcatenarLetras();
        lcd.print(Cadena);
        if (digitalRead(btnAceptar) == HIGH)
        {
          if(verificarUsuario(Cadena)){
            delay(100);
            Serial.println(Cadena);
            usu = Cadena;
            Cadena = "";
            lcd.clear();
            estado_login = 1;
            delay(100);
            break;
          }else{
            estado_login = 0;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Usuario no");
            lcd.setCursor(0, 1);
            lcd.print("encontrado");
            delay(1000);
            lcd.clear();
            break;
          }
        }
        else if (digitalRead(btnCancelar) == HIGH)
        {
          // limpiar solo la cadena
          Cadena = "";
          lcd.clear();
        }
      }
    }
  }
  else if (estado_login == 1)
  {
    int cont = 2;
    bool encontrado = false;

    while (!encontrado)
    {
      if (!metodoEntrada())
      {
        Serial.println("APP");
        verificarConexion();
        con = recibirApp("L", "C", "Contrasenia");
        bool aceptado = false;
        bool bad_pass = false;

        // Encriptado Real
        String nombre_user = "GBKOH,525565";
        String password_user = "ATSVI7";

        if (nombre_user == (EncryptDecrypt(EncryptDecrypt(usu, 3), 5)) && password_user == (EncryptDecrypt(EncryptDecrypt(con, 3), 5)))
        {
          Serial.println("Bienvenido administrador");
          estado = 4;
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("Bienvenido Admin");
          delay(1000);
          lcd.clear();
          agregar_logs(1, "Login Admin");
          bad_pass = true;
          encontrado = true;
          return;
        }
        int siguiente_direccion = offset_usuarios;

        for (int i = offset_usuarios; i < EEPROM.length() - 1; i += sizeof(Usuario))
        {
          Usuario usuario_leido;
          EEPROM.get(i, usuario_leido);
          String nombres = usuario_leido.nombre;
          String passs = usuario_leido.password;

          if (EncryptDecrypt(EncryptDecrypt(usu, 3), 5) == nombres && EncryptDecrypt(EncryptDecrypt(con, 3), 5) == passs)
          {
            encontrado = true;
            bad_pass = true;
            break;
          }
          siguiente_direccion = i + sizeof(Usuario);
        }
        delay(300);
        if (encontrado == true)
        {
          Serial.println("Bienvenido Usuario");
          lcd.clear();
          lcd.setCursor(2, 1);
          lcd.print("Bienvenido");
          lcd.setCursor(2, 2);
          lcd.print(usu);
          agregar_logs(1, "Login Usu");
          delay(300);
          usuario_logueado = siguiente_direccion;
          lcd.clear();
          estado = 5;
          return;
        }

        if (bad_pass == false)
        {
          cont = cont - 1;
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("Contrasenia ");
          lcd.setCursor(0, 2);
          lcd.print("Incorrecta");
          delay(500);
          agregar_logs(1, "Error Login");
          lcd.clear();
        }

        if (cont <= 0)
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Intento fallido");
          // agregar_logs(1, "Error Login");
          delay(1000);
          lcd.clear();
          lcd.setCursor(2, 1);
          lcd.print("Sistema");
          lcd.setCursor(2, 2);
          lcd.print("Bloqueado");
          es_Interrupcion = true;
          bloqueo_login = true;
          estado = 6;
          return;
        }
      }
      else
      {
        lcd.clear();
        Cadena = "";
        detenerkeypadpaso = true;
        while (true)
        {
          detenerkeypadpaso = false;
          lcd.setCursor(0, 1);
          lcd.print("Contrasenia: ");
          lcd.setCursor(0, 2);
          ConcatenarLetras();
          lcd.print(Cadena);
          if (digitalRead(btnAceptar) == HIGH)
          {
            delay(100);
            con = Cadena;
            Cadena = "";
            lcd.clear();
            delay(100);
            break;
          }
          else if (digitalRead(btnCancelar) == HIGH)
          {
            // limpiar solo la cadena
            Cadena = "";
            lcd.clear();
          }
        }

        Serial.println("LLEGUE AQUI MI AMOR ERNES<3");

        bool aceptado = false;
        bool bad_pass = false;

        // Encriptado Real
        String nombre_user = "GBKOH,525565";
        String password_user = "ATSVI7";

        if (nombre_user == (EncryptDecrypt(EncryptDecrypt(usu, 3), 5)) && password_user == (EncryptDecrypt(EncryptDecrypt(con, 3), 5)))
        {
          Serial.println("Bienvenido administrador");
          estado = 4;
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("Bienvenido Admin");
          delay(1000);
          lcd.clear();
          agregar_logs(1, "Login Admin");
          bad_pass = true;
          encontrado = true;
          return;
        }
        int siguiente_direccion = offset_usuarios;

        for (int i = offset_usuarios; i < EEPROM.length() - 1; i += sizeof(Usuario))
        {
          Usuario usuario_leido;
          EEPROM.get(i, usuario_leido);
          String nombres = usuario_leido.nombre;
          String passs = usuario_leido.password;

          if (EncryptDecrypt(EncryptDecrypt(nombres, 3), 5) == usu && EncryptDecrypt(EncryptDecrypt(passs, 3), 5) == con)
          {
            encontrado = true;
            bad_pass = true;
            break;
          }
          siguiente_direccion = i + sizeof(Usuario);
        }

        if (encontrado == true)
        {
          Serial.println("Bienvenido Usuario");
          lcd.clear();
          lcd.setCursor(2, 1);
          lcd.print("Bienvenido");
          lcd.setCursor(2, 2);
          lcd.print(usu);
          agregar_logs(1, "Login Usu");
          delay(300);
          usuario_logueado = siguiente_direccion;
          lcd.clear();
          estado = 5;
          return;
        }

        if (bad_pass == false)
        {
          cont = cont - 1;
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("Contrasenia ");
          lcd.setCursor(0, 2);
          lcd.print("Incorrecta");
          delay(500);
          agregar_logs(1, "Error Login");
          lcd.clear();
        }

        if (cont <= 0)
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Intento fallido");
          // agregar_logs(1, "Error Login");
          delay(1000);
          lcd.clear();
          lcd.setCursor(2, 1);
          lcd.print("Sistema");
          lcd.setCursor(2, 2);
          lcd.print("Bloqueado");
          es_Interrupcion = true;
          bloqueo_login = true;
          estado = 6;
          return;
        }
      }
    }
  }
}

bool CheckPassword(String password)
{
  bool numero = false;
  bool letra = false;
  bool signo = false;

  if (password.length() < 8 || password.length() > 12)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Contrasenia ");
    lcd.setCursor(0, 1);
    lcd.print("Debe tener 8-12");
    lcd.setCursor(0, 2);
    lcd.print("caracteres");
    delay(500);
    lcd.clear();
    return false;
  }

  for (int x = 0; x < password.length(); x++)
  {
    char temp = password[x];
    if (temp >= char(65) && temp <= char(90))
    {
      letra = true;
    }
    else if (temp >= char(48) && temp <= char(57))
    {
      numero = true;
    }
    else if (temp == char(42) || temp == char(33) || temp == char(35) || temp == char(36))
    {
      signo = true;
    }
    else
    {
      numero = false;
      letra = false;
      signo = false;
      break;
    }
  }
  if (numero && letra && signo)
  {
    return true;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Contrasenia ");
  lcd.setCursor(0, 1);
  lcd.print("Debe tener min");
  lcd.setCursor(0, 2);
  lcd.print("1 letra, 1 num");
  lcd.setCursor(0, 3);
  lcd.print("y 1 signo: *!#$");
  delay(500);
  lcd.clear();
  return false;
}

bool CheckNumber(String Number)
{
  bool numero = false;
  if (Number.length() != 8)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No. Telefono");
    lcd.setCursor(0, 1);
    lcd.print("Debe tener");
    lcd.setCursor(0, 2);
    lcd.print("8 digitos");
    delay(500);
    lcd.clear();
    return false;
  }
  for (int x = 0; x < Number.length(); x++)
  {
    char temp = Number[x];
    if (temp >= char(48) && temp <= char(57))
    {
      numero = true;
    }
    else
    {
      numero = false;
      break;
    }
  }
  if (numero)
  {
    return true;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Telefono");
  lcd.setCursor(0, 1);
  lcd.print("Solo admite");
  lcd.setCursor(0, 2);
  lcd.print("Numeros");
  delay(500);
  lcd.clear();
  return false;
}

bool CheckNombre(String name)
{
  bool Letra = false;
  bool numero = false;

  if (name.length() > 12 || name.length() < 8)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nombre ");
    lcd.setCursor(0, 1);
    lcd.print("Debe tener 8-12");
    lcd.setCursor(0, 2);
    lcd.print("caracteres");
    delay(500);
    lcd.clear();
    return false;
  }

  for (int x = 0; x < name.length(); x++)
  {
    char temp = name[x];
    if (temp >= char(65) && temp <= char(90))
    {
      Letra = true;
    }
    else if (temp >= char(48) && temp <= char(57))
    {
      numero = true;
    }
    else
    {
      numero = false;
      Letra = false;
      break;
    }
  }
  if (numero || Letra)
  {
    return true;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nombre ");
  lcd.setCursor(0, 1);
  lcd.print("Solo admite");
  lcd.setCursor(0, 2);
  lcd.print("letras y numeros");
  delay(500);
  lcd.clear();
  return false;
}

void mostrar_estados()
{
  verificarConexion();
  lcd.clear();
  lcd.setCursor(0, 3);
  String cant = String(cantidad_de_usuarios);
  lcd.print("Usuarios Act: " + cant);
  // cantidad global de intentos fallidos
  enviarApp(contar_cel(), contar_fallidos(), contar_incidentes(), cant);
  while (true)
  {
    if (digitalRead(btnCancelar) == HIGH)
    {
      // limpiar solo la cadena

      Cadena = "";
      lcd.clear();
      lcd.setCursor(2, 2);
      lcd.print("Saliendo...");
      delay(200);
      lcd.clear();
      Serial1.print("r"); // Limpia la pantalla de app
      break;
    }
  }
}

String contar_cel()
{
  int compartimiento;
  int cont_cell = 0;
  int cursor = 0;
  for (int i = offset_compartimentos; i <= 21; i += 2)
  {
    EEPROM.get(i, compartimiento);
    if (compartimiento == 0)
    {
    }
    else
    {
      cont_cell++;
      cursor++;
    }
  }
  Serial.println(cont_cell);
  lcd.setCursor(0, 0);
  // cont_cell a string
  String nu = String(cont_cell);
  lcd.print("Cel ingresados:" + nu);
  return nu;
}

String contar_incidentes()
{
  int cantidad_incidentes = 0;
  Logs log;
  for (int i = offset_logs; i < EEPROM.length() - 1; i += sizeof(log))
  {

    EEPROM.get(i, log);

    if (EncryptDecrypt(EncryptDecrypt(String(log.descripcion), 3), 5) == "Incidente")
    {

      cantidad_incidentes++;
    }
  }

  lcd.setCursor(0, 2);
  String cant = String(cantidad_incidentes);
  lcd.print("Incidentes:" + cant);
  return cant;
}

String contar_fallidos()
{
  int cantidad_intentos_fallidos = 0;
  Logs log;
  for (int i = offset_logs; i < EEPROM.length() - 1; i += sizeof(log))
  {

    EEPROM.get(i, log);

    if (EncryptDecrypt(EncryptDecrypt(String(log.descripcion), 3), 5) == "Error Login")
    {

      cantidad_intentos_fallidos++;
    }
    if (EncryptDecrypt(EncryptDecrypt(String(log.descripcion), 3), 5) == "Error Ingreso")
    {
      cantidad_intentos_fallidos++;
    }
    if (EncryptDecrypt(EncryptDecrypt(String(log.descripcion), 3), 5) == "Error Retiro")
    {
      cantidad_intentos_fallidos++;
    }
    if (EncryptDecrypt(EncryptDecrypt(String(log.descripcion), 3), 5) == "Error Temp")
    {
      cantidad_intentos_fallidos++;
    }
  }
  Serial.println("intento fallidos");
  Serial.println(cantidad_intentos_fallidos);
  lcd.setCursor(0, 1);
  String cant = String(cantidad_intentos_fallidos);
  lcd.print("Intentos Fall:" + cant);
  return cant;
}

/*Se mantiene esperando una conexión*/
void verificarConexion()
{
  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print("Esperando");
  lcd.setCursor(3, 2);
  lcd.print("Conexion...");
  // delay(1000);
  // lcd.clear();
  while (true)
  {
    if (Serial1.available())
    {
      msg = Serial1.readStringUntil('\n');
      Serial.print("Recibido: ");
      Serial.println(msg);
    }
    if (msg == "conectado")
    {
      lcd.clear();
      lcd.setCursor(3, 1);
      lcd.print("Conexion");
      lcd.setCursor(3, 2);
      lcd.print("Establecida");
      delay(1000);
      lcd.clear();
      limpiarBuffer();
      return;
    }
    // aqui iba lo anterior
  }
}

/*Acepta la entrada enviada desde la aplicacion*/
boolean entradaAceptada()
{
  while (true)
  {
    if (digitalRead(btnAceptar))
    {
      delay(200);
      return true;
    }
    if (digitalRead(btnCancelar))
    {
      delay(200);
      return false;
    }
  }
}

/*Limpia el buffer de entrada por bluetooth*/
void limpiarBuffer()
{
  int t0 = millis();
  int t1 = millis();
  while (true)
  {
    t1 = millis();
    while (Serial1.available())
      Serial1.read();
    if ((t1 - t0 >= 1000) && !Serial1.available())
      break;
  }
  msg = "";
}

void limpiarBuffer2()
{
  int t0 = millis();
  int t1 = millis();
  while (true)
  {
    t1 = millis();
    while (Serial3.available())
      Serial3.read();
    if ((t1 - t0 >= 1000) && !Serial3.available())
      break;
  }
}

/*Recibe la informacion solicitada a la app mediante un estado, un codigo, y muestra el texto especificado.
R -> Registro, L -> Login, X -> Estado Sistema, Nombre, C -> Contraseña, T -> Telefono, r -> Reinicio.
*/
String recibirApp(String menu, String codigo, String texto)
{
  Serial.println("Enviado:" + menu);
  Serial1.print(menu); // Código para el estado Registro/Login
  delay(22);           // Le da tiempo a la app de procesar lo enviado
  Serial.println("Enviado: " + codigo);
  Serial1.print(codigo); // Código para Nombre/Telefono/Contraseña
  lcd.clear();
  while (true)
  {
    lcd.setCursor(0, 0);
    lcd.print(texto);
    while (!Serial1.available())
    {
      lcd.setCursor(0, 1);
      lcd.print("Esperando...");
      if (digitalRead(btnCancelar))
      {
        delay(200);
        Serial1.print(menu); // Envia el codigo de nuevo si se queda trabado
        delay(22);
        Serial1.print(codigo); // Envia el codigo de nuevo si se queda trabado
        delay(22);
        Serial.println("Enviado: " + menu);   // Envia el codigo de nuevo si se queda trabad
        Serial.println("Enviado: " + codigo); // Envia el codigo de nuevo si se queda trabado
      }
    }
    if (Serial1.available())
    {
      msg = Serial1.readStringUntil('\n');
    }
    lcd.setCursor(0, 1);
    lcd.print("             ");
    lcd.setCursor(0, 1);
    String entrada = msg;
    lcd.print(entrada);
    lcd.setCursor(0, 3);
    lcd.print("Correcto?");
    // delay(500);
    limpiarBuffer();
    if (entradaAceptada())
    {
      Serial1.print("r"); // Código para limpiar screen en app
      return entrada;
    }
    else
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ingrese de nuevo");
      delay(500);
      Serial1.print(codigo); // Código
      delay(22);
      Serial.println("Enviado: " + codigo);
      lcd.clear();
    }
  }
}

void enviarApp(String d1, String d2, String d3, String d4)
{
  Serial1.print("X"); // Código para Estados del Sistema
  delay(122);         // Le da tiempo a la app de procesar lo enviado

  Serial1.print("F"); // Código para el dato1
  delay(722);
  Serial1.print(d1); // dato1
  delay(722);
  Serial1.print("G"); // Código para el dato2
  delay(722);
  Serial1.print(d2); // dato2
  delay(722);
  Serial1.print("H"); // Código para el dato3
  delay(722);
  Serial1.print(d3); // dato3
  delay(722);
  Serial1.print("I"); // Código para el dato4
  delay(722);
  Serial1.print(d4); // dato4
  delay(722);
}

/*Convierte una cadena de texto de minusculas a mayusculas*/
String toUpper(String texto)
{
  String mayus = "";
  for (int x = 0; x < texto.length(); x++)
  {
    char temp = texto[x];
    if (temp >= char(97) && temp <= char(122))
    {
      mayus += char(temp - 32);
    }
    else
    {
      mayus += temp;
    }
  }
  return mayus;
}

int contar_cel_app()
{
  int compartimiento;
  int cont_cell = 0;
  for (int i = offset_compartimentos; i <= 21; i += 2)
  {
    EEPROM.get(i, compartimiento);
    if (compartimiento != 0)
    {
      cont_cell++;
    }
  }
  return cont_cell;
}