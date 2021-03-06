﻿void loadOrwriteDefaults() {

	int canWriteDefaults;
	EEPROM_readAnything(80, canWriteDefaults);
	//canWriteDefaults = -1;

	if (canWriteDefaults == -1) {

		conf.breathsPerMinute = 12;
		conf.airVol = 600;
		conf.inhaleExhaleRatio = 1;
		conf.motionLength = 10200;
		conf.stepperSpeed = 1000;
		EEPROM_writeAnything(0, conf);
		EEPROM_writeAnything(80, 1);
	}

	if (canWriteDefaults == 1) {

		EEPROM_readAnything(0, conf);

	}
}


long hardLimiter(long min, long max, long value) {

	if (value > max) {
		Serial.println("Value Out Of Range");
		return -1;
	}

	if (value < min) {
		Serial.println("Value Out Of Range");
		return -1;
	}

	return value;
}

void zero_position() {

	ventMotor.setSpeed(1000);
	ventMotor.move(-1000000);

	pinMode(ZERO_POS, INPUT_PULLUP);
	while (!digitalRead(ZERO_POS))
	{
		ventMotor.run();
	}
	ventMotor.setCurrentPosition(0);
	ventMotor.move(1000);

	while (digitalRead(ZERO_POS))
	{
		ventMotor.run();
	}

	ventMotor.move(homePosition);
	ventMotor.runToPosition();

	Serial.println("Zeroed");


	ventPos = 0;
	zeroed = 1;

	conf.stepperSpeed = 5000;

	ventMotor.setAcceleration(acceleration);
	ventMotor.setSpeed(conf.stepperSpeed);
	ventMotor.setMaxSpeed(conf.stepperSpeed);
}


float getInhaleTime(int BPM) {

	float timeToBreathe = 60.0 / BPM;
	//Serial.println("timeToBreathe: " + String(timeToBreathe));
	float inhaleTime;

	if (conf.inhaleExhaleRatio == 1) {
		inhaleTime = timeToBreathe / 2.0f;
		//Serial.println("timeToInhale 1:1 " + String(inhaleTime));
		return inhaleTime * 1000; // returning time in ms
	}

	if (conf.inhaleExhaleRatio == 2) {
		inhaleTime = timeToBreathe / 3.0f;
		//Serial.println("timeToInhale 1:2 " + String(inhaleTime));
		return inhaleTime * 1000; 
	}


	if (conf.inhaleExhaleRatio == 3) {
		inhaleTime = timeToBreathe / 4.0f;
		//Serial.println("timeToInhale 1:2 " + String(inhaleTime));
		return inhaleTime * 1000;
	}


}


float getExhaleTime(int BPM) {

	float timeToBreathe = 60.0 / BPM;
	//Serial.println("timeToBreathe: " + String(timeToBreathe));
	float exhaleTime;

	if (conf.inhaleExhaleRatio == 1) {
		exhaleTime = timeToBreathe / 2.0f;
		//Serial.println("timeToInhale 1:1 " + String(inhaleTime));
		return exhaleTime * 1000;
	}

	if (conf.inhaleExhaleRatio == 2) {
		exhaleTime = (timeToBreathe / 3.0f) * 2.0f;
		//Serial.println("timeToInhale 1:2 " + String(inhaleTime));
		return exhaleTime * 1000;
	}


	if (conf.inhaleExhaleRatio == 3) {
		exhaleTime = (timeToBreathe / 4.0f) * 3.0f;
		//Serial.println("timeToInhale 1:2 " + String(inhaleTime));
		return exhaleTime * 1000;
	}

}


void debugAtInterval(String message) {

	unsigned long current_millis = millis();

	if (current_millis - lastDebug >= 1000) {

		Serial.println(message);
		lastDebug = millis();

	}


}


void setMotorSpeed(float timeToReach) {

	long stepsToGo = conf.motionLength - homePosition;
	Serial.println(stepsToGo);
	int speed = (stepsToGo * 1000) / timeToReach;
	conf.stepperSpeed = speed;
	Serial.print("Setting Motor Speed");
	Serial.println(speed);
}


void setAirVolume() {

	int airVolPercentage = conf.airVol / (maxAirVol / 100);
	conf.motionLength = (maxMotionLength / 100) * airVolPercentage;
	

}

void serialCom() {
	
	int conf_changed = 0;

	if (Serial.available()) {
		input = Serial.readStringUntil('\n');
		StaticJsonDocument<200> command;

		DeserializationError error = deserializeJson(command, input.c_str());

		if (error) {
			Serial.print(F("deserializeJson() failed: "));
			Serial.println(error.c_str());
			return;
		}

		if (command["breathsPerMinute"]) {

			setBreathsPerMinuteParam(command["breathsPerMinute"]);
			conf_changed = 1;

		}

		if (command["ieRate"]) {

			setInhaleExhaleRateParam(command["ieRate"]);
			conf_changed = 1;
		}

		if (command["airVol"]) {

			setAirVolParam(command["airVol"]);
			conf_changed = 1;
		}

		//if (command["motionLength"]) {
		//
		//	setMotionLength(command["motionLength"]);
		//	conf_changed = 1;
		//	zeroed = false;
		//}



		//if (command["stepSpeed"]) {

		//	conf.stepperSpeed = command["stepSpeed"];


		//}

		//if (command["motor"]) {
		//	if (command["motor"] == "on") {
		//		motorShouldRun = true;
		//	}
		//	
		//	if (command["motor"] == "off") {

		//		motorShouldRun = false;

		//	}


		//}

	}

	if (conf_changed) {

		EEPROM_writeAnything(0, conf);
		conf_changed = 0;
		Serial.println("Saved Config");
	}


}
