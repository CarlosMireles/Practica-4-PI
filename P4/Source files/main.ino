/* Práctica Lab4: Reloj despertador
 * 
 * Descripción: Proyecto base con esquema, definiciones y programa demo
 * Programa demo: uso básico pantalla LCD
 *
 * Fichero: 	23-24_plab4_reloj_base.pdsprj
 * Creado: 		14 noviembre 2023
 * Autor:			--------
*/



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

// Definición de las teclas del nuevo teclado
char teclado_map[][3]={ {'1','2','3'},
                        {'4','5','6'},
                        {'7','8','9'},
                        {'*','0','#'}};

String NumTec;				// String que almacena los numeros introducidos por teclado
int numero;				// Valor del contador	



boolean alarma1Activa = false;
boolean alarma2Activa = false;
boolean formatoAM_PM = false;

int modo=1;										// 0: Vizualización 1: Configuración
int opcion=0;

// Setup
void setup() {
		// put your setup code here, to run once:
		// habilitar canal TX0/RX0, canal de comunicaciones serie con el virtual terminal.
		Serial.begin(9600);
			
		digitalWrite(ESC_SCL,   HIGH);
		digitalWrite(ESC_SDA,   HIGH); 	
		pinMode(LEE_SDA,        INPUT);
		pinMode(LEE_SCL,        INPUT);
		pinMode(ESC_SDA,       OUTPUT);
		pinMode(ESC_SCL,       OUTPUT);

 
		// PORTA: Segmentos a-f
		DDRA=0xFF;    				// PORTA de salida
		PORTA=0xFF;    			// activamos segmentos a-g
	
		// PORTL[7:4]: filas del teclado
		DDRL=0x0F;    				// input;
		PORTL=0xFF;     			// pull-up activos, cátodos/columnas teclado desactivadas 
	
		// PORTC: Pulsadores y altavoz
		DDRC=0x01;    				//PC7:1 input: PC0: output-speaker
		PORTC= 0xFE;   			// pull-up activos menos el speaker que es de salida
	
  	
		// habilitar canal TX3/RX3, canal de comunicaciones serie con la pantalla LCD (MILFORD 4x20 BKP)
		Serial3.begin(9600); 			//canal 3, 9600 baudios,				// 8 bits, no parity, 1 stop bit
													
		delay(150);									
		muestra_menu();
		
		cli();			
		// ISR 1 --> Modo 15 
		TCCR1A = (0 << WGM11) | (1 << WGM10); 
		TCCR1B = (1 << WGM13) | (1 << WGM12);
		// Configurar prescaler de 1024 
		TCCR1B = (1 << CS12) | (0 << CS11) | (1 << CS10);
		// TOP 
		OCR1A = 7812;
		// Habilitar interrupción de desbordamiento
		TIMSK1 = (1 << TOIE1); 
		sei();


  }


/*  FUNCIONES  PARA TRABAJAR CON EL BUS I2C  */ 

void i2c_start() {    // funcion naive para start
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH); 
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH); 
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, LOW);  // Flanco de bajada =>Esto causa el start
  digitalWrite(ESC_SCL, LOW );  digitalWrite(ESC_SDA, LOW);  // Se pone para asegurarse de deja CLK a 0
}

void i2c_stop() {    // funcion naive para stop 
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, LOW); 
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, LOW); 
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH);  // Flanco de subida =>Esto causa el stop
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH);   // Si se quita sigue funcionando!
}

void i2c_Ebit1() {    // funcion naive enviar un 1
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, HIGH); 
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH); 
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH);  // Si se quita sigue funcionando!
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, HIGH);   
}

void i2c_Ebit0() {    // funcion naive enviar un 0
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, LOW);  
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, LOW);  
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, LOW);   // Si se quita sigue funcionando!
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, LOW);    
}

void i2c_Ebit(bool val) {    // funcion naive enviar un bit
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, val); 
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, val); 
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, val);  // Si se quita sigue funcionando!
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, val);   
}

int i2c_Rbit() {        // funcion naive leer un bit
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, HIGH); 
  digitalWrite(ESC_SCL, HIGH);  digitalWrite(ESC_SDA, HIGH); 
  digitalWrite(ESC_SCL, HIGH);  int val = digitalRead(LEE_SDA);  // Aqui se produce la lectura y se guarda en val!
  digitalWrite(ESC_SCL, LOW);   digitalWrite(ESC_SDA, HIGH);   
  return val;                                                      // Aqui se devuelve val!
}

