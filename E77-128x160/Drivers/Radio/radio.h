#ifndef __RADIO_H__
#define __RADIO_H__

#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

/* Private typedef -----------------------------------------------------------*/

//those from radio_def.h:

//brief Radio driver callback functions
typedef struct
{
    void    ( *TxDone )( void );

    void    ( *TxTimeout )( void );

    void    ( *RxDone )( uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo );

    void    ( *RxTimeout )( void );

    void    ( *RxError )( void );

    void ( *FhssChangeChannel )( uint8_t currentChannel );

    void ( *CadDone ) ( bool channelActivityDetected );
}RadioEvents_t;

/*******************************************Radio LORA enum*****************************************/
typedef enum radio_status_e
{
    RADIO_STATUS_OK,
    RADIO_STATUS_UNSUPPORTED_FEATURE,
    RADIO_STATUS_UNKNOWN_VALUE,
    RADIO_STATUS_ERROR,
} radio_status_t;

typedef enum
{
//    MODEM_FSK = 0,
    MODEM_LORA,
}RadioModems_t;

//those from radio_ex.h:

typedef enum
{
//    GENERIC_FSK = 0,
    GENERIC_LORA,
}GenericModems_t;

typedef enum
{
    RADIO_LORA_SF5                                = 0x05,
    RADIO_LORA_SF6                                = 0x06,
    RADIO_LORA_SF7                                = 0x07,
    RADIO_LORA_SF8                                = 0x08,
    RADIO_LORA_SF9                                = 0x09,
    RADIO_LORA_SF10                               = 0x0A,
    RADIO_LORA_SF11                               = 0x0B,
    RADIO_LORA_SF12                               = 0x0C,
}RADIO_LoRaSpreadingFactors_t;

typedef enum
{
    RADIO_LORA_CR_4_5                             = 0x01,
    RADIO_LORA_CR_4_6                             = 0x02,
    RADIO_LORA_CR_4_7                             = 0x03,
    RADIO_LORA_CR_4_8                             = 0x04,
}RADIO_LoRaCodingRates_t;

typedef enum
{
    RADIO_LORA_BW_500                             = 6,
    RADIO_LORA_BW_250                             = 5,
    RADIO_LORA_BW_125                             = 4,
    RADIO_LORA_BW_062                             = 3,
    RADIO_LORA_BW_041                             = 10,
    RADIO_LORA_BW_031                             = 2,
    RADIO_LORA_BW_020                             = 9,
    RADIO_LORA_BW_015                             = 1,
    RADIO_LORA_BW_010                             = 8,
    RADIO_LORA_BW_007                             = 0,
}RADIO_LoRaBandwidths_t;

typedef enum
{
    RADIO_LORA_PACKET_VARIABLE_LENGTH             = 0x00,         //!< The packet is on variable size, header included
    RADIO_LORA_PACKET_FIXED_LENGTH                = 0x01,         //!< The packet is known on both sides, no header included in the packet
    RADIO_LORA_PACKET_EXPLICIT                    = RADIO_LORA_PACKET_VARIABLE_LENGTH,
    RADIO_LORA_PACKET_IMPLICIT                    = RADIO_LORA_PACKET_FIXED_LENGTH,
}RADIO_LoRaPacketLengthsMode_t;

typedef enum
{
    RADIO_LORA_CRC_ON                             = 0x01,         //!< CRC activated
    RADIO_LORA_CRC_OFF                            = 0x00,         //!< CRC not used
}RADIO_LoRaCrcModes_t;

typedef enum
{
    RADIO_LORA_IQ_NORMAL                          = 0x00,
    RADIO_LORA_IQ_INVERTED                        = 0x01,
}RADIO_LoRaIQModes_t;

