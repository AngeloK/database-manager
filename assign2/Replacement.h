# ifndef REPLACEMENT_H
# define REPLACMENT_H

# include "structures.h"

void ReplaceByFIFO (BufferStorage*, BM_PageHandle*, BM_PageFrame*);
void ReplaceByLRU (BufferStorage*, BM_PageHandle*, BM_PageFrame*);