void i2c_esc_byte(byte dato){
   for (int i=0;i<8;i++){
      if ((dato&128) != 0) {i2c_Ebit1();} else {i2c_Ebit0();}
	 dato = dato << 1;
   }
}

byte i2c_lee_byte(){
     byte ibyte = 0;
     for (byte i=0;i<8;i++){
      ibyte= (ibyte<<1)|(i2c_Rbit() & 1);
     }
     return ibyte;
}


byte leerDeUnaDireccion(byte dir){
				start:
					i2c_start();
			
					i2c_esc_byte(0xD0); // chip 0 y seleccionamos escritura
					if (i2c_Rbit() != 0) goto start; // si ACK devuelve 1 nadie respondio entncs al start otra vez
	    
					// Mandar la direccion en la que se va a leer
					i2c_esc_byte(dir); 
					if (i2c_Rbit() != 0) goto start; // si ACK devuelve 1 nadie respondio entncs al start otra vez
			
					i2c_start();
			
					i2c_esc_byte(0xD1); // chip 0 y seleccionamos lectura
					if (i2c_Rbit() != 0) goto start; // si ACK devuelve 1 nadie respondio entncs al start otra vez
			
					
					byte datoLeido = i2c_lee_byte();		// Leemos el byte de la memoria 
					i2c_Ebit1();										// El maestro manda un 1 para no recibir mas
					i2c_stop();

					return datoLeido;
}


/*------------------------------------------------------------------------------------------------*/

void muestra_menu(){
		Serial.println("                   MENU DE CONFIGURACIÓN                      ");
		Serial.println("---------------------------------------------------------------");
		Serial.println("---------------------------------------------------------------");
		Serial.println("1. Ajustar hora");
		Serial.println("2. Ajustar fecha");
		Serial.print("3. Cambiar formato: ");
		Serial.println(formatoDeHora());
		Serial.println("---------------------------------------------------------------");
		Serial.println("     - Configurar alarma 1");
		Serial.println("");
		Serial.println("   4. Alarma 1 ON");
		Serial.println("   5. Alarma 1 OFF");
		Serial.println("---------------------------------------------------------------");
		Serial.println("     - Configurar alarma 2");
		Serial.println("");
		Serial.println("   6. Alarma 2 ON");
		Serial.println("   7. Alarma 2 OFF");
		Serial.println("---------------------------------------------------------------");
}


String formatoDeHora(){
		if (formatoAM_PM) return "(De 12h a 24h)";
		return "(De 24h a 12h)";
}


byte bcdADec(byte val) {
		return ( (val/16*10) + (val%16) );
}


String numAString(byte num){
		if (num <10)return "0"+ String(num);
		return String(num);
}

String signoTemperatura (byte num){
		if (num>=0) return "+";
		return "-";
}

void imprimirEnLED( String s, int espacios){
		for (int i=0;i<espacios;i++)Serial3.write(0x20);			// Poner espacios
		Serial3.print(s);
}

void seleccionarLineaLED(int linea){
		//Prefijo
		Serial3.write(0xFE);

		// Selección de la línea en la que se imprime
		if (linea==1) Serial3.write(0x00);
		if (linea==2) Serial3.write(0xC0);
		if (linea==3) Serial3.write(0x94);
		if (linea==4) Serial3.write(0xD4);
}

String calcularMes(byte month){
		if (month == 1) return "ENE";
		if (month == 2) return "FEB";
		if (month == 3) return "MAR";
		if (month == 4) return "ABR";
		if (month == 5) return "MAY";
		if (month == 6) return "JUN";
		if (month == 7) return "JUL";
		if (month == 8) return "AGO";
		if (month == 9) return "SEP";
		if (month == 10) return "OCT";
		if (month == 11) return "NOV";
		if (month == 12) return "DIC";
		else return "XXX";
}


void alarmaActiva(boolean alarma){
		if (alarma) {
				imprimirEnLED("*", 0);
				return;
		}
		imprimirEnLED(" ", 0);
		return;
}

byte horasAM(byte horas){
		if (horas > 12) return horas - 12;
		return horas;
}

String sufijoHora(byte horas){
		if (horas > 12) return "PM";
		return "AM";
}

