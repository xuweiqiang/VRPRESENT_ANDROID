// local copy of Shared/Lib/GenLib/utillist's CUTILrwqueue

#ifndef _LINUXRWQUEUE_H
#define _LINUXRWQUEUE_H

#include <windows.h>

typedef void (__cdecl *PITEMDESTRUCTOR)(void *ptr);

struct ISearchCriteria
	{
	virtual ~ISearchCriteria() {}
	virtual bool Found(void *item) = 0;
	};

class CLNrwqueue
	{
public:
	CLNrwqueue(unsigned long max, unsigned long size);
	~CLNrwqueue();
	void *PeekItem(unsigned long offset);
	bool SearchItem(void *pItem, ISearchCriteria* pTarget=0, bool bRemove=0);
	void *GetReadItem();
	void CancelReadItem();
	void CommitReadItem();
	int ReadItem(void *item);
	void *GetWriteItem();
	void CancelWriteItem();
	void CommitWriteItem();
	int WriteItem(void *item);
	int EndOfFile();
	int Flush(PITEMDESTRUCTOR pdelete = NULL);

	inline unsigned long GetCount();
	inline unsigned long GetMax();

protected:
	HANDLE mutex;
	HANDLE semaphore_read, semaphore_write;
	unsigned char *buf;
	unsigned long max, size;
	unsigned long count;
	unsigned long idx_read, idx_write;
	BOOL eof;
	};

unsigned long CLNrwqueue::GetCount()
	{
	return count;
	}

unsigned long CLNrwqueue::GetMax()
	{
	return max;
	}

#endif /* _LINUXRWQUEUE_H */
