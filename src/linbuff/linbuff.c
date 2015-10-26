#include <linbuff/linbuff.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

void linbuff_wrap(struct linbuff *lb, void *buff, size_t size)
{
	lb->buff = buff;
	lb->size = size;
	lb->wrix = 0;
	lb->rdix = 0;
}

bool linbuff_alloc(struct linbuff *lb, size_t size)
{
	if (size > 0 && (lb->buff = malloc(size)) != NULL) {
		lb->size = size;
		lb->wrix = 0;
		lb->rdix = 0;
		return true;
	} else
		return false;
}

void linbuff_free(struct linbuff *lb)
{
	free(lb->buff);
	lb->buff = NULL;
}

void linbuff_clear(struct linbuff *lb)
{
	lb->wrix = lb->rdix = 0;
}

void linbuff_compact(struct linbuff *lb)
{
	size_t mv;

	if (lb->rdix == 0)
		return;

	if ((mv = lb->wrix - lb->rdix) == 0) {
		lb->wrix = 0;
		lb->rdix = 0;
		return;
	}

	memmove(lb->buff, (uint8_t *)lb->buff + lb->rdix, mv);
	lb->rdix = 0;
	lb->wrix = mv;
}

size_t linbuff_towr(const struct linbuff *lb)
{
	return lb->size - lb->wrix;
}

size_t linbuff_tord(const struct linbuff *lb)
{
	return lb->wrix - lb->rdix;
}

size_t linbuff_write(struct linbuff *lb, const void *buff, size_t size)
{
	size_t towr = lb->size - lb->wrix;
	size_t wr   = towr < size ? towr : size;

	if (wr > 0) {
		memcpy((uint8_t *)lb->buff + lb->wrix, buff, wr);
		lb->wrix += wr;
	}

	return wr;
}

size_t linbuff_read(struct linbuff *lb, void *buff, size_t size)
{
	size_t tord = lb->wrix - lb->rdix;
	size_t rd   = tord < size ? tord : size;

	if (rd > 0) {
		memcpy(buff, (uint8_t *)lb->buff + lb->rdix, rd);
		lb->rdix += rd;
	}

	return rd;
}

size_t linbuff_peek(struct linbuff *lb, void *buff, size_t size)
{
	size_t tord = lb->wrix - lb->rdix;
	size_t rd   = tord < size ? tord : size;

	if (rd > 0)
		memcpy(buff, (uint8_t *)lb->buff + lb->rdix, rd);

	return rd;
}

size_t linbuff_skip(struct linbuff *lb, size_t size)
{
	size_t tord = lb->wrix - lb->rdix;
	size_t rd   = tord < size ? tord : size;

	if (rd > 0)
		lb->rdix += rd;

	return rd;
}

size_t linbuff_forward(struct linbuff *lb, size_t size)
{
	size_t towr = lb->size - lb->wrix;
	size_t wr   = towr < size ? towr : size;

	if (wr > 0)
		lb->wrix += wr;

	return wr;
}

void linbuff_print(const struct linbuff *lb)
{
	size_t i;

	printf("size: %u\n", (unsigned int)lb->size);
	printf("wrix: %u\n", (unsigned int)lb->wrix);
	printf("rdix: %u\n", (unsigned int)lb->rdix);
	printf("towr: %u\n", (unsigned int)linbuff_towr(lb));
	printf("tord: %u\n", (unsigned int)linbuff_tord(lb));

	printf("buff: ");
	for (i = 0; i < lb->size; ++i)
		printf("0x%02X,", *((uint8_t *)lb->buff + i));
	printf("\n");

	printf("data: ");
	for (i = lb->rdix; i < lb->wrix; ++i)
		printf("0x%02X,", *((uint8_t *)lb->buff + i));
	printf("\n");
}

