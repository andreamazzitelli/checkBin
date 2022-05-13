#include <stdio.h>
#include <string.h>

#include "fmt.h"

#include "net/loramac.h"
#include "semtech_loramac.h"

#include "xtimer.h"
#include "random.h"

#define MAX 10
#define MIN 0
extern semtech_loramac_t loramac;

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

    semtech_loramac_set_deveui(&loramac, deveui); // setting deveui
    semtech_loramac_set_appeui(&loramac, appeui); // setting appeui
    semtech_loramac_set_appkey(&loramac, appkey); // setting appkey
    semtech_loramac_set_dr(&loramac, dr);         // setting dr

    // End Setup
    // Start Join

    join_type = LORAMAC_JOIN_OTAA;
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
    default: /* should not happen */
        break;
    }
    puts("SetUp and Join Successful");
    // End Join Procedure

    return 0;
}

int loramac_send(char *message)
{
    uint8_t cnf = CONFIG_LORAMAC_DEFAULT_TX_MODE;  /* Default: confirmable */
    uint8_t port = CONFIG_LORAMAC_DEFAULT_TX_PORT; /* Default: 2 */

    semtech_loramac_set_tx_mode(&loramac, cnf);
    semtech_loramac_set_tx_port(&loramac, port);


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
    return 0;
}

int fill_level_generator(int previous){        
        
        if(previous == 9){
            previous = 0;
            return previous;
        }
        
        int r_norm = random_uint32_range(MIN, MAX);
        if(r_norm == 0)r_norm+=1;
        
        if(r_norm + previous > 9)previous=9;
        else{
            previous += r_norm;
        }
        printf("RANDOM NUMBER: %d\n", r_norm);
        return previous;
    
}


int main(void){

    char *deveui = "0000000000000000";
    char *appeui = "0000000000000000";
    char *appkey = "00000000000000000000000000000000";
    char *dr = "5";

    loramac_setup(deveui, appeui, appkey, dr);

   int previous = 0;
   char msg[10];
   char *m = msg;

   random_init(1);
   while(1){
	printf("BEFORE\n");
	previous = fill_level_generator(previous);
	sprintf(msg, "%d", previous);
	
	loramac_send(m);
	printf("SEND: %s\n", msg);
	printf("Sent: %s\n", m);

	printf("AFTER--\n");
	xtimer_sleep(20);
}

    return 0;
}
