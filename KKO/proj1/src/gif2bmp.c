/**
 * @author Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>
 * @brief GIF to BMP conversion library implementation
 * @date 4.4.2018
 * @file gif2bmp.c
 */
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <map>
#include <sys/types.h>

#include "gif2bmp.h"

int bmpWrite(tGIF *gifData, tCTable &ctable, FILE *outputFile, int64_t *size)
{
    assert(outputFile != NULL);

    uint16_t word = 0;
    uint32_t dword = 0;
    std::vector<uint8_t> pixels;

    if(bmpCreatePixelArray(gifData, &pixels) != 0) {
        fprintf(stderr, "Error: Failed to create BMP pixel array\n");
        return 1;
    }

    // Bitmap header
    // Header field (BM)
    uint8_t hfield[] = {0x42, 0x4d};
    BFWRITE(hfield, sizeof(*hfield), 2, outputFile);

    // BMP file size (14B BMP header, 40B DIB header)
    dword = 14 + 40 + (ctable.size() * (sizeof(ctable[0]) + 1)) + pixels.size();
    *size = dword;
    BFWRITE(&dword, sizeof(dword), 1, outputFile);

    // Reserved bytes
    dword = 0x0000;
    BFWRITE(&dword, sizeof(dword), 1, outputFile);

    // Starting address of the pixel array (BMP + DIB header + color table)
    dword = 14 + 40 + (ctable.size() * (sizeof(ctable[0]) + 1));
    BFWRITE(&dword, sizeof(dword), 1, outputFile);

    // DIB header (BITMAPINFOHEADER)
    // DIB header size (40B)
    dword = 40;
    BFWRITE(&dword, sizeof(dword), 1, outputFile);

    // Bitmap width
    dword = gifData->canvWidth;
    BFWRITE(&dword, sizeof(dword), 1, outputFile);

    // Bitmap height
    dword = -gifData->canvHeight;
    BFWRITE(&dword, sizeof(dword), 1, outputFile);

    // # of color planes (must be 1)
    word = 1;
    BFWRITE(&word, sizeof(word), 1, outputFile);

    // # of bits per pixel
    word = 8;
    BFWRITE(&word, sizeof(word), 1, outputFile);

    // Compression method (none - 0x0000)
    dword = 0;
    BFWRITE(&dword, sizeof(dword), 1, outputFile);

    // Size of raw bitmap data (for uncompressed data can be 0)
    dword = 0;
    BFWRITE(&dword, sizeof(dword), 1, outputFile);

    // Horizontal resolution for printing (0 - no preference)
    dword = 0;
    BFWRITE(&dword, sizeof(dword), 1, outputFile);

    // Vertical resolution for printing (0 - no preference)
    dword = 0;
    BFWRITE(&dword, sizeof(dword), 1, outputFile);

    // Number of colors (0 defaults to 2^n)
    dword = ctable.size();
    BFWRITE(&dword, sizeof(dword), 1, outputFile);

    // Number of important colors (0 - every color is important, generally ignored)
    dword = 0;
    BFWRITE(&dword, sizeof(dword), 1, outputFile);

    if(bmpCreateCTable(ctable, outputFile) != 0) {
        fprintf(stderr, "Failed to create BMP color table\n");
        return 1;
    }

    for(uint8_t px : pixels)
        BFWRITE(&px, sizeof(px), 1, outputFile);

    return 0;
}

int bmpCreateCTable(tCTable &ctable, FILE *outputFile)
{
    assert(outputFile != NULL);

    uint8_t term = 0x00;

    for(uint16_t i = 0; i < ctable.size(); i++) {
        // Colors are stored in B, G, R, 0x00 order
        BFWRITE(&ctable[i].b, sizeof(ctable[i].b), 1, outputFile);
        BFWRITE(&ctable[i].g, sizeof(ctable[i].g), 1, outputFile);
        BFWRITE(&ctable[i].r, sizeof(ctable[i].r), 1, outputFile);
        BFWRITE(&term, sizeof(term), 1, outputFile);
    }

    return 0;
}

int bmpCreatePixelArray(tGIF *gifData, std::vector<uint8_t> *pxArray)
{
    assert(gifData != NULL);
    assert(pxArray != NULL);

    uint64_t bytes_written = 0;

    pxArray->clear();

    for(size_t i = 0; i < gifData->imageData.indexStream.size(); i++) {
        pxArray->push_back(gifData->imageData.indexStream[i]);
        bytes_written++;

        // Each row has to be padded to 4 bytes
        if((i + 1) % gifData->canvWidth == 0) {
            for(uint8_t k = 0; k < bytes_written % 4; k++) {
                pxArray->push_back(0);
                bytes_written++;
            }
        }
    }

    return 0;;
}

