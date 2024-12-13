// Declaración de variables
#define LEE_SCL  40  // puerto de entrada para leer el estado de la línea SCL
#define LEE_SDA  41  // puerto de entrada para leer el estado de la línea SDA
#define ESC_SCL  4  // puerto de salida para escribir el valor de la línea SCL-out
#define ESC_SDA  39  // puerto de salida para escribir el valor de la línea SDA-out

#define PRIGHT  30    // pulsador right
#define PDOWN   31    // "" down
#define PLEFT   32    // "" left
#define PENTER 	33    // "" center
#define PUP			34    // "" up
#define SPEAKER 37    // speaker

#define D4    0xFE    // 1111 1110 unidades
#define D3    0xFD    // 1111 1101 decenas
#define D2    0xFB    // 1111 1011 centenas
#define D1    0xF7    // 1111 0111 millares
#define DOFF  0xFF    // 1111 1111 apagado: todos los cátados comunes a "1"
#define DON   0xF0    // 1111 0000   todos los cátados comunes a "0"




//  Array que contiene los números del 0-9 y letras A-F para mostrar en el display			
int num[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x58, 0x5E, 0x79, 0x71};
String NumTec;
int contador=0;
int digit = 0;

// Definición de las teclas del nuevo teclado
char teclado_map[][3] = { {'1','2','3'},
										 {'4','5','6'},
										 {'7','8','9'},
										 {'*','0','#'} };

int modo = 0;														// 0 = visualización - 1 = configuración
int formatoHoras = 1; 										// 0 = 12h - 1 = 24h
boolean alarma1 = false;									// false = apagada - true = encencida
boolean alarma2 = false;									// false = apagada - true = encencida
boolean sonidoAlarmas = true;							// false = sin sonido - true = con sonido
boolean flagAlarma = false;


int opcion = 0;													// Opciones para el menú de configuración
unsigned long tiempoEspera = 5000; 				// Tiempo de espera en ms para que el usuario introduzca un valor en el terminal

void setup() {
	// habilitar canal TX0/RX0, canal de comunicaciones serie con el virtual terminal.
	Serial.begin(9600);
	
	// Bus i2c
	pinMode(LEE_SDA, INPUT);
	pinMode(LEE_SCL, INPUT);
	pinMode(ESC_SDA, OUTPUT);
	pinMode(ESC_SCL, OUTPUT);
	
	// Dejar libre el bus i2c
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH); 	



	// PORTA: Segmentos a-f
	DDRA = 0xFF;    	// PORTA de salida
	PORTA = 0xFF;    // activamos segmentos a-f

	// PORTL[7:4]: filas del teclado
	DDRL = 0x0F;    	// input;
	PORTL = 0xFF;     // pull-up activos, cátodos/columnas teclado desactivadas 

	// PORTC: Pulsadores y altavoz
	DDRC = 0x01;    	//PC7:1 input: PC0: output-speaker
	PORTC = 0xFE;   // pull-up activos menos el speaker que es de salida

	// Habilitar canal TX3/RX3, canal de comunicaciones serie con la pantalla LCD (MILFORD 4x20 BKP)
	Serial3.begin(9600); //canal 3, 9600 baudios,

	// Interrupciones
	cli();
		TCCR3A = 0;
		TCCR3B = 0;	
		TCCR3C = 0;		
		TCNT3 = 0;

		TCCR1A = 0;
		TCCR1B = 0;	
		TCCR1C = 0;		
		TCNT1= 0;

		// TOP
		OCR1A = 7812;

		// Fast PWM (TOP = OCR1A)  N= 1024  cada 0.5 segundos (2 Hz)
		TCCR1A = B00000011; 				
		TCCR1B = B00011101; 
			
		// Habilitación de la interrupcion por OVF
		TIMSK1 |= (0<< OCIE1A)  | (0<<OCIE1B)  | (0<<OCIE1C)   | (1<<TOIE1) | (0<<ICIE1); 
		
		// TOP
		OCR3A = 1249;	
	
		// CTC (TOP = OCR3A)  N= 64  cada 10ms (100Hz)
		TCCR3A = B01000000; 				
		TCCR3B = B00001011; 				
	
		// Habilitación de la interrupcion TIMER3_COMPA_vect 
		TIMSK3  |= (1<< OCIE3A)  | (0<<OCIE3B)  | (0<<OCIE3C)   | (0<<TOIE3) | (0<<ICIE3); 
	sei();

		habilitarINTAlarmas();

		delay(200);
		muestra_menu();
  }

  
