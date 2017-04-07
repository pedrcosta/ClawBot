#include <BasicLinearAlgebra.h>

//Na aplicação da interface, o utilizador prime para inicializar o sistema e escolhe a cor do cubo que deseja transportar.
//A aplicação, através da cor escolhida, vai procurar a cor através de visão por computador.
//Se e o cubo desejado se encontrar na posição 1, a aplicação enviará por porta série o byte '1', se estiver na posição 2 envia '2'... até à cor 4. Se não encontrar o cubo não envia nada para o arduino
//O micro recebe como mensagem a posição onde está o cubo pretendido e desloca-se para essa posição e volta à origem, entregando o cubo.


//      VALORES DE PINOS
#define ENCODER_A 2  //int externas
#define ENCODER_B 3
#define FRONT 32
#define BACK 30
#define ENABLE_BASE 6  //controlado pelo timer 4  //pwm dos motores
#define TETA1 0   //Pino analógico do potenciómetro


//    VARIÁVEIS GLOBAIS
unsigned char input = 0;
bool fetch = false;
bool error = false;     //À deteção de erros activa-se esta flag 
volatile int encoder_pos = 0;
uint8_t pwm_base = 120;  //Vai de 0 a 255  //pwm do motor da base

struct coordinates{   //Coordenadas da garra
  float x;
  float y;
  float z;
} claw, origin;

uint16_t result = 0;

//    FUNÇÕES

void readEncoderA(){
    // procura transição ascendente no canal A
  if (digitalRead(ENCODER_A) == HIGH) { 

    // Verifica o canal B para ver em que sentido o encoder está a rodar
    if (digitalRead(ENCODER_B) == LOW) {  
      encoder_pos--;         // CW
    } 
    else {
      encoder_pos++;         // CCW
    }
  }

  else   // transição descendente no canal A                                       
  { 
    // Verifica o canal B para ver em que sentido o encoder está a rodar  
    if (digitalRead(ENCODER_B) == HIGH) {   
      encoder_pos--;          // CW
    } 
    else {
      encoder_pos++;          // CCW
    }
  }
 }
      
void readEncoderB(){
    // procura transição ascendente no canal B
  if (digitalRead(ENCODER_B) == HIGH) {   

   // Verifica o canal A para ver em que sentido o encoder está a rodar
    if (digitalRead(ENCODER_A) == HIGH) {  
      encoder_pos--;         // CW
    } 
    else {
      encoder_pos++;         // CCW
    }
  }

  // transição descendente no canal B 

  else { 
    // Verifica o canal A para ver em que sentido o encoder está a rodar  
    if (digitalRead(ENCODER_A) == LOW) {   
      encoder_pos--;          // CW
    } 
    else {
      encoder_pos++;          // CCW
    }
  }
}


void direction_front(){
  digitalWrite(FRONT, HIGH);
  digitalWrite(BACK, LOW);
  analogWrite(ENABLE_BASE, pwm_base);
}

void direction_back(){
  digitalWrite(BACK, HIGH);
  digitalWrite(FRONT, LOW);
  analogWrite(ENABLE_BASE, pwm_base);
}

void translate(){ //Cinemática inversa de deslocação pela base até a posição no eixo dos x
  //calcula o valor final das juntas através do claw.x
  while(encoder_pos < result){
    direction_front();
  }
}

void grab(){     //Cinemática de agarrar o objecto
  //calcula o valor final das juntas através do claw.y/z
  while(analogRead(TETA1) < result){
    //desloca até ao cubo
  }
  //aplica força na garra
}

void retrieve(){  //Cinemática de regressar à posição de origem e largar objecto
  //calcula o valor final das juntas através do origin.x/y/z
  while(encoder_pos > result){
    direction_back();
  }
}

void restart(){   //Cinemática de voltar ao estado inicial
  
}

void system_error(){  //Em caso de deteção de erros o braço volta à posição inicial, fazendo reset ao sistema
  
}

void setup() {
  // Inicialização dos pinos
  Serial.begin(9600);
  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);
  pinMode(FRONT, OUTPUT);
  pinMode(BACK, OUTPUT);
  pinMode(ENABLE_BASE, OUTPUT);
  attachInterrupt(0, readEncoderA, CHANGE);
  attachInterrupt(1, readEncoderB, CHANGE);
  origin.x = 0;
  origin.y = 0;
  origin.z = 0;
}


void loop() {  
  
  if (Serial.available()) {
    
    input = Serial.read();
    fetch = true;
      
    if(input == '1'){
      claw.x = 1;
      claw.y = 1;
      claw.z = 1;
    }
      
    else if(input == '2'){
      claw.x = 2;
      claw.y = 1;
      claw.z = 1;
    }

    else if(input == '3'){
      claw.x = 3;
      claw.y = 1;
      claw.z = 1;
    }

    else if(input == '4'){
      claw.x = 4;
      claw.y = 1;
      claw.z = 1;
    }

    input = 0;  
  }
    
  if (fetch){
    fetch = false;
    while(!error){
      translate();
      grab();
      retrieve();
      restart();
    }

    if (error){
      error = false;
      system_error();
    }
      
  }
}
