/**
 * @author Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>
 * @brief GIF to BMP conversion library header
 * @date 4.4.2018
 * @file gif2bmp.h
 */
#ifndef __GIF2BMP_H_INCLUDED
#define __GIF2BMP_H_INCLUDED

#include <cstdint>
#include <cstdio>
#include <vector>

/**
 * @brief Convert code size to Clear Code (defined as 2^n)
 */
#define CLEARCODE(csize) (1 << (csize))
/**
 * @brief Convert code size to End of Information code (defined as 2^n + 1)
 */
#define EOICODE(csize) ((1 << (csize)) + 1)

/**
 * @brief Error-checking wrapper around fread
 * @see man fread(3)
 */
#define BFREAD(dest, size, file) do { \
    if(fread(dest, size, 1, file) != 1) { \
        perror("fread()"); \
        return 1; \
    } \
} while(0)

/**
 * @brief Error-checking wrapper around fwrite
 * @see man fwrite(3)
 */
#define BFWRITE(src, size, count, file) do { \
    if(fwrite(src, size, count, file) != count) { \
        perror("fwrite()"); \
        return 1; \
    } \
} while(0)

/**
 * @brief Compare two bytes, print an error message and return 1 on mismatch
 */
#define CHECKBYTE(value, expected) do { \
    if(value != expected) { \
        fprintf(stderr, "Invalid file format (L:%d, B: %x)\n", __LINE__, value); \
        return 1; \
    } \
} while(0)

/**
 * @brief Print a byte in binary form
 */
#define PRINTB(byte) do { \
    for(int8_t i = 7; i >= 0; i--) \
        printf("%u", (1 & (byte >> i))); \
    printf("\n"); \
} while(0)

typedef std::vector<struct tColor> tCTable;

/**
 * @brief Structure representing an RGB pixel
 */
typedef struct tColor {
    uint8_t r;  /**< Red color */
    uint8_t g;  /**< Green color */
    uint8_t b;  /**< Blue color */
} tColor;

/**
 * @brief Structure representing an Image Descriptor part of a GIF
 * @see gifProcessImage for Image Descript structure description
 */
typedef struct {
    // Image Descriptor
    uint16_t leftPos;   /**< Image left position */
    uint16_t topPos;    /**< Image top position */
    uint16_t width;     /**< Image width */
    uint16_t height;    /**< Image height */

    struct {
        uint8_t localCTSize : 3;    /**< Local color table size */
        uint8_t reserved : 2;       /**< Reserved */
        uint8_t sortFlag : 1;       /**< Sort flag */
        uint8_t interlaceFlag : 1;  /**< Interlace flag */
        uint8_t localCT : 1;        /**< Local color table flag */
    } packed;

    tCTable lColorTable; /**< Local color table */

    std::vector<uint8_t> indexStream; /**< Image index stream */
} tImage;

/**
 * @brief Structure representing a Graphic Control Extension part of a GIF
 * @see gifProcessHeader for GCE structure description
 */
typedef struct {
    uint8_t blockSize; /**< Block size */

    struct {
        uint8_t transpColorFlag : 1; /**< Transparent color flag */
        uint8_t userInputFlag : 1;   /**< User input flag */
        uint8_t disposalMethod : 3;  /**< Disposal method */
        uint8_t reserved : 3;        /**< Reserved */
    } packed;

    uint16_t delayTime;         /**< Delay time */
    uint8_t transpColorIndex;   /**< Transparent color index */
} tGCExtension;

/**
 * @brief Structure representing a GIF file
 * @see tGCExtension, tImage
 * @see gifProcessHeader for GIF header structure description
 */
typedef struct {
    // Logical Screen Description
    uint16_t canvWidth;     /**< Canvas width */
    uint16_t canvHeight;    /**< Canvas height */

    struct {
        uint8_t globalCTSize : 3; /**< Global color table size */
        uint8_t sortFlag : 1;     /**< Sort flag */
        uint8_t colorRes : 3;     /**< Color resolution */
        uint8_t globalCT : 1;     /**< Global color table flag */
    } packed;

    uint8_t bgColorIndex;   /**< Background color index */
    uint8_t pxAspectRatio;  /**< Pixel aspect ration */

    tCTable gColorTable;    /**< Global color table */

    tGCExtension *gCExtension;  /**< Graphics Control Extension */

    tImage imageData; /**< Image data */
} tGIF;

/**
 * @brief Structure for image size data
 */
typedef struct {
    int64_t bmpSize;    /**< BMP file size (in bytes) */
    int64_t gifSize;    /**< GIF file size (in bytes) */
} tGIF2BMP;

/**
 * @brief Write a BMP file from the given data
 * @see Comments inside the bmpWrite function for BMP header structure description
 *
 * @param gifData Valid pointer to tGIF structure with parsed GIF data
 * @param ctable Used color table (can be global or local)
 * @param outputFile Valid pointer to an output file handle
 * @param size Valid pointer to an 64-bit integer where the final file
 *             size will be stored in
 *
 * @returns 0 on success, 1 otherwise
 */
int bmpWrite(tGIF *gifData, tCTable &ctable, FILE *outputFile, int64_t *size);

/**
 * @brief Create a BMP color table
 *
 * @param ctable GIF ctable
 * @param outputFile Valid pointer to an output file handle
 *
 * @returns 0 on success, 1 otherwise
 */
int bmpCreateCTable(tCTable &ctable, FILE *outputFile);

/**
 * @brief Create a BMP pixel array
 * @details In 8-bit BMP each pixel represents an index into a BMP color table
 *
 * @param gifData Valid pointer to tGif structure with parsed GIF data
 * @param pxArray Valid pointer to a std::vector where the pixels will be
 *                stored in
 *
 * @returns 0 on success, 1 otherwise
 */
int bmpCreatePixelArray(tGIF *gifData, std::vector<uint8_t> *pxArray);

/**
 * @brief Parse GIF header
 * @see Comments inside the gifProcessHeader for GIF header description
 *
 * @param gifData Valid pointer to a tGIF structure where the parsed data
 *                will be stored in
 * @param inputFile Valid pointer to an input file handle
 *
 * @returns 0 on success, 1 otherwise
 */
int gifProcessHeader(tGIF *gifData, FILE *inputFile);

/**
 * @brief Parse GIF color table (global/local)
 *
 * @param cTable tCTable structure where the parsed color table
 *               will be stored in
 * @param size Color table size
 * @param inputFile Valid pointer to an input file handle
 *
 * @returns 0 on success, 1 otherwise
 */
int gifProcessCT(tCTable &cTable, uint16_t size, FILE *inputFile);

/**
 * @brief Process GIF Graphics Control Extension
 *
 * @param ext Valid pointer to a tGCExtension structure
 * @param inputFile Valid pointer to an input file handle
 *
 * @returns 0 on success, 1 otherwise
 */
int gifProcessGCE(tGCExtension *ext, FILE *inputFile);

/**
 * @brief Process GIF Image Descriptor
 * @see Comments inside the gifProcessImage for Image Descriptor description
 *
 * @param gifData Valid pointer to a tGIF structure where the parsed data
 *                will be stored in
 * @param imageData Valid pointer to a tImage structure where the parsed
 *                  data will be stored in (most likely &gifData->imageData)
 *
 * @param inputFile Valid pointer to an input file handle
 *
 * @returns 0 on success, 1 otherwise
 */
int gifProcessImage(tGIF *gifData, tImage *imageData, FILE *inputFile);

/**
 * @brief Release memory allocated by a tGIF structure
 *
 * @param gifData Pointer to a gifData structure
 */
void freeGIF(tGIF *gifData);

/**
 * @brief Main library function which converts a GIF image to a BMP image
 *
 * @param gif2bmp Valid pointer to a tGIF2BMP structure where the final
 *                file sizes will be stored in
 * @param inputFile Valid pointer to a input file handle
 * @param outputFile Valid pointer to a output file handle
 *
 * @returns 0 on success, 1 otherwise
 */
int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile);

/**
 * @brief Debug function for dumping data from a tGIF structure
 *
 * @param gifData Valid pointer to a gifData structure
 */
void dumpGIF(tGIF *gifData);

#endif
