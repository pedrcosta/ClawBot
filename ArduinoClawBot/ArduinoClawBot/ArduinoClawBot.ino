/*
 Name:		ArduinoClawBot.ino
 Created:	5/7/2017 12:03:29 AM
 Author:	PedrCosta
*/

#include <Servo.h>
#include <BasicLinearAlgebra.h>

//Na aplica��o da interface, o utilizador prime para inicializar o sistema e escolhe a cor do cubo que deseja transportar.
//A aplica��o, atrav�s da cor escolhida, vai procurar a cor atrav�s de vis�o por computador.
//Se e o cubo desejado se encontrar na posi��o 1, a aplica��o enviar� por porta s�rie o byte '1', se estiver na posi��o 2 envia '2'... at� � cor 4. Se n�o encontrar o cubo n�o envia nada para o arduino
//O micro recebe como mensagem a posi��o onde est� o cubo pretendido e desloca-se para essa posi��o e volta � origem, entregando o cubo.

// PWM SERVO MOTORES
#define S_BASE   4
#define S_JOIN   5
#define S_CLAW   6

Servo servoBase;
Servo servoJoin;
Servo servoClaw;

#define MAX_CLAW  100   //Abertura m�xima
#define MIN_CLAW   10   //Abertura minima para agarrar um objeto

// MOTOR DC BASE
#define ENCODER_A 2  //INT4
#define ENCODER_B 3  //INT5

#define ENABLE_MOTOR_BASE  10  //PWM
#define IN1_MOTOR_BASE     8
#define IN2_MOTOR_BASE     9

// FIM DE CURSO
#define FIM_CURSO  20 //INT1

// POTENCIOMETROS JUNTAS
#define P_BASE  0
#define P_JOIN  1
#define P_CLAW  2


//    VARI�VEIS GLOBAIS
unsigned char input = 0;
bool fetch = false;
bool error = false;     //� dete��o de erros activa-se esta flag 
volatile int encoder_pos = 0;
uint8_t pwm_motorDC = 120;  //Vai de 0 a 255  //pwm do motor da base

struct coordinates {   //Coordenadas da garra
	float x;
	float y;
	float z;
} claw, origin;

uint16_t result = 0;

int data;

//    FUN��ES

void readEncoderA() {
	// procura transi��o ascendente no canal A
	if (digitalRead(ENCODER_A) == HIGH) {

		// Verifica o canal B para ver em que sentido o encoder est� a rodar
		if (digitalRead(ENCODER_B) == LOW) {
			encoder_pos--;         // CW
		}
		else {
			encoder_pos++;         // CCW
		}
	}

	else   // transi��o descendente no canal A                                       
	{
		// Verifica o canal B para ver em que sentido o encoder est� a rodar  
		if (digitalRead(ENCODER_B) == HIGH) {
			encoder_pos--;          // CW
		}
		else {
			encoder_pos++;          // CCW
		}
	}
}

void readEncoderB() {
	// procura transi��o ascendente no canal B
	if (digitalRead(ENCODER_B) == HIGH) {

		// Verifica o canal A para ver em que sentido o encoder est� a rodar
		if (digitalRead(ENCODER_A) == HIGH) {
			encoder_pos--;         // CW
		}
		else {
			encoder_pos++;         // CCW
		}
	}

	// transi��o descendente no canal B 

	else {
		// Verifica o canal A para ver em que sentido o encoder est� a rodar  
		if (digitalRead(ENCODER_A) == LOW) {
			encoder_pos--;          // CW
		}
		else {
			encoder_pos++;          // CCW
		}
	}
}

void resetEncoder() {

}

void moveRight() {
	digitalWrite(IN1_MOTOR_BASE, HIGH);
	digitalWrite(IN2_MOTOR_BASE, LOW);
	analogWrite(ENABLE_MOTOR_BASE, pwm_motorDC);
}

void moveLeft() {
	digitalWrite(IN1_MOTOR_BASE, LOW);
	digitalWrite(IN2_MOTOR_BASE, HIGH);
	analogWrite(ENABLE_MOTOR_BASE, pwm_motorDC);
}

