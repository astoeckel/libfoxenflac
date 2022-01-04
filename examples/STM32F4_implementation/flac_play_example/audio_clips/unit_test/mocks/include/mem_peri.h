/*! \file "mem_peri.h"
   \brief Memory Abstraction layer.
   Provides functions to interface with HAL drivers.
   Implements a simple file system consisting of "Chunks" and "Elements".

  Memory is arranged as follows:
  CHUNKS
  Each chunk of "x" sectors has a MemSectorObject attached to it,
  and is composed of:
       - 1x Chunk block: Status of chunk
           - Byte 0 (0-7): Cs: Number of sectors in Chunk (0 to 254, 0xFF denotes empty)
       - Elements:
           - Byte 1 onwards
       - Element blocks:
           - Located at end of Chunk in reverse order (if X is end of Chunk,
             and Y is the size of the block, ElementBlock 0 and 1 is at X-Y and X-2Y respectively)
           - Byte 0 (0): Written (0: written, 1: empty)
           - Byte 0 (1-7) to Byte 2 (0-3): Offset of END of element from start of chunk (19bits)
           - A 1x Element block buffer is placed between element data and element blocks
           - When writing an element block, the offset of the next element is written
             at the position of the next block

  Total amount of space available in a Chunk is:
       - (chunk_number_sectors * sector_size) - chunk_block_size - (element_block_size * (num_sectors + 1))

*/

#ifndef MEM_H_
#define MEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* INCLUDES ------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Driver Header files */


/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* CONSTANTS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

//Memory parameters
#define MEM_CHUNK_MAXELEMENTS           255
#define MEM_CHUNK_MINELEMENTS           0

/*!\var size_t MEM_START_ADDRESS
	Start of User Memory in NVS
*/
static const size_t MEM_START_ADDRESS = 0x0000;     //Start of User memory

/*!\var size_t MEM_END_ADDRESS
	End of User Memory in NVS
*/
static const size_t MEM_END_ADDRESS = 0x1DFFFF;     //End of User memory

/*!\var uint8_t MEM_BLOCK_WRITTEN
	Value in first byte of MemChunk if written
*/
static const uint8_t MEM_BLOCK_WRITTEN = 0;

/*!\var uint8_t MEM_CHUNK_EMPTY
	Value in first byte of MemChunk if empty
*/
static const uint8_t MEM_CHUNK_EMPTY = 0xFF;

/*!\var uint8_t MEM_CHUNKBLK_SIZE
	Size of MemChunk header in bytes
*/
static const uint8_t MEM_CHUNKBLK_SIZE = 1;

/*!\var uint8_t MEM_ELEMBLK_SIZE
	Size of Element header in bytes
*/
static const uint8_t MEM_ELEMBLK_SIZE = 3;

static const uint32_t MEM_ELEMBLK_WRITTEN_MASK = 0x01;
static const uint32_t MEM_ELEMBLK_OFFSET_MASK = 0xFFFFFE;
static const uint8_t MEM_ELEMBLK_OFFSET_SHIFT = 1;

/*!\def MEM_BUFF_SIZE
 BEWARE of making buffers too large. This can cause indeterminate behaviour in the system
 such as BT not connecting
*/
#define MEM_BUFF_SIZE       512

//Status bits
static const uint8_t MEM_STATBIT_RDY = 0;
static const uint8_t MEM_STATBIT_WEN = 1;

//Commands
static const uint8_t MEM_CMD_RDLP = 0x03;
static const uint8_t MEM_CMD_RDSR = 0x05;
static const uint8_t MEM_CMD_SSE = 0x20;
static const uint8_t MEM_CMD_WREN = 0x06;
static const uint8_t MEM_CMD_WRDI = 0x04;
static const uint8_t MEM_CMD_PP = 0x02;
static const uint8_t MEM_CMD_RJID = 0x9F;
static const uint8_t MEM_CMD_DP = 0xB9;
static const uint8_t MEM_CMD_RID = 0xAB;
static const uint8_t MEM_CMD_RSTEN = 0x66;
static const uint8_t MEM_CMD_RST = 0x99;
static const uint8_t MEM_CMD_CHE = 0x60;

//SSE
/*!\var uint32_t MEM_SECTORERASE_TIMEMAX
	Time to wait for an SSE in msec
*/
static const uint16_t MEM_SECTORERASE_TIMEMAX = 120;
//PP
static const uint16_t MEM_PP_TYPICAL = 1;

/*!\var uint16_t MEM_PP_TIMEOUT
	Time to wait for a PP in msec
*/
static const uint16_t MEM_PP_TIMEOUT = 10;
#define MEM_PP_MAXBYTES		256

