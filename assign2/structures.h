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
} BM_PageFrame;

typedef struct FQ_Node {
	BM_PageFrame *pf;
	struct FQ_Node *next;
	struct FQ_Node *previous;
} FQ_Node;

typedef struct BM_FrameQueue {
	struct FQ_Node *head;
	struct FQ_Node *tail;
	int frameCount;
} BM_FrameQueue;

typedef struct BufferStorage {
	BM_FrameQueue *queue;
	int array[];
	int PoolCapacity;
} BufferStorage;



