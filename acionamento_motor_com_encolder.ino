/* ========================================================================================================
 
  MATHEUS GOMES SILVEIRA - gomes.matheus@engenharia.ufjf.br
   
======================================================================================================== */
//#include <Adafruit_INA219.h>        // ina219 lib
#include <Wire.h>


// ========================================================================================================
// --- Mapeamento de Hardware ---
#define    encoder_C1   2                     //Conexão C1 do encoder
#define    encoder_C2   3                     //Conexão C2 do encoder
#define    pwm_out      13                     //Saída pwm para controle de velocida do motor
#define    motor1       6                     //Controle In1 do driver L298N
#define    motor2       7                     //Controle In2 do driver L298N
#define    pot          A15                    //Entrada para leitura de potenciômetro


// ========================================================================================================
// --- Funções ---
void motor_control();                         //Função para controle do motor
void Read_INA219_Values(void);                //Função para leitura de corrente     
void Serial_Send(void);                       //Função para escrita no serial

// ========================================================================================================
// --- Variáveis Globais ---
byte      Encoder_C1Last;
int       pulse_number,
          adc,
          pwm_value = 128;
          int atualizacao=10;
          long int RPM;
          int i=0;
float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float tensao=0;
float loadvoltage = 0;
float sine;
int sine_i;
float trem=99999;
boolean direction_m;
const int INA_addr = 0x40;  // INA219 address

//Adafruit_INA219 ina219(INA_addr); //Chama a biblioteca e nomeira como ina219
// ========================================================================================================
// --- Configurações Iniciais ---
void setup()
{
  //TCCR0B = TCCR0B & B11111000 | B00000001; // Altera frequência do PWM nos pinos D4 e D13 of 62500 Hz https://www.etechnophiles.com/how-to-change-pwm-frequency-of-arduino-mega/
  Serial.begin(115200);                       //Inicializa comunicação serial
 // ina219.begin();
  
//  pinMode(pot,         INPUT);                //Configura entrada para potenciômetro
  pinMode(encoder_C1,  INPUT);                //Configura entrada C1 para leitura do encoder
  pinMode(encoder_C2,  INPUT);                //Configura entrada C2 para leitura do encoder
  pinMode(motor1,     OUTPUT);                //Configura saída para controle do motor (in1 do driver)
  pinMode(motor2,     OUTPUT);                //Configura saída para controle do motor (in3 do driver)
 
  
  attachInterrupt(0, count_pulses, CHANGE);   //Interrupção externa 0 por mudança de estado
                  
  
} //end setup


// ========================================================================================================
// --- Loop Infinito ---
void loop()
{
//|||||||||||||||||||||||||||||||||||||||||||| 

   motor_control();                           //chama função para controle do motor
   //Read_INA219_Values();                    //Chama a função Read_INA219_Values
  // Serial_Send();                           //Chama a função Serial_Send
  // delay(atualizacao);                                //taxa de atualização

} //end loop


// ========================================================================================================
// --- Função para controle do Motor ---
void motor_control()
{
  
  Serial.print("Num. Pulsos: ");              //Imprime
 // Serial.print(pulse_number);                 //Imprime
  RPM=pulse_number*8.333;   //Conversão para RPM, deve dividir pela resolução e transformar de ms para minutos
  Serial.print(RPM);                 //Imprime
  Serial.print("   ");                 //Imprime
  Serial.print(tensao);                 //Imprime

  Serial.println(" "); //Imprime
//
//  adc = analogRead(pot);                      //Lê entrada analógica do potenciômetro e armazena em adc
//  
//  if (adc >= 512)                             //adc maior ou igual a 512? (metade do valor de 10 bits 1024)
//  {                                           //Sim...
//    digitalWrite(motor1, LOW);                //Desliga bit de controle motor1
//    digitalWrite(motor2, HIGH);               //Liga bit de controle motor2
//    Serial.println(" Sentido: Anti-horario"); //Imprime
//    pwm_value = map(adc, 512, 1023, 0, 255);  //normaliza valor do pwm de acordo com potenciômetro
//    analogWrite(pwm_out, pwm_value);          //gera pwm proporcional
//    
//  } //end if adc
//  
//  else                                        //Senão...
//  {                                           //adc menor que 512
//    digitalWrite(motor1, HIGH);               //Liga bit de controle motor1
//    digitalWrite(motor2, LOW);                //Desliga bit de controle motor2
//    Serial.println(" Sentido: Horario");      //Imprime
//    pwm_value = map(adc, 511, 0, 0, 255);     //normaliza valor do pwm de acordo com potenciômetro
//    analogWrite(pwm_out, pwm_value);          //gera pwm proporcional
//  }
    digitalWrite(motor1, LOW);                //Desliga bit de controle motor1
    digitalWrite(motor2, HIGH);               //Liga bit de controle motor2


// Degrau
//  if(i>500)
// {
// analogWrite(pwm_out,179);
// }
//  else
//  {
//  analogWrite(pwm_out,0);
//  }
//
//  tensao=analogRead(pwm_out);

//  i++;

  for(int j=1;j<=1000000;j=10*j)
  {
      for(int i=-180;i<=180;i+=1)
      {
         sine = sin(i*(3.1415/180.0)*j);
         sine_i = 127+int(sine*255.0/2.0);
         analogWrite(pwm_out, sine_i);
         Serial.print(sine_i);
         delay(10);
        Serial.print("   ");              //Imprime
         RPM=pulse_number*8.333;   //Conversão para RPM, deve dividir pela resolução e transformar de ms para minutos
         Serial.println(RPM);                 //Imprime
         //Serial.println(" "); //Imprime

          pulse_number = 0x00;                        //reinicia pulse_number
      }

    Serial.println(trem);

  } //end for

 // pulse_number = 0x00;                        //reinicia pulse_number
  
  
} //Termina a função


// ========================================================================================================
// --- Função de contagem de pulsos do encoder ---
//
// *baseada no artigo do site http://www.arduinoecia.com.br/2016/10/motor-dc-com-encoder-arduino.html
//
//
void count_pulses()
{

  int Lstate = digitalRead(encoder_C1);       //Lê estado de encoder_C1 e armazena em Lstate
  
  if(!Encoder_C1Last && Lstate)               //Encoder_C1Last igual a zero e Lstate diferente de zero?
  {                                           
    int val = digitalRead(encoder_C2);        //Lê estado de encoder_C2 e armazena em val
    
    if(!val && direction_m) direction_m = false;      //Sentido reverso
    
    else if(val && !direction_m) direction_m = true;  //Sentido direto
    
  } //end if 
 
  Encoder_C1Last = Lstate;                    //Encoder_C1Last recebe o valor antigo

  if(!direction_m)  pulse_number++;           //incrementa número do pulso se direction limpa
  else              pulse_number--;           //senão decrementa
  

  
} //end count_pulses

//void Read_INA219_Values(void)
//{
//  shuntvoltage = ina219.getShuntVoltage_mV();    //Pega o valor de tensão
//  busvoltage = ina219.getBusVoltage_V();        
//  current_mA = ina219.getCurrent_mA();          //Pega o valor de corrente
//  loadvoltage = busvoltage + (shuntvoltage / 1000);
//}


 void Serial_Send(void)
{
 // Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  //Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
 // Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.println("");
}
