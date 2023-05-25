// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define PGNUM(pa) (((uint64)pa - KERNBASE) / PGSIZE)
void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct {
  int refcount[(PHYSTOP - KERNBASE)/PGSIZE];
  struct spinlock lock;
} ref;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&ref.lock, "ref");
  freerange(end, (void*)PHYSTOP);
  memset(ref.refcount, 0, sizeof(ref.refcount));
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
  //printf("here\n");
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  acquire(&ref.lock);
  if (ref.refcount[PGNUM(pa)] > 1){
    //printf("%p %d %d\n", pa, PGNUM(pa), ref.refcount[PGNUM(pa)]);
    ref.refcount[PGNUM(pa)] -= 1;
    release(&ref.lock);
    return;
  }
  release(&ref.lock);
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  acquire(&ref.lock);
  memset(pa, 1, PGSIZE);
  ref.refcount[PGNUM(pa)] = 0;
  release(&ref.lock);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r){
    ref.refcount[PGNUM(r)] = 1;
    memset((char*)r, 5, PGSIZE); // fill with junk
  }
  return (void*)r;
}

void 
reflock_acquire(void)
{
  acquire(&ref.lock);
}

void 
reflock_release(void)
{
  release(&ref.lock);
}

void
refcnt_set(uint64 pa, int n)
{
  ref.refcount[PGNUM(pa)] = n;
}

void
refcnt_inc(uint64 pa)
{
  ref.refcount[PGNUM(pa)] += 1;
}

void
refcnt_dec(uint64 pa)
{
  ref.refcount[PGNUM(pa)] -= 1;
}

int
refcnt_get(uint64 pa)
{
  return ref.refcount[PGNUM(pa)];
}