void translate() { //Cinem�tica inversa de desloca��o pela base at� a posi��o no eixo dos x
				   //calcula o valor final das juntas atrav�s do claw.x
	while (encoder_pos < result) {
		moveRight();
	}
}

void grab() {     //Cinem�tica de agarrar o objecto
				  //calcula o valor final das juntas atrav�s do claw.y/z
				  //  while(analogRead(TETA1) < result){
				  //desloca at� ao cubo
				  //}
				  //aplica for�a na garra
}

void retrieve() {  //Cinem�tica de regressar � posi��o de origem e largar objecto
				   //calcula o valor final das juntas atrav�s do origin.x/y/z
	while (encoder_pos > result) {
		moveLeft();
	}
}

void restart() {   //Cinem�tica de voltar ao estado inicial

}

void system_error() {  //Em caso de dete��o de erros o bra�o volta � posi��o inicial, fazendo reset ao sistema

}

// the setup function runs once when you press reset or power the board
void setup() {
	// Inicializa��o dos pinos
	Serial.begin(9600);

	// PWM SERVOS
	servoBase.attach(S_BASE, 0, 160);
	servoJoin.attach(S_JOIN, 0, 180);
	servoClaw.attach(S_CLAW, 0, 160);


	// MOTOR DC BASE
	pinMode(ENCODER_A, INPUT);
	pinMode(ENCODER_B, INPUT);
	attachInterrupt(digitalPinToInterrupt(ENCODER_A), readEncoderA, CHANGE);
	attachInterrupt(digitalPinToInterrupt(ENCODER_B), readEncoderB, CHANGE);

	pinMode(ENABLE_MOTOR_BASE, OUTPUT); //PWM
	pinMode(IN1_MOTOR_BASE, OUTPUT);
	pinMode(IN2_MOTOR_BASE, OUTPUT);

	// FIM DE CURSO
	pinMode(FIM_CURSO, INPUT);
	attachInterrupt(digitalPinToInterrupt(FIM_CURSO), resetEncoder, RISING);

	//moveRight();  //Ir at� ao fim de curso para fazer reset do encoder

	servoBase.write(60);
	servoJoin.write(50);
	servoClaw.write(0);

	origin.x = 0;
	origin.y = 0;
	origin.z = 0;
}

String s_angleBase = "";
String s_angleJoin = "";
String s_angleClaw = "";

int i_angleBase = 0;
int i_angleJoin = 0;
int i_angleClaw = 0;

// the loop function runs over and over again until power down or reset
void loop() {

	if (Serial.available()) {
		int lastAngleBase = servoBase.read();
		int lastAngleJoin = servoJoin.read();

		s_angleBase = Serial.readStringUntil(' ');
		s_angleJoin = Serial.readStringUntil(' ');
		s_angleClaw = Serial.readStringUntil('\n');

		Serial.print("angleBase: ");
		i_angleBase = s_angleBase.toInt();
		Serial.print(i_angleBase);
		servoBase.write(i_angleBase);

		Serial.print(" | angleJoin: ");
		i_angleJoin = s_angleJoin.toInt();
		Serial.print(i_angleJoin);
		servoJoin.write(i_angleJoin);

		Serial.print(" | angleClaw: ");
		i_angleClaw = s_angleClaw.toInt();
		Serial.println(i_angleClaw);
		servoClaw.write(i_angleClaw);


		//    input = Serial.read();
		//    fetch = true;
		//      
		//    if(input == '1'){
		//      claw.x = 1;
		//      claw.y = 1;
		//      claw.z = 1;
		//    }
		//      
		//    else if(input == '2'){
		//      claw.x = 2;
		//      claw.y = 1;
		//      claw.z = 1;
		//    }
		//
		//    else if(input == '3'){
		//      claw.x = 3;
		//      claw.y = 1;
		//      claw.z = 1;
		//    }
		//
		//    else if(input == '4'){
		//      claw.x = 4;
		//      claw.y = 1;
		//      claw.z = 1;
		//    }
		//
		//    input = 0;  
		//  }
		//    
		//  if (fetch){
		//    fetch = false;
		//    while(!error){
		//      translate();
		//      grab();
		//      retrieve();
		//      restart();
		//    }
		//
		//    if (error){
		//      error = false;
		//      system_error();
		//    }

	}
}