#include <math.h>
#include "radio.h"
#include "timer.h"
#include "radio_driver.h"
#include "radio_conf.h"

/* Private typedef -----------------------------------------------------------*/
/*!
 * Radio hardware and global parameters
 */
typedef struct SubgRf_s
{
    RadioModems_t Modem;
    bool RxContinuous;
    uint32_t TxTimeout;
    uint32_t RxTimeout;
    struct
    {
        bool Previous;
        bool Current;
    } PublicNetwork;
    PacketParams_t PacketParams;
    PacketStatus_t PacketStatus;
    ModulationParams_t ModulationParams;
    RadioIrqMasks_t RadioIrq;
    uint8_t AntSwitchPaSelect;
    uint32_t RxDcPreambleDetectTimeout; /* 0:RxDutyCycle is off, otherwise on with  2*rxTime + sleepTime (See STM32WL Errata: RadioSetRxDutyCycle)*/
} SubgRf_t;

/* Private macro -------------------------------------------------------------*/
#define RADIO_BIT_MASK(__n)  (~(1<<__n))

/**
  * \brief Calculates ceiling division of ( X / N )
  *
  * \param [in] X numerator
  * \param [in] N denominator
  *
  */
#ifndef DIVC
#define DIVC( X, N )                ( ( ( X ) + ( N ) - 1 ) / ( N ) )
#endif

/**
  * \brief Calculates rounding division of ( X / N )
  *
  * \param [in] X numerator
  * \param [in] N denominator
  *
  */
#ifndef DIVR
#define DIVR( X, N )                ( ( ( X ) + ( ((X)>0?(N):(N))>>1 ) ) / ( N ) )
#endif

/* Private define ------------------------------------------------------------*/
/* */
/*can be overridden in radio_conf.h*/
#ifndef RADIO_LR_FHSS_IS_ON
#define RADIO_LR_FHSS_IS_ON 0
#endif /* !RADIO_LR_FHSS_IS_ON */
/*can be overridden in radio_conf.h*/
#ifndef XTAL_FREQ
#define XTAL_FREQ                                   32000000UL
#endif
/*can be overridden in radio_conf.h*/
#ifndef RADIO_IRQ_PROCESS_INIT
#define RADIO_IRQ_PROCESS_INIT()
#endif
/*can be overridden in radio_conf.h*/
#ifndef RADIO_IRQ_PROCESS
#define RADIO_IRQ_PROCESS()        RadioIrqProcess()
#endif
/*can be overridden in radio_conf.h*/
#ifndef RADIO_RX_TIMEOUT_PROCESS
#define RADIO_RX_TIMEOUT_PROCESS() RadioOnRxTimeoutProcess()
#endif
/*can be overridden in radio_conf.h*/
#ifndef RADIO_TX_TIMEOUT_PROCESS
#define RADIO_TX_TIMEOUT_PROCESS() RadioOnTxTimeoutProcess()
#endif
/*can be overridden in radio_conf.h*/
#ifndef IRQ_TX_DBG
#define IRQ_TX_DBG ((uint16_t) 0)
#endif
/*can be overridden in radio_conf.h*/
#ifndef IRQ_RX_DBG
#define IRQ_RX_DBG ((uint16_t) 0)
#endif
/*can be overridden in radio_conf.h*/
#ifndef RADIO_GENERIC_CONFIG_ENABLE
#define RADIO_GENERIC_CONFIG_ENABLE 1
#endif

#define RADIO_BUF_SIZE 255

/* Private function prototypes -----------------------------------------------*/

static void RadioInit( RadioEvents_t *events );

static RadioState_t RadioGetStatus( void );

static void RadioSetModem( RadioModems_t modem );

static void RadioSetChannel( uint32_t freq );

//remark The FSK modem is always used for this task as we can select the Rx bandwidth at will.
static bool RadioIsChannelFree( uint32_t freq, uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime );

/*!
 * \brief Generates a 32 bits random value based on the RSSI readings
 * \remark This function sets the radio in LoRa modem mode and disables
 *         all interrupts.
 *         After calling this function either Radio.SetRxConfig or
 *         Radio.SetTxConfig functions must be called.
 */
static uint32_t RadioRandom( void );

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
 * \param [in] FreqHopOn    Enables disables the intra-packet frequency hopping
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: OFF, 1: ON]
 * \param [in] HopPeriod    Number of symbols between each hop
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: Number of symbols
 * \param [in] iqInverted   Inverts IQ signals (LoRa only)
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: not inverted, 1: inverted]
 * \param [in] rxContinuous Sets the reception in continuous mode
 *                          [false: single mode, true: continuous mode]
 */
static void RadioSetRxConfig( RadioModems_t modem, uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint32_t bandwidthAfc, uint16_t preambleLen,
                              uint16_t symbTimeout, bool fixLen,
                              uint8_t payloadLen,
                              bool crcOn, bool FreqHopOn, uint8_t HopPeriod,
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
 * \param [in] FreqHopOn    Enables disables the intra-packet frequency hopping
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: OFF, 1: ON]
 * \param [in] HopPeriod    Number of symbols between each hop
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: Number of symbols
 * \param [in] iqInverted   Inverts IQ signals (LoRa only)
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: not inverted, 1: inverted]
 * \param [in] timeout      Transmission timeout [ms]
 */
static void RadioSetTxConfig( RadioModems_t modem, int8_t power, uint32_t fdev,
                              uint32_t bandwidth, uint32_t datarate,
                              uint8_t coderate, uint16_t preambleLen,
                              bool fixLen, bool crcOn, bool FreqHopOn,
                              uint8_t HopPeriod, bool iqInverted, uint32_t timeout );

/*!
 * \brief Checks if the given RF frequency is supported by the hardware
 *
 * \param [in] frequency RF frequency to be checked
 * \retval isSupported [true: supported, false: unsupported]
 */
