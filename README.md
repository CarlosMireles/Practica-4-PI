# Diseño e Implementación de un Reloj Despertador

## Descripción General
Este proyecto consiste en el diseño e implementación de un reloj despertador utilizando un microcontrolador Arduino Mega 2560 y varios periféricos. El dispositivo combina hardware y software para proporcionar funcionalidades básicas y avanzadas de un reloj despertador, como visualización de hora y fecha, configuración de alarmas, y reproducción de sonidos.

---

## Objetivos de la Práctica
1. Aplicar conocimientos sobre sistemas de entrada/salida en sistemas computarizados.
2. Manejar componentes periféricos básicos, como teclados, pantallas LCD y RTCs.
3. Integrar y conectar hardware para diseñar un dispositivo funcional.
4. Diseñar e implementar el software de control que opera el reloj.
5. Verificar, depurar y mejorar la integración hardware-software.

---

## Funcionalidad del Reloj Despertador
- Visualización continua en una pantalla LCD de:
  - Hora y fecha (formato 12/24 horas con indicación AM/PM).
  - Temperatura (extraída del chip RTC DS3232).
  - Alarmas activas.
- Configuración a través de un teclado 3x4:
  - `*#` para entrar en modo configuración.
  - `#*` para salir del modo configuración.
- Generación de señales audibles al activarse una alarma (melodías diferenciadas por alarma).

---

## Componentes Utilizados
### Hardware:
- **Arduino Mega 2560**: Microcontrolador principal.
- **RTC DS3232**: Proporciona datos precisos de hora y temperatura.
- **Pantalla LCD**: Para la visualización de información.
- **Teclado 3x4**: Para la entrada de comandos y configuración.
- **Altavoz**: Para generar alertas sonoras.
- **Otros opcionales**: Display de 7 segmentos y memoria externa 24LC64.

### Software:
- Interacciones a través del bucle principal `loop()`.
- Manejadores de interrupciones para actualización de pantalla y gestión de alarmas.
- Configuración modular para diferentes modos operativos (visualización y configuración).

---

## Organización del Software
### Principales Interrupciones:
1. **ISR(TIMER3_COMPA_vect)**:
   - Exploración del teclado cada 10 ms.
   - Detección y codificación de teclas.
   - Cambios entre modos de funcionamiento.
2. **ISR(TIMER1_OVF_vect)**:
   - Actualización de la pantalla LCD cada 0.5 segundos.
3. **ISR(INT0_vect)**:
   - Gestión de alarmas generadas por el RTC DS3232.

### Modos de Operación:
1. **Modo Visualización**:
   - Muestra hora, fecha, temperatura, y alarmas.
2. **Modo Configuración**:
   - Gestión de menús para ajustar hora, fecha y alarmas.

---

## Conclusión
Este proyecto integra múltiples componentes hardware y software para crear un reloj despertador funcional. Los conocimientos adquiridos abarcan la programación de microcontroladores, gestión de interrupciones, diseño modular y depuración de sistemas integrados.