int gifProcessHeader(tGIF *gifData, FILE *inputFile)
{
    if(!inputFile) {
        fprintf(stderr, "%d: Invalid file handle passed\n", __LINE__);
        return 1;
    }

    /* HEADER FORMAT
     *
     * Header Block (6b) [required]
     * signature:       3 bytes
     * version:         3 bytes
     *
     * Logical Screen Description (7b) [required]
     * canvas width:    2 bytes
     * canvas height:   2 bytes
     * packed field:    1 byte
     *      0 (MSB) global color table flag
     *      1 - 3   color resolution
     *      4       sort flag
     *      5 - 7   global color table size (actually 2^(N + 1))
     * background color index: 1 byte
     * pixel aspect ratio:    1 byte
     *
     * Global Color Table (2^(N + 1) colors, where color = 3b) [optional]
     *      R channel: 1 byte
     *      G channel: 1 byte
     *      B channel: 1 byte
     *
     * Graphics Control Extension [optional]
     * extension introducer:    1 byte (always 21)
     * graphic control label:   1 byte (always F9)
     * block size (in bytes):   1 byte
     * packed field:            1 byte
     *      0 (MSB) - 2     reserved for future use
     *      3 - 5           disposal method
     *      6               user input flag
     *      7               transparent color flag
     * delay time (unsigned):   2 bytes
     * transparent color index: 1 byte
     * block terminator:        1 byte (always 00)
     */

    char signature[4] = {0, };
    char version[4] = {0, };

    // Header Block
    fread(signature, sizeof(char), 3, inputFile);
    if(strncmp(signature, "GIF", 3) != 0) {
        fprintf(stderr, "Invalid file signature (%s)\n", signature);
        return 1;
    }

    fread(version, sizeof(char), 3, inputFile);
    if(strncmp(version, "89a", 3) != 0) {
        fprintf(stderr, "Unsupported GIF version (got: %s, supported: 89a)\n",
                version);
        return 1;
    }

    // Logical Screen Descriptor
    BFREAD(&(gifData->canvWidth), sizeof(gifData->canvWidth), inputFile);
    BFREAD(&(gifData->canvHeight), sizeof(gifData->canvHeight), inputFile);
    BFREAD(&(gifData->packed), sizeof(gifData->packed), inputFile);
    BFREAD(&(gifData->bgColorIndex), sizeof(gifData->bgColorIndex), inputFile);
    BFREAD(&(gifData->pxAspectRatio), sizeof(gifData->pxAspectRatio), inputFile);

    // Global Color Table
    if(gifData->packed.globalCT != 0) {
        // Size of GCT: 2^(N + 1)
        uint16_t size = pow(2, gifData->packed.globalCTSize + 1);
        gifData->gColorTable.resize(size);

        if(gifProcessCT(gifData->gColorTable, size, inputFile) != 0)
            return 1;
    }

    uint8_t byte;
    BFREAD(&byte, sizeof(byte), inputFile);

    // Global Control Extension (extension introducer byte = 0x21)
    if(byte == 0x21) {
        gifData->gCExtension = (tGCExtension*)malloc(sizeof(*(gifData->gCExtension)));
        if(gifData->gCExtension == NULL) {
            perror("malloc()");
            return 1;
        }

        if(gifProcessGCE(gifData->gCExtension, inputFile) != 0)
            return 1;

        BFREAD(&byte, sizeof(byte), inputFile);
    }

    // 0x2C - First byte of Image Descriptor (image separator byte)
    CHECKBYTE(byte, 0x2C);

    return 0;
}

int gifProcessCT(tCTable &cTable, uint16_t size, FILE *inputFile)
{
    assert(inputFile != NULL);

    for(uint16_t i = 0; i < size; i++) {
        BFREAD(&(cTable[i].r), sizeof(cTable[i].r), inputFile);
        BFREAD(&(cTable[i].g), sizeof(cTable[i].g), inputFile);
        BFREAD(&(cTable[i].b), sizeof(cTable[i].b), inputFile);
    }

    return 0;
}