void loop() {
		if (modo == 1){
		// Modo Configuración		
			switch (opcion) {
				case 0:
					seleccionarOpcion();
					break;
				case 1:
					cambiarHora();
					opcion = 0;
					break;
				case 2:
					cambiarFecha();
					opcion = 0;
					break;
				case 3:
					cambiarFormato();
					opcion = 0;
					break;
				case 4:
					configurarAlarma(1);
					opcion = 0;
					break;
				case 5:
					ponerAlarma(1, "alarma 1");
					opcion = 0;
					break;
				case 6:
					apagarAlarma(1, "alarma 1");
					opcion = 0;
					break;
				case 7:
					configurarAlarma(2);
					opcion = 0;
					break;
				case 8:
					ponerAlarma(2, "alarma 2");
					opcion = 0;
					break;
				case 9:
					apagarAlarma(2, "alarma 2");
					opcion = 0;
					break;
				case 10:
					apagarSonidoAlarmas();
					opcion = 0;
					break;
				case 11:
					muestra_menu();
					opcion = 0;
					break;
			}
		}
		if (flagAlarma){
			if(sonidoAlarmas){
				tone(37,100,5000);
				Serial.println("BRRRR");
			}
			bajarAlarma1Flag();
			bajarAlarma2Flag();
			flagAlarma = false;
		}
}


// ----------------------------------------- FUNCIONES BASICAS I2C ---------------------------------------------------------

void i2c_start() {    // funcion naive para start
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH);	// SCL = 1 y SDA = 1
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH); 	// SCL = 1 y SDA = 1
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, LOW);  	// SCL = 1 y SDA = 0 El flanco de bajada causa el start
  digitalWrite(ESC_SCL, LOW );  digitalWrite(ESC_SDA, LOW);  	// SCL = 0 y SDA = 0
}

void i2c_stop() {    // funcion naive para stop 
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, LOW); 	// SCL = 0 y SDA = 0
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, LOW); 	// SCL = 1 y SDA = 0
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH);  	// SCL = 1 y SDA = 1 El flanco de subida causa el stop
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH);   	// SCL = 1 y SDA = 1
}

void i2c_Ebit1() {    // funcion naive enviar un 1
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, HIGH); 	// SCL = 0 y SDA = 1
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH); 	// SCL = 1 y SDA = 1
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH);  	// SCL = 1 y SDA = 1
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, HIGH);   	// SCL = 0 y SDA = 1
}

void i2c_Ebit0() {    // funcion naive enviar un 0
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, LOW);  	// SCL = 0 y SDA = 0
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, LOW);  	// SCL = 1 y SDA = 0
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, LOW);   	// SCL = 1 y SDA = 0
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, LOW);    	// SCL = 0 y SDA = 0
}

void i2c_Ebit(bool val) {    // funcion naive enviar un bit
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, val); 	// SCL = 0 y SDA = val
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, val); 	// SCL = 1 y SDA = val
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, val);  	// SCL = 1 y SDA = val
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, val);   	// SCL = 0 y SDA = val
}

int i2c_Rbit() {        // funcion naive leer un bit
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, HIGH); 	// SCL = 0 y SDA = 1
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH); 	// SCL = 1 y SDA = 1
  digitalWrite(ESC_SCL, HIGH);  int val = digitalRead(LEE_SDA); // Aqui se produce la lectura y se guarda en val!
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, HIGH);   	// SCL = 0 y SDA = 1
  return val;                                                   
}

void i2c_write_byte(byte ibyte){
   for (int i=0;i<8;i++){
      if ((ibyte&128) != 0) {i2c_Ebit1();} 
      else {i2c_Ebit0();}
      ibyte = ibyte << 1;
   }
}

byte i2c_read_byte(){
     byte ibyte = 0;
     for (int i=0;i<8;i++){
      ibyte= (ibyte<<1)|(i2c_Rbit() & 1);
     }
     return ibyte;
}


