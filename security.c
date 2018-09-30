//gcc -Wall -o project merged.c -lwiringPi -lmpg123 -lao
//sudo ./project
#include <wiringPi.h>
#include <stdio.h>
#include <ao/ao.h>
#include <mpg123.h>
#include <string.h>
#include <stdlib.h>
#define BITS 8


int main(void){
	int texting = 0;
	//char web[300];
	//strcpy(web, "x-www-browser https://api.thingspeak.com/update?api_key=WJTVL22M3SM5CKCW&field3=1");
	char smsTimer[300];
	strcpy(smsTimer,"twilio_c_sms/bin/c_sms -a AC83271e39e29005a43519fa9a00192a9a -s 6c4b40e2818ade9efec9fdc5beca9265 -t \"+18606041754\" -f \"+18604216734\" -m \"Timer: Your door has been open too long!\"");
	char smsAlarm[300];
	strcpy(smsAlarm,"twilio_c_sms/bin/c_sms -a AC83271e39e29005a43519fa9a00192a9a -s 6c4b40e2818ade9efec9fdc5beca9265 -t \"+18606041754\" -f \"+18604216734\" -m \"ALARM: Someone opened your door!\"");
	mpg123_handle *mh, *mh2;
   	unsigned char *buffer, *buffer2;
    	size_t buffer_size, buffer_size2;
   	size_t done, done2;
	int err, err2;

  	int driver, driver2;
  	ao_device *dev, *dev2;

   	ao_sample_format format, format2;
   	int channels, channels2, encoding, encoding2;
  	long rate, rate2;

	ao_initialize();
   	driver = ao_default_driver_id();
    	driver2 = ao_default_driver_id();
  	mpg123_init();
  	mh = mpg123_new(NULL, &err);
  	mh2 = mpg123_new(NULL,&err2);
    	buffer_size = mpg123_outblock(mh);
    	buffer_size2 = mpg123_outblock(mh2);
    	buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
    	buffer2 = (unsigned char*) malloc(buffer_size2 * sizeof(unsigned char));

    	/* open the file and get the decoding format */
    	mpg123_open(mh, "/home/pi/Documents/CS/Project/Pizza.mp3");
    	mpg123_open(mh2,"/home/pi/Documents/CS/Project/Alarm.mp3");
    
    	mpg123_getformat(mh, &rate, &channels, &encoding);
    	mpg123_getformat(mh2, &rate2, &channels2, &encoding2);

    	/* set the output format and open the output device */
    	format.bits = mpg123_encsize(encoding) * BITS;
    	format.rate = rate;
    	format.channels = channels;
    	format.byte_format = AO_FMT_NATIVE;
    	format.matrix = 0;
    	dev = ao_open_live(driver, &format, NULL);
    
    
    	format2.bits = mpg123_encsize(encoding2) * BITS;
    	format2.rate = rate2;
    	format2.channels = channels2;
    	format2.byte_format = AO_FMT_NATIVE;
    	format2.matrix = 0;
    	dev2 = ao_open_live(driver2, &format2, NULL);
	


	const int SWITCH = 20;
	const int SLED = 21;
	const int BUTTON = 23;
	const int BLED = 24;

	wiringPiSetupGpio();

	pinMode(SLED, OUTPUT);
	pinMode(SWITCH, INPUT);
	pinMode(BLED, OUTPUT);
	pinMode(BUTTON, INPUT);
	digitalWrite(SLED, LOW);
	digitalWrite(BLED, LOW);

	int switchVal = 0;
	int switchPrev = 0;
	int switchState = 0;

	int buttonVal = 0;
	int buttonPrev = 0;
	int buttonState = 0;

	double timer = 0;
	double clock = 10;
	while(1){
		//printf("button: %d , switch: %d\n",buttonState,switchState);
		//Switch code
		if(!digitalRead(SWITCH))
			switchVal = 0; //closed
		else
			switchVal = 1; //open

		if((switchVal != switchPrev)){
			switchState = !switchState;
			if(switchState == 1){
				printf("button: %d , switch: %d\n",buttonState,switchState);
				digitalWrite(SLED, HIGH);
			}
			else{
				printf("button: %d , switch: %d\n",buttonState,switchState);
				digitalWrite(SLED, LOW);
				timer = 0;
			}
		}
		switchPrev = switchVal;
		
		//Button code
		if(!digitalRead(BUTTON))
			buttonVal = 1; //pressed
		else
			buttonVal = 0; //not pressed

		if((buttonVal == 1)&&(buttonVal != buttonPrev)){
			buttonState = !buttonState;
			if(buttonState == 1){
				printf("button: %d , switch: %d\n",buttonState,switchState);
				digitalWrite(BLED, HIGH);
			}
			else{
				printf("button: %d , switch: %d\n",buttonState,switchState);
				digitalWrite(BLED, LOW);
			}
		}
		buttonPrev = buttonVal;
		
		if(buttonState == 1 && switchState == 1){
			//printf("SEND NOTIF\n");
			if(texting)
				system(smsAlarm);
			//system(web);
			while(mpg123_read(mh2, buffer2, buffer_size2,&done2)==MPG123_OK
				&& digitalRead(SWITCH))
				ao_play(dev2, buffer2, done2);
		}

		if(timer>clock){
			//printf("SEND NOTIF\n");
			if(texting)
				system(smsTimer);
			while(mpg123_read(mh2, buffer2, buffer_size2,&done2)==MPG123_OK
				&& digitalRead(SWITCH))
				ao_play(dev2, buffer2, done2);
		}
		if(buttonState == 0 && switchState == 1){
			while(mpg123_read(mh, buffer, buffer_size,&done)==MPG123_OK
				&& digitalRead(SWITCH) && timer<clock){
				ao_play(dev, buffer, done);
				timer+= 0.4;
			}
		}
		delay(100); //avoid bounce
	}
	return 0;
}