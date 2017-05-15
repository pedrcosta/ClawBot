/*
 Name:		ArduinoClawBot.ino
 Created:	5/7/2017 12:03:29 AM
 Author:	PedrCosta
*/

#include <Servo.h>
#include <BasicLinearAlgebra.h>

//Na aplicação da interface, o utilizador prime para inicializar o sistema e escolhe a cor do cubo que deseja transportar.
//A aplicação, através da cor escolhida, vai procurar a cor através de visão por computador.
//Se e o cubo desejado se encontrar na posição 1, a aplicação enviará por porta série o byte '1', se estiver na posição 2 envia '2'... até à cor 4. Se não encontrar o cubo não envia nada para o arduino
//O micro recebe como mensagem a posição onde está o cubo pretendido e desloca-se para essa posição e volta à origem, entregando o cubo.

#define SERVOS   1
#define MOTOR_DC 0


// PWM SERVO MOTORES
#define S_BASE   6
#define S_JOIN   5
#define S_CLAW   4

Servo servoBase;
Servo servoJoin;
Servo servoClaw;

#define MAX_CLAW  85    //Abertura máxima
#define MIN_CLAW  105   //Abertura minima para agarrar um objeto

#define MAX_BASE  160    //Angulo máximo base
#define MIN_BASE  20     //Angulo minimo base

#define LIM_BASE  3740  //Valor encoder na posiçao maxima 

// MOTOR DC BASE
#define ENCODER_A 2  //INT4
#define ENCODER_B 3  //INT5

#define ENABLE_MOTOR_BASE  10  //PWM
#define IN1_MOTOR_BASE     8
#define IN2_MOTOR_BASE     9

#define MAX_PWM_MOTOR_DC   50

// FIM DE CURSO
#define FIM_CURSO  20 //INT1

// POTENCIOMETROS JUNTAS
#define P_BASE  2
#define P_JOIN  1
#define P_CLAW  0


// VARIÁVEIS GLOBAIS
unsigned char input = 0;
bool fetch = false;
bool error = false;     //À deteção de erros activa-se esta flag 

volatile int encoder_pos = 0;
uint8_t pwm_motorDC = MAX_PWM_MOTOR_DC;  //Vai de 0 a 255  //pwm do motor da base
bool b_resetEncoder = false;

int valor_analogico = 0;

String s_angleBase = "";
String s_angleJoin = "";
String s_angleClaw = "";
String s_posBase = "";

uint8_t  ui_angleBase = 70;
uint8_t  ui_angleJoin = 0;
uint8_t  ui_angleClaw = 150;

uint8_t  ui_last_angleBase = 70;
uint8_t  ui_last_angleJoin = 0;

int ui_posBase   = 0;

struct coordinates {   //Coordenadas da garra
	float x;
	float y;
	float z;
} claw, origin;

uint16_t result = 0;

int data;

//    FUNÇÕES

void readEncoderA() {
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
	else { // transição descendente no canal A     
		// Verifica o canal B para ver em que sentido o encoder está a rodar  
		if (digitalRead(ENCODER_B) == HIGH) {
			encoder_pos--;          // CW
		}
		else {
			encoder_pos++;          // CCW
		}
	}
}

void readEncoderB() {
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
	else { // transição descendente no canal B 
		// Verifica o canal A para ver em que sentido o encoder está a rodar  
		if (digitalRead(ENCODER_A) == LOW) {
			encoder_pos--;          // CW
		}
		else {
			encoder_pos++;          // CCW
		}
	}
}

void resetEncoder() {
	Serial.print("Fim de Curso: ");
	Serial.println(encoder_pos);
	stopMotor();
	encoder_pos = -10;
	b_resetEncoder = true;
}

void posReset() {
	ui_angleBase = 150;
	ui_angleJoin = 180;
	ui_angleClaw = 100;
}

/*
* DECREMENTA ENCODER_POS
*/
void moveRight() {
	digitalWrite(IN1_MOTOR_BASE, LOW);
	digitalWrite(IN2_MOTOR_BASE, HIGH);
	analogWrite(ENABLE_MOTOR_BASE, pwm_motorDC);
}

/*
* INCREMENTA ENCODER_POS
*/
void moveLeft() {
	digitalWrite(IN1_MOTOR_BASE, HIGH);
	digitalWrite(IN2_MOTOR_BASE, LOW);
	analogWrite(ENABLE_MOTOR_BASE, pwm_motorDC);
}

void stopMotor() {
	digitalWrite(IN1_MOTOR_BASE, HIGH);
	digitalWrite(IN2_MOTOR_BASE, HIGH);
	analogWrite(ENABLE_MOTOR_BASE, 255);
}

void translate() { //Cinemática inversa de deslocação pela base até a posição no eixo dos x
				   //calcula o valor final das juntas através do claw.x
	while (encoder_pos < result) {
		moveRight();
	}
}

void grab() {     //Cinemática de agarrar o objecto
				  //calcula o valor final das juntas através do claw.y/z
				  //  while(analogRead(TETA1) < result){
				  //desloca até ao cubo
				  //}
				  //aplica força na garra
}

void retrieve() {  //Cinemática de regressar à posição de origem e largar objecto
				   //calcula o valor final das juntas através do origin.x/y/z
	while (encoder_pos > result) {
		moveLeft();
	}
}

void restart() {   //Cinemática de voltar ao estado inicial

}

void system_error() {  //Em caso de deteção de erros o braço volta à posição inicial, fazendo reset ao sistema

}

