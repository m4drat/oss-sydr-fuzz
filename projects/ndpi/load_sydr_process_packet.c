#include "ndpi_api.h"
#include "fuzz_common_code.h"

#include <stdint.h>
#include <stdio.h>

struct ndpi_detection_module_struct *ndpi_info_mod = NULL;
static ndpi_serializer json_serializer = {};
static ndpi_serializer csv_serializer = {};

int main(int argc, char **argv) {
        uint8_t protocol_was_guessed;

        fuzz_init_detection_module(&ndpi_info_mod, 0);
        ndpi_init_serializer(&json_serializer, ndpi_serialization_format_json);
        ndpi_init_serializer(&csv_serializer, ndpi_serialization_format_csv);

        struct ndpi_flow_struct *ndpi_flow = ndpi_flow_malloc(SIZEOF_FLOW_STRUCT);
        memset(ndpi_flow, 0, SIZEOF_FLOW_STRUCT);

        FILE *fd = fopen(argv[1], "rb");

        if (fd == NULL) return 1;
        fseek(fd, 0, SEEK_END);
        size_t Size = ftell(fd);
        fseek(fd, 0, SEEK_SET);

        unsigned char* Data = (unsigned char*) malloc(sizeof(unsigned char) * Size);
        fread(Data, 1, Size, fd);
        fclose(fd);

        ndpi_protocol detected_protocol =
                ndpi_detection_process_packet(ndpi_info_mod, ndpi_flow, Data, Size, 0, NULL);
        ndpi_protocol guessed_protocol =
                ndpi_detection_giveup(ndpi_info_mod, ndpi_flow, 1, &protocol_was_guessed);

        ndpi_reset_serializer(&json_serializer);
        ndpi_reset_serializer(&csv_serializer);
        if (protocol_was_guessed == 0)
        {
                ndpi_dpi2json(ndpi_info_mod, ndpi_flow, detected_protocol, &json_serializer);
                ndpi_dpi2json(ndpi_info_mod, ndpi_flow, detected_protocol, &csv_serializer);
        } else {
                ndpi_dpi2json(ndpi_info_mod, ndpi_flow, guessed_protocol, &json_serializer);
                ndpi_dpi2json(ndpi_info_mod, ndpi_flow, guessed_protocol, &csv_serializer);
        }

        ndpi_free_flow(ndpi_flow);
        free(Data);
        return 0;
}