// ----------------------------------------- MANEJO MEMORIA ---------------------------------------------------------

byte leerByte(byte dir){
		byte status = SREG;
		cli();
		start:
			i2c_start();

			i2c_write_byte(0xD0); 							// chip 0 y seleccionamos escritura
			if (i2c_Rbit() != 0) goto start; 				// si ACK devuelve 1 nadie ha respondido y se vuelve a realizar el proceso

			i2c_write_byte(dir); 								// Se envía la dirección en la que se va a leer
			if (i2c_Rbit() != 0) goto start; 				// si ACK devuelve 1 nadie ha respondido y se vuelve a realizar el proceso

			i2c_start();

			i2c_write_byte(0xD1); 							// chip 0 y seleccionamos lectura
			if (i2c_Rbit() != 0) goto start; 				// si ACK devuelve 1 nadie ha respondido y se vuelve a realizar el proceso

			
			byte datoLeido = i2c_read_byte();		// Se lee el byte de la memoria en la dirección indicada 
			i2c_Ebit1();											// El maestro manda un 1 para no recibir mas
			
			i2c_stop();
			
		SREG = status;
			return datoLeido;
}

void escribirByte(byte dir, byte data){
		byte status = SREG;
		cli();
		start:
			i2c_start();

			i2c_write_byte(0xD0); 							// chip 0 y seleccionamos escritura
			if (i2c_Rbit() != 0) goto start; 				// si ACK devuelve 1 nadie ha respondido y se vuelve a realizar el proceso

			i2c_write_byte(dir); 								// Se envía la dirección en la que se va a escribir
			if (i2c_Rbit() != 0) goto start; 				// si ACK devuelve 1 nadie ha respondido y se vuelve a realizar el proceso

			
			i2c_write_byte(data);								// Se escribe el byte en memoria en la dirección indicada 
			if (i2c_Rbit() != 0) goto start; 				// si ACK devuelve 1 nadie ha respondido y se vuelve a realizar el proceso
			
			i2c_stop();
			
		SREG = status;
}


// ----------------------------------------- FUNCIONES ANEXAS---------------------------------------------------------

byte bcdADec(byte val) {
		return ( (val/16*10) + (val%16) );
}

byte decABcd(byte val) { 
		return ( (val/10*16) + (val%10) ); 
}

byte horasAM(byte horas){
		if (horas > 12) return horas - 12;
		return horas;
}

String sufijoHora(byte horas){
		if (horas > 12) return " PM";
		return " AM";
}

String numAString(byte num){
		if (num <10)return "0"+ String(num);
		return String(num);
}

String signoTemperatura(byte temp) {
		if (temp && 0x80 != 0) return "+";
		return "-";
}

String comprobarAlarma(boolean alarma){
		if (alarma) {
				return "*";
		}
		return " ";
}

String calcularMes(byte mes){
		switch (mes){
			case 1:
				return "ENE";
			case 2:
				return "FEB";
			case 3:
				return "MAR";
			case 4:
				return "ABR";
			case 5:
				return "MAY";
			case 6:
				return "JUN";
			case 7:
				return "JUL";
			case 8:
				return "AGO";
			case 9:
				return "SEP";
			case 10:
				return "OCT";
			case 11:
				return "NOV";
			case 12:
				return "DIC";
			default:
				return "XXX";
		}
}


// ----------------------------------------- MANEJO LED ---------------------------------------------------------

void imprimirEnLED( String s, int saltos){
		for (int i=0;i<saltos;i++)Serial3.write(0x20);	// Mover cursor
		Serial3.print(s);
}

void seleccionarLineaLED(int linea){
		//Prefijo
		Serial3.write(0xFE);

		// Selección de la línea en la que se imprime
		if (linea==1) Serial3.write(0x80);
		if (linea==2) Serial3.write(0xC0);
		if (linea==3) Serial3.write(0x94);
		if (linea==4) Serial3.write(0xD4);
}


// ----------------------------------------- TECLADO ---------------------------------------------------------

void volverAVisualizacion(){
		modo=0;
		Serial.println("");
		Serial.println("... fin de configuracion");
		Serial.println("Modo actual --> Modo Visualizacion");		
}