static bool RadioCheckRfFrequency( uint32_t frequency );

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
static uint32_t RadioTimeOnAir( RadioModems_t modem, uint32_t bandwidth,
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
static radio_status_t RadioSend( uint8_t *buffer, uint8_t size );

static void RadioSleep( void );

static void RadioStandby( void );
//[0: continuous, others timeout]
static void RadioRx( uint32_t timeout );

static void RadioStartCad( void );
//brief Sets the radio in continuous wave transmission mode Transmission mode timeout [s]
static void RadioSetTxContinuousWave( uint32_t freq, int8_t power, uint16_t time );

static int16_t RadioRssi( RadioModems_t modem );
//brief Writes the radio register at the specified address
static void RadioWrite( uint16_t addr, uint8_t data );
//brief Reads the radio register at the specified address
static uint8_t RadioRead( uint16_t addr );
//brief Writes multiple radio registers starting at address
static void RadioWriteRegisters( uint16_t addr, uint8_t *buffer, uint8_t size );
//brief Reads multiple radio registers starting at address
static void RadioReadRegisters( uint16_t addr, uint8_t *buffer, uint8_t size );

static void RadioSetMaxPayloadLength( RadioModems_t modem, uint8_t max );
//brief Sets the network to public or private. Updates the sync byte.
static void RadioSetPublicNetwork( bool enable );
//brief Gets the time required for the board plus radio to get out of sleep.[ms]
static uint32_t RadioGetWakeupTime( void );

static void RadioIrqProcess( void );
//brief Sets the radio in reception mode with Max LNA gain for the given time
static void RadioRxBoosted( uint32_t timeout );
//brief Sets the Rx duty cycle management parameters
static void RadioSetRxDutyCycle( uint32_t rxTime, uint32_t sleepTime );
//brief radio IRQ callback
static void RadioOnDioIrq( RadioIrqMasks_t radioIrq );
//brief Tx timeout timer callback
static void RadioOnTxTimeoutIrq( void *context );
//brief Rx timeout timer callback
static void RadioOnRxTimeoutIrq( void *context );

static void RadioOnRxTimeoutProcess( void );

static void RadioOnTxTimeoutProcess( void );
//brief Sets the Transmitter in continuous PRBS mode
static void RadioTxPrbs( void );
//brief Sets the Transmitter in continuous un-modulated Carrier mode at power dBm
static void RadioTxCw( int8_t power );

/*!
 * \brief Sets the reception parameters
 *
 * \param [in] modem        Radio modem to be used [GENERIC_FSK or GENERIC_FSK]
 * \param [in] config       configuration of receiver
 *                          fsk field to be used if modem =GENERIC_FSK
*                           lora field to be used if modem =GENERIC_LORA
 * \param [in] rxContinuous Sets the reception in continuous mode
 *                          [0: single mode, otherwise continuous mode]
 * \param [in] symbTimeout  Sets the RxSingle timeout value
 *                          FSK : timeout in number of bytes
 *                          LoRa: timeout in symbols
 * \return 0 when no parameters error, -1 otherwise
 */
static int32_t RadioSetRxGenericConfig( GenericModems_t modem, RxConfigGeneric_t *config,
                                        uint32_t rxContinuous, uint32_t symbTimeout );

static int32_t RadioSetTxGenericConfig( GenericModems_t modem, TxConfigGeneric_t *config,
                                        int8_t power, uint32_t timeout );
//brief Convert the bandwidth enum to Hz value
static uint32_t RadioGetLoRaBandwidthInHz( RadioLoRaBandwidths_t bw );

/*!
 * \brief Computes the time on air LoRa numerator
 *
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
 * \retval numerator        time on air LoRa numerator
 */
static uint32_t RadioGetLoRaTimeOnAirNumerator( uint32_t bandwidth,
                                                uint32_t datarate, uint8_t coderate,
                                                uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                                                bool crcOn );

/* Private variables ---------------------------------------------------------*/
/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
    RadioInit,
    RadioGetStatus,
    RadioSetModem,
    RadioSetChannel,
    RadioIsChannelFree,
    RadioRandom,
    RadioSetRxConfig,
    RadioSetTxConfig,
    RadioCheckRfFrequency,
    RadioTimeOnAir,
    RadioSend,
    RadioSleep,
    RadioStandby,
    RadioRx,
    RadioStartCad,
    RadioSetTxContinuousWave,
    RadioRssi,
    RadioWrite,
    RadioRead,
    RadioWriteRegisters,
    RadioReadRegisters,
    RadioSetMaxPayloadLength,
    RadioSetPublicNetwork,
    RadioGetWakeupTime,
    RadioIrqProcess,
    RadioRxBoosted,
    RadioSetRxDutyCycle,
    RadioTxPrbs,
    RadioTxCw,
    RadioSetRxGenericConfig,
    RadioSetTxGenericConfig,
    /* LrFhss extended radio functions */
};

const RadioLoRaBandwidths_t Bandwidths[] = { LORA_BW_125, LORA_BW_250, LORA_BW_500 };

static uint8_t MaxPayloadLength = RADIO_BUF_SIZE;

static uint8_t RadioBuffer[RADIO_BUF_SIZE];

/*
 * Radio callbacks variable
 */
static RadioEvents_t *RadioEvents;

/*!
 * Radio hardware and global parameters
 */
SubgRf_t SubgRf;

/*!
 * Tx and Rx timers
 */
TimerEvent_t TxTimeoutTimer;
TimerEvent_t RxTimeoutTimer;

/* Private  functions ---------------------------------------------------------*/

static void RadioInit( RadioEvents_t *events )
{
    RadioEvents = events;

    SubgRf.RxContinuous = false;
    SubgRf.TxTimeout = 0;
    SubgRf.RxTimeout = 0;
    /*See STM32WL Errata: RadioSetRxDutyCycle*/
    SubgRf.RxDcPreambleDetectTimeout = 0;

    SUBGRF_Init( RadioOnDioIrq );
    /*SubgRf.publicNetwork set to false*/
    SubgRf.PublicNetwork.Current = false;
    SubgRf.PublicNetwork.Previous = false;

    RADIO_IRQ_PROCESS_INIT();

    SUBGRF_SetRegulatorMode( );

    SUBGRF_SetBufferBaseAddress( 0x00, 0x00 );
    SUBGRF_SetTxParams( RFO_LP, 0, RADIO_RAMP_200_US );
    SUBGRF_SetDioIrqParams( IRQ_RADIO_ALL, IRQ_RADIO_ALL, IRQ_RADIO_NONE, IRQ_RADIO_NONE );

    RadioSleep();
    // Initialize driver timeout timers
    TimerInit( &TxTimeoutTimer, RadioOnTxTimeoutIrq );
    TimerInit( &RxTimeoutTimer, RadioOnRxTimeoutIrq );
    TimerStop( &TxTimeoutTimer );
    TimerStop( &RxTimeoutTimer );
}