int gifProcessGCE(tGCExtension *ext, FILE *inputFile)
{
    if(ext == NULL || inputFile == NULL) {
        fprintf(stderr, "%d: Invalid arguments\n", __LINE__);
        return 1;
    }

    uint8_t byte;

    // Graphic Control Label - always F9
    BFREAD(&byte, sizeof(byte), inputFile);
    CHECKBYTE(byte, 0xF9);

    BFREAD(&(ext->blockSize), sizeof(ext->blockSize), inputFile);
    BFREAD(&(ext->packed), sizeof(ext->packed), inputFile);
    BFREAD(&(ext->delayTime), sizeof(ext->delayTime), inputFile);
    BFREAD(&(ext->transpColorIndex), sizeof(ext->transpColorIndex), inputFile);

    // Block Terminator - always 00
    BFREAD(&byte, sizeof(byte), inputFile);
    CHECKBYTE(byte, 0x00);

    return 0;
}

int gifProcessImage(tGIF *gifData, tImage *imageData, FILE *inputFile)
{
    /* IMAGE FORMAT
     *
     * Image Descriptor 10b [required]
     * image separator  1 byte (always 2C)
     * image left       2 bytes
     * image top        2 bytes
     * image width      2 bytes
     * image height     2 bytes
     * packed field:    1 byte
     *      0 (MSB) local color table flag
     *      1       interlace flag
     *      2       sort flag
     *      3 - 4   reserved for future use
     *      5 - 7   local color table size (actually 2^(N + 1))
     *
     * Local Color Table (2^(N + 1) colors, where color = 3b) [optional]
     *      R channel: 1 byte
     *      G channel: 1 byte
     *      B channel: 1 byte
     */

    // 1) Image descriptor
    BFREAD(&(imageData->leftPos), sizeof(imageData->leftPos), inputFile);
    BFREAD(&(imageData->topPos), sizeof(imageData->topPos), inputFile);
    BFREAD(&(imageData->width), sizeof(imageData->width), inputFile);
    BFREAD(&(imageData->height), sizeof(imageData->height), inputFile);
    BFREAD(&(imageData->packed), sizeof(imageData->packed), inputFile);

    // 2) Local color table
    if(imageData->packed.localCT != 0) {
        // Size of LCT: 2^(N + 1)
        uint16_t size = pow(2, imageData->packed.localCTSize + 1);
        imageData->lColorTable.resize(size);

        if(gifProcessCT(imageData->lColorTable, size, inputFile) != 0)
            return 1;
    }

    // 3) Image data
    uint8_t code_size;
    uint8_t byte;
    uint8_t block_byte;
    std::vector<uint8_t> byte_data;

    BFREAD(&code_size, sizeof(code_size), inputFile);
    if(code_size < 2 || code_size > 12) {
        fprintf(stderr, "Invalid LZW minimum code size (%u)\n", code_size);
        return 1;
    }

    // Parse sub-blocks
    BFREAD(&byte, sizeof(byte), inputFile);

    while(byte != 0) {
        for(uint8_t i = 0; i < byte; i++) {
            BFREAD(&block_byte, sizeof(block_byte), inputFile);
            byte_data.push_back(block_byte);
        }

        BFREAD(&byte, sizeof(byte), inputFile);
    }

    code_size++;

    // Process sub-blocks
    std::map<uint16_t, std::vector<uint8_t> > code_table;
    tCTable *ctable;
    uint8_t init = 0;
    uint8_t ef_code_size = code_size;
    uint16_t code = 0;
    uint16_t prev_code = 0;
    uint16_t tmp_code = 0;
    uint32_t key;
    uint32_t bits_read = 0;

    if(imageData->packed.localCT != 0)
        ctable = &imageData->lColorTable;
    else
        ctable = &gifData->gColorTable;

    imageData->indexStream.clear();
    // Split the sub-blocks into codes
    // Algorithm:
    // 1) Init
    //  - initialize code table
    //  - let code be the first code in the code stream
    //  - output code to the index stream
    //  - set prev_code to code
    //  2) Loop
    //   - let code be the next code in the code stream
    //   - if code IS in the code table
    //      - output code_table[code] to the index stream
    //      - let K be the first index in code_table[code]
    //      - add code_table[prev_code] + K to the code table
    //   - otherwise
    //      - let K be the first index of code_table[prev_code]
    //      - output code_table[prev_code] + K to the index stream
    //      - add code_table[prev_code] + K to the code table
    //   - set prev_code to code
    while(bits_read <= (byte_data.size() - 1) * 8) {
        code = 0;
        for(int8_t j = 0; j < ef_code_size; j++) {
            code = code | ((1 & (byte_data[bits_read / 8] >> (bits_read % 8))) << j);
            bits_read++;
        }

        if(code == CLEARCODE(code_size - 1)) {
            // Initialize code table
            ef_code_size = code_size;
            code_table.clear();
            // Copy color indexes from the color table
            for(uint32_t i = 0; i < ctable->size(); i++)
                code_table[i].push_back(i);

            key = code_table.rbegin()->first;
            code_table[++key].push_back(-1);
            code_table[++key].push_back(-1);

            init = 1;
            continue;
        } else if(code == EOICODE(code_size - 1)) {
            // End of Information code
            break;
        } else if(code == (1 << ef_code_size) - 1) {
            // Increase code size, if possible
            if(ef_code_size < 12)
                ef_code_size++;
        }

        if(init != 0) {
            // First code
            imageData->indexStream.push_back(code);
            init = 0;
        } else {
            if(code_table.find(code) == code_table.end()) {
                // Code IS NOT in the code table
                if(code_table.find(prev_code) == code_table.end()) {
                    fprintf(stderr, "Invalid code encountered\n");
                    return 1;
                }

                tmp_code = code_table[prev_code][0];
                code_table[code] = code_table[prev_code];
                code_table[code].push_back(tmp_code);
                imageData->indexStream.insert(imageData->indexStream.end(),
                        code_table[code].begin(), code_table[code].end());
            } else {
                // Code IS in the code table
                imageData->indexStream.insert(imageData->indexStream.end(),
                        code_table[code].begin(), code_table[code].end());
                tmp_code = code_table[code][0];
                key = code_table.rbegin()->first + 1;
                code_table[key] = code_table[prev_code];
                code_table[key].push_back(tmp_code);
                if(key == (1u << ef_code_size) - 1) {
                    if(ef_code_size < 12)
                        ef_code_size++;
                }
            }
        }

        prev_code = code;
    }

    // Ignore rest of the GIF file, as it does not contain anything interesting

    return 0;
}

