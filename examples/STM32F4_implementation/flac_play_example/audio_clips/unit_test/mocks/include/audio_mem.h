/*! \file "audio_mem.h"
   \brief Audio_mem header file

 * AUDIO_MEM serves to provide the following:
 *  1) A read only interface from NVS Memory to CODEC/DSP
 *  2) A read only interface from Local RAM to CODEC/DSP
 *  3) A r/w interface between Local RAM and NVS Memory
 *
 * Data is stored in NVS in two formats:
 *  1) AudMemReg Sequence:
 *      - Header:
 *          Byte 0 (0:7) to Byte 1 (0:7): Number of AudMemReg in block
 *      - Sequence of AudMemReg: A pairing of memory offset and data (both 32bit)
 *  2) AudMemBlock: A block of memory data preceded by offset and data length (all 32bit)
 *
 * Data stored on Local RAM take two forms:
 * 1) AudioOutputConfig: Configurations for output channels
 * 2) AudioInputConfig: Configurations for input channels
 *
 * Data formats are described below.
*/

#ifndef AUDIO_AUDIO_MEM_H_
#define AUDIO_AUDIO_MEM_H_

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

#include "audio_types.h"
#include "mem_peri.h"

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* MEMORY MAP INDICES --------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*!\var uint8_t AUD_MEMCHUNK_CFG_IND
  Index of Audio Configuration Chunk on memory map
*/
static const uint8_t AUD_MEMCHUNK_CFG_IND = 0;

/*!\var uint8_t AUD_MEMCHUNK_ADJ_IND
 Index of Audio Adjustment Chunk on memory map
*/
static const uint8_t AUD_MEMCHUNK_ADJ_IND = 1;

/*!\var uint8_t AUD_MEMCHUNK_DSPCFG_IND
 Index of Audio DSP Configuration Chunk on memory map
*/
static const uint8_t AUD_MEMCHUNK_DSPCFG_IND = 2;

/*!\var uint8_t AUD_MEMCHUNK_DSPPARAM_IND
 Index of Audio DSP Parameters Chunk on memory map
*/
static const uint8_t AUD_MEMCHUNK_DSPPARAM_IND = 3;

/*!\var uint8_t AUD_MEMCHUNK_DSPPROG_IND
 Index of Audio DSP Programs Chunk on memory map
*/
static const uint8_t AUD_MEMCHUNK_DSPPROG_IND = 4;

/*!\var uint8_t AUD_MEMCHUNK_CLIP_IND
 Index of Audio Clip Chunk on memory map
*/
static const uint8_t AUD_MEMCHUNK_CLIP_IND = 7;

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* NVS DATA TYPES ------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*!\var uint8_t AUDMEMREG_HEADER_SZ
 Size of AudMemReg Header in bytes
*/
static const uint8_t AUDMEMREG_HEADER_SZ = 2;

/*!\typedef AudMemReg
 *
 \var uint32_t AudMemReg::offset
 Offset of destination
 \var uint32_t AudMemReg::data
 Data at destination
*/
typedef struct
{
    uint32_t offset;
    uint32_t data;
} AudMemReg;

/*!\typedef AudMemBlock
 *
 \var uint32_t AudMemBlock::offset
 Offset of destination within CODEC/DSP
 \var uint32_t AudMemBlock::length
 Amount of data in bytes
*/
typedef struct
{
    uint32_t offset;
    uint32_t length;
} AudMemBlock;

/*!\enum AudMemStatus
 * List of possible statuses functions will return
 */
typedef enum
{
    AUDMEM_SUCCESS = 0,
    AUDMEM_SPIW_FAIL,
    AUDMEM_SPIR_FAIL,
    AUDMEM_NOELEM,
    AUDMEM_MEMFAIL,
    AUDMEM_BADDATA,
    AUDMEM_READBACKFAIL
} AudMemStatus;

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* CODEC CONTROL  ------------------------------------------------------*/
/*----------------------------------------------------------------------*/

static const uint32_t AUD_IRQ1_STAT1 = 0x1880;
static const uint32_t AUD_IRQ2_STAT1 = 0x1980;
static const uint8_t AUD_BOOT_INPROGRESS = 0;
static const uint8_t AUD_BOOT_DONE = 1;

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* DSP CONTROL CONSTANTS  ----------------------------------------------*/
/*----------------------------------------------------------------------*/