static RadioState_t RadioGetStatus( void )
{
    switch( SUBGRF_GetOperatingMode( ) )
    {
        case MODE_TX:
            return RF_TX_RUNNING;
        case MODE_RX:
            return RF_RX_RUNNING;
        case MODE_CAD:
            return RF_CAD;
        default:
            return RF_IDLE;
    }
}

static void RadioSetModem( RadioModems_t modem )
{
    SubgRf.Modem = modem;

        SUBGRF_SetPacketType( PACKET_TYPE_LORA );
        // Public/Private network register is reset when switching modems
        if( SubgRf.PublicNetwork.Current != SubgRf.PublicNetwork.Previous )
        {
            SubgRf.PublicNetwork.Current = SubgRf.PublicNetwork.Previous;
            RadioSetPublicNetwork( SubgRf.PublicNetwork.Current );
        }
}

static void RadioSetChannel( uint32_t freq )
{
    SUBGRF_SetRfFrequency( freq );
}

static bool RadioIsChannelFree( uint32_t freq, uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime )
{
    bool status = true;
//    int16_t rssi = 0;
//    uint32_t carrierSenseTime = 0;
//
//    RadioStandby( );
//
//    RadioSetModem( MODEM_FSK );
//
//    RadioSetChannel( freq );
//
//    // Set Rx bandwidth. Other parameters are not used.
//    RadioSetRxConfig( MODEM_FSK, rxBandwidth, 600, 0, rxBandwidth, 3, 0, false,
//                      0, false, 0, 0, false, true );
//    RadioRx( 0 );
//
//    RADIO_DELAY_MS( RadioGetWakeupTime( ) );
//
//    carrierSenseTime = TimerGetCurrentTime( );
//
//    // Perform carrier sense for maxCarrierSenseTime
//    while( TimerGetElapsedTime( carrierSenseTime ) < maxCarrierSenseTime )
//    {
//        rssi = RadioRssi( MODEM_FSK );
//
//        if( rssi > rssiThresh )
//        {
//            status = false;
//            break;
//        }
//    }
//    RadioStandby( );
    return status;
}

static uint32_t RadioRandom( void )
{
    uint32_t rnd = 0;

    /*
     * Radio setup for random number generation
     */
    // Disable modem interrupts
    SUBGRF_SetDioIrqParams( IRQ_RADIO_NONE, IRQ_RADIO_NONE, IRQ_RADIO_NONE, IRQ_RADIO_NONE );

    rnd = SUBGRF_GetRandom();

    return rnd;
}

