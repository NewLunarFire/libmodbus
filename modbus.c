#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "modbus.h"

typedef struct modbus_dll_item {
    modbus_packet_t packet;
    struct modbus_dll_item *next;
} modbus_dll_item;

typedef struct modbus_decoder_it {
    size_t offset;
    uint16_t length;
    modbus_dll_item* first_packet;
    modbus_dll_item* last_packet;
    uint16_t transaction_id;
    uint16_t protocol_id;
    uint8_t unit_id;
    uint8_t function_code;
    uint8_t* data;
} modbus_decoder_it;

modbus_decoder_t createModbusDecoder() {
    modbus_decoder_it* decoder = malloc(sizeof(modbus_decoder_it));
    memset(decoder, 0, sizeof(modbus_decoder_it));
    return decoder;
}

void freeModbusDecoder(modbus_decoder_t decoder) {
    free(decoder);
}

int decodeModbusPacket(modbus_decoder_t in_decoder, uint8_t* data, size_t length) {
    modbus_decoder_it* decoder = (modbus_decoder_it*) in_decoder;
    modbus_dll_item* dll_item;
    int decoded = 0, i;

    for(i = 0; i < length; i++) {
        if(decoder->offset == 0 || decoder->offset == 1) {
            decoder->transaction_id = (decoder->transaction_id << 8) + data[i];
        } else if(decoder->offset == 2 || decoder->offset == 3) {
            decoder->protocol_id = (decoder->protocol_id << 8) + data[i];
        } else if(decoder->offset == 4 || decoder->offset == 5) {
            decoder->length = (decoder->length << 8) + data[i];
        } else if(decoder->offset == 6) {
            decoder->data = malloc(sizeof(uint8_t) * (decoder->length - 2));
            decoder->unit_id = data[i];
        } else if(decoder->offset == 7) {
            decoder->function_code = data[i];
        } else if(decoder->offset > 7) {
            decoder->data[decoder->offset - 8] = data[i];
        }

        decoder->offset++;
        if(decoder->offset > 7 && (decoder->offset - decoder->length) >= 6) {
            // Create new leaf node
            dll_item = malloc(sizeof(modbus_dll_item));
            dll_item->packet.transaction_id = decoder->transaction_id;
            dll_item->packet.protocol_id = decoder->protocol_id;
            dll_item->packet.data_length = decoder->length - 2;
            dll_item->packet.unit_id = decoder->unit_id;
            dll_item->packet.function_code = decoder->function_code;
            dll_item->packet.data = decoder->data;

            if(decoder->first_packet == NULL) {
                // No item in list, add as first and last
                decoder->first_packet = dll_item;
                decoder->last_packet = dll_item;
            } else {
                // Add at end
                decoder->last_packet->next = dll_item;
                decoder->last_packet = dll_item;
            }

            // Reset decoder for new packet
            decoder->offset = 0;
            decoder->transaction_id = 0;
            decoder->protocol_id = 0;
            decoder->length = 0;
            decoder->unit_id = 0;
            decoder->function_code = 0;
            decoder->data = NULL;

            // Save decoded count
            decoded++;
        }
    }

    return decoded;
}

int encodeModbusPaket(modbus_packet_t packet, uint8_t* buffer) {
	int i, j = 0;
	uint16_t packetLength = packet.data_length + 2;
	buffer[j++] = packet.transaction_id >> 8;
	buffer[j++] = packet.transaction_id & 0xFF;
	buffer[j++] = packet.protocol_id >> 8;
	buffer[j++] = packet.protocol_id & 0xFF;
	buffer[j++] = packetLength >> 8;
	buffer[j++] = packetLength & 0xFF;
	buffer[j++] = packet.unit_id;
	buffer[j++] = packet.function_code;

	for(i = 0; i < packet.data_length; i++) {
		buffer[j++] = packet.data[i];
	}

	return j;
}

modbus_packet_t getNextModbusPacket(modbus_decoder_t in_decoder) {
    modbus_decoder_it* decoder = (modbus_decoder_it*) in_decoder;
    modbus_packet_t packet = decoder->first_packet->packet;
    decoder->first_packet = decoder->first_packet->next;

    return packet;
}
