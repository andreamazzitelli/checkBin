#include <stdio.h>
#include <string.h>
#include "periph/gpio.h"
#include "xtimer.h"
#include "thread.h"
#include "cpu.h"
#include "board.h"
#include "fmt.h"
#include "net/loramac.h"
#include "semtech_loramac.h"
#include "periph/i2c.h"
#include "timex.h"
#include "ztimer.h"
#include "u8g2.h"
#include "u8x8_riotos.h"
#include "main.h"

#define CYCLE_TIMEOUT 5
#define MEASURE_TIMEOUT 2
#define ZERO_LOAD_CELL 8498

//Specific weights g/m3
#define SW_PLASTIC 25000
#define SW_METAL 100000
#define SW_GLASS 200000
#define SW_PAPER 200000
#define SW_FOOD 250000
#define SW_MIXED 85000

#ifndef WASTE_TYPE
#define SW SW_MIXED
#elif WASTE_TYPE == WASTE_TYPE_PLASTIC
#define SW SW_PLASTIC
#elif WASTE_TYPE == WASTE_TYPE_METAL
#define SW SW_METAL
#elif WASTE_TYPE == WASTE_TYPE_GLASS
#define SW SW_GLASS
#elif WASTE_TYPE == WASTE_TYPE_PAPER
#define SW SW_PAPER
#elif WASTE_TYPE == WASTE_TYPE_FOOD
#define SW SW_FOOD
#elif WASTE_TYPE == WASTE_TYPE_MIXED
#define SW SW_MIXED
#endif

#ifndef BIN_ID
#error "MISSING BIN ID"
#endif
#ifndef DEVEUI
#error "MISSING LoRa DEVEUI"
#endif
#ifndef APPEUI
#error "MISSING LoRa APPEUI"
#endif
#ifndef APPKEY
#error "MISSING LoRa APPKEY"
#endif
#ifndef MAX_DISTANCE
#error "MISSING Max_Distance"
#endif
#ifndef AREA
#error "MISSING Area"
#endif

//ultrasonic sensor
gpio_t trigger_pin = GPIO_PIN(PORT_A, 9); //D8 -> trigger
gpio_t echo_pin = GPIO_PIN(PORT_B, 12); //D9 -> echo
uint32_t echo_time;
uint32_t echo_time_start;

//load cell
gpio_t sck = GPIO_PIN(PORT_B, 14); //D12 -> SCK
gpio_t dt = GPIO_PIN(PORT_B, 15); //D11 -> DT

//stepper motor
gpio_t pin_step_1 = GPIO_PIN(PORT_B, 5); //D4 -> IN1
gpio_t pin_step_2 = GPIO_PIN(PORT_B, 7); //D5 -> IN2
gpio_t pin_step_3 = GPIO_PIN(PORT_B, 13); //D3 -> IN3
gpio_t pin_step_4 = GPIO_PIN(PORT_A, 8); //D7 -> IN4

//display pin SDA D14 and SCK D15
#define TEST_OUTPUT_I2C 4
u8g2_t u8g2;
u8x8_riotos_t user_data =
{
    .device_index = TEST_I2C,
    .pin_cs = TEST_PIN_CS,
    .pin_dc = TEST_PIN_DC,
    .pin_reset = TEST_PIN_RESET,
};

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
	return echo_time/58;
}

void set_stepper(int flag){ //stepper motor
    int steps = 0;
    int count = 500;
    while (count) {
        switch(steps) {
            case 0:
            gpio_clear(pin_step_1);
            gpio_clear(pin_step_2);
            gpio_clear(pin_step_3);
            gpio_set(pin_step_4);
            break;
            case 1:
            gpio_clear(pin_step_1);
            gpio_clear(pin_step_2);
            gpio_set(pin_step_3);
            gpio_clear(pin_step_4);
            break;
            case 2:
            gpio_clear(pin_step_1);
            gpio_set(pin_step_2);
            gpio_clear(pin_step_3);
            gpio_clear(pin_step_4);
            break;
            case 3:
            gpio_set(pin_step_1);
            gpio_clear(pin_step_2);
            gpio_clear(pin_step_3);
            gpio_clear(pin_step_4);
            break;
        } 
        steps=steps+flag;
        xtimer_msleep(2);
        if (steps>3){
            steps=0;
        }
        if (steps<0){
            steps=3;
        }
        count--;
    }
    gpio_clear(pin_step_1);
    gpio_clear(pin_step_2);
    gpio_clear(pin_step_3);
    gpio_clear(pin_step_4);

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
    val=val/1000;
    if (val>ZERO_LOAD_CELL){
        val=ZERO_LOAD_CELL;
    }
    int grams = (ZERO_LOAD_CELL-val)/0.104;
    if (grams<=0) return 0;
    else return grams;
}


