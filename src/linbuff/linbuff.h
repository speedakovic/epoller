/**
 *
 * @file    linbuff.h
 * @author  speedak
 * @version 1.2
 * @brief   Simple linear buffer.
 *
 * history:
 * 1.2 - add useful macros
 * 1.1 - add extern C
 * 1.0 - first version
 *
 */

#ifndef LINBUFF_H
#define LINBUFF_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Gets pointer to first byte free to be written.
 */
#define LINBUFF_WR_PTR(lb) ((uint8_t *)(lb)->buff + (lb)->wrix)

/**
 * @brief Gets pointer to first byte available to be read.
 */
#define LINBUFF_RD_PTR(lb) ((uint8_t *)(lb)->buff + (lb)->rdix)

/**
 * @brief Gets pointer to byte at given position.
 */
#define LINBUFF_PTR_POS(lb, pos) ((uint8_t *)(lb)->buff + pos)

/**
 * @brief Gets value of byte at given position.
 */
#define LINBUFF_VAL_POS(lb, pos) (*((uint8_t *)(lb)->buff + pos))

/**
 * @brief Gets pointer to byte at given position relative to write index.
 */
#define LINBUFF_WR_PTR_POS(lb, pos) ((uint8_t *)(lb)->buff + (lb)->wrix + pos)

/**
 * @brief Gets pointer to byte at given position relative to read index.
 */
#define LINBUFF_RD_PTR_POS(lb, pos) ((uint8_t *)(lb)->buff + (lb)->rdix + pos)

/**
 * @brief Gets value of byte at given position relative to write index.
 */
#define LINBUFF_WR_VAL_POS(lb, pos) (*((uint8_t *)(lb)->buff + (lb)->wrix + pos))

/**
 * @brief Gets value of byte at given position relative to read index.
 */
#define LINBUFF_RD_VAL_POS(lb, pos) (*((uint8_t *)(lb)->buff + (lb)->rdix + pos))

/**
 * @brief Linear buffer
 */
struct linbuff
{
	void   *buff; /**< buffer                                         */
	size_t  size; /**< buffer size                                    */
	size_t  wrix; /**< index to first byte free to be written         */
	size_t  rdix; /**< index to first byte available to be read       */
	void   *user; /**< user pointer, not used by any linbuff function */
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes linear buffer object and uses given buffer as its internal one.
 */
void linbuff_wrap(struct linbuff *lb, void *buff, size_t size);

/**
 * @brief Initializes linear buffer object including allocation of internal buffer.
 * @return @c true if initialization and allocation were successful, otherwise @c false
 */
bool linbuff_alloc(struct linbuff *lb, size_t size);

/**
 * @brief Frees internal buffer.
 */
void linbuff_free(struct linbuff *lb);

/**
 * @brief Clears buffer.
 */
void linbuff_clear(struct linbuff *lb);

/**
 * @brief Compacts buffer. Bytes available to be read are moved to beginning of buffer.
 */
void linbuff_compact(struct linbuff *lb);

/**
 * @brief Gets number of bytes free to be written.
 */
size_t linbuff_towr(const struct linbuff *lb);

/**
 * @brief Gets number of bytes available to be read.
 */
size_t linbuff_tord(const struct linbuff *lb);

/**
 * @brief Writes bytes to buffer.
 * @return number of written bytes
 */
size_t linbuff_write(struct linbuff *lb, const void *buff, size_t size);

/**
 * @brief Reads bytes from buffer.
 * @return number of actually read bytes
 */
size_t linbuff_read(struct linbuff *lb, void *buff, size_t size);

/**
 * @brief Peeks bytes from buffer. Like #linbuff_read, but doesn't consume any bytes.
 * @return number of actually peeked bytes
 */
size_t linbuff_peek(struct linbuff *lb, void *buff, size_t size);

/**
 * @brief Skips bytes in buffer. Like #linbuff_read, but doesn't copy any bytes.
 * @return number of actually skipped bytes
 */
size_t linbuff_skip(struct linbuff *lb, size_t size);

/**
 * @brief Forwards buffer. Like #linbuff_write, but doesn't write any bytes.
 * @return number of bytes buffer was actually forwarded by.
 */
size_t linbuff_forward(struct linbuff *lb, size_t size);

/**
 * @brief Prints some info about given buffer.
 */
void linbuff_print(const struct linbuff *lb);

#ifdef __cplusplus
}
#endif

#endif /* LINBUFF_H */

