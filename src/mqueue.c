/* TODO: Add copyright */

#include "stdinc.h"
#include "mqueue.h"
#include "hash.h"

static BlockHeap *mqueue_heap = NULL;
static BlockHeap *fmsg_heap   = NULL;

void
init_mqueue()
{
  mqueue_heap = BlockHeapCreate("mqueue", sizeof(struct MessageQueue), MQUEUE_HEAP_SIZE);
  fmsg_heap = BlockHeapCreate("fmsg", sizeof(struct FloodMsg), FMSG_HEAP_SIZE);
}

void
cleanup_mqueue()
{
  BlockHeapDestroy(mqueue_heap);
  BlockHeapDestroy(fmsg_heap);
}

struct MessageQueue *
mqueue_new(const char *name, unsigned int type, int max, int msg_time,
int lne_time)
{
  struct MessageQueue *queue = BlockHeapAlloc(mqueue_heap);

  DupString(queue->name, name);
  assert(queue->name != NULL);

  queue->max  = max;
  queue->msg_enforce_time = msg_time;
  queue->lne_enforce_time = lne_time;
  queue->type = type;

  assert(queue->name != NULL);
  return queue;
}

void
mqueue_hash_free(struct MessageQueue **hash, dlink_list *list)
{
  dlink_node *ptr = NULL, *next_ptr = NULL;
  if(hash != NULL)
  {
    DLINK_FOREACH_SAFE(ptr, next_ptr, list->head)
    {
      struct MessageQueue *queue = ptr->data;
      if(queue->name == NULL)
      {
        ilog(L_DEBUG, "Trying to free already free'd MessageQueue");
        abort();
      }
      assert(queue->name != NULL);
      hash_del_mqueue(hash, queue);
      dlinkDelete(ptr, list);
      mqueue_free(queue);
    }
    MyFree(hash);
  }
}

void
mqueue_free(struct MessageQueue *queue)
{
  if(queue != NULL)
  {
    dlink_node *ptr = NULL, *nptr = NULL;

    MyFree(queue->name);
    queue->name = NULL;

    DLINK_FOREACH_SAFE(ptr, nptr, queue->entries.head)
    {
      struct FloodMsg *data = ptr->data;
      dlinkDelete(ptr, &queue->entries);
      floodmsg_free(data);
    }

    BlockHeapFree(mqueue_heap, queue);
  }
}

void
floodmsg_free(struct FloodMsg *entry)
{
  if(entry != NULL)
  {
    MyFree(entry->message);
    entry->message = NULL;
    BlockHeapFree(fmsg_heap, entry);
    entry = NULL;
  }
}

struct FloodMsg *
floodmsg_new(const char *message)
{
  struct FloodMsg *entry = BlockHeapAlloc(fmsg_heap);
  entry->time = CurrentTime;
  DupString(entry->message, message);
  return entry;
}

