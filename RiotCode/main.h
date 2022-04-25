#ifndef MAIN_FUNCTIONS
#define MAIN_FUNCTIONS

void echo_cb(void* arg);
int read_distance(void);
void set_stepper(int flag);
unsigned long read_weight(void);
void write_oled(char* message);
int loramac_setup(char *deui, char *aeui, char *akey, char *xdr);
int loramac_send(char *message);
void components_init(void);

//define WASTE_TYPE = PLASTIC | METAL | GLASS | PAPER | FOOD | MIXED
#define WASTE_TYPE "MIXED"
//define LoRa configuration parameters
#define DEVEUI "70B3D57ED004E9C9"
#define APPEUI "0000000000000000"
#define APPKEY "ACF56E9005262992A0D06E5C42192FD7"
//define bin id
#define BIN_ID 0
//define bin height
#define MAX_DISTANCE 45
//define area in square meters
#define AREA 0.05

#define WASTE_TYPE_PLASTIC "PLASTIC"
#define WASTE_TYPE_METAL "METAL"
#define WASTE_TYPE_GLASS "GLASS"
#define WASTE_TYPE_PAPER "PAPER"
#define WASTE_TYPE_FOOD "FOOD"
#define WASTE_TYPE_MIXED "MIXED"

#endif