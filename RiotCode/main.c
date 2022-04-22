#include <stdio.h>
#include <string.h>
#include "periph/gpio.h"
#include "xtimer.h"
#include "thread.h"
#include "cpu.h"
#include "board.h"
#include "periph/pwm.h"
#include "servo.h"
#include "hd44780.h"
#include "hd44780_params.h"
#include "fmt.h"
#include "net/loramac.h"
#include "semtech_loramac.h"
#include "main.h"

#define APPEUI "0000000000000000"
#define DEVEUI "70B3D57ED004E9C9"
#define APPKEY "ACF56E9005262992A0D06E5C42192FD7"

//pins ultrasonic sensor
gpio_t trigger_pin = GPIO_PIN(PORT_A, 9); //D8 -> trigger
gpio_t echo_pin = GPIO_PIN(PORT_A, 8); //D7 -> echo

//pins load cell
gpio_t sck = GPIO_PIN(PORT_B, 13); //vicino D4
gpio_t dt = GPIO_PIN(PORT_B, 14); //vicino D5

//servo
#define DEV         PWM_DEV(0) //vicino 5V
#define CHANNEL     0
#define SERVO_MIN        (1000U)
#define SERVO_MAX        (2000U)
static servo_t servo;

//ultrasonic sensor
uint32_t echo_time;
uint32_t echo_time_start;

//LCD
hd44780_t dev;

/* PINS LCD
Pin 1 is connected directly to GND.
Pin 2 is connected directly to VCC +5V.
Pin 3 is used to set LCD contrast, for max use +5V or a 10k potentiometer.
Pin 4 (RS or "register select") is connected to pin 2 on the Arduino
Pin 5 (RW or "read/write") is connected directly to GND, i.e., unused. Also note: if you connect RW to your board that the LCD is driven by 5V, while many boards internally run at 3.3V - so this could fry the board :/
Pin 6 (EN or "enable") is connected to pin 3 on the Arduino.
Pins 7 - 10: Not connected.
Pin 11 on the LCD is connected to pin 4 on the Arduino.
Pin 12 on the LCD is connected to pin 5 on the Arduino.
Pin 13 on the LCD is connected to pin 6 on the Arduino.
Pin 14 on the LCD is connected to pin 7 on the Arduino.
Pin 15 is connected to one end of a 1k resistor, and its other end to VCC +5V.
Pin 16 is connected directly to GND.
*/

//LoRa
extern semtech_loramac_t loramac;

void echo_cb(void* arg){ //callback function - ultrasonic sensor
	int val = gpio_read(echo_pin);
	uint32_t echo_time_stop;

    (void) arg;

	if(val){
		echo_time_start = xtimer_now_usec();
	}
    else{
		echo_time_stop = xtimer_now_usec();
		echo_time = echo_time_stop - echo_time_start;
	}
}

int read_distance(void){ //ultrasonic sensor
	echo_time = 0;
	gpio_clear(trigger_pin);
	xtimer_usleep(20);
	gpio_set(trigger_pin);
	xtimer_msleep(100);
	return echo_time;
}

void set_servo(int flag){
    if (flag==0){
        servo_set(&servo, SERVO_MAX);
    }
    if (flag==1){
        servo_set(&servo, SERVO_MIN);
    }
}

unsigned long read_weight(void){ //load cell
    unsigned long val;
    val=0;
    for (int i=0; i<24; i++){
        gpio_set(sck);
        xtimer_usleep(20);
        val = val<<1;
        gpio_clear(sck);
        if (gpio_read(dt)>0){
            val++;
        }
    }
    gpio_set(sck);
    xtimer_usleep(20);
    val = val ^ 0x800000;
    gpio_clear(sck);
    return val;
}

void write_lcd(char* message){
    hd44780_clear(&dev);
    hd44780_home(&dev);
    hd44780_print(&dev, "Fill level");
    xtimer_sleep(1);
    hd44780_set_cursor(&dev, 0, 1);
    hd44780_print(&dev, message);
}

