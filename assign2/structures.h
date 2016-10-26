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
	FQ_Node *next;
	FQ_Node *previous;
} FQ_Node;

typedef struct BM_FrameQueue {
	FQ_Node *head;
	FQ_Node *tail;
	int frameCount;
} BM_FrameQueue;

typedef struct BufferStorage {
	BM_FrameQueue *queue;
	int array[];
} BufferStorage;



