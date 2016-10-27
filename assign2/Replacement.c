# include <stdio.h>
# include <sys/time.h>
# include "buffer_mgr.h"
# include "structures.h"

/*
 ***************************************************************************
 * Replacement Strategy: FIFO
 * Parameter: 	Pointer to BufferStorage struct to access pageFrames,
 * 	      	Pointer to PageHandle of the page to be added to buffer,
 *	      	Pointer to Frame that must be removed from Buffer.
 * Returns:   	Void
 * Note:      	Writing to disk done by calling function.
 * 		RemovedFrame must be freed by the calling function.
 ***************************************************************************
 */

void ReplaceByFIFO (BufferStorage *bs, BM_PageHandle *const ph, 
		    BM_PageFrame *removedFrame) {
	
	// Create a new PageFrame
	BM_PageFrame *pf = (BM_PageFrame*) malloc(sizeof(BM_PageFrame));
	if (pf == NULL) {
		printf("ERROR: Could not allocate memory " 
			"to BM_PageFrame *pf: ReplaceByFIFO");
	}

	pf->pageHandle = ph;
	pf->fixcount = 1;
	pf->dirty = false;
	time(&(pf->lastUsed));

	// Create a new FQ_Node
	FQ_Node *node = (FQ_Node*) malloc(sizeof(FQ_Node));
	if (FQ_Node == NULL) {
		printf("ERROR: Could not allocate memory "
				"to FQ_Node *node: ReplaceByFIFO");
	}

	node->pf = pf;
	node->next = null;
	node->previous = null;
	
	// Get Queue from BufferStorage
	fq = bs->queue;
	
	// Add pointer to node in array
	bs->array[(int)ph->pageNum] = node;

	// Add node to Queue
	fq->tail->next = node;
	node->previous = fq->tail;
	fq->tail = node;

	// Assign remove frame to head of Queue
	removeFrame = fq->head->pf;
	
	// Move head by one position in queue
	FQ_Node *del = fq->head;
	fq->head = fq->head->next;
	free(del);
}

/*
 ****************************************************************************
 * Replacement Strategy: LRU
 * Paramter: 	Pointer to BufferStorage to access the pageFrames,
 * 		Pointer to PageHandle of the page needed to be pinned,
 * 		Pointer to removedFrame
 * Returns:	Void
 * Note:	The removedFrame must be written to file by calling function
 * 		RemovedFrame must be freed by the calling funciton
 ****************************************************************************
 */
void ReplaceByLRU (BufferStorage *bs, BM_PageHandle *const ph,
		   BM_PageFrame *removedFrame) {

	// Create a new PageFrame
	BM_PageFrame *pf = (BM_PageFrame*) malloc(sizeof(BM_PageFrame));
	if (pf == NULL) {
		printf("ERROR: Could not allocate memory "
			"to BM_PageFrame *pf: ReplaceByLRU");
	}

	pf->pageHandle = ph;
	pf->fixcount = 1;
	pf->dirty = false;
	time(&(pf->lastUsed));

	// Create a new FQ_Node
	FQ_Node *node = (FQ_Node*) malloc(sizeof(FQ_Node));
	if (FQ_Node == NULL) {
		printf("ERROR: Could not allocate memory "
			"to FQ_Node *node: ReplaceByLRU");
	}

	node->pf = pf;
	node->next = null;
	node->previous = null;
	
	// Get Queue from buffer storage
	fq = bs->queue;

	// Add pointer to node in array
	bs->array[(int)ph->pageNum] = node;
	
	// Add node to Queue
	fq->tail->next = node;
	node->previous = fq->tail;
	fq->tail = node;
	
	// Find LRU node using linear pass
	FQ_Node *LRU = fq->head;

	FQ_Node *current = fq->head;
	while (current != NULL) {
		if (difftime(LRU->pf->lastUsed, current->pf->lastUsed) > 0) {
			LRU = current;
			current = current->next;
		}
	}
	
	// Assign removedFrame to LRU
	removedFrame = LRU->pf;

	// Delete LRU node
	LRU->previous->next = LRU->next;
	LRU->next->previous = LRU->previous;
	free(LRU);
}

