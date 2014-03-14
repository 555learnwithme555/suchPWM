/* SETUP:
          -SRCLK en pin 2
          -RCLK en pin 3
          -SERIAL en pin 4
          -LEDs en shift register
*/
 /**** VARIABLES ****/
 /*pins*/
 char shift_pin=2;
 char latch_pin=3;
 char serial_pin=4;
 /*constantes*/
 const char N_LEDS = 8;
 
 unsigned char cont=0; //contador. Rebalsa en 255
 unsigned char led[N_LEDS];
 
/****METODOS****/
/* manejo del timer2 */
void initTimer2(){
  /* Desactivar interrupciones por overflow del contador*/
  TIMSK2  &= ~(1<<TOIE2);
  /* Activar interrupciones por Compare Match A*/
  TIMSK2 |= (1<<OCIE2A);
  /* Limpiar registros TCCR2A y TCCR2B*/
  TCCR2A = 0;
  TCCR2B = 0;
  /* Setear modo CTC (Clear Timer on Compare match): se resetea
  automáticamente el timer al igualarse al valor OCR2A por fijar*/
  TCCR2A |= (1<<WGM21);
  /* Configurar el prescaler a 8*/
  TCCR2B |= (1<<CS21);
  /* Setear el valor para el cual se generará una interrupción.
  El contador del timer 2 llega a 255. Se llegará a mcd*2 -1.
  Usando un mcd=71 (valor empírico) se tiene que este valor es
  igual a 141.*/
  OCR2A=100;
}
ISR(TIMER2_COMPA_vect) {
  //debug
  //digitalWrite(13,HIGH);
  
  /* restablecer contador timer2 */
  //TCNT2 = 0;
  /* aumentar contador leds */
  cont++;
  /* escribir valores para cada led */
  for (char i=0;i<N_LEDS;i++){
    if (cont<led[i])
      PORTD |= (1<<serial_pin);
    else
      PORTD &= ~(1<<serial_pin);
    /* serial seteado. Hacer shift */
    //quizás acá ganar un poco de tiempo para que se estabilice el valor del serial
    PORTD |= (1<<shift_pin);
    //quizás ganar tiempo
    PORTD &= ~(1<<shift_pin);
  }
  /* leds configurados. Hacer latch*/
  //quizás ganar tiempo
  PORTD |= (1<<latch_pin);
  //quizás ganar tiempo
  PORTD &= ~(1<<latch_pin);
  
  //debug
  //digitalWrite(13,LOW);
}

/****MAIN****/

void setup(){
  
  //debug
  pinMode(13,OUTPUT);
  /* init pins */
  pinMode(shift_pin,OUTPUT);
  pinMode(latch_pin,OUTPUT);
  pinMode(serial_pin,OUTPUT);
  PORTD = 0; //clocks en fall
  
  /* init leds*/
  for (int i=0;i<N_LEDS;i++){
    led[i]=(int)(255.0*i/N_LEDS);
  }
  /* init timer2 */
  initTimer2();
}

void loop(){
  for (int i=0;i<N_LEDS;i++){
    led[i]++;
  }
  delay(5);
}


/* 
 Se quiere poder enviar PWM a muchos leds, desde pocos pines, con ayuda de shift registers.
 Idea 1: utilizar timer interno para contar cuanto tiempo debo tener encendido cada led por cada ciclo de 256. 
 Para cada ciclo, hay que enviar valores al shift register de una sola vez, para cada led; es decir, cada ciclo, 
 se envían N bits al serial, 256 veces. Cuando finaliza el ciclo, hacer Latch al shift register y recomenzar.
 
 Cada cierto tiempo, hay que actualizar los valores en el shift register. Esto implica enviar por serial_pin los bits 
 que corresponden a cada led. El periodo con que esta actualización sucede define la calidad del PWM generado. 
 Problema: dónde ejecutar el código que setea el shift register? dentro de una rutina handler de interrupt del timer?
 dentro de loop()? 
 Tiene más sentido que se ejecute dentro de un inteerrupt, pues se quiere que el PWM ocurra de fondo, mientras se 
 editan valores de variables y se hacen otras cosas con el arduino. Se quiere que el control del pwm ocurra en segundo
 plano. El único punto relevante que hay que considerar al usar interrupt es mantener la rutina del handler lo suficiente-
 mente corta para que no se salte un interrupt, pero que el seteo del shift register se haga lo suficientemente lento para
 obtener resultados correctos, por las limitaciones del shift register.
 
 Entonces: El timer lanza una interrupción cada cierto tiempo. El tiempo con que ocurra esto se debe fijar observando la calidad
 visual del pwm obtenido empíricamente. La interrupción aumenta el contador, setea los bits de los leds según sus valores de brillo, y 
 los almacena (prende los leds).
 */