void write_oled(char* message){ //Display

    u8g2_FirstPage(&u8g2);

    do {
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_SetFont(&u8g2, u8g2_font_helvB12_tf);
        u8g2_DrawStr(&u8g2, 12, 22, message);
    } while (u8g2_NextPage(&u8g2));

}

int loramac_setup(char *deui, char *aeui, char *akey, char *xdr){ //LoRa setup
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

int loramac_send(char *message){ //LoRa send
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

void components_init(void){ //initialize all pins and components
    gpio_init(sck, GPIO_OUT);
    gpio_init(dt, GPIO_IN);

    gpio_init(trigger_pin, GPIO_OUT);
    gpio_init_int(echo_pin, GPIO_IN, GPIO_BOTH, &echo_cb, NULL);

    read_distance();

    gpio_init(pin_step_1, GPIO_OUT);
    gpio_init(pin_step_2, GPIO_OUT);
    gpio_init(pin_step_3, GPIO_OUT);
    gpio_init(pin_step_4, GPIO_OUT);

    TEST_DISPLAY(&u8g2, U8G2_R0, u8x8_byte_hw_i2c_riotos, u8x8_gpio_and_delay_riotos);
    u8g2_SetUserPtr(&u8g2, &user_data);
    u8g2_SetI2CAddress(&u8g2, TEST_ADDR);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    char *deveui = DEVEUI;
    char *appeui = APPEUI;
    char *appkey = APPKEY;
    char *dr = "5";
    loramac_setup(deveui, appeui, appkey, dr);
}

int main(void){
    
    components_init();
    int distance_1;
    int distance_2;
    int distance_3;
    int distance;
    int fill_level=0;
    unsigned long weight;
    int estimated_weight;
    float max_weight = AREA*MAX_DISTANCE*0.01*SW;
    int old_fill_level;
    char st_fill[2];
    char msg[8];
    int stepper_status=0; //0 open, 1 closed

    while (true){
        distance=0;

        distance_1 = read_distance();
        xtimer_sleep(MEASURE_TIMEOUT);
        distance_2 = read_distance();
        xtimer_sleep(MEASURE_TIMEOUT);
        distance_3 = read_distance();

        if (distance_1-distance_2<3 && distance_2-distance_1<3){
            distance=distance_1;
        }
        else if(distance_2-distance_3<3 && distance_3-distance_2<3){
            distance=distance_2;
        }
        else if(distance_1-distance_3<3 && distance_3-distance_1<3){
            distance=distance_3;
        }
        
        if (distance>=MAX_DISTANCE){
            distance=MAX_DISTANCE-1;
        }

        old_fill_level = fill_level;
        fill_level = 9-(10*distance/MAX_DISTANCE);

        weight = read_weight();

        estimated_weight = (MAX_DISTANCE-distance)*0.01*AREA*SW;
        
        printf("distance: %d, weight: %lu, estimated_weigth: %d, fill_level: %d\n", distance, weight, estimated_weight, fill_level);
        
        if (weight>(1.2*max_weight) && fill_level<8){ //leave the bin open and set fill level to 9 (20% margin)
            if (stepper_status==1){
                set_stepper(-1);
                stepper_status=0;
            }
            sprintf(st_fill, "%d", 9);
            loramac_send(st_fill);
            puts("1. weight exceed but fill level low");
        }
        else if (old_fill_level!=fill_level){
            sprintf(st_fill, "%d", fill_level);
            loramac_send(st_fill);

            sprintf(msg, "Fill: %d", fill_level);
            write_oled(msg);

            puts("2. fill level changed");

            if (fill_level>=8 && weight<(0.8*estimated_weight)){ //leave the bin open (20% margin)
                if (stepper_status==1){
                    set_stepper(-1);
                    stepper_status=0;
                }
                puts("3. weight low but fill level high");
            }
            else if (fill_level>=8){ //close the bin
                if (stepper_status==0){
                    set_stepper(1);
                    stepper_status=1;
                }
                puts("4. fill level high");
            }
            else if (stepper_status==1){
                puts("5. open");
                set_stepper(-1);
                stepper_status=0;
            }
        }

        xtimer_sleep(CYCLE_TIMEOUT);
    }

    return 0;
}