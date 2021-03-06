#ifndef VENUS638FLPX_H
#define VENUS638FLPX_H

#include <stdint.h>

#include "common/drivers/devices/gps/gps.h"
#include "common/drivers/serial.h"

#include "common/control/control_template.h"

#define READ_BUFFER_SIZE 1024

namespace Gps {

class Venus638FLPx 
        : public ControlTemplate< Venus638FLPx >
{

    struct Message {

        enum Id {
            NONE                        = 0x00
            , QUERY_SOFTWARE_VERSION    = 0x02
            , QUERY_SOFTWARE_CRC        = 0x03
            , SET_FACTORY_DEFAULTS      = 0x04
            , CONFIGURE_SERIAL_PORT     = 0x05
            , SOFTWARE_VERSION          = 0x81
            , SOFTWARE_CRC              = 0x82
            , ACK                       = 0x83
            , NACK                      = 0x84
        };

        enum SoftwareVersion {
            MESSAGE_ID          = 0x00
            , SOFTWARE_TYPE     = 0x01
            , KERNEL_VERSION    = 0x02
            , ODM_VERSION       = 0x06
            , REVISION          = 0x0A
        };

        uint8_t *mData;
        uint32_t mDataLength;

        static const uint8_t start_sequence[];
        static const uint8_t end_sequence[];
        static const uint16_t payload_length;
        static const uint16_t message_id_length;
        static const uint16_t checksum_length;
        static const uint16_t start_sequence_length;
        static const uint16_t end_sequence_length;

        Message( uint8_t id, uint8_t *messageBody, uint16_t messageSize );
        ~Message();

        const uint8_t *getData();
        uint32_t getDataLength();
    };

public:

    Venus638FLPx( Serial *serial );

    void dumpVersion();

    int32_t sendMessage( Message &message, uint8_t *response, uint32_t size );
    int32_t sendQuery( Message &message, uint8_t *response, uint32_t size );

    int32_t getSentence( Nmea::Sentence sentence, uint8_t *buffer, uint32_t size );

    uint32_t getBaudRate();
    int32_t setBaud( Serial::Speed baud );

private:


    static const char start_sequence[];
    static const char end_sequence[];

    static const char gps_sentence_start_sequence[];
    static const char gps_sentence_end_sequence[];

    static const int8_t start_sequence_length;
    static const int8_t end_sequence_length;
    static const int8_t payload_length;
    static const int8_t message_id_length;
    static const int8_t checksum_length;

    Serial *mSerial;
};

}

#endif // VENUS638FLPX_H