typedef enum
{
    RADIO_LORA_LOWDR_OPT_OFF                         = 0x00,  /*Forced to 0*/
    RADIO_LORA_LOWDR_OPT_ON                          = 0x01,  /*Forced to 1*/
    RADIO_LORA_LOWDR_OPT_AUTO                        = 0x02,  /*Forced to 1 when SF11 or SF12, 0 otherwise*/
}RADIO_Ld_Opt_t;

typedef struct
{
  uint32_t StopTimerOnPreambleDetect; /*0 inactive, otherwise active*/
  RADIO_LoRaSpreadingFactors_t SpreadingFactor;
  RADIO_LoRaBandwidths_t Bandwidth;
  RADIO_LoRaCodingRates_t Coderate;
  RADIO_Ld_Opt_t LowDatarateOptimize;/*0 inactive, 1 active, otherwise auto (active for SF11 and SF12)*/
  uint16_t PreambleLen;
  RADIO_LoRaPacketLengthsMode_t LengthMode;
  uint8_t MaxPayloadLength;
  RADIO_LoRaCrcModes_t CrcMode;
  RADIO_LoRaIQModes_t IqInverted;
} generic_param_rx_lora_t;

typedef struct
{
//  generic_param_rx_fsk_t fsk;
  generic_param_rx_lora_t lora;
} RxConfigGeneric_t;

typedef struct
{
  RADIO_LoRaSpreadingFactors_t SpreadingFactor;
  RADIO_LoRaBandwidths_t Bandwidth;
  RADIO_LoRaCodingRates_t Coderate;
  RADIO_Ld_Opt_t LowDatarateOptimize;                 /*0 inactive, otherwise active*/
  uint16_t PreambleLen;
  RADIO_LoRaPacketLengthsMode_t LengthMode;
  RADIO_LoRaCrcModes_t CrcMode;
  RADIO_LoRaIQModes_t IqInverted;
} generic_param_tx_lora_t;

typedef union
{
//  generic_param_tx_fsk_t fsk;
  generic_param_tx_lora_t lora;
} TxConfigGeneric_t;

//Radio driver internal state machine states definition
typedef enum
{
    RF_IDLE = 0,   //!< The radio is idle
    RF_RX_RUNNING, //!< The radio is in reception state
    RF_TX_RUNNING, //!< The radio is in transmission state
    RF_CAD,        //!< The radio is doing channel activity detection
} RadioState_t;