// Rutina de servicio del timer 1. Se ejecuta cada 0.5 segundos (2 Hz)
// Actualiza información de la pantalla del reloj a partir de los datos
ISR (TIMER1_OVF_vect){

		// Obtiene del chip segundos, minutos y la hora actual
		byte segundos = bcdADec(leerDeUnaDireccion(0x00));		
		byte minutos = bcdADec(leerDeUnaDireccion(0x01));		
		byte horas = bcdADec(leerDeUnaDireccion(0x02));	
		
		// Obtiene la fecha actual
		byte day = bcdADec(leerDeUnaDireccion(0x04));		
		byte month = bcdADec(leerDeUnaDireccion(0x05));	
		byte year = bcdADec(leerDeUnaDireccion(0x06));	

		// Obtiene la temperatura
		byte temp = bcdADec(leerDeUnaDireccion(0x11));	

		// Obtiene minutos y hora de la Alarma 1
		byte minutosA1 = bcdADec(leerDeUnaDireccion(0x08));		
		byte horasA1 = bcdADec(leerDeUnaDireccion(0x09));	

		// Obtiene minutos y hora de la Alarma 2
		byte minutosA2 = bcdADec(leerDeUnaDireccion(0x0B));		
		byte horasA2 = bcdADec(leerDeUnaDireccion(0x0C));	

		// MUESTRA EN EL LED //

		seleccionarLineaLED(1);
		
		if (!formatoAM_PM){
				imprimirEnLED(numAString(horas)  + ":", 5);
				imprimirEnLED(numAString(minutos)  + ":", 0);
				imprimirEnLED(numAString(segundos),  0);
				imprimirEnLED("  " ,1);	
		}
		if (formatoAM_PM){
				imprimirEnLED(numAString(horasAM(horas))  + ":", 5);
				imprimirEnLED(numAString(minutos)  + ":", 0);
				imprimirEnLED(numAString(segundos),  0);
				imprimirEnLED(sufijoHora(horas), 1);	
		}


		seleccionarLineaLED(2);

		imprimirEnLED("ALARM", 0);
		imprimirEnLED("T=", 9);
		imprimirEnLED(signoTemperatura(temp), 0);
		imprimirEnLED(numAString(temp),  0);
		imprimirEnLED("C", 0);

		seleccionarLineaLED(3);

		imprimirEnLED(numAString(horasA1)  + ":", 0);
		imprimirEnLED(numAString(minutosA1) , 0);
		alarmaActiva(alarma1Activa);
		imprimirEnLED("DDMMMYY", 7);
		
		seleccionarLineaLED(4);

		imprimirEnLED(numAString(horasA2)  + ":", 0);
		imprimirEnLED(numAString(minutosA2) , 0);
		alarmaActiva(alarma2Activa);
		imprimirEnLED(numAString(day),  7);
		imprimirEnLED(calcularMes(month), 0);
		imprimirEnLED(numAString(year),  0);
		
		seleccionarLineaLED(1);
		imprimirEnLED("                        ",0);
} 


ISR (TIMER3_COMPA_vect) {
		
}         

 

boolean cambiarFormato(){
		if (formatoAM_PM) {
				return false;
		}
		return true;
}


void obtenerOpcion(){
		if (Serial.available() >0 ) {
				byte lectura = Serial.read() - 0x30;
				opcion = lectura;
		}
		Serial.println("Se ha seleccionado: " + String(opcion));
}


void loop() {
		
		// Modo visualizacion
		if (modo == 0){

		}

		// Modo Configuracion
		if (modo == 1){
				obtenerOpcion();

				switch (opcion){
						case 1:
								break;
						case 2:
								break;
						case 3:
								formatoAM_PM  = cambiarFormato();
								opcion=0;
								break;
						case 4:
								break;
						case 5:
								break;
						case 6:
								break;
						case 7:		
								break;				
				}
				Serial.println("formatoAM_PM = "  +  String(formatoAM_PM));
				delay(300);
		}
}



void teclado(int digit){
    if (digitalRead(42) == 0){
        while (digitalRead(42) == 0){}    
        NumTec += teclado_map[0][digit];

    }    if (digitalRead(43) == 0){
        while (digitalRead(43) == 0){}    
        NumTec += teclado_map[1][digit];

    }    if (digitalRead(44) == 0){
        while (digitalRead(44) == 0){}    
        NumTec += teclado_map[2][digit];

    }    if (digitalRead(45) == 0 && digit ==1){
	   while (digitalRead(45) == 0){}
	   NumTec += teclado_map[3][digit];
	   
    }	if (digitalRead(45) == 0 && digit == 2){
            numero = NumTec.toInt();
            while (digitalRead(45) == 0){}
            Serial.print("Contador = ");
	    Serial.println(numero);
            NumTec = "";
    }
}
