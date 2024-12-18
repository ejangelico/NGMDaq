/*
 * Copyright (C) 2009-2012 Chris McClelland
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "liberror.h"
#include "libbuffer.h"

// Initialise the promRecords structure.
// Returns BUF_SUCCESS or BUF_NO_MEM.
//
DLLEXPORT(BufferStatus) bufInitialise(
	struct Buffer *self, uint32 initialSize, uint8 fill, const char **error)
{
	uint8 *ptr;
	const uint8 *endPtr;
	self->fill = fill;
	self->data = (uint8 *)malloc(initialSize);
	if ( !self->data ) {
		errRender(error, "Cannot allocate memory for buffer");
		return BUF_NO_MEM;
	}
	ptr = self->data;
	endPtr = ptr + initialSize;
	while ( ptr < endPtr ) {
		*ptr++ = self->fill;
	}
	self->capacity = initialSize;
	self->length = 0;
	return BUF_SUCCESS;
}

// Free up any memory associated with the buffer structure.
//
DLLEXPORT(void) bufDestroy(struct Buffer *self) {
	free(self->data);
	self->data = NULL;
	self->capacity = 0;
	self->length = 0;
	self->fill = 0;
}

// Either deep copy into an already-constructed buffer, or copy-construct into an uninitialised
// buffer.
//
DLLEXPORT(BufferStatus) bufDeepCopy(
	struct Buffer *dst, const struct Buffer *src, const char **error)
{
	uint8 *ptr;
	const uint8 *endPtr;
	if ( dst->data && dst->capacity < src->capacity ) {
		// The dst has been initialised, but there is not enough room for the copy.
		bufDestroy(dst);
	}
	if ( !dst->data ) {
		// The dst needs to be allocated.
		dst->capacity = src->capacity;
		dst->data = (uint8 *)malloc(dst->capacity);
		if ( !dst->data ) {
			errRender(error, "Cannot allocate memory for buffer");
			return BUF_NO_MEM;
		}
	}
	dst->length = src->length;
	dst->fill = src->fill;
	memcpy(dst->data, src->data, dst->length);
	ptr = dst->data + dst->length;
	endPtr = dst->data + dst->capacity;
	while ( ptr < endPtr ) {
		*ptr++ = dst->fill;
	}
	return BUF_SUCCESS;
}

DLLEXPORT(void) bufSwap(
	struct Buffer *x, struct Buffer *y)
{
	uint8 *const tmpData = x->data;
	const uint32 tmpLength = x->length;
	const uint32 tmpCapacity = x->capacity;
	const uint8 tmpFill = x->fill;

	x->data = y->data;
	x->length = y->length;
	x->capacity = y->capacity;
	x->fill = y->fill;

	y->data = tmpData;
	y->length = tmpLength;
	y->capacity = tmpCapacity;
	y->fill = tmpFill;
}

// Clean the buffer structure so it can be reused.
//
DLLEXPORT(void) bufZeroLength(struct Buffer *self) {
	uint32 i;
	self->length = 0;
	for ( i = 0; i < self->capacity; i++ ) {
		self->data[i] = self->fill;
	}
}

// Reallocate the memory for the buffer by doubling the capacity and zeroing the extra storage.
//
static BufferStatus reallocate(
	struct Buffer *self, uint32 newCapacity, uint32 blockEnd, const char **error)
{
	// The data will not fit in the buffer - we need to make the buffer bigger
	//
	uint8 *ptr;
	const uint8 *endPtr;
	do {
		newCapacity *= 2;
	} while ( blockEnd > newCapacity );
	self->data = (uint8 *)realloc(self->data, newCapacity);
	if ( !self->data ) {
		errRender(error, "Cannot reallocate memory for buffer");
		return BUF_NO_MEM;
	}
	self->capacity = newCapacity;
	
	// Now zero from the end of the block to the end of the new capacity
	//
	ptr = self->data + blockEnd;
	endPtr = self->data + newCapacity;
	while ( ptr < endPtr ) {
		*ptr++ = self->fill;
	}
	return BUF_SUCCESS;
}

// If the data will not fit in the buffer, make the buffer bigger
//
#define ENSURE_CAPACITY() \
	if ( blockEnd > self->capacity ) { \
		BufferStatus status = reallocate(self, self->capacity, blockEnd, error); \
		if ( status != BUF_SUCCESS ) { \
			return status; \
		} \
	}

DLLEXPORT(BufferStatus) bufAppendByte(struct Buffer *self, uint8 byte, const char **error) {
	const uint32 blockEnd = self->length + 1;
	ENSURE_CAPACITY();
	*(self->data + self->length) = byte;
	self->length++;
	return BUF_SUCCESS;
}

DLLEXPORT(BufferStatus) bufAppendWordLE(struct Buffer *self, uint16 word, const char **error) {
	const uint32 blockEnd = self->length + 2;
	union {
		uint16 word;
		uint8 byte[2];
	} u;
	u.word = word;
	ENSURE_CAPACITY();
	#if BYTE_ORDER == 1234
		*(self->data + self->length) = u.byte[0];
		*(self->data + self->length + 1) = u.byte[1];
	#else
		*(self->data + self->length) = u.byte[1];
		*(self->data + self->length + 1) = u.byte[0];
	#endif
	self->length += 2;
	return BUF_SUCCESS;
}

DLLEXPORT(BufferStatus) bufAppendWordBE(struct Buffer *self, uint16 word, const char **error) {
	const uint32 blockEnd = self->length + 2;
	union {
		uint16 word;
		uint8 byte[2];
	} u;
	u.word = word;
	ENSURE_CAPACITY();
	#if BYTE_ORDER == 1234
		*(self->data + self->length) = u.byte[1];
		*(self->data + self->length + 1) = u.byte[0];
	#else
		*(self->data + self->length) = u.byte[0];
		*(self->data + self->length + 1) = u.byte[1];
	#endif
	self->length += 2;
	return BUF_SUCCESS;
}

DLLEXPORT(BufferStatus) bufAppendLongLE(struct Buffer *self, uint32 lword, const char **error) {
	const uint32 blockEnd = self->length + 4;
	union {
		uint32 lword;
		uint8 byte[4];
	} u;
	u.lword = lword;
	ENSURE_CAPACITY();
	#if BYTE_ORDER == 1234
		*(self->data + self->length) = u.byte[0];
		*(self->data + self->length + 1) = u.byte[1];
		*(self->data + self->length + 2) = u.byte[2];
		*(self->data + self->length + 3) = u.byte[3];
	#else
		*(self->data + self->length) = u.byte[3];
		*(self->data + self->length + 1) = u.byte[2];
		*(self->data + self->length + 2) = u.byte[1];
		*(self->data + self->length + 3) = u.byte[0];
	#endif
	self->length += 4;
	return BUF_SUCCESS;
}

DLLEXPORT(BufferStatus) bufAppendLongBE(struct Buffer *self, uint32 lword, const char **error) {
	const uint32 blockEnd = self->length + 4;
	union {
		uint32 lword;
		uint8 byte[4];
	} u;
	u.lword = lword;
	ENSURE_CAPACITY();
	#if BYTE_ORDER == 1234
		*(self->data + self->length) = u.byte[3];
		*(self->data + self->length + 1) = u.byte[2];
		*(self->data + self->length + 2) = u.byte[1];
		*(self->data + self->length + 3) = u.byte[0];
	#else
		*(self->data + self->length) = u.byte[0];
		*(self->data + self->length + 1) = u.byte[1];
		*(self->data + self->length + 2) = u.byte[2];
		*(self->data + self->length + 3) = u.byte[3];
	#endif
	self->length += 4;
	return BUF_SUCCESS;
}

// Append a block of a given constant to the end of the buffer, and return a ptr to the next free
// byte after the end.
//
DLLEXPORT(BufferStatus) bufAppendConst(
	struct Buffer *self, uint8 value, uint32 count, const char **error)
{
	const uint32 blockEnd = self->length + count;
	ENSURE_CAPACITY();
	memset(self->data + self->length, value, count);
	self->length = blockEnd;
	return BUF_SUCCESS;
}

// Write the supplied data to the buffer structure.
// Returns BUF_SUCCESS or BUF_NO_MEM.
//
DLLEXPORT(BufferStatus) bufAppendBlock(
	struct Buffer *self, const uint8 *srcPtr, uint32 count, const char **error)
{
	const uint32 blockEnd = self->length + count;
	ENSURE_CAPACITY();
	memcpy(self->data + self->length, srcPtr, count);
	self->length = blockEnd;
	return BUF_SUCCESS;
}

// Used by bufWriteXXX() to ensure sufficient capacity for the operation.
//
static BufferStatus maybeReallocate(
	struct Buffer *const self, const uint32 bufAddress, const uint32 count, const char **error)
{
	// There are three possibilities:
	//   * The block to be written starts after the end of the current buffer
	//   * The block to be written starts within the current buffer, but ends beyond it
	//   * The block to be written ends within the current buffer
	//
	const uint32 blockEnd = bufAddress + count;
	if ( bufAddress >= self->length ) {
		// Begins outside - reallocation may be necessary, zeroing definitely necessary
		//
		uint8 *ptr, *endPtr;
		ENSURE_CAPACITY();
		
		// Now fill from the end of the old length to the start of the block
		//
		ptr = self->data + self->length;
		endPtr = self->data + bufAddress;
		while ( ptr < endPtr ) {
			*ptr++ = self->fill;
		}
		
		self->length = blockEnd;
	} else if ( bufAddress < self->length && blockEnd > self->length ) {
		// Begins inside, ends outside - reallocation and zeroing may be necessary
		//
		ENSURE_CAPACITY();
		self->length = blockEnd;
	}
	return BUF_SUCCESS;
}

// Write a single byte into the target buffer. The target offset may be outside the current extent
// (or even capacity) of the target buffer.
//
DLLEXPORT(BufferStatus) bufWriteByte(
	struct Buffer *self, uint32 offset, uint8 byte, const char **error)
{
	BufferStatus status, returnCode = BUF_SUCCESS;
	status = maybeReallocate(self, offset, 1, error);
	CHECK_STATUS(status, "bufWriteByte()", status);
	self->data[offset] = byte;
cleanup:
	return returnCode;
}

// Write a uint16 into the target buffer in little-endian format. The target offset may be outside
// the current extent (or even capacity) of the target buffer.
//
DLLEXPORT(BufferStatus) bufWriteWordLE(
	struct Buffer *self, uint32 offset, uint16 word, const char **error)
{
	BufferStatus status, returnCode = BUF_SUCCESS;
	union {
		uint16 word;
		uint8 byte[2];
	} u;
	u.word = word;
	status = maybeReallocate(self, offset, 2, error);
	CHECK_STATUS(status, "bufWriteWordLE()", status);
	#if BYTE_ORDER == 1234
		*(self->data + offset) = u.byte[0];
		*(self->data + offset + 1) = u.byte[1];
	#else
		*(self->data + offset) = u.byte[1];
		*(self->data + offset + 1) = u.byte[0];
	#endif
cleanup:
	return returnCode;
}

// Write a uint16 into the target buffer in big-endian format. The target offset may be outside
// the current extent (or even capacity) of the target buffer.
//
DLLEXPORT(BufferStatus) bufWriteWordBE(
	struct Buffer *self, uint32 offset, uint16 word, const char **error)
{
	BufferStatus status, returnCode = BUF_SUCCESS;
	union {
		uint16 word;
		uint8 byte[2];
	} u;
	u.word = word;
	status = maybeReallocate(self, offset, 2, error);
	CHECK_STATUS(status, "bufWriteWordBE()", status);
	#if BYTE_ORDER == 1234
		*(self->data + offset) = u.byte[1];
		*(self->data + offset + 1) = u.byte[0];
	#else
		*(self->data + offset) = u.byte[0];
		*(self->data + offset + 1) = u.byte[1];
	#endif
cleanup:
	return returnCode;
}

// Write a uint16 into the target buffer in little-endian format. The target offset may be outside
// the current extent (or even capacity) of the target buffer.
//
DLLEXPORT(BufferStatus) bufWriteLongLE(
	struct Buffer *self, uint32 offset, uint32 lword, const char **error)
{
	BufferStatus status, returnCode = BUF_SUCCESS;
	union {
		uint32 lword;
		uint8 byte[4];
	} u;
	u.lword = lword;
	status = maybeReallocate(self, offset, 4, error);
	CHECK_STATUS(status, "bufWriteLongLE()", status);
	#if BYTE_ORDER == 1234
		*(self->data + offset) = u.byte[0];
		*(self->data + offset + 1) = u.byte[1];
		*(self->data + offset + 2) = u.byte[2];
		*(self->data + offset + 3) = u.byte[3];
	#else
		*(self->data + offset) = u.byte[3];
		*(self->data + offset + 1) = u.byte[2];
		*(self->data + offset + 2) = u.byte[1];
		*(self->data + offset + 3) = u.byte[0];
	#endif
cleanup:
	return returnCode;
}

// Write a uint16 into the target buffer in little-endian format. The target offset may be outside
// the current extent (or even capacity) of the target buffer.
//
DLLEXPORT(BufferStatus) bufWriteLongBE(
	struct Buffer *self, uint32 offset, uint32 lword, const char **error)
{
	BufferStatus status, returnCode = BUF_SUCCESS;
	union {
		uint32 lword;
		uint8 byte[4];
	} u;
	u.lword = lword;
	status = maybeReallocate(self, offset, 4, error);
	CHECK_STATUS(status, "bufWriteLongBE()", status);
	#if BYTE_ORDER == 1234
		*(self->data + offset) = u.byte[3];
		*(self->data + offset + 1) = u.byte[2];
		*(self->data + offset + 2) = u.byte[1];
		*(self->data + offset + 3) = u.byte[0];
	#else
		*(self->data + offset) = u.byte[0];
		*(self->data + offset + 1) = u.byte[1];
		*(self->data + offset + 2) = u.byte[2];
		*(self->data + offset + 3) = u.byte[3];
	#endif
cleanup:
	return returnCode;
}

// Set a range of bytes of the target buffer to a given value. The target offset may be outside the
// current extent (or even capacity) of the target buffer.
//
DLLEXPORT(BufferStatus) bufWriteConst(
	struct Buffer *self, uint32 offset, uint8 value, uint32 count, const char **error)
{
	BufferStatus status, returnCode = BUF_SUCCESS;
	status = maybeReallocate(self, offset, count, error);
	CHECK_STATUS(status, "bufWriteConst()", status);
	memset(self->data + offset, value, count);
cleanup:
	return returnCode;
}

// Copy a bunch of bytes from a source pointer into the buffer. The target address may be outside
// the current extent (or even capacity) of the target buffer.
//
DLLEXPORT(BufferStatus) bufWriteBlock(
	struct Buffer *self, uint32 offset, const uint8 *ptr, uint32 count, const char **error)
{
	BufferStatus status, returnCode = BUF_SUCCESS;
	status = maybeReallocate(self, offset, count, error);
	CHECK_STATUS(status, "bufWriteConst()", status);
	memcpy(self->data + offset, ptr, count);
cleanup:
	return returnCode;
}