void cambiarAConfiguracion(){
		modo=1;
		Serial.println("");
		Serial.println("... fin de visualizacion");
		Serial.println("Modo actual --> Modo Configuracion");		
}

void comprobarTeclado(int digit){
		if (digitalRead(42) == 0){
			while (digitalRead(42) == 0) {}
			//Serial.print(teclado_map[0][digit]);
			NumTec += teclado_map[0][digit];
		}
    
		if (digitalRead(43) == 0){
			while (digitalRead(43)== 0) {}
			//Serial.print(teclado_map[1][digit]);
			NumTec += teclado_map[1][digit];
		}    

		if (digitalRead(44) == 0){
			while (digitalRead(44) == 0) {}
			//Serial.print(teclado_map[2][digit]);
			NumTec += teclado_map[2][digit];
		}    

		if (digitalRead(45) == 0){
			while (digitalRead(45) == 0) {}
			//Serial.print(teclado_map[3][digit]);
			NumTec += teclado_map[3][digit];
		}

		// Detectar secuencias de teclas 
		if (NumTec.endsWith("*#")) { 
			modo = 1;
			cambiarAConfiguracion();
			NumTec = "";
		}
		if (NumTec.endsWith("#*")) { 
			volverAVisualizacion();
			NumTec = "";
		}
}

// ----------------------------------------- FUNCIONES MENU CONFIGURACIÓN ---------------------------------------------------------

void muestra_menu(){
		Serial.println("** Menu de configuracion **");
		Serial.println("1. - Ajustar hora");
		Serial.println("2. - Ajustar fecha");
		Serial.println("3. - Cambiar formato");
		Serial.println("");
		Serial.println("4. - Configurar alarma 1");
		Serial.println("5. - Alarma 1 ON");
		Serial.println("6. - Alarma 1 OFF");
		Serial.println("");
		Serial.println("7. - Configurar alarma 2");
		Serial.println("8. - Alarma 2 ON");
		Serial.println("9. - Alarma 2 OFF");
		Serial.println("");
		Serial.println("10. Apagar sonido de las alarmas");
		Serial.println("11. Mostrar Opciones del menu");
		Serial.println("");
		Serial.println("[PULSA LA TECLA ENTER PARA CONFIRMAR LA OPCION INTRODUCIDA]");
}

int leerTerminal(unsigned long retardo, String msg, int limite1, int limite2){
    boolean checkeo = false;
	int lectura = 0;

	while (checkeo == false){
		Serial.println("");
		Serial.print(msg);
		unsigned long startTime = millis();
		String inputString = "";


		while (millis() - startTime < retardo) {		// Esperar a que el usuario ingrese el numero o se agote el tiempo
			if (Serial.available() > 0) {
				char inChar = (char)Serial.read();
				if (isDigit(inChar)) { 							// Verificar si es un dígito
					inputString += inChar; 					// Agregar el carácter al string
					Serial.print(inChar); 						// Eco del carácter ingresado
				} else if (inChar == '\n') { 					// Si se presiona Enter, salir del bucle
					break;
				}
			}
		}
		lectura = inputString.toInt();					// Convertir el string a un entero
		if (lectura >= limite1 && lectura <= limite2) {checkeo = true;}
		Serial.println("");
	}
    return lectura;
}

byte obtenerDiaTerminal(byte mes) {
		byte dia = 1;
		if (mes == 1 || mes == 3 || mes == 5 || mes == 7 || mes == 8 || mes == 10 || mes == 12) {
			String msg = "Introduce el dia del mes (" + calcularMes(mes) + " tiene 31 dias): ";
			dia = leerTerminal(tiempoEspera, msg, 1, 31);
		}
		else if (mes == 4 || mes == 6 || mes == 9 || mes == 11){
			String msg = "Introduce el dia del mes (" + calcularMes(mes) + " tiene 30 dias): ";
			dia = leerTerminal(tiempoEspera, msg, 1, 30);
		}
		else if (mes == 2){
			String msg = "Introduce el dia del mes (" + calcularMes(mes) + " tiene 28 dias): ";
			dia = leerTerminal(tiempoEspera, msg, 1, 28);				
		}
		return dia;
}


