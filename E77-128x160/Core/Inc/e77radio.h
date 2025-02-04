#ifndef RADIO_HEADER
#define RADIO_HEADER

typedef enum
{
  RX_DONE,
  RX_TO,
  RX_ERR,
  TX_START,
  TX_DONE,
  RX_START,
  TX_TO,
} States_t;

void radio_init(void);
void set_transmit_data(void);
void timer1_scanRadio_handle(void);


#endif /*RADIO_HEADER*/
