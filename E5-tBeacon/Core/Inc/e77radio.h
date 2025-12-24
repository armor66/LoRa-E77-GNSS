#ifndef RADIO_HEADER
#define RADIO_HEADER

//#define LORA_SPREADING_FACTOR    	11         /* [SF7..SF12] */
//#define LORA_CODINGRATE          	2         /* [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
#define LORA_BANDWIDTH             	0         /* [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] */
//#define LORA_PREAMBLE_LENGTH      8         /* Same for Tx and Rx(the hardware adds 4 more symbols) */
#define LORA_SYMBOL_TIMEOUT       	5         /* Symbols */
#define LORA_FIX_LENGTH_PAYLOAD_ON	true
#define CRC_ON						true
#define CRC_OFF						false
#define LORA_IQ_NORMAL				0
#define LORA_IQ_INVERTED			1
#define TX_TIMEOUT_VALUE            1150	//for SF12 payload
//#define TCXO_WORKAROUND_TIME_MARGIN	50

//typedef enum
//{
//  RX_DONE,
//  RX_TO,
//  RX_ERR,
//  TX_START,
//  TX_DONE,
//  RX_START,
//  TX_TO,
//} States_t;

void radio_init(void);
void set_transmit_data(void);
void timer1_scanRadio_handle(void);


#endif /*RADIO_HEADER*/