void pick(bool state) {
	if(state)
		servoClaw.write(MIN_CLAW);  //Close the claw
	else
		servoClaw.write(MAX_CLAW);  //Open the claw
}

// the setup function runs once when you press reset or power the board
void setup() {
	// Inicialização dos pinos
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
	attachInterrupt(digitalPinToInterrupt(FIM_CURSO), resetEncoder, FALLING);
		
	posReset();
	
	servoBase.write(ui_angleBase);
	servoJoin.write(ui_angleJoin);
	servoClaw.write(ui_angleClaw);
	
	Serial.print("angleBase: ");
	Serial.print(ui_angleBase);

	Serial.print(" | angleJoin: ");
	Serial.print(ui_angleJoin);

	Serial.print(" | angleClaw: ");
	Serial.println(ui_angleClaw);

	moveRight();  //Ir até ao inicio para fazer reset do encoder
		
	origin.x = 0;
	origin.y = 0;
	origin.z = 0;
}


bool targetPos = false;


// the loop function runs over and over again until power down or reset
void loop() {

	//if (b_resetEncoder) {
	//	//moveLeft();
	//	moveRight();
	//	Serial.print("Encoder: ");
	//	Serial.println(encoder_pos);
	//}
		
		/*Serial.print("Encoder: ");
		Serial.print(encoder_pos);
		Serial.print(" | posBase: ");
		Serial.println(ui_posBase);

		if (encoder_pos < ui_posBase)
			moveRight();
		else if (encoder_pos > ui_posBase)
			moveLeft();
		else
			stopMotor();
	}*/

	if (Serial.available() && b_resetEncoder) {

#if SERVOS
		s_angleBase = Serial.readStringUntil(' ');
		s_angleJoin = Serial.readStringUntil(' ');
		s_angleClaw = Serial.readStringUntil('\n');

		Serial.print("angleBase: ");
		ui_angleBase = s_angleBase.toInt();
		Serial.print(ui_angleBase);
		
		// Tentativa de suavização do servo
		for (int i, j = ui_last_angleBase; i <= ui_angleBase, j >= ui_angleBase; i++, j--) {
			delay(1000);
			if (ui_last_angleBase < ui_angleBase)
				servoBase.write(i);
			else
				servoBase.write(j);
			//servoBase.write(ui_angleBase);
		}
		
		Serial.print(" | angleJoin: ");
		ui_angleJoin = s_angleJoin.toInt();
		Serial.print(ui_angleJoin);
		servoJoin.write(ui_angleJoin);

		// Tentativa de suavização do servo
		for (int i, j = ui_last_angleJoin; i <= ui_angleJoin, j >= ui_angleJoin; i++, j--) {
			delay(1000);
			if (ui_last_angleJoin < ui_angleJoin)
				servoJoin.write(i);
			else
				servoJoin.write(j);
			//servoBase.write(ui_angleBase);
		}

		Serial.print(" | angleClaw: ");
		ui_angleClaw = s_angleClaw.toInt();
		Serial.println(ui_angleClaw);
		servoClaw.write(ui_angleClaw);

		ui_last_angleBase = ui_angleBase;
		ui_last_angleJoin = ui_angleJoin;

#endif // SERVOS

#if MOTOR_DC
		s_posBase = Serial.readStringUntil('\n');
		Serial.print("posBase: ");
		ui_posBase = s_posBase.toInt();
		Serial.println(ui_posBase);

		if (ui_posBase > LIM_BASE)
			ui_posBase = LIM_BASE;
		if (ui_posBase < 0)
			ui_posBase = 0;

		do {
			Serial.print("Encoder: ");
			Serial.print(encoder_pos);
			Serial.print(" | posBase: ");
			Serial.println(ui_posBase);

			if ((encoder_pos - ui_posBase) == 0) {
				stopMotor();
				targetPos = true;
				Serial.println("S");
			}
			else if (encoder_pos > ui_posBase) {
				while ((encoder_pos > ui_posBase) && !targetPos)
				{
					if (abs(encoder_pos - ui_posBase) <= 50) {
						pwm_motorDC = 25 + abs(encoder_pos - ui_posBase) / 3;
					}
					else {
						pwm_motorDC = MAX_PWM_MOTOR_DC;
					}
					moveRight();
					Serial.print("R ");
					Serial.println(pwm_motorDC);
					Serial.print("Encoder: ");
					Serial.print(encoder_pos);
					Serial.print(" | posBase: ");
					Serial.println(ui_posBase);
				}
			}
			else if (encoder_pos < ui_posBase) {
				while ((encoder_pos < ui_posBase) && !targetPos) 
				{
					if (abs(encoder_pos - ui_posBase) <= 50) {
						pwm_motorDC = 25 + abs(encoder_pos - ui_posBase) / 3;
					}
					else {
						pwm_motorDC = MAX_PWM_MOTOR_DC;
					}
					moveLeft();
					Serial.print("L ");
					Serial.println(pwm_motorDC);
					Serial.print("Encoder: ");
					Serial.print(encoder_pos);
					Serial.print(" | posBase: ");
					Serial.println(ui_posBase);
				}
			}	
		} while (!targetPos);

		targetPos = false;
		pwm_motorDC = MAX_PWM_MOTOR_DC;
		stopMotor();
		Serial.println("SS");
		Serial.print("Encoder: ");
		Serial.print(encoder_pos);
		Serial.print(" | posBase: ");
		Serial.println(ui_posBase);

		
		

		
#endif // MOTOR_DC

		
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