int loramac_setup(char *deui, char *aeui, char *akey, char *xdr){
    uint8_t deveui[LORAMAC_DEVEUI_LEN];
    uint8_t appeui[LORAMAC_APPEUI_LEN];
    uint8_t appkey[LORAMAC_APPKEY_LEN];
    uint8_t dr;
    uint8_t join_type;

    fmt_hex_bytes(deveui, deui);
    fmt_hex_bytes(appeui, aeui);
    fmt_hex_bytes(appkey, akey);
    dr = atoi(xdr);

    semtech_loramac_set_deveui(&loramac, deveui);
    semtech_loramac_set_appeui(&loramac, appeui);
    semtech_loramac_set_appkey(&loramac, appkey);
    semtech_loramac_set_dr(&loramac, dr);

    join_type = LORAMAC_JOIN_OTAA;
    semtech_loramac_join(&loramac, join_type);
    /*
    switch (semtech_loramac_join(&loramac, join_type))
    {
    case SEMTECH_LORAMAC_DUTYCYCLE_RESTRICTED:
        puts("Cannot join: dutycycle restriction");
        return 1;
    case SEMTECH_LORAMAC_BUSY:
        puts("Cannot join: mac is busy");
        return 1;
    case SEMTECH_LORAMAC_JOIN_FAILED:
        puts("Join procedure failed!");
        return 1;
    case SEMTECH_LORAMAC_ALREADY_JOINED:
        puts("Warning: already joined!");
        return 1;
    case SEMTECH_LORAMAC_JOIN_SUCCEEDED:
        puts("Join procedure succeeded!");
        break;
    default:
        break;
    }
    puts("SetUp and Join Successful");
    */
    return 0;
}

int loramac_send(char *message){
    uint8_t cnf = CONFIG_LORAMAC_DEFAULT_TX_MODE;
    uint8_t port = CONFIG_LORAMAC_DEFAULT_TX_PORT;

    semtech_loramac_set_tx_mode(&loramac, cnf);
    semtech_loramac_set_tx_port(&loramac, port);

    semtech_loramac_send(&loramac,(uint8_t *)message, strlen(message));
    
    /*
    switch (semtech_loramac_send(&loramac,(uint8_t *)message, strlen(message)))
    {
    case SEMTECH_LORAMAC_NOT_JOINED:
        puts("Cannot send: not joined");
        return 1;

    case SEMTECH_LORAMAC_DUTYCYCLE_RESTRICTED:
        puts("Cannot send: dutycycle restriction");
        return 1;

    case SEMTECH_LORAMAC_BUSY:
        puts("Cannot send: MAC is busy");
        return 1;

    case SEMTECH_LORAMAC_TX_ERROR:
        puts("Cannot send: error");
        return 1;

    case SEMTECH_LORAMAC_TX_CNF_FAILED:
        puts("Fail to send: no ACK received");
        return 1;
    }
    puts("Message Sent");
    */
    
    return 0;
}

void components_init(void){
    gpio_init(sck, GPIO_OUT);
    gpio_init(dt, GPIO_IN);

    gpio_init(trigger_pin, GPIO_OUT);
    gpio_init_int(echo_pin, GPIO_IN, GPIO_BOTH, &echo_cb, NULL);

    read_distance();

    servo_init(&servo, DEV, CHANNEL, SERVO_MIN, SERVO_MAX);

    hd44780_init(&dev, &hd44780_params[0]);

    char *deveui = DEVEUI;
    char *appeui = APPEUI;
    char *appkey = APPKEY;
    char *dr = "5";
    loramac_setup(deveui, appeui, appkey, dr);
}

int main(void){

    components_init();

    char *message = "test";
    loramac_send(message);

    set_servo(0); //chiudi
    set_servo(1); //apri

    int distance = read_distance();

    unsigned long weight = read_weight();

    write_lcd("0 - 100");

}