void seleccionarOpcion(){
		opcion = leerTerminal(tiempoEspera, "Entrar opcion: ", 0, 11);
}

void cambiarHora(){
		byte nuevaHora = leerTerminal(tiempoEspera, "Introduce un nuevo valor para la hora (0-24): ", 0, 24);
		byte nuevosMinutos = leerTerminal(tiempoEspera, "Introduce un nuevo valor para los minutos (0-59): ", 0, 59);
		byte nuevosSegundos = leerTerminal(tiempoEspera, "Introduce un nuevo valor para los segundos (0-59): ", 0, 59);

		escribirByte(0x00, decABcd(nuevosSegundos));
		escribirByte(0x01, decABcd(nuevosMinutos));
		escribirByte(0x02, decABcd(nuevaHora));

		Serial.println("Nueva hora: " + String(nuevaHora) + ":" + String(nuevosMinutos) + ":" + String(nuevosSegundos));
}

void cambiarFecha(){
		byte nuevoYear = leerTerminal(tiempoEspera, "Introduce el ano (Ejemplo: 25): ", 0, 99);
		byte nuevoMes = leerTerminal(tiempoEspera, "Introduce el mes del ano (1-12): ", 1, 12);
		byte nuevoDia = obtenerDiaTerminal(nuevoMes);

		escribirByte(0x04, decABcd(nuevoDia));
		escribirByte(0x05, decABcd(nuevoMes));
		escribirByte(0x06, decABcd(nuevoYear));

		Serial.println("Nueva fecha: " + String(nuevoDia) + ":" + calcularMes(nuevoMes) + ":" + String(nuevoYear));
}

void cambiarFormato(){
		if (formatoHoras == 0){
			formatoHoras = 1;
			Serial.println("");
			Serial.println("Formato Actual: 24h");
		}
		else if (formatoHoras == 1){
			formatoHoras = 0;
			Serial.println("");
			Serial.println("Formato Actual: 12h");
		}
}

void configurarAlarma(int alarma){
		byte dirMinutos = 0x08;
		byte dirHoras = 0x09;
		String textoAlarma = "Alarma 1: ";
		if (alarma == 2){
			dirMinutos = 0x0B;
			dirHoras = 0x0C;
			textoAlarma = "Alarma 2: ";
		}
		byte nuevaHora = leerTerminal(tiempoEspera, "Introduce un nuevo valor para la hora (0-24): ", 0, 24);
		byte nuevosMinutos = leerTerminal(tiempoEspera, "Introduce un nuevo valor para los minutos (0-59): ", 0, 59);

		escribirByte(dirMinutos, decABcd(nuevosMinutos));
		escribirByte(dirHoras, decABcd(nuevaHora));

		Serial.println(textoAlarma + String(nuevaHora) + ":" + String(nuevosMinutos));
}

void apagarAlarma(int alarma, String texto){
		if (alarma == 1) {
			alarma1 = false;
			byte anterior = leerByte(0x0E) & 0xFE;
			escribirByte(0x0E,anterior);
		}
		else if (alarma == 2) {
			alarma2 = false;
			byte anterior = leerByte(0x0E) & 0xFD;
			escribirByte(0x0E,anterior);
		}
		Serial.println("");
		Serial.println("Se ha apagado la " + texto);
}

void ponerAlarma(int alarma, String texto){
		if (alarma == 1) {
			alarma1 = true;
			modoDisparoAlarma1();
			byte anterior = leerByte(0x0E) & 0xFE;
			escribirByte(0x0E,anterior | 0x01);
		}
		else if (alarma == 2) {
			alarma2 = true;
			modoDisparoAlarma2();
			byte anterior = leerByte(0x0E) & 0xFD;
			escribirByte(0x0E,anterior | 0x02);
		}
		sonidoAlarmas = true;
		Serial.println("");
		Serial.println("Se ha puesto la " + texto);
}

void apagarSonidoAlarmas(){
		sonidoAlarmas = false;
}

void modoDisparoAlarma1(){
		// Match con hh:mm:ss
		byte anterior = leerByte(0x07) & 0x7F;
		escribirByte(0x07,anterior);
		anterior = leerByte(0x08) & 0x7F;
		escribirByte(0x08,anterior);
		anterior = leerByte(0x09) & 0x7F;
		escribirByte(0x09,anterior);
		anterior = leerByte(0x0A) | 0x80;
		escribirByte(0x0A,anterior);
}

void modoDisparoAlarma2(){
		// Match con hh:mm
		byte anterior = leerByte(0x0B) & 0x7F;
		escribirByte(0x0B,anterior);
		anterior = leerByte(0x0C) & 0x7F;
		escribirByte(0x0C,anterior);
		anterior = leerByte(0x0D) | 0x80;
		escribirByte(0x0D,anterior);
}

void bajarAlarma1Flag(){
	byte anterior = leerByte(0x0F) & 0xFE;
	escribirByte(0x0F,anterior);
}


void bajarAlarma2Flag(){
	byte anterior = leerByte(0x0F) & 0xFD;
	escribirByte(0x0F,anterior);
}

void habilitarINTAlarmas(){
	byte anterior = leerByte(0x0E);
	escribirByte(0x0E,anterior | 0x04);
}


// ----------------------------------------- INTERRUPCIONES ---------------------------------------------------------

// Rutina de servicio del timer 1. Se ejecuta cada 0.5 segundos (2 Hz)
// Actualiza información de la pantalla del reloj a partir de los datos
ISR(TIMER1_OVF_vect){
		byte segundos = bcdADec(leerByte(0x00));		
		byte minutos = bcdADec(leerByte(0x01));		
		byte horas = bcdADec(leerByte(0x02));	
		
		byte dia = bcdADec(leerByte(0x04));		
		byte mes = bcdADec(leerByte(0x05));	
		byte year = bcdADec(leerByte(0x06));	

		byte temp = bcdADec(leerByte(0x11));	

		byte minutosAlarma1 = bcdADec(leerByte(0x08));		
		byte horasAlarma1 = bcdADec(leerByte(0x09));	

		byte minutosAlarma2 = bcdADec(leerByte(0x0B));		
		byte horasAlarma2 = bcdADec(leerByte(0x0C));	

		seleccionarLineaLED(1);
		// Formato 12h
		if (formatoHoras == 0){			
			imprimirEnLED(numAString(horasAM(horas))  + ":" + numAString(minutos) + ":" + numAString(segundos) + sufijoHora(horas),  5);
		}

		// Formato 24h
		else if (formatoHoras == 1){
				imprimirEnLED(numAString(horas)  + ":" + numAString(minutos) + ":" + numAString(segundos) + "   ",  5);
		}

		seleccionarLineaLED(2);
		imprimirEnLED("ALARM", 0);
		imprimirEnLED("T=" + signoTemperatura(temp) + numAString(temp) + "C", 9);

		seleccionarLineaLED(3);
		imprimirEnLED(numAString(horasAlarma1)  + ":" + numAString(minutosAlarma1)  + comprobarAlarma(alarma1), 0);
		imprimirEnLED("DDMMMYY", 7);

		seleccionarLineaLED(4);
		imprimirEnLED(numAString(horasAlarma2)  + ":" + numAString(minutosAlarma2)  + comprobarAlarma(alarma2), 0);
		imprimirEnLED(numAString(dia)  + calcularMes(mes) + numAString(year), 7);

}


// Rutina de servicio del Timer 3. Se ejecuta cada 10ms (100Hz)
// Explora el display y el teclado actualizando la variable de modo de funcionamiento, si procede.
/** 			*#: Modo configuración - #*: Modo visualización 		**/
ISR(TIMER3_COMPA_vect){
		PORTL = DOFF;
		switch(digit){
			case 0:
				PORTA = num[contador % 10];
				PORTL = D4;
				comprobarTeclado(digit);
				digit++;
				break;
			case 1:
				PORTA = num[int(contador/10) % 10];
				PORTL = D3;
				comprobarTeclado(digit);
				digit++;
				break;
			case 2:
				PORTA = num[int(contador/100) % 10];
				PORTL = D2;
				comprobarTeclado(digit);
				digit++;
				break;
			case 3:
				PORTA = num[int(contador/1000) % 10];
				PORTL = D1;
				digit = 0;
				break;
			}
}

ISR(INT0_vect){
	flagAlarma = true;
}
