//questo codice fa muovere il motore brushed 

void accendi_motore(int MOTOR_PIN,int delaytime){
//pinMode(MOTOR_PIN, OUTPUT);
digitalWrite(MOTOR_PIN, HIGH);
delay(10);
digitalWrite(MOTOR_PIN, LOW);
delay(delaytime);
}