//CHE
/*!\var uint16_t MEM_CHE_TIMEMAX
	Time to wait for a CHE in msec
*/
static const uint16_t MEM_CHE_TIMEMAX = 2400;

//PD
/*!\var uint16_t MEM_PD_TIME
	Time to wait for a deep power down event in msec
*/
static const uint16_t MEM_PD_TIME = 1;

//PD
/*!\var uint16_t MEM_RPD_TIME
	Time to wait for a deep power down recovery  event in msec
*/
static const uint16_t MEM_RPD_TIME = 1;

//OAD verification
#define OAD_IMAGE_HEADER_SIZE		128
#define OAD_IMAGE_HEADER_HASHOFFS	4
#define OAD_IMAGE_HEADER_HASHSZ		32

#define OAD_HASHVERIFY_SUCCESS			0
#define OAD_HASHVERIFY_RECMISMATCH		1
#define OAD_HASHVERIFY_CALCMISMATCH		2
#define OAD_HASHVERIFY_HASHPROCERR		3

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* VARIABLE DEFINITIONS ------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*!\enum MemOpStatus
	Memory operation statuses that may be returned by a memory function

	MEM_NVS_FAIL       NVS Access failure
    MEM_OP_FAIL        Operation failed, either chunk not written or details wrong
 	MEM_OP_SUCCESS     Success
*/
typedef enum
{
    MEM_NVS_FAIL = 0,
    MEM_OP_FAIL,
    MEM_OP_SUCCESS
} MemOpStatus;

/*!\typedef MemChunk
	Memory Chunk object. Contains all the details necessary to access
	a Chunk of memory in NVS

 \var uint8_t MemChunk::num_sectors
 Size of chunk in sectors
 \var size_t MemChunk::offset
 Offset of chunk within memory
*/
typedef struct
{
    uint8_t num_sectors;         //Size of chunk in sectors
    size_t offset;               //Offset of chunk within memory
} MemChunk;

/*!\typedef MemElement
	Memory Element object. Contains all the details necessary to access
	an element stored within a given chunk

 \var size_t MemElement::offset
 Offset of element within chunk (19 bits)
 \var size_t MemElement::size
 Size of element in bytes
*/
typedef struct
{
    size_t offset;               //Offset of element within chunk (19 bits)
    size_t size;                 //Size of data
} MemElement;

/*!\typedef MemObject
	Memory object. Contains details required to access NVS memory IC.

 \var size_t MemObject::sectorSize
 Size of Sector on NVS IC
 \var size_t MemObject::totalSize
 Size of NVS IC
 \var uint32_t MemObject::chip_id
 Chip ID of NVS
 \var uint8_t MemObject::dev_id
 Device ID of NVS
 \var MemChunk MemObject::*chunks
 Pointer to Memory Map
 \var uint16_t MemObject::num_chunks
 Number of Chunks
 \var SpiObject MemObject::spi
 SPI Object to be initialized at startup
 \var SpiTransaction MemObject::trans
 Transfer settings
 \var GPIO_TypeDef MemObject::*cs_port
 Port of NVS IC CS pin
 \var uint16_t MemObject::cs_pin
 NVS IC CS pin
 \var uint8_t MemObject::buffer
 TX/RX SPI buffer
 \var uint8_t MemObject::verify_buffer
 Verification buffer
*/
typedef uint32_t SpiObject;
typedef uint32_t SpiTransaction;
typedef void* GPIO_TypeDef;
typedef void* SPI_HandleTypeDef;
typedef struct
{
    size_t sectorSize;
    size_t totalSize;

    uint32_t chip_id;
    uint8_t dev_id;

    MemChunk *chunks;
    uint16_t num_chunks;

    SpiObject spi;
    SpiTransaction trans;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;

    uint8_t buffer[MEM_BUFF_SIZE];
#ifdef MEM_VERIFY
    uint8_t verify_buffer[MEM_PP_MAXBYTES];
#endif

    //OAD HASH verification

} MemObject;

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* FUNCTIONS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*! \brief Initialize Memory Object

    \param *mem Pointer to MemObject object
    \param *handle Pointer to HAL SPI handle
    \param spi_index Index of SPI peripheral used for NVS
    \param *cs_port Port of NVS IC CS pin
    \param cs_pin NVS IC CS pin
    \param *chunks Pointer to Memory Map
    \param num_chunks Number of Chunks
    \param sectorSize Size of Sector on NVS IC
*/
bool memInit(MemObject *mem, SPI_HandleTypeDef *handle, uint8_t spi_index,
		GPIO_TypeDef *cs_port, uint16_t cs_pin,
		MemChunk *chunks, uint16_t num_chunks, size_t sectorSize, size_t totalSize);

//Chunk Level operations
/*! \brief Write new chunk details at location

    \param *mem Pointer to MemObject object
    \param *chunk Pointer to Chunk details
*/
MemOpStatus memInitChunk(MemObject *mem, MemChunk *chunk);

/*! \brief Check if chunk is already written, and details match

    \param *mem Pointer to MemObject object
    \param *chunk Pointer to Chunk details
*/
MemOpStatus memCheckChunk(MemObject *mem, MemChunk *chunk);

//Element Level operations
/*! \brief Add an element to a Chunk with specified details

    \param *mem Pointer to MemObject object
    \param *chunk Pointer to Chunk details
    \param elem_size Size of element in bytes
    \param *elem_return Pointer to MemElement object to write details of element after
    adding
    \param *elemindex_return Return element index
*/
MemOpStatus memAddElement(MemObject *mem, MemChunk *chunk, size_t elem_size,
		MemElement *elem_return, uint16_t *elemindex_return);

/*! \brief Write new data to element

    \param *mem Pointer to MemObject object
    \param *chunk Pointer to Chunk details
    \param *elem Pointer to Element details
    \param offset Offset (within element) to write new data
    \param *source Source of data in RAM
    \param write_size Size of data to write in bytes
*/
MemOpStatus memWriteToElement(MemObject *mem, MemChunk *chunk, MemElement *elem, size_t offset,
                       void *source, size_t write_size);

/*! \brief Read element details from NVS within specified Chunk

    \param *mem Pointer to MemObject object
    \param *chunk Pointer to Chunk details
    \param *elem Location to write element details
    \param element_index Index of Element to read
*/
MemOpStatus memReadElementParam(MemObject *mem, MemChunk *chunk, MemElement *elem,
                    uint16_t element_index);

/*! \brief Read new data from element

    \param *mem Pointer to MemObject object
    \param *chunk Pointer to Chunk details
    \param *elem Pointer to Element details
    \param offset Offset (within element) to write new data
    \param *dest Destination of data in RAM
    \param read_size Size of data to read in bytes
*/
MemOpStatus memReadFromElement(MemObject *mem, MemChunk *chunk, MemElement *elem, size_t offset,
                        void *dest, size_t read_size);

/*! \brief Check the Hash of an OAD image stored in specified chunk

    \param *mem Pointer to MemObject object
    \param *chunk Pointer to Chunk details
    \param *refhash Pointer to reference hash to check against

    \return uint8_t Success (0), Recorded hash incorrect (1), Calculated hash incorrect (2)
*/
uint8_t memCheckHashOAD(MemObject *mem, MemChunk *chunk, uint8_t *refhash);

/*! \brief Check if peripheral is busy

    \param *mem Pointer to MemObject
    \return True: Busy. False: Idle.
*/
bool memCheckBusy(MemObject *mem);

/*! \brief Read data from NVS to a specified location

    \param *mem Pointer to MemObject object
    \param offset Offset in NVS from which to read
    \param *dest Destination in RAM to read to
    \param size Size of data to read
    \return Pass or Fail?
*/
bool nvsReadTo(MemObject *mem, size_t offset, void *dest, size_t size);

/*! \brief Write data to NVS from specified location

    \param *mem Pointer to MemObject object
    \param offset Offset in NVS to write to
    \param *source Source in RAM
    \param size Size of data to write
    \return Pass or Fail?
*/
bool nvsWriteFrom(MemObject *mem, size_t offset, void *source, size_t size);

/*! \brief Erase data from NVS

    \param *mem Pointer to MemObject object
    \param offset Offset in NVS to start Erase operation
    \param size Size of data (in sectors) to erase
    \return Pass or Fail?
*/
bool nvsErase(MemObject *mem, size_t offset, uint16_t size);

/*! \brief Calculate the SHA256 hash of an element, starting from a given offset,
 * ending at a given length

	\param hash Pointer to hash outputs
    \param *mem Pointer to MemObject object
    \param *memchunk Pointer to MemChunk
    \param *elem Pointer to MemElement
    \param elemoffs Offset within element
    \param len Length of data to hash out
    \return Pass or Fail?
*/
void calc_sha_256(uint8_t hash[32],
		MemObject *mem, MemChunk *memchunk, MemElement *elem, size_t elemoffs,
		size_t len);

/*! \brief Calculate the SHA256 hash of data stored on onchip FLASH, starting from a given offset,
 * ending at a given length

	\param hash Pointer to hash outputs
	\param input Pointer to input data
    \param len Length of data to hash out
    \return Pass or Fail?
*/
void calc_sha_256_flash(uint8_t hash[32],
		const void * input,
		size_t len);

/* THREADS --------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* MEM_H_ */