static void RadioSetRxConfig( RadioModems_t modem, uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint32_t bandwidthAfc, uint16_t preambleLen,
                              uint16_t symbTimeout, bool fixLen,
                              uint8_t payloadLen,
                              bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                              bool iqInverted, bool rxContinuous )
{
    SubgRf.RxContinuous = rxContinuous;

    if( rxContinuous == true )
    {
        symbTimeout = 0;
    }
    if( fixLen == true )
    {
        MaxPayloadLength = payloadLen;
    }
    else
    {
        MaxPayloadLength = 0xFF;
    }

//case MODEM_LORA:
            SUBGRF_SetStopRxTimerOnPreambleDetect( false );
            SubgRf.ModulationParams.PacketType = PACKET_TYPE_LORA;
            SubgRf.ModulationParams.Params.LoRa.SpreadingFactor = ( RadioLoRaSpreadingFactors_t )datarate;
            SubgRf.ModulationParams.Params.LoRa.Bandwidth = Bandwidths[bandwidth];
            SubgRf.ModulationParams.Params.LoRa.CodingRate = ( RadioLoRaCodingRates_t )coderate;

            if( ( ( bandwidth == 0 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
                ( ( bandwidth == 1 ) && ( datarate == 12 ) ) )
            {
                SubgRf.ModulationParams.Params.LoRa.LowDatarateOptimize = 0x01;
            }
            else
            {
                SubgRf.ModulationParams.Params.LoRa.LowDatarateOptimize = 0x00;
            }

            SubgRf.PacketParams.PacketType = PACKET_TYPE_LORA;

            if( ( SubgRf.ModulationParams.Params.LoRa.SpreadingFactor == LORA_SF5 ) ||
                ( SubgRf.ModulationParams.Params.LoRa.SpreadingFactor == LORA_SF6 ) )
            {
                if( preambleLen < 12 )
                {
                    SubgRf.PacketParams.Params.LoRa.PreambleLength = 12;
                }
                else
                {
                    SubgRf.PacketParams.Params.LoRa.PreambleLength = preambleLen;
                }
            }
            else
            {
                SubgRf.PacketParams.Params.LoRa.PreambleLength = preambleLen;
            }

            SubgRf.PacketParams.Params.LoRa.HeaderType = ( RadioLoRaPacketLengthsMode_t )fixLen;

            SubgRf.PacketParams.Params.LoRa.PayloadLength = MaxPayloadLength;
            SubgRf.PacketParams.Params.LoRa.CrcMode = ( RadioLoRaCrcModes_t )crcOn;
            SubgRf.PacketParams.Params.LoRa.InvertIQ = ( RadioLoRaIQModes_t )iqInverted;

            RadioStandby( );
            RadioSetModem( MODEM_LORA );
            SUBGRF_SetModulationParams( &SubgRf.ModulationParams );
            SUBGRF_SetPacketParams( &SubgRf.PacketParams );
            SUBGRF_SetLoRaSymbNumTimeout( symbTimeout );

            /* WORKAROUND - Set the step threshold value to 1 to avoid to miss low power signal after an interferer jam the chip in LoRa modulaltion */
            SUBGRF_WriteRegister(SUBGHZ_AGCCFG,SUBGRF_ReadRegister(SUBGHZ_AGCCFG)&0x1);

            /* WORKAROUND - Optimizing the Inverted IQ Operation, see STM32WL Erratasheet */
            if( SubgRf.PacketParams.Params.LoRa.InvertIQ == LORA_IQ_INVERTED )
            {
                // RegIqPolaritySetup = @address 0x0736
                SUBGRF_WriteRegister( SUBGHZ_LIQPOLR, SUBGRF_ReadRegister( SUBGHZ_LIQPOLR ) & ~( 1 << 2 ) );
            }
            else
            {
                // RegIqPolaritySetup @address 0x0736
                SUBGRF_WriteRegister( SUBGHZ_LIQPOLR, SUBGRF_ReadRegister( SUBGHZ_LIQPOLR ) | ( 1 << 2 ) );
            }
            /* WORKAROUND END */

            // Timeout Max, Timeout handled directly in SetRx function
            SubgRf.RxTimeout = 0xFFFF;
}

static void RadioSetTxConfig( RadioModems_t modem, int8_t power, uint32_t fdev,
                              uint32_t bandwidth, uint32_t datarate,
                              uint8_t coderate, uint16_t preambleLen,
                              bool fixLen, bool crcOn, bool freqHopOn,
                              uint8_t hopPeriod, bool iqInverted, uint32_t timeout )
{
//case MODEM_LORA:
            SubgRf.ModulationParams.PacketType = PACKET_TYPE_LORA;
            SubgRf.ModulationParams.Params.LoRa.SpreadingFactor = ( RadioLoRaSpreadingFactors_t ) datarate;
            SubgRf.ModulationParams.Params.LoRa.Bandwidth =  Bandwidths[bandwidth];
            SubgRf.ModulationParams.Params.LoRa.CodingRate= ( RadioLoRaCodingRates_t )coderate;

            if( ( ( bandwidth == 0 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
                ( ( bandwidth == 1 ) && ( datarate == 12 ) ) )
            {
                SubgRf.ModulationParams.Params.LoRa.LowDatarateOptimize = 0x01;
            }
            else
            {
                SubgRf.ModulationParams.Params.LoRa.LowDatarateOptimize = 0x00;
            }

            SubgRf.PacketParams.PacketType = PACKET_TYPE_LORA;

            if( ( SubgRf.ModulationParams.Params.LoRa.SpreadingFactor == LORA_SF5 ) ||
                ( SubgRf.ModulationParams.Params.LoRa.SpreadingFactor == LORA_SF6 ) )
            {
                if( preambleLen < 12 )
                {
                    SubgRf.PacketParams.Params.LoRa.PreambleLength = 12;
                }
                else
                {
                    SubgRf.PacketParams.Params.LoRa.PreambleLength = preambleLen;
                }
            }
            else
            {
                SubgRf.PacketParams.Params.LoRa.PreambleLength = preambleLen;
            }
            SubgRf.PacketParams.Params.LoRa.HeaderType = ( RadioLoRaPacketLengthsMode_t )fixLen;
            SubgRf.PacketParams.Params.LoRa.PayloadLength = MaxPayloadLength;
            SubgRf.PacketParams.Params.LoRa.CrcMode = ( RadioLoRaCrcModes_t )crcOn;
            SubgRf.PacketParams.Params.LoRa.InvertIQ = ( RadioLoRaIQModes_t )iqInverted;

            RadioStandby( );
            RadioSetModem( MODEM_LORA );
            SUBGRF_SetModulationParams( &SubgRf.ModulationParams );
            SUBGRF_SetPacketParams( &SubgRf.PacketParams );

    SubgRf.AntSwitchPaSelect = SUBGRF_SetRfTxPower( power );
    /* WORKAROUND - Trimming the output voltage power_ldo to 3.3V */
    SUBGRF_WriteRegister(REG_DRV_CTRL, 0x7 << 1);

    SubgRf.TxTimeout = timeout;
}

static bool RadioCheckRfFrequency( uint32_t frequency )
{
    return true;
}

static uint32_t RadioGetLoRaBandwidthInHz( RadioLoRaBandwidths_t bw )
{
    uint32_t bandwidthInHz = 0;

    switch( bw )
    {
    case LORA_BW_007:
        bandwidthInHz = 7812UL;
        break;
    case LORA_BW_010:
        bandwidthInHz = 10417UL;
        break;
    case LORA_BW_015:
        bandwidthInHz = 15625UL;
        break;
    case LORA_BW_020:
        bandwidthInHz = 20833UL;
        break;
    case LORA_BW_031:
        bandwidthInHz = 31250UL;
        break;
    case LORA_BW_041:
        bandwidthInHz = 41667UL;
        break;
    case LORA_BW_062:
        bandwidthInHz = 62500UL;
        break;
    case LORA_BW_125:
        bandwidthInHz = 125000UL;
        break;
    case LORA_BW_250:
        bandwidthInHz = 250000UL;
        break;
    case LORA_BW_500:
        bandwidthInHz = 500000UL;
        break;
    }

    return bandwidthInHz;
}

static uint32_t RadioGetLoRaTimeOnAirNumerator( uint32_t bandwidth,
                                                uint32_t datarate, uint8_t coderate,
                                                uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                                                bool crcOn )
{
    int32_t crDenom           = coderate + 4;
    bool    lowDatareOptimize = false;

    // Ensure that the preamble length is at least 12 symbols when using SF5 or SF6
    if( ( datarate == 5 ) || ( datarate == 6 ) )
    {
        if( preambleLen < 12 )
        {
            preambleLen = 12;
        }
    }

    if( ( ( bandwidth == 0 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
        ( ( bandwidth == 1 ) && ( datarate == 12 ) ) )
    {
        lowDatareOptimize = true;
    }

    int32_t ceilDenominator;
    int32_t ceilNumerator = ( payloadLen << 3 ) +
                            ( crcOn ? 16 : 0 ) -
                            ( 4 * datarate ) +
                            ( fixLen ? 0 : 20 );

    if( datarate <= 6 )
    {
        ceilDenominator = 4 * datarate;
    }
    else
    {
        ceilNumerator += 8;

        if( lowDatareOptimize == true )
        {
            ceilDenominator = 4 * ( datarate - 2 );
        }
        else
        {
            ceilDenominator = 4 * datarate;
        }
    }

    if( ceilNumerator < 0 )
    {
        ceilNumerator = 0;
    }

    // Perform integral ceil()
    int32_t intermediate =
        ( ( ceilNumerator + ceilDenominator - 1 ) / ceilDenominator ) * crDenom + preambleLen + 12;

    if( datarate <= 6 )
    {
        intermediate += 2;
    }

    return ( uint32_t )( ( 4 * intermediate + 1 ) * ( 1 << ( datarate - 2 ) ) );
}

static uint32_t RadioTimeOnAir( RadioModems_t modem, uint32_t bandwidth,
                                uint32_t datarate, uint8_t coderate,
                                uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                                bool crcOn )
{
    uint32_t numerator = 0;
    uint32_t denominator = 1;

//    case MODEM_LORA:
            numerator   = 1000U * RadioGetLoRaTimeOnAirNumerator( bandwidth, datarate,
                                    coderate, preambleLen, fixLen, payloadLen, crcOn );
            denominator = RadioGetLoRaBandwidthInHz( Bandwidths[bandwidth] );

    // Perform integral ceil()
    return DIVC( numerator, denominator );
}

static radio_status_t RadioSend( uint8_t *buffer, uint8_t size )
{
    SUBGRF_SetDioIrqParams( IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_TX_DBG,
                            IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_TX_DBG,
                            IRQ_RADIO_NONE,
                            IRQ_RADIO_NONE );
    /* Set RF switch */
    SUBGRF_SetSwitch( SubgRf.AntSwitchPaSelect, RFSWITCH_TX );
    /* WORKAROUND - Modulation Quality with 500 kHz LoRaTM Bandwidth*/
    /* RegTxModulation = @address 0x0889 */
    if( ( SubgRf.Modem == MODEM_LORA ) && ( SubgRf.ModulationParams.Params.LoRa.Bandwidth == LORA_BW_500 ) )
    {
        SUBGRF_WriteRegister( SUBGHZ_SDCFG0R, SUBGRF_ReadRegister( SUBGHZ_SDCFG0R ) & ~( 1 << 2 ) );
    }
    else
    {
        SUBGRF_WriteRegister( SUBGHZ_SDCFG0R, SUBGRF_ReadRegister( SUBGHZ_SDCFG0R ) | ( 1 << 2 ) );
    }
   /* WORKAROUND END */

            SubgRf.PacketParams.Params.LoRa.PayloadLength = size;
            SUBGRF_SetPacketParams( &SubgRf.PacketParams );
            SUBGRF_SendPayload( buffer, size, 0 );

        TimerSetValue( &TxTimeoutTimer, SubgRf.TxTimeout );
        TimerStart( &TxTimeoutTimer );

    return RADIO_STATUS_OK;
}

static void RadioSleep( void )
{
    SleepParams_t params = { 0 };

    params.Fields.WarmStart = 1;
    SUBGRF_SetSleep( params );

    RADIO_DELAY_MS( 2 );
}

static void RadioStandby( void )
{
    SUBGRF_SetStandby( STDBY_RC );
}

static void RadioRx( uint32_t timeout )
{

        SUBGRF_SetDioIrqParams( IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR | IRQ_RX_DBG,
                                IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR | IRQ_RX_DBG,
                                IRQ_RADIO_NONE,
                                IRQ_RADIO_NONE );

    if( timeout != 0 )
    {
        TimerSetValue( &RxTimeoutTimer, timeout );
        TimerStart( &RxTimeoutTimer );
    }
    /* switch off RxDcPreambleDetect See STM32WL Errata: RadioSetRxDutyCycle*/
    SubgRf.RxDcPreambleDetectTimeout = 0;

    /* RF switch configuration */
    SUBGRF_SetSwitch( SubgRf.AntSwitchPaSelect, RFSWITCH_RX );

    if( SubgRf.RxContinuous == true )
    {
        SUBGRF_SetRx( 0xFFFFFF ); // Rx Continuous
    }
    else
    {
        SUBGRF_SetRx( SubgRf.RxTimeout << 6 );
    }
}

static void RadioRxBoosted( uint32_t timeout )
{
        SUBGRF_SetDioIrqParams( IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR | IRQ_RX_DBG,
                                IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR | IRQ_RX_DBG,
                                IRQ_RADIO_NONE,
                                IRQ_RADIO_NONE );

    if( timeout != 0 )
    {
        TimerSetValue( &RxTimeoutTimer, timeout );
        TimerStart( &RxTimeoutTimer );
    }
    /* switch off RxDcPreambleDetect See STM32WL Errata: RadioSetRxDutyCycle*/
    SubgRf.RxDcPreambleDetectTimeout = 0;

    /* RF switch configuration */
    SUBGRF_SetSwitch( SubgRf.AntSwitchPaSelect, RFSWITCH_RX );

    if( SubgRf.RxContinuous == true )
    {
        SUBGRF_SetRxBoosted( 0xFFFFFF ); // Rx Continuous
    }
    else
    {
        SUBGRF_SetRxBoosted( SubgRf.RxTimeout << 6 );
    }
}

static void RadioSetRxDutyCycle( uint32_t rxTime, uint32_t sleepTime )
{
    /*See STM32WL Errata: RadioSetRxDutyCycle*/
    SubgRf.RxDcPreambleDetectTimeout = 2 * rxTime + sleepTime;
    /*Enable also the IRQ_PREAMBLE_DETECTED*/
    SUBGRF_SetDioIrqParams( IRQ_RADIO_ALL, IRQ_RADIO_ALL, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
    /* RF switch configuration */
    SUBGRF_SetSwitch( SubgRf.AntSwitchPaSelect, RFSWITCH_RX );
    /* Start Rx DutyCycle*/
    SUBGRF_SetRxDutyCycle( rxTime, sleepTime );
}

static void RadioStartCad( void )
{
    /* RF switch configuration */
    SUBGRF_SetSwitch( SubgRf.AntSwitchPaSelect, RFSWITCH_RX );

    SUBGRF_SetDioIrqParams( IRQ_CAD_CLEAR | IRQ_CAD_DETECTED,
                            IRQ_CAD_CLEAR | IRQ_CAD_DETECTED,
                            IRQ_RADIO_NONE,
                            IRQ_RADIO_NONE );
    SUBGRF_SetCad( );
}

static void RadioSetTxContinuousWave( uint32_t freq, int8_t power, uint16_t time )
{
    uint32_t timeout = ( uint32_t )time * 1000;
    uint8_t antswitchpow;

    SUBGRF_SetRfFrequency( freq );

    antswitchpow = SUBGRF_SetRfTxPower( power );

    /* WORKAROUND - Trimming the output voltage power_ldo to 3.3V */
    SUBGRF_WriteRegister(REG_DRV_CTRL, 0x7 << 1);

    /* Set RF switch */
    SUBGRF_SetSwitch( antswitchpow, RFSWITCH_TX );

    SUBGRF_SetTxContinuousWave( );

    TimerSetValue( &TxTimeoutTimer, timeout );
    TimerStart( &TxTimeoutTimer );
}

static int16_t RadioRssi( RadioModems_t modem )
{
    return SUBGRF_GetRssiInst( );
}

static void RadioWrite( uint16_t addr, uint8_t data )
{
    SUBGRF_WriteRegister( addr, data );
}

static uint8_t RadioRead( uint16_t addr )
{
    return SUBGRF_ReadRegister( addr );
}

static void RadioWriteRegisters( uint16_t addr, uint8_t *buffer, uint8_t size )
{
    SUBGRF_WriteRegisters( addr, buffer, size );
}

static void RadioReadRegisters( uint16_t addr, uint8_t *buffer, uint8_t size )
{
    SUBGRF_ReadRegisters( addr, buffer, size );
}

static void RadioSetMaxPayloadLength( RadioModems_t modem, uint8_t max )
{
    if( modem == MODEM_LORA )
    {
        SubgRf.PacketParams.Params.LoRa.PayloadLength = MaxPayloadLength = max;
        SUBGRF_SetPacketParams( &SubgRf.PacketParams );
    }
    else
    {
        if( SubgRf.PacketParams.Params.Gfsk.HeaderType == RADIO_PACKET_VARIABLE_LENGTH )
        {
            SubgRf.PacketParams.Params.Gfsk.PayloadLength = MaxPayloadLength = max;
            SUBGRF_SetPacketParams( &SubgRf.PacketParams );
        }
    }
}

static void RadioSetPublicNetwork( bool enable )
{
    SubgRf.PublicNetwork.Current = SubgRf.PublicNetwork.Previous = enable;

    RadioSetModem( MODEM_LORA );
    if( enable == true )
    {
        // Change LoRa modem SyncWord
        SUBGRF_WriteRegister( REG_LR_SYNCWORD, ( LORA_MAC_PUBLIC_SYNCWORD >> 8 ) & 0xFF );
        SUBGRF_WriteRegister( REG_LR_SYNCWORD + 1, LORA_MAC_PUBLIC_SYNCWORD & 0xFF );
    }
    else
    {
        // Change LoRa modem SyncWord
        SUBGRF_WriteRegister( REG_LR_SYNCWORD, ( LORA_MAC_PRIVATE_SYNCWORD >> 8 ) & 0xFF );
        SUBGRF_WriteRegister( REG_LR_SYNCWORD + 1, LORA_MAC_PRIVATE_SYNCWORD & 0xFF );
    }
}

static uint32_t RadioGetWakeupTime( void )
{
    return SUBGRF_GetRadioWakeUpTime() + RADIO_WAKEUP_TIME;
}

static void RadioOnTxTimeoutIrq( void *context )
{
    RADIO_TX_TIMEOUT_PROCESS();
}

static void RadioOnRxTimeoutIrq( void *context )
{
    RADIO_RX_TIMEOUT_PROCESS();
}

static void RadioOnTxTimeoutProcess( void )
{
    if( ( RadioEvents != NULL ) && ( RadioEvents->TxTimeout != NULL ) )
    {
        RadioEvents->TxTimeout( );
    }
}

static void RadioOnRxTimeoutProcess( void )
{
    if( ( RadioEvents != NULL ) && ( RadioEvents->RxTimeout != NULL ) )
    {
        RadioEvents->RxTimeout( );
    }
}

static void RadioOnDioIrq( RadioIrqMasks_t radioIrq )
{
    SubgRf.RadioIrq = radioIrq;

    RADIO_IRQ_PROCESS();
}

static void RadioIrqProcess( void )
{
    uint8_t size = 0;
    int32_t cfo = 0;

    switch( SubgRf.RadioIrq )
    {
    case IRQ_TX_DONE:

        TimerStop( &TxTimeoutTimer );

        //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
        SUBGRF_SetStandby( STDBY_RC );

        if( ( RadioEvents != NULL ) && ( RadioEvents->TxDone != NULL ) )
        {
            RadioEvents->TxDone( );
        }
        break;

    case IRQ_RX_DONE:

        TimerStop( &RxTimeoutTimer );
        if( SubgRf.RxContinuous == false )
        {
            //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
            SUBGRF_SetStandby( STDBY_RC );

            /* WORKAROUND - Implicit Header Mode Timeout Behavior, see STM32WL Erratasheet */
            SUBGRF_WriteRegister( SUBGHZ_RTCCTLR, 0x00 );
            SUBGRF_WriteRegister( SUBGHZ_EVENTMASKR, SUBGRF_ReadRegister( SUBGHZ_EVENTMASKR ) | ( 1 << 1 ) );
            /* WORKAROUND END */
        }
        SUBGRF_GetPayload( RadioBuffer, &size, 255 );
        SUBGRF_GetPacketStatus( &( SubgRf.PacketStatus ) );
        if( ( RadioEvents != NULL ) && ( RadioEvents->RxDone != NULL ) )
        {
            switch( SubgRf.PacketStatus.packetType )
            {
            case PACKET_TYPE_LORA:
                RadioEvents->RxDone( RadioBuffer, size, SubgRf.PacketStatus.Params.LoRa.RssiPkt,
                                     SubgRf.PacketStatus.Params.LoRa.SnrPkt );
                break;
            default:
                SUBGRF_GetCFO( SubgRf.ModulationParams.Params.Gfsk.BitRate, &cfo );
                RadioEvents->RxDone( RadioBuffer, size, SubgRf.PacketStatus.Params.Gfsk.RssiAvg, ( int8_t ) DIVR( cfo, 1000 ) );
                break;
            }
        }
        break;

    case IRQ_CAD_CLEAR:
        //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
        SUBGRF_SetStandby( STDBY_RC );
        if( ( RadioEvents != NULL ) && ( RadioEvents->CadDone != NULL ) )
        {
            RadioEvents->CadDone( false );
        }
        break;
    case IRQ_CAD_DETECTED:
        //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
        SUBGRF_SetStandby( STDBY_RC );
        if( ( RadioEvents != NULL ) && ( RadioEvents->CadDone != NULL ) )
        {
            RadioEvents->CadDone( true );
        }
        break;

    case IRQ_RX_TX_TIMEOUT:
        if( SUBGRF_GetOperatingMode( ) == MODE_TX )
        {
            TimerStop( &TxTimeoutTimer );
            //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
            SUBGRF_SetStandby( STDBY_RC );
            if( ( RadioEvents != NULL ) && ( RadioEvents->TxTimeout != NULL ) )
            {
                RadioEvents->TxTimeout( );
            }
        }
        else if( SUBGRF_GetOperatingMode( ) == MODE_RX )
        {
            TimerStop( &RxTimeoutTimer );
            //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
            SUBGRF_SetStandby( STDBY_RC );
            if( ( RadioEvents != NULL ) && ( RadioEvents->RxTimeout != NULL ) )
            {
                RadioEvents->RxTimeout( );
            }
        }
        break;
    case IRQ_PREAMBLE_DETECTED:
        /*See STM32WL Errata: RadioSetRxDutyCycle*/
        if( SubgRf.RxDcPreambleDetectTimeout != 0 )
        {
            /* Update Radio RTC period */
            Radio.Write( SUBGHZ_RTCPRDR2, ( SubgRf.RxDcPreambleDetectTimeout >> 16 ) & 0xFF ); /*Update Radio RTC Period MSB*/
            Radio.Write( SUBGHZ_RTCPRDR1, ( SubgRf.RxDcPreambleDetectTimeout >> 8 ) & 0xFF ); /*Update Radio RTC Period MidByte*/
            Radio.Write( SUBGHZ_RTCPRDR0, ( SubgRf.RxDcPreambleDetectTimeout ) & 0xFF ); /*Update Radio RTC Period lsb*/
            Radio.Write( SUBGHZ_RTCCTLR, Radio.Read( SUBGHZ_RTCCTLR ) | 0x1 ); /*restart Radio RTC*/
            SubgRf.RxDcPreambleDetectTimeout = 0;
            /*Clear IRQ_PREAMBLE_DETECTED mask*/
            SUBGRF_SetDioIrqParams( IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR | IRQ_RX_DBG,
                                    IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR | IRQ_RX_DBG,
                                    IRQ_RADIO_NONE,
                                    IRQ_RADIO_NONE );

        }
        break;

    case IRQ_SYNCWORD_VALID:
        break;

    case IRQ_HEADER_VALID:
        break;

    case IRQ_HEADER_ERROR:
        TimerStop( &RxTimeoutTimer );
        if( SubgRf.RxContinuous == false )
        {
            //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
            SUBGRF_SetStandby( STDBY_RC );
        }
        if( ( RadioEvents != NULL ) && ( RadioEvents->RxTimeout != NULL ) )
        {
            RadioEvents->RxTimeout( );
        }
        break;

    case IRQ_CRC_ERROR:

        if( SubgRf.RxContinuous == false )
        {
            //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
            SUBGRF_SetStandby( STDBY_RC );
        }
        if( ( RadioEvents != NULL ) && ( RadioEvents->RxError ) )
        {
            RadioEvents->RxError( );
        }
        break;

    default:
        break;
    }
}

static void RadioTxPrbs( void )
{
    SUBGRF_SetSwitch( SubgRf.AntSwitchPaSelect, RFSWITCH_TX );
    Radio.Write( SUBGHZ_GPKTCTL1AR, 0x2d );  // sel mode prbs9 instead of preamble
    SUBGRF_SetTxInfinitePreamble( );
    SUBGRF_SetTx( 0x0fffff );
}

static void RadioTxCw( int8_t power )
{
    uint8_t paselect = SUBGRF_SetRfTxPower( power );
    /* WORKAROUND - Trimming the output voltage power_ldo to 3.3V */
    SUBGRF_WriteRegister(REG_DRV_CTRL, 0x7 << 1);
    SUBGRF_SetSwitch( paselect, RFSWITCH_TX );
    SUBGRF_SetTxContinuousWave( );
}

static int32_t RadioSetRxGenericConfig( GenericModems_t modem, RxConfigGeneric_t *config, uint32_t rxContinuous,
                                        uint32_t symbTimeout )
{
#if (RADIO_GENERIC_CONFIG_ENABLE == 1)
    int32_t status = 0;

    uint8_t MaxPayloadLength;

    if( rxContinuous != 0 )
    {
        symbTimeout = 0;
    }
    SubgRf.RxContinuous = ( rxContinuous == 0 ) ? false : true;

//case GENERIC_LORA:
        if( config->lora.PreambleLen == 0 ) return -1;

        if( config->lora.LengthMode == RADIO_LORA_PACKET_FIXED_LENGTH )
        {
            MaxPayloadLength = config->lora.MaxPayloadLength;
        }
        else MaxPayloadLength = 0xFF;

        SUBGRF_SetStopRxTimerOnPreambleDetect( ( config->lora.StopTimerOnPreambleDetect == 0 ) ? false : true );
        SUBGRF_SetLoRaSymbNumTimeout( symbTimeout );

        SubgRf.ModulationParams.PacketType = PACKET_TYPE_LORA;
        SubgRf.ModulationParams.Params.LoRa.SpreadingFactor = ( RadioLoRaSpreadingFactors_t ) config->lora.SpreadingFactor;
        SubgRf.ModulationParams.Params.LoRa.Bandwidth = ( RadioLoRaBandwidths_t ) config->lora.Bandwidth;
        SubgRf.ModulationParams.Params.LoRa.CodingRate = ( RadioLoRaCodingRates_t ) config->lora.Coderate;
        switch( config->lora.LowDatarateOptimize )
        {
        case RADIO_LORA_LOWDR_OPT_OFF:
            SubgRf.ModulationParams.Params.LoRa.LowDatarateOptimize = 0;
            break;
        case RADIO_LORA_LOWDR_OPT_ON:
            SubgRf.ModulationParams.Params.LoRa.LowDatarateOptimize = 1;
            break;
        case RADIO_LORA_LOWDR_OPT_AUTO:
            if( ( config->lora.SpreadingFactor == RADIO_LORA_SF11 ) || ( config->lora.SpreadingFactor == RADIO_LORA_SF12 ) )
            {
                SubgRf.ModulationParams.Params.LoRa.LowDatarateOptimize = 1;
            }
            else
            {
                SubgRf.ModulationParams.Params.LoRa.LowDatarateOptimize = 0;
            }
            break;
        default:
            break;
        }

        SubgRf.PacketParams.PacketType = PACKET_TYPE_LORA;
        SubgRf.PacketParams.Params.LoRa.PreambleLength = config->lora.PreambleLen;
        SubgRf.PacketParams.Params.LoRa.HeaderType = ( RadioLoRaPacketLengthsMode_t ) config->lora.LengthMode;
        SubgRf.PacketParams.Params.LoRa.PayloadLength = MaxPayloadLength;
        SubgRf.PacketParams.Params.LoRa.CrcMode = ( RadioLoRaCrcModes_t ) config->lora.CrcMode;
        SubgRf.PacketParams.Params.LoRa.InvertIQ = ( RadioLoRaIQModes_t ) config->lora.IqInverted;

        RadioStandby( );
        RadioSetModem( MODEM_LORA );
        SUBGRF_SetModulationParams( &SubgRf.ModulationParams );
        SUBGRF_SetPacketParams( &SubgRf.PacketParams );

        /* WORKAROUND - Optimizing the Inverted IQ Operation, see STM32WL Erratasheet */
        if( SubgRf.PacketParams.Params.LoRa.InvertIQ == LORA_IQ_INVERTED )
        {
            SUBGRF_WriteRegister( SUBGHZ_LIQPOLR, SUBGRF_ReadRegister( SUBGHZ_LIQPOLR ) & ~( 1 << 2 ) );
        }
        else
        {
            SUBGRF_WriteRegister( SUBGHZ_LIQPOLR, SUBGRF_ReadRegister( SUBGHZ_LIQPOLR ) | ( 1 << 2 ) );
        }
        /* WORKAROUND END */

        // Timeout Max, Timeout handled directly in SetRx function
        SubgRf.RxTimeout = 0xFFFF;

    return status;
#else /* RADIO_GENERIC_CONFIG_ENABLE == 1*/
    return -1;
#endif /* RADIO_GENERIC_CONFIG_ENABLE == 0*/
}

static int32_t RadioSetTxGenericConfig( GenericModems_t modem, TxConfigGeneric_t *config, int8_t power,
                                        uint32_t timeout )
{
//case GENERIC_LORA:
        SubgRf.ModulationParams.PacketType = PACKET_TYPE_LORA;
        SubgRf.ModulationParams.Params.LoRa.SpreadingFactor = ( RadioLoRaSpreadingFactors_t ) config->lora.SpreadingFactor;
        SubgRf.ModulationParams.Params.LoRa.Bandwidth = ( RadioLoRaBandwidths_t ) config->lora.Bandwidth;
        SubgRf.ModulationParams.Params.LoRa.CodingRate = ( RadioLoRaCodingRates_t ) config->lora.Coderate;
        switch( config->lora.LowDatarateOptimize )
        {
        case RADIO_LORA_LOWDR_OPT_OFF:
            SubgRf.ModulationParams.Params.LoRa.LowDatarateOptimize = 0;
            break;
        case RADIO_LORA_LOWDR_OPT_ON:
            SubgRf.ModulationParams.Params.LoRa.LowDatarateOptimize = 1;
            break;
        case RADIO_LORA_LOWDR_OPT_AUTO:
            if( ( config->lora.SpreadingFactor == RADIO_LORA_SF11 ) || ( config->lora.SpreadingFactor == RADIO_LORA_SF12 ) )
            {
                SubgRf.ModulationParams.Params.LoRa.LowDatarateOptimize = 1;
            }
            else
            {
                SubgRf.ModulationParams.Params.LoRa.LowDatarateOptimize = 0;
            }
            break;
        default:
            break;
        }

        SubgRf.PacketParams.PacketType = PACKET_TYPE_LORA;
        SubgRf.PacketParams.Params.LoRa.PreambleLength = config->lora.PreambleLen;
        SubgRf.PacketParams.Params.LoRa.HeaderType = ( RadioLoRaPacketLengthsMode_t ) config->lora.LengthMode;
        SubgRf.PacketParams.Params.LoRa.CrcMode = ( RadioLoRaCrcModes_t ) config->lora.CrcMode;
        SubgRf.PacketParams.Params.LoRa.InvertIQ = ( RadioLoRaIQModes_t ) config->lora.IqInverted;

        RadioStandby( );
        RadioSetModem( MODEM_LORA );
        SUBGRF_SetModulationParams( &SubgRf.ModulationParams );
        SUBGRF_SetPacketParams( &SubgRf.PacketParams );

        /* WORKAROUND - Modulation Quality with 500 kHz LoRa Bandwidth, see STM32WL Erratasheet */
        if( SubgRf.ModulationParams.Params.LoRa.Bandwidth == LORA_BW_500 )
        {
            // RegTxModulation = @address 0x0889
            SUBGRF_WriteRegister( SUBGHZ_SDCFG0R, SUBGRF_ReadRegister( SUBGHZ_SDCFG0R ) & ~( 1 << 2 ) );
        }
        else
        {
            // RegTxModulation = @address 0x0889
            SUBGRF_WriteRegister( SUBGHZ_SDCFG0R, SUBGRF_ReadRegister( SUBGHZ_SDCFG0R ) | ( 1 << 2 ) );
        }
        /* WORKAROUND END */

    SubgRf.AntSwitchPaSelect = SUBGRF_SetRfTxPower( power );

    SubgRf.TxTimeout = timeout;
    return 0;
}

