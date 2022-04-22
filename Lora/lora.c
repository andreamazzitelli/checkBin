#include <stdio.h>
#include <string.h>

#include "fmt.h"

#include "net/loramac.h"
#include "semtech_loramac.h"

extern semtech_loramac_t loramac;

/* Application key is 16 bytes long (e.g. 32 hex chars), and thus the longest
   possible size (with application session and network session keys) */

int loramac_setup(char *deui, char *aeui, char *akey, char *xdr) //prova a cambare il char *xdr con un int tanto lui fa atoi quindi forse lo possiamo proprio togliere ci sta LORAMAC_DR_5
{ // NOTA non so se li posso passare in questo modo

    // se mettiamo le cose statiche non serve fare i conttrolli su lunghezze di DevEui etc comuqnue le metto qui per reference
    // - (strlen(deveui) != LORAMAC_DEVEUI_LEN * 2)
    // - (strlen(appeui) != LORAMAC_APPEUI_LEN * 2)
    // - (strlen(appkey) != LORAMAC_APPKEY_LEN * 2)
    // Per controllare il dr invece devo avere che il valore che voglio assegnargli NON sia dr > LORAMAC_DR_15

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

    switch (semtech_loramac_send(&loramac,(uint8_t *)message, strlen(message))) //mmmm questo cast qui non mi piace per nulla
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

int main(void){

    char *deveui = "70B3D57ED004E9C9";
    char *appeui = "0000000000000000";
    char *appkey = "ACF56E9005262992A0D06E5C42192FD7";
    char *dr = "5";
    char *message = "test";

    loramac_setup(deveui, appeui, appkey, dr);
    loramac_send(message);

    return 0;
}