/*!\var uint16_t AUD_DSPCFG_MEMELEM_INDEX
 Element index of the AudioDSPConfig object in DSPCFG chunk
*/
static const uint16_t AUD_DSPCFG_MEMELEM_INDEX = 0;

/*!\def AUDIO_DSP_CORES_MAX
 Number of DSP Cores
*/
#define AUDIO_DSP_CORES_MAX        7

/*!\var uint32_t AUD_DSP_ADDR_JUMP
 Number of registers to jump between DSPs
*/
static const uint32_t AUD_DSP_ADDR_JUMP           = 0x80000;

static const uint32_t AUD_DSP_CFG1_BASE           = 0xFFE00;
static const uint32_t AUD_DSP_CFG2_BASE           = 0xFFE02;
static const uint32_t AUD_DSP_STAT2_BASE          = 0xFFE06;
static const uint32_t AUD_DSP_STAT3_BASE          = 0xFFE08;
static const uint32_t AUD_DSP_DMACFG1_BASE        = 0xFFE30;
static const uint32_t AUD_DSP_WDMACFG2_BASE       = 0xFFE31;
static const uint32_t AUD_DSP_RDMACFG1_BASE       = 0xFFE34;

static const uint32_t AUD_DSP_CFG1_MEMCLEAR       = 0x00000000;
static const uint32_t AUD_DSP_CFG1_MEMEN          = 0x00000010;
static const uint32_t AUD_DSP_CFG1_COREEN         = 0x00000012;
static const uint32_t AUD_DSP_CFG1_START          = 0x00000013;

/*!\def AUD_DSP_CFG2_CLK
 Default setting for DSP clocks (147MHz)
*/
#define               AUD_DSP_CFG2_CLK              0x000024DD

/*!\var AudioDSPConfig audio_dsps
 Default DSP configuration settings. Updated on audio subsystem initialization
*/
static AudioDSPConfig audio_dsps[AUDIO_DSP_CORES_MAX]= {
     {true, AUD_DSP_CFG2_CLK},     {false, AUD_DSP_CFG2_CLK},     {false, AUD_DSP_CFG2_CLK},
     {false, AUD_DSP_CFG2_CLK},     {false, AUD_DSP_CFG2_CLK},     {false, AUD_DSP_CFG2_CLK},
     {false, AUD_DSP_CFG2_CLK},
};

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* INPUT CONFIGURATIONS ------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*!\var uint16_t AUD_INPUTVOL_MEMELEM_INDEX
 Element index of the Input Volume object in AUD_ADJ chunk
*/
static const uint16_t AUD_INPUTVOL_MEMELEM_INDEX = 0;

/*!\var uint16_t AUD_INPUTADJ_MEMELEM_INDEX
 Element index of the Input Adjustment object in AUD_ADJ chunk
*/
static const uint16_t AUD_INPUTADJ_MEMELEM_INDEX = 2;

/*!\def AUD_INPUT_NUM_CHANNELS
 Number of input channels
*/
#define AUD_INPUT_NUM_CHANNELS      2

/*!\var uint32_t AUD_INPUT_VOL_REGBASE
 First input volume register
*/
static const uint32_t AUD_INPUT_VOL_REGBASE  = 0x321;

/*!\var uint32_t AUD_INPUT_VOL_REGHOP
 Number of registers to next
*/
static const uint16_t AUD_INPUT_VOL_REGHOP   = 4;

/*!\var uint32_t AUD_INPUT_LHPF_REGBASE
 First LHPF config register
*/
static const uint16_t AUD_INPUT_LHPF_REGBASE = 0xEC0;

/*!\var uint32_t AUD_INPUT_LHPF_REGHOP
 Number of registers to next
*/
static const uint16_t AUD_INPUT_LHPF_REGHOP  = 4;

//Input Volume control (10x dB)
/*!\var uint32_t AUD_INVOL_DBMUL
 How many dB(x10) does each integer in volume register represent
*/
static const uint16_t AUD_INVOL_DBMUL        = 5;

/*!\var uint32_t AUD_INVOL_MINDB
 Lowest dB(x10) value represented by register
*/
static const int16_t AUD_INVOL_MINDB        = -640;

/*!\var uint32_t AUD_INVOL_MAXDB
 Highest dB(x10) value represented by register
*/
static const int16_t AUD_INVOL_MAXDB        = 315;

