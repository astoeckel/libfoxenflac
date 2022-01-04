#ifndef __WRAPPER_MEMRD_H__
#define __WRAPPER_MEMRD_H__

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* INCLUDES ------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#include "audio_clip_defs.h"
#include "i2s/i2s_peri.h"

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* VARIABLES -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*!\typedef MemElement
	Memory Element object. Contains all the details necessary to access
	data stored in memory
 \var size_t MemElement::offset
 Offset of data element
 \var size_t MemElement::size
 Size of data element in bytes
*/
typedef struct
{
    size_t offset;               //Offset of element
    size_t size;                 //Size of data
} MemElement;

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* FUNCTIONS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*! \brief Read from Memory
    \param *buf Destination of data
    \param offset Offset within memory to read
    \param size Size to read
    \return 0 if SUCCESS, -1 if FAIL
*/
int32_t wrp_mem_read(uint8_t *buf, uint32_t offset, uint32_t size);

/*! \brief Retrieve clip size and offset in memory.
    \param idx Index of clip
    \return 0 if SUCCESS, -1 if FAIL
*/
int32_t wrp_mem_set_element(uint8_t idx);

/*! \brief Get the size of the clip in bytes
    \return Size of the clip
*/
int32_t wrp_mem_get_size(void);

/*! \brief Get address of the clip
    \return Address of the clip
*/
int32_t wrp_mem_get_address(void);


#endif // __WRAPPER_MEMRD_H__
