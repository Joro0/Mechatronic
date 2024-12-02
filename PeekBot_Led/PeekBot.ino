// Variables:
byte cont = 0;  // Counter for detected persons
byte temp = 0;  // Temporary timer variable
bool ciclo = true;  // Loop control variable

// Functions:
void GeneralCheck();
void PresenceCheck();
void AbsenceCheck();

void setup() {
  cli();  // Disable interrupts

  // PORTS
  DDRB = 0x08;  // B0,1: IR sensors 1,2 (IN) - IR=0: obstacle detected
                // B2:   LDR sensor     (IN) - LDR=1: no light
                // B3:   LED            (OUT)

  Serial.begin(9600);

  // TIMER2 Configuration
  TCCR2A = 0x00;
  TCCR2B = 0x07;  // Timer with prescaler 1024
  TIMSK2 = 0x01;  // Enable overflow interrupt
  TCNT2 = 0x00;   // Initialize timer counter
}

void loop() {
  switch (PINB & 0x03) {
    case 0x03:
      GeneralCheck();
      break;
    case 0x02:
      PresenceCheck();
      break;
    case 0x01:
      AbsenceCheck();
      break;
    default:
      break;
  }
}

void GeneralCheck() {  // Check for people when it's dark
  Serial.print("People detected: ");
  Serial.print(cont);
  Serial.print("\n");

  if (cont > 0 && bitRead(PINB, 2)) bitWrite(PORTB, 3, 1);  // Turn on LED
  else bitWrite(PORTB, 3, 0);  // Turn off LED
}

void PresenceCheck() {  // Check if someone entered fully
  int aux = 0;
  ciclo = true;
  while (ciclo) {  // Loop until it confirms if the person entered fully
    switch (PINB & 0x03) {
      case 0x02:  // Wait for change
        aux = 0;
        Serial.print("Waiting\n");
        TCNT2 = 0x00;  // Reset counter
        temp = 0;
        break;

      case 0x01:  // Possible entry, turn on temporary LED
      case 0x00:
        aux = 1;
        Serial.print("Possible Entry\n");
        if (bitRead(PINB, 2)) {
          bitWrite(PORTB, 3, 1);
          Serial.print("Temporary LED ON\n");
        }
        TCNT2 = 0x00;  // Reset counter
        temp = 0;
        break;

      case 0x03:  // Confirmation if fully entered or exited
        Serial.print("Confirmation: Entered or Exited\n");
        sei();  // Enable counter interrupt

        if (temp >= 122) {  // Confirm after ~2 seconds
          if (aux == 1) cont += 1;
          cli();  // Disable counter interrupt
          ciclo = false;  // Exit loop
        }
        break;

      default:
        break;
    }
  }
}

void AbsenceCheck() {  // Check if someone left fully
  int aux = 0;
  ciclo = true;
  while (ciclo) {
    switch (PINB & 0x03) {
      case 0x01:  // Wait for change
        aux = 0;
        Serial.print("Waiting\n");
        TCNT2 = 0x00;  // Reset counter
        temp = 0;
        break;

      case 0x02:  // Possible exit
      case 0x00:
        aux = 1;
        Serial.print("Possible Exit\n");
        TCNT2 = 0x00;  // Reset counter
        temp = 0;
        break;

      case 0x03:  // Confirmation if fully exited or entered
        Serial.print("Confirmation: Exited or Entered\n");
        sei();  // Enable counter interrupt

        if (temp >= 122) {  // Confirm after ~2 seconds
          if (aux == 1 && cont > 0) cont -= 1;
          cli();  // Disable counter interrupt
          ciclo = false;  // Exit loop
        }
        break;

      default:
        break;
    }
  }
}

// Interrupt Service Routine for TIMER2 overflow
ISR(TIMER2_OVF_vect) {
  temp += 1;
}