/*!\var AudioInputConfig audio_inputs
 Default input configurations. Updated on audio subsystem initialization
*/
static AudioInputConfig audio_inputs[AUD_INPUT_NUM_CHANNELS] = {
    {0, false},    {0, false}
};

/*!\var int16_t audio_inadj
 Default input adjustments. Updated on audio subsystem initialization
*/
static int16_t audio_inadj[AUD_INPUT_NUM_CHANNELS] = { 0, 0};

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* OUTPUT CONFIGURATIONS -----------------------------------------------*/
/*----------------------------------------------------------------------*/

/*!\var uint16_t AUD_OUTPUTVOL_MEMELEM_INDEX
 Element index of the Output Volume object in AUD_ADJ chunk
*/
static const uint16_t AUD_OUTPUTVOL_MEMELEM_INDEX = 1;

/*!\var uint16_t AUD_OUTPUTVOL_MEMELEM_INDEX
 Element index of the Output Volume object in AUD_ADJ chunk
*/
static const uint16_t AUD_OUTPUTADJ_MEMELEM_INDEX = 3;

/*!\def AUD_OUTPUT_NUM_CHANNELS
 Number of output channels
*/
#define AUD_OUTPUT_NUM_CHANNELS     2

/*!\var uint32_t AUD_OUTPUT_VOL_REGBASE
 First input volume register
*/
static const uint32_t AUD_OUTPUT_VOL_REGBASE = 0x431;

/*!\var uint32_t AUD_OUTPUT_VOL_REGHOP
 Number of registers to next
*/
static const uint16_t AUD_OUTPUT_VOL_REGHOP  = 4;

/*!\var uint32_t AUD_OUTVOL_DBMUL
 How many dB(x10) does each integer in volume register represent
*/
static const uint16_t AUD_OUTVOL_DBMUL       = 5;

/*!\var uint32_t AUD_OUTVOL_MINDB
 Lowest dB(x10) value represented by register
*/
static const int16_t AUD_OUTVOL_MINDB       = -640;

/*!\var uint32_t AUD_OUTVOL_MAXDB
 Highest dB(x10) value represented by register
*/
static const int16_t AUD_OUTVOL_MAXDB       = 315;

/*!\var AudioOutputConfig audio_outputs
 Default output configurations. Updated on audio subsystem initialization
*/
static AudioOutputConfig audio_outputs[AUD_OUTPUT_NUM_CHANNELS]= {
   {100, false},   {100, false},
};

/*!\var int16_t audio_outadj
 Default output adjustments. Updated on audio subsystem initialization
*/
static int16_t audio_outadj[AUD_OUTPUT_NUM_CHANNELS]= { 0, 0};

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* SPI -----------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#define CS_SPI_PACKETSIZE   sizeof(uint16_t)
#define CS_SPI_HEADERSZ     3
#define CS_SPI_RSIZE        7
#define CS_SPI_WSIZE        (MEM_BUFF_SIZE >> 1) - 7
#define CS_SPI_WDATAMAX     (CS_SPI_WSIZE - CS_SPI_HEADERSZ)
#define CS_SPI_WBYTEMAX     CS_SPI_WDATAMAX * 2

/*!\var uint32_t AUDIO_SPIBUFF_BYTESZ
 Number of bytes in SPI Buffer
*/
static const uint32_t AUDIO_SPIBUFF_BYTESZ = CS_SPI_WBYTEMAX - (CS_SPI_WBYTEMAX % sizeof(uint32_t));

/*!\var uint32_t AUDIO_SPI_BUFF_WORDSZ
 Number of words in SPI Buffer
*/
static const uint32_t AUDIO_SPI_BUFF_WORDSZ = (CS_SPI_WBYTEMAX - (CS_SPI_WBYTEMAX % sizeof(uint32_t))) >> 1;

/*!\var uint32_t AUDIO_SPIBUFF_HEADERBYTESZ
 Number of bytes in SPI Buffer Header
*/
static const uint32_t AUDIO_SPIBUFF_HEADERBYTESZ = CS_SPI_HEADERSZ * sizeof(uint16_t);

/*!\var uint32_t AUDIO_SPIBUFF_HEADERWORDSZ
 Number of word in SPI Buffer Header
*/
static const uint32_t AUDIO_SPIBUFF_HEADERWORDSZ = (CS_SPI_HEADERSZ * sizeof(uint16_t)) >> 1;

/*!\var uint32_t AUDIO_SPI_RETRIES
 Number of times to retry a failed SPI transaction before reporting a failure
*/
static const uint32_t AUDIO_SPI_RETRIES     = 3;

/*!\var uint32_t AUDIO_DSP_32BIT_START
 CS47L90 address where 32bit addressing begins
*/
static const uint32_t AUDIO_DSP_32BIT_START  = 0x3000;

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* GLOBAL FUNCTIONS ----------------------------------------------------*/
/*----------------------------------------------------------------------*/

/* 1) NVS MEMORY to CODEC/DSP -------------------------------------------*/

/*! \brief Load all the AudMemRegs stored in a specified chunk->element

    \param *state Pointer to Audio object
    \param *chunk Pointer to Memory chunk object
    \param elem_index
    \return Pass or fail
*/
bool audioNVSToDSPLoadAudMemReg(AudioState *state, MemChunk *chunk, uint16_t elem_index);

/*! \brief Load contents of AudMemBlock stored in a specified chunk->element

    \param *state Pointer to Audio object
    \param *chunk Pointer to Memory chunk object
    \param elem_index
    \return AudMemStatus
*/
AudMemStatus audioNVSToDSPLoadAudMemBlock(AudioState *state, MemChunk *chunk, uint16_t elem_index);

/* 2) RAM to CODEC/DSP --------------------------------------------------*/

/*! \brief Update Input configurations for a specified channel

    \param *state Pointer to Audio object
    \param channel Which channel to update
    \param update Should we update volume for all channels?
    \return Pass or fail
*/
bool audioRAMToDSPLoadInputConfig(AudioState *state, uint8_t channel, bool update);

/*! \brief Update Output configurations for a specified channel

    \param *state Pointer to Audio object
    \param channel Which channel to update
    \param update Should we update volume for all channels?
    \return Pass or fail
*/
bool audioRAMToDSPLoadOutputConfig(AudioState *state, uint8_t channel, bool update);

/* 3) RAM to/from NVS MEMORY --------------------------------------------*/

/*! \brief Load Adjustment configurations stored in RAM to NVS

    \param *state Pointer to Audio object
    \return Pass or fail
*/
bool audioRAMToNVSLoadAdjConfig(AudioState *state);

/*! \brief Load DSP configurations stored in RAM to NVS

    \param *state Pointer to Audio object
    \return Pass or fail
*/
bool audioRAMToNVSLoadDSPConfig(AudioState *state);

/*! \brief Load DSP configurations stored in NVS to RAM

    \param *state Pointer to Audio object
    \return Pass or fail
*/
bool audioNVSToRAMLoadDSPConfig(AudioState *state);

/*! \brief Load Adjustment configurations stored in NVS to RAM

    \param *state Pointer to Audio object
    \return Pass or fail
*/
bool audioNVSToRAMLoadOutputConfig(AudioState *state);

/*! \brief Load Input configurations stored in NVS to RAM

    \param *state Pointer to Audio object
    \return Pass or fail
*/
bool audioNVSToRAMLoadInputConfig(AudioState *state);

/*! \brief Load AudMemReg(s) stored in RAM to NVS

    \param *state Pointer to Audio object
    \param *chunk Pointer to Memory chunk object
    \param *reg Pointer to AudMemReg(s) to load
    \param num_regs Number of AudMemRegs at *reg to load
    \return Pass or fail
*/
bool audioRAMToNVSLoadAudMemReg(AudioState *state, MemChunk *chunk, AudMemReg *reg, uint16_t num_regs);

/* 4) Single Register Reads/Writes ---------------------------------------*/

/*! \brief Write data to a single register address

    \param *state Pointer to Audio object
    \param addr Address on CS47L90 to write to
    \param data Data to write
    \return Pass or fail
*/
bool audio_write_address(AudioState *state, uint32_t addr, uint32_t data);

/*! \brief Read data from a single register address

    \param *state Pointer to Audio object
    \param addr Address on CS47L90 to read from
    \param *data Location to transfer read data to
    \return Pass or fail
*/
bool audio_read_address(AudioState *state, uint32_t addr, uint32_t *data);

#endif /* AUDIO_AUDIO_MEM_H_ */
