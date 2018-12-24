#include "qtk_handle.h"
#include "qtk_context.h"
#include "os/qtk_rwlock.h"
#include "os/qtk_alloc.h"
#include "os/qtk_base.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define DEFAULT_SLOT_SIZE 4
#define MAX_SLOT_SIZE 0x40000000

typedef struct qtk_handle_storage qtk_handle_storage_t;
typedef struct qtk_handle_name qtk_handle_name_t;

struct qtk_handle_name {
	char *name;
	uint32_t handle;
};

struct qtk_handle_storage {
	struct qtk_rwlock lock;

	uint32_t handle_index;
	int slot_size;
	struct qtk_context **slot;

	int name_cap;
	int name_count;
	qtk_handle_name_t *name;
};

qtk_handle_storage_t *H = NULL;

uint32_t qtk_handle_register(qtk_context_t *ctx)
{
	qtk_handle_storage_t *s = H;
    assert(H != NULL);

	qtk_rwlock_wlock(&s->lock);

	for (;;) {
		int i;
		for (i=0; i<s->slot_size; i++) {
			uint32_t handle = i + s->handle_index;
			int hash = handle & (s->slot_size-1);
			if (s->slot[hash] == NULL) {
				s->slot[hash] = ctx;
				s->handle_index = handle + 1;

				qtk_rwlock_wunlock(&s->lock);

				return handle;
			}
		}

        int new_slot_size = 2 * s->slot_size * sizeof(qtk_context_t *);
        qtk_context_t **new_slot = qtk_malloc(new_slot_size);
		memset(new_slot, 0, new_slot_size);
		for (i=0; i<s->slot_size; i++) {
            int hash = CTX_GET_HANDLE(s->slot[i]) & (2 * s->slot_size - 1);
			assert(new_slot[hash] == NULL);
			new_slot[hash] = s->slot[i];
		}

		qtk_free(s->slot);
		s->slot = new_slot;
		s->slot_size *= 2;
	}
}

int qtk_handle_retire(uint32_t handle)
{
	int ret = 0;
	qtk_handle_storage_t *s = H;

	qtk_rwlock_wlock(&s->lock);

	uint32_t hash = handle & (s->slot_size-1);
    qtk_context_t *ctx = s->slot[hash];

	if (ctx != NULL && CTX_GET_HANDLE(ctx) == handle) {
		s->slot[hash] = NULL;
		ret = 1;
		int i;
		int j=0, n=s->name_count;
		for (i=0; i<n; ++i) {
			if (s->name[i].handle == handle) {
				qtk_free(s->name[i].name);
				continue;
			} else if (i!=j) {
				s->name[j] = s->name[i];
			}
			++j;
		}
		s->name_count = j;
	} else {
		ctx = NULL;
	}

	qtk_rwlock_wunlock(&s->lock);

	if (ctx) {
		// release ctx may call qtk_handle_* , so wunlock first.
        qtk_context_release(ctx);
	}

	return ret;
}

void qtk_handle_retireall()
{
	qtk_handle_storage_t *s = H;
	for (;;) {
		int n=0;
		int i;
		for (i=0; i<s->slot_size; i++) {
			qtk_rwlock_rlock(&s->lock);
			qtk_context_t * ctx = s->slot[i];
			uint32_t handle = 0;
			if (ctx)
				handle = CTX_GET_HANDLE(ctx);
			qtk_rwlock_runlock(&s->lock);
			if (handle != 0) {
				if (qtk_handle_retire(handle)) {
					++n;
				}
			}
		}
		if (n == 0)
			return;
	}
}

qtk_context_t *qtk_handle_grab(uint32_t handle)
{
	qtk_handle_storage_t *s = H;
	qtk_context_t * result = NULL;

	qtk_rwlock_rlock(&s->lock);

	uint32_t hash = handle & (s->slot_size-1);
	qtk_context_t * ctx = s->slot[hash];
	if (ctx && CTX_GET_HANDLE(ctx) == handle) {
		result = ctx;
        qtk_context_grab(result);
	}

	qtk_rwlock_runlock(&s->lock);

	return result;
}

uint32_t qtk_handle_findname(const char * name)
{
	qtk_handle_storage_t *s = H;

	qtk_rwlock_rlock(&s->lock);

	uint32_t handle = 0;

	int begin = 0;
	int end = s->name_count - 1;
	while (begin<=end) {
		int mid = (begin+end)/2;
		qtk_handle_name_t *n = &s->name[mid];
		int c = strcmp(n->name, name);
		if (c==0) {
			handle = n->handle;
			break;
		}
		if (c<0) {
			begin = mid + 1;
		} else {
			end = mid - 1;
		}
	}

	qtk_rwlock_runlock(&s->lock);

	return handle;
}

static void _insert_name_before(qtk_handle_storage_t *s, char *name,
                                uint32_t handle, int before)
{
	if (s->name_count >= s->name_cap) {
		s->name_cap *= 2;
		assert(s->name_cap <= MAX_SLOT_SIZE);
		qtk_handle_name_t *n = qtk_malloc(s->name_cap * sizeof(qtk_handle_name_t));
		int i;
		for (i=0; i<before; i++) {
			n[i] = s->name[i];
		}
		for (i=before; i<s->name_count; i++) {
			n[i+1] = s->name[i];
		}
		qtk_free(s->name);
		s->name = n;
	} else {
		int i;
		for (i=s->name_count; i>before; i--) {
			s->name[i] = s->name[i-1];
		}
	}

	s->name[before].name = name;
	s->name[before].handle = handle;
	s->name_count ++;
}

static const char * _insert_name(struct qtk_handle_storage *s, const char * name, uint32_t handle)
{
	int begin = 0;
	int end = s->name_count - 1;
	while (begin<=end) {
		int mid = (begin+end)/2;
		qtk_handle_name_t *n = &s->name[mid];
		int c = strcmp(n->name, name);
		if (c==0) {
			return NULL;
		}
		if (c<0) {
			begin = mid + 1;
		} else {
			end = mid - 1;
		}
	}
	char * result = strdup(name);

	_insert_name_before(s, result, handle, begin);

	return result;
}

const char *qtk_handle_namehandle(uint32_t handle, const char *name)
{
	qtk_rwlock_wlock(&H->lock);

	const char * ret = _insert_name(H, name, handle);

	qtk_rwlock_wunlock(&H->lock);

	return ret;
}


void qtk_handle_init()
{
    assert(H == NULL);
	qtk_handle_storage_t *s = qtk_malloc(sizeof(*H));
	s->slot_size = DEFAULT_SLOT_SIZE;
	s->slot = qtk_malloc(s->slot_size * sizeof(struct skynet_context *));
	memset(s->slot, 0, s->slot_size * sizeof(struct skynet_context *));

	qtk_rwlock_init(&s->lock);
	// reserve 0 for system
	s->handle_index = 1;
	s->name_cap = 2;
	s->name_count = 0;
	s->name = qtk_malloc(s->name_cap * sizeof(qtk_handle_name_t));

	H = s;
}


/*static void qtk_handle_storage_clean()
{
    int i;
    for (i=0; i<H->slot_size; i++) {
        if (NULL != H->slot[i]) { qtk_context_delete(H->slot[i]); }
    }
    for (i=0; i<H->name_count; i++) {
        if (H->name[i].name != NULL) { qtk_free(H->name[i].name); }
    }
}*/


void qtk_handle_clean()
{
    if (H) {
        qtk_free(H->name);
        qtk_free(H->slot);
        qtk_free(H);
        H = NULL;
    }
}
