#ifndef MAIN_FUNCTIONS
#define MAIN_FUNCTIONS

void echo_cb(void* arg);
int read_distance(void);
void set_tepper(int flag);
unsigned long read_weight(void);
void write_lcd(char* message);
int loramac_setup(char *deui, char *aeui, char *akey, char *xdr);
int loramac_send(char *message);
void components_init(void);

#endif