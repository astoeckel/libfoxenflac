/*
 *  libfoxenmen -- Utilities for heap-free memory management
 *  Copyright (C) 2018  Andreas Stöckel
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <foxen/mem.h>

/******************************************************************************
 * PRIVATE IMPLEMENTATION DETAILS                                             *
 ******************************************************************************/

/* http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn */

static const int multiply_de_bruijn_tbl[32U] = {
    0,  1,  28, 2,  29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4,  8,
    31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6,  11, 5,  10, 9};

static inline uint32_t _fx_lsb(uint32_t v) {
	return multiply_de_bruijn_tbl[(uint32_t)((v & -v) * 0x077CB531UL) >> 27U];
}

/******************************************************************************
 * PUBLIC C API                                                               *
 ******************************************************************************/

uint32_t fx_mem_pool_alloc(uint32_t allocated_ptr[], uint32_t *free_idx_ptr,
                           uint32_t *n_allocated_ptr, uint32_t n_available) {
	const uint32_t free_idx = __atomic_load_n(free_idx_ptr, __ATOMIC_SEQ_CST);
	uint32_t idx = free_idx & (~(32U - 1U));
	while (true) {
		/* Load the current bitmap entry and search the first free zero-bit. If
		   all bits are set, go to the next bitmap entry. */
		uint32_t *allocated_slot_ptr = allocated_ptr + idx / 32U;
		uint32_t allocated =
		    __atomic_load_n(allocated_slot_ptr, __ATOMIC_SEQ_CST);
		uint32_t offs = 32U; /* Assume there is no free bit */
		if (~allocated) {
			offs = _fx_lsb(~allocated);
		}

		/* Update the result index, start from the beginning of the list if we
		   passed the end */
		idx += offs;
		if (idx >= n_available) {
			/* We wrapped around. Abort if there is no more space and all slots
			 * have been allocated. Note that we may wrongly abort here if a
			 * slot is just in the progress of being freed, but this is okay. */
			uint32_t n_allocated =
				__atomic_load_n(n_allocated_ptr, __ATOMIC_SEQ_CST);
			if (n_allocated >= n_available) {
				return n_available;
			}
			idx = 0U;
			continue;
		}
		if (offs >= 32U) {
			continue; /* No free bit, continue */
		}

		/* Check whether the entry is in use. If yes, continue with the next
		 * iteration, then try to write the new bitmap entry. If no, try to
		 * write the updated value to the bitmap. If this is successful, update
		 * the number of allocated elements, otherwise continue with the outer
		 * loop and search for a free bitmap entry. */
		const uint32_t mask = (1U << offs);
		if (__atomic_compare_exchange_n(allocated_slot_ptr, &allocated,
		                                allocated | mask, false,
		                                __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
			/* Writing the new bitmap entry was successful, we need to update
			 * n_allocated by incrementing it by one. There may be a period
			 * where the slot is allocated by setting the corresponding bit in
			 * the bitmap, but the n_allocated counter has not been incremented,
			 * but as explained above, this is fine. */
				uint32_t n_allocated =
					__atomic_load_n(n_allocated_ptr, __ATOMIC_SEQ_CST);
			while (!__atomic_compare_exchange_n(
			    n_allocated_ptr, &n_allocated, n_allocated + 1U, true,
			    __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
				;

			/* We found a free slot, so update the free_idx to point at the
			   index after the free_idx. */
			const uint32_t idx_next = (idx + 1U) % n_available;
			__atomic_store_n(free_idx_ptr, idx_next, __ATOMIC_SEQ_CST);
			return idx;
		}

		/* Make sure the index is rounded appropriately, then try again */
		idx = idx & (~(32U - 1U));
	}
}

void fx_mem_pool_free(uint32_t idx, uint32_t allocated_ptr[],
                      uint32_t *free_idx_ptr, uint32_t *n_allocated_ptr) {
	/* Load all state variables */
	uint32_t *allocated_slot_ptr = allocated_ptr + idx / 32U;
	uint32_t allocated = __atomic_load_n(allocated_ptr, __ATOMIC_SEQ_CST);
	uint32_t n_allocated = __atomic_load_n(n_allocated_ptr, __ATOMIC_SEQ_CST);
	uint32_t free_idx = __atomic_load_n(free_idx_ptr, __ATOMIC_SEQ_CST);

	/* Reset the corresponding bit in the bitmap. */
	uint32_t mask = ~(1U << (idx % 32U));
	while (!__atomic_compare_exchange_n(allocated_slot_ptr, &allocated,
	                                    allocated & mask, true,
	                                    __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
		;

	/* Decrement the n_allocated counter. Note that the value stored in
	 * n_allocated is temporarily too large (since the slot was already freed),
	 * but this is fine, and at most may temporarily cause fx_mem_alloc() to
	 * fail. This is correct, since fx_mem_free() has not yet finished. */
	while (!__atomic_compare_exchange_n(n_allocated_ptr, &n_allocated,
	                                    n_allocated - 1U, true,
	                                    __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
		;

	/* Try to update the free_idx, if the given index is smaller than the old
	 * free_idx. This way the allocation will be forced to reuse slots with
	 * small indices and slots at higher addresses have a lower probability of
	 * being used. In situations where this allocator code is used to manage
	 * pages of memory backed by mmap() and "freed" with madvise(), this can
	 * prevent both memory fragmentation and reduce the amount of residual
	 * memory by attempting to keep countiguous memory areas with high addresses
	 * free. */
	while (idx <= free_idx &&
	       !__atomic_compare_exchange_n(free_idx_ptr, &free_idx, idx, true,
	                                    __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
		;
}