void freeGIF(tGIF *gifData)
{
    if(gifData == NULL)
        return;

    if(gifData->gCExtension != NULL)
        free(gifData->gCExtension);
}

int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile)
{
    if(gif2bmp == NULL || inputFile == NULL || outputFile == NULL) {
        fprintf(stderr, "gif2bmp: Invalid arguments\n");
        exit(EXIT_FAILURE);
    }

    tGIF gifData;
    tCTable *ctable = NULL;
    uint8_t ec = 0;
    size_t fpos;

    memset(&gifData, 0, sizeof(gifData));

    fpos = ftell(inputFile);
    if(gifProcessHeader(&gifData, inputFile) != 0) {
        ec = 1;
        goto cleanup;
    }

    if(gifProcessImage(&gifData, &(gifData.imageData), inputFile) != 0) {
        ec = 1;
        goto cleanup;
    }

    if(!gifData.imageData.lColorTable.empty())
        ctable = &gifData.imageData.lColorTable;
    else
        ctable = &gifData.gColorTable;

    if(bmpWrite(&gifData, *ctable, outputFile, &gif2bmp->bmpSize) != 0) {
        ec = 1;
        goto cleanup;
    }

    gif2bmp->gifSize = ftell(inputFile) - fpos + 1;

cleanup:
    freeGIF(&gifData);

    return ec;
}

void dumpGIF(tGIF *gifData)
{
    printf("Canvas width: %d\n"
           "Canvas height: %d\n"
           "Global CT: %x\n"
           "Color resolution: %x\n"
           "Sort flag: %x\n"
           "Global CT size: %x\n"
           "Global CT colors: %lu\n"
           "BG color index: %x\n"
           "PX aspect ratio: %x\n"
           "Local CT: %x\n"
           "Image width: %u\n"
           "Image height: %u\n"
           "Image top position: %u\n"
           "Image left position: %u\n",
           gifData->canvWidth, gifData->canvHeight,
           gifData->packed.globalCT, gifData->packed.colorRes,
           gifData->packed.sortFlag, gifData->packed.globalCTSize,
           gifData->gColorTable.size(),
           gifData->bgColorIndex, gifData->pxAspectRatio,
           gifData->imageData.packed.localCT,
           gifData->imageData.width, gifData->imageData.height,
           gifData->imageData.topPos, gifData->imageData.leftPos);
}

