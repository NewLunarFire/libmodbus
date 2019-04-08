#ifndef __MODBUS_H__
#define __MODBUS_H__

#include <stdint.h>
#include <stdlib.h>

#define MODBUS_READ_COILS 1
#define MODBUS_READ_DISCRETE_INPUTS 2
#define MODBUS_READ_MULTIPLE_HOLDING_REGISTERS 3
#define MODBUS_READ_INPUT_REGISTERS 4
#define MODBUS_WRITE_SINGLE_COIL 5
#define MODBUS_READ_SINGLE_HOLDING_REGISTER 6
#define MODBUS_WRITE_MULTIPLE_COILS 15
#define MODBUS_WRITE_MULTIPLE_HOLDING_REGISTERS 16

typedef void* modbus_decoder_t;

typedef struct modbus_packet_t {
    uint16_t transaction_id;
    uint16_t protocol_id;
    uint16_t data_length;
    uint8_t unit_id;
    uint8_t function_code;
    uint8_t* data;
} modbus_packet_t;

modbus_decoder_t createModbusDecoder();
int decodeModbusPacket(modbus_decoder_t decoder, uint8_t* data, size_t length);
int encodeModbusPaket(modbus_packet_t packet, uint8_t* buffer);
modbus_packet_t getNextModbusPacket(modbus_decoder_t decoder);
void freeModbusDecoder(modbus_decoder_t decoder);

#endif
