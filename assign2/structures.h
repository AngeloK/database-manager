#include <sys/time.h>
#include "buffer_mgr.h"

/*
 ****************************************************************************
 * Define required structs for buffer implementation
 * and replacement startegies.
 ****************************************************************************
 */

typedef struct BM_PageFrames {
	BM_PageHandle *pageHandle;
	int fixcount;
	bool dirty;
	time_t lastUsed;
} BM_PageFrame;

typedef struct FQ_Node FQ_Node;

struct FQ_Node {
	BM_PageFrame *pf;
	FQ_Node *next;
	FQ_Node *previous;
};

typedef struct BM_FrameQueue {
	struct FQ_Node *head;
	struct FQ_Node *tail;
	int frameCount;
} BM_FrameQueue;

typedef struct BufferStorage {
	BM_FrameQueue *queue;
	FQ_Node **array;
	int PoolCapacity;
} BufferStorage;