struct Radio_s
{
    /*!
     * \brief Initializes the radio
     *
     * \param [in] events Structure containing the driver callback functions
     */
    void    ( *Init )( RadioEvents_t *events );
    /*!
     * Return current radio status
     *
     * \return status Radio status.[RF_IDLE, RF_RX_RUNNING, RF_TX_RUNNING]
     */
    RadioState_t ( *GetStatus )( void );
    /*!
     * \brief Configures the radio with the given modem
     *
     * \param [in] modem Modem to be used [0: FSK, 1: LoRa]
     */
    void    ( *SetModem )( RadioModems_t modem );
    /*!
     * \brief Sets the channel frequency
     *
     * \param [in] freq         Channel RF frequency
     */
    void    ( *SetChannel )( uint32_t freq );
    /*!
     * \brief Checks if the channel is free for the given time
     *
     * \remark The FSK modem is always used for this task as we can select the Rx bandwidth at will.
     *
     * \param [in] freq                Channel RF frequency in Hertz
     * \param [in] rxBandwidth         Rx bandwidth in Hertz
     * \param [in] rssiThresh          RSSI threshold in dBm
     * \param [in] maxCarrierSenseTime Max time in milliseconds while the RSSI is measured
     *
     * \retval isFree         [true: Channel is free, false: Channel is not free]
     */
    bool    ( *IsChannelFree )( uint32_t freq, uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime );
    /*!
     * \brief Generates a 32 bits random value based on the RSSI readings
     *
     * \remark This function sets the radio in LoRa modem mode and disables
     *         all interrupts.
     *         After calling this function either Radio.SetRxConfig or
     *         Radio.SetTxConfig functions must be called.
     *
     * \retval randomValue    32 bits random value
     */
    uint32_t ( *Random )( void );
    /*!
     * \brief Sets the reception parameters
     *
     * \param [in] modem        Radio modem to be used [0: FSK, 1: LoRa]
     * \param [in] bandwidth    Sets the bandwidth
     *                          FSK : >= 2600 and <= 250000 Hz
     *                          LoRa: [0: 125 kHz, 1: 250 kHz,
     *                                 2: 500 kHz, 3: Reserved]
     * \param [in] datarate     Sets the Datarate
     *                          FSK : 600..300000 bits/s
     *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
     *                                10: 1024, 11: 2048, 12: 4096  chips]
     * \param [in] coderate     Sets the coding rate (LoRa only)
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     * \param [in] bandwidthAfc Sets the AFC Bandwidth (FSK only)
     *                          FSK : >= 2600 and <= 250000 Hz
     *                          LoRa: N/A ( set to 0 )
     * \param [in] preambleLen  Sets the Preamble length
     *                          FSK : Number of bytes
     *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
     * \param [in] symbTimeout  Sets the RxSingle timeout value
     *                          FSK : timeout in number of bytes
     *                          LoRa: timeout in symbols
     * \param [in] fixLen       Fixed length packets [0: variable, 1: fixed]
     * \param [in] payloadLen   Sets payload length when fixed length is used
     * \param [in] crcOn        Enables/Disables the CRC [0: OFF, 1: ON]
     * \param [in] freqHopOn    Enables disables the intra-packet frequency hopping
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: OFF, 1: ON]
     * \param [in] hopPeriod    Number of symbols between each hop
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: Number of symbols
     * \param [in] iqInverted   Inverts IQ signals (LoRa only)
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: not inverted, 1: inverted]
     * \param [in] rxContinuous Sets the reception in continuous mode
     *                          [false: single mode, true: continuous mode]
     */
    void    ( *SetRxConfig )( RadioModems_t modem, uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint32_t bandwidthAfc, uint16_t preambleLen,
                              uint16_t symbTimeout, bool fixLen,
                              uint8_t payloadLen,
                              bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                              bool iqInverted, bool rxContinuous );
    /*!
     * \brief Sets the transmission parameters
     *
     * \param [in] modem        Radio modem to be used [0: FSK, 1: LoRa]
     * \param [in] power        Sets the output power [dBm]
     * \param [in] fdev         Sets the frequency deviation (FSK only)
     *                          FSK : [Hz]
     *                          LoRa: 0
     * \param [in] bandwidth    Sets the bandwidth (LoRa only)
     *                          FSK : 0
     *                          LoRa: [0: 125 kHz, 1: 250 kHz,
     *                                 2: 500 kHz, 3: Reserved]
     * \param [in] datarate     Sets the Datarate
     *                          FSK : 600..300000 bits/s
     *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
     *                                10: 1024, 11: 2048, 12: 4096  chips]
     * \param [in] coderate     Sets the coding rate (LoRa only)
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     * \param [in] preambleLen  Sets the preamble length
     *                          FSK : Number of bytes
     *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
     * \param [in] fixLen       Fixed length packets [0: variable, 1: fixed]
     * \param [in] crcOn        Enables disables the CRC [0: OFF, 1: ON]
     * \param [in] freqHopOn    Enables disables the intra-packet frequency hopping
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: OFF, 1: ON]
     * \param [in] hopPeriod    Number of symbols between each hop
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: Number of symbols
     * \param [in] iqInverted   Inverts IQ signals (LoRa only)
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: not inverted, 1: inverted]
     * \param [in] timeout      Transmission timeout [ms]
     */
    void    ( *SetTxConfig )( RadioModems_t modem, int8_t power, uint32_t fdev,
                              uint32_t bandwidth, uint32_t datarate,
                              uint8_t coderate, uint16_t preambleLen,
                              bool fixLen, bool crcOn, bool freqHopOn,
                              uint8_t hopPeriod, bool iqInverted, uint32_t timeout );
    /*!
     * \brief Checks if the given RF frequency is supported by the hardware
     *
     * \param [in] frequency RF frequency to be checked
     * \retval isSupported [true: supported, false: unsupported]
     */
    bool    ( *CheckRfFrequency )( uint32_t frequency );
    /*!
     * \brief Computes the packet time on air in ms for the given payload
     *
     * \remark Can only be called once SetRxConfig or SetTxConfig have been called
     *
     * \param [in] modem      Radio modem to be used [0: FSK, 1: LoRa]
     * \param [in] bandwidth    Sets the bandwidth
     *                          FSK : >= 2600 and <= 250000 Hz
     *                          LoRa: [0: 125 kHz, 1: 250 kHz,
     *                                 2: 500 kHz, 3: Reserved]
     * \param [in] datarate     Sets the Datarate
     *                          FSK : 600..300000 bits/s
     *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
     *                                10: 1024, 11: 2048, 12: 4096  chips]
     * \param [in] coderate     Sets the coding rate (LoRa only)
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     * \param [in] preambleLen  Sets the Preamble length
     *                          FSK : Number of bytes
     *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
     * \param [in] fixLen       Fixed length packets [0: variable, 1: fixed]
     * \param [in] payloadLen   Sets payload length when fixed length is used
     * \param [in] crcOn        Enables/Disables the CRC [0: OFF, 1: ON]
     *
     * \retval airTime        Computed airTime (ms) for the given packet payload length
     */
    uint32_t  ( *TimeOnAir )( RadioModems_t modem, uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                              bool crcOn );
    /*!
     * \brief Sends the buffer of size. Prepares the packet to be sent and sets
     *        the radio in transmission
     *
     * \param [in] buffer     Buffer pointer
     * \param [in] size       Buffer size
     *
     * \retval status        (OK, ERROR, ...)
     */
    radio_status_t    ( *Send )( uint8_t *buffer, uint8_t size );
    /*!
     * \brief Sets the radio in sleep mode
     */
    void    ( *Sleep )( void );
    /*!
     * \brief Sets the radio in standby mode
     */
    void    ( *Standby )( void );
    /*!
     * \brief Sets the radio in reception mode for the given time
     * \param [in] timeout Reception timeout [ms]
     *                     [0: continuous, others timeout]
     */
    void    ( *Rx )( uint32_t timeout );
    /*!
     * \brief Start a Channel Activity Detection
     */
    void    ( *StartCad )( void );
    /*!
     * \brief Sets the radio in continuous wave transmission mode
     *
     * \param [in] freq       Channel RF frequency
     * \param [in] power      Sets the output power [dBm]
     * \param [in] time       Transmission mode timeout [s]
     */
    void    ( *SetTxContinuousWave )( uint32_t freq, int8_t power, uint16_t time );
    /*!
     * \brief Reads the current RSSI value
     *
     * \retval rssiValue Current RSSI value in [dBm]
     */
    int16_t ( *Rssi )( RadioModems_t modem );
    /*!
     * \brief Writes the radio register at the specified address
     *
     * \param [in] addr Register address
     * \param [in] data New register value
     */
    void    ( *Write )( uint16_t addr, uint8_t data );
    /*!
     * \brief Reads the radio register at the specified address
     *
     * \param [in] addr Register address
     * \retval data Register value
     */
    uint8_t ( *Read )( uint16_t addr );
    /*!
     * \brief Writes multiple radio registers starting at address
     *
     * \param [in] addr   First Radio register address
     * \param [in] buffer Buffer containing the new register's values
     * \param [in] size   Number of registers to be written
     */
    void    ( *WriteRegisters )( uint16_t addr, uint8_t *buffer, uint8_t size );
    /*!
     * \brief Reads multiple radio registers starting at address
     *
     * \param [in] addr First Radio register address
     * \param [out] buffer Buffer where to copy the registers data
     * \param [in] size Number of registers to be read
     */
    void    ( *ReadRegisters )( uint16_t addr, uint8_t *buffer, uint8_t size );
    /*!
     * \brief Sets the maximum payload length.
     *
     * \param [in] modem      Radio modem to be used [0: FSK, 1: LoRa]
     * \param [in] max        Maximum payload length in bytes
     */
    void    ( *SetMaxPayloadLength )( RadioModems_t modem, uint8_t max );
    /*!
     * \brief Sets the network to public or private. Updates the sync byte.
     *
     * \remark Applies to LoRa modem only
     *
     * \param [in] enable if true, it enables a public network
     */
    void    ( *SetPublicNetwork )( bool enable );
    /*!
     * \brief Gets the time required for the board plus radio to get out of sleep.[ms]
     *
     * \retval time Radio plus board wakeup time in ms.
     */
    uint32_t  ( *GetWakeupTime )( void );
    /*!
     * \brief Process radio irq
     */
    void    ( *IrqProcess )( void );
    /*!
     * \brief Sets the radio in reception mode with Max LNA gain for the given time
     *
     * \param [in] timeout Reception timeout [ms]
     *                     [0: continuous, others timeout]
     */
    void    ( *RxBoosted )( uint32_t timeout );
    /*!
     * \brief Sets the Rx duty cycle management parameters
     *
     * \param [in]  rxTime        Structure describing reception timeout value
     * \param [in]  sleepTime     Structure describing sleep timeout value
     */
    void    ( *SetRxDutyCycle )( uint32_t rxTime, uint32_t sleepTime );
    /*!
     * @brief Sets the Transmitter in continuous PRBS mode
     *
     * \remark power and datarate shall be configured prior calling TxPrbs
     */
    void    ( *TxPrbs )( void );
    /*!
     * \brief Sets the Transmitter in continuous un-modulated Carrier mode at power dBm
     *
     * \param [in] power Tx power in dBm
     */
    void    ( *TxCw )( int8_t power );
    /*!
     * \brief Sets the reception parameters
     *
     * \param [in] modem        Radio modem to be used [GENERIC_FSK or GENERIC_FSK]
     * \param [in] config       configuration of receiver
     *                          fsk field to be used if modem =GENERIC_FSK
     *                          lora field to be used if modem =GENERIC_LORA
     * \param [in] rxContinuous Sets the reception in continuous mode
     *                          [0: single mode, otherwise continuous mode]
     * \param [in] symbTimeout  Sets the RxSingle timeout value
     *                          FSK : timeout in number of bytes
     *                          LoRa: timeout in symbols
     * \return 0 when no parameters error, -1 otherwise
     */
    int32_t ( *RadioSetRxGenericConfig )( GenericModems_t modem, RxConfigGeneric_t* config, uint32_t rxContinuous, uint32_t symbTimeout );
    /*!
     * \brief Sets the transmission parameters
     *
     * \param [in] modem        Radio modem to be used [GENERIC_FSK or GENERIC_FSK or GENERIC_BPSK]
     * \param [in] config       configuration of receiver
     *                          fsk field to be used if modem =GENERIC_FSK
     *                          lora field to be used if modem =GENERIC_LORA
     *                          bpsk field to be used if modem =GENERIC_BPSK
     * \param [in] power        Sets the output power [dBm]
     * \param [in] timeout      Reception timeout [ms]
     * \return 0 when no parameters error, -1 otherwise
     */
    int32_t ( *RadioSetTxGenericConfig )( GenericModems_t modem, TxConfigGeneric_t* config, int8_t power, uint32_t timeout );
};

extern const struct Radio_s Radio;

#ifdef __cplusplus
}
#endif

#endif // __RADIO_H__
