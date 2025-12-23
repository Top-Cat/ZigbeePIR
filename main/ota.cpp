/*#include <string.h>

#include "bspatch.h"
#include "ota.h"

struct esp_parititon_write {
    esp_partition_t* parition;
    uint64_t position;
};

static esp_err_t partitionInput(const struct bspatch_stream_i* stream, void* buffer, int pos, int length) {
    return esp_partition_read((esp_partition_t*)stream->opaque, pos, buffer, length);
}

static esp_err_t partitionInputSimple(const struct bspatch_stream* stream, void* buffer, int length) {
    return esp_partition_read((esp_partition_t*)stream->opaque, 0, buffer, length);
}

static esp_err_t partitionOutput(const struct bspatch_stream_n* stream, const void* buffer, int length) {
    esp_parititon_write* newInfo = (esp_parititon_write*) stream->opaque;
    esp_err_t ret = esp_partition_write(newInfo->parition, newInfo->position, buffer, length);
    if (ret == ESP_OK) {
        newInfo->position += length;
    }
    return ret;
}

void applyPatch(esp_partition_t* otaTemp, esp_partition_t* otaOld, esp_partition_t* otaNew) {
    bspatch_stream stream;
    bspatch_stream_i input;
    bspatch_stream_n output;

    stream.read = partitionInputSimple;
    stream.opaque = otaOld;

    input.read = partitionInput;
    input.opaque = otaTemp;

    output.write = partitionOutput;
    esp_parititon_write outputInfo = {
        .parition = otaNew,
        .position = 0
    };
    output.opaque = &outputInfo;

    bspatch(&input, otaTemp->size, &output, otaNew->size, &stream);
}

void otaTest() {
    printf("Hello World");
}
*/
