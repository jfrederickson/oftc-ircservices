/*
 *  hash.c: Maintains hashtables.
 *
 *  Copyright (C) 2002 by the past and present ircd coders, and others.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 *  $Id: hash.c 470 2006-02-17 05:07:43Z db $
 */

#include "stdinc.h"

/*static BlockHeap *userhost_heap = NULL;
static BlockHeap *namehost_heap = NULL;
static struct UserHost *find_or_add_userhost(const char *);
*/
static unsigned int ircd_random_key = 0;

/* The actual hash tables, They MUST be of the same HASHSIZE, variable
 * size tables could be supported but the rehash routine should also
 * rebuild the transformation maps, I kept the tables of equal size 
 * so that I can use one hash function.
 */
static struct Client *idTable[HASHSIZE];
static struct Client *clientTable[HASHSIZE];
static struct Channel *channelTable[HASHSIZE];
static struct UserHost *userhostTable[HASHSIZE];

/* init_hash()
 *
 * inputs       - NONE
 * output       - NONE
 * side effects - Initialize the maps used by hash
 *                functions and clear the tables
 */
void
init_hash(void)
{
  unsigned int i;

  /* Default the userhost/namehost sizes to CLIENT_HEAP_SIZE for now,
   * should be a good close approximation anyway
   * - Dianora
   */
//  userhost_heap = BlockHeapCreate("userhost", sizeof(struct UserHost), CLIENT_HEAP_SIZE);
//  namehost_heap = BlockHeapCreate("namehost", sizeof(struct NameHost), CLIENT_HEAP_SIZE);

  ircd_random_key = rand() % 256;  /* better than nothing --adx */

  /* Clear the hash tables first */
  for (i = 0; i < HASHSIZE; ++i)
  {
    idTable[i]          = NULL;
    clientTable[i]      = NULL;
//    channelTable[i]     = NULL;
//    userhostTable[i]    = NULL;
  }
}

/*
 * New hash function based on the Fowler/Noll/Vo (FNV) algorithm from
 * http://www.isthe.com/chongo/tech/comp/fnv/
 *
 * Here, we use the FNV-1 method, which gives slightly better results
 * than FNV-1a.   -Michael
 */
unsigned int
strhash(const char *name)
{
  const unsigned char *p = (const unsigned char *)name;
  unsigned int hval = FNV1_32_INIT;

  if (*p == '\0')
    return 0;
  for (; *p != '\0'; ++p)
  {
    hval += (hval << 1) + (hval <<  4) + (hval << 7) +
            (hval << 8) + (hval << 24);
    hval ^= (ToLower(*p) ^ ircd_random_key);
  }

  return (hval >> FNV1_32_BITS) ^ (hval & ((1 << FNV1_32_BITS) -1));
}

/************************** Externally visible functions ********************/

/* Optimization note: in these functions I supposed that the CSE optimization
 * (Common Subexpression Elimination) does its work decently, this means that
 * I avoided introducing new variables to do the work myself and I did let
 * the optimizer play with more free registers, actual tests proved this
 * solution to be faster than doing things like tmp2=tmp->hnext... and then
 * use tmp2 myself which would have given less freedom to the optimizer.
 */

/* hash_add_client()
 *
 * inputs       - pointer to client
 * output       - NONE
 * side effects - Adds a client's name in the proper hash linked
 *                list, can't fail, client must have a non-null
 *                name or expect a coredump, the name is infact
 *                taken from client->name
 */
void
hash_add_client(struct Client *client)
{
  unsigned int hashv = strhash(client->name);

  client->hnext = clientTable[hashv];
  clientTable[hashv] = client;
}

/* hash_add_channel()
 *
 * inputs       - pointer to channel
 * output       - NONE
 * side effects - Adds a channel's name in the proper hash linked
 *                list, can't fail. chptr must have a non-null name
 *                or expect a coredump. As before the name is taken
 *                from chptr->name, we do hash its entire lenght
 *                since this proved to be statistically faster
 */
/*void
hash_add_channel(struct Channel *chptr)
{
  unsigned int hashv = strhash(chptr->chname);

  chptr->hnextch = channelTable[hashv];
  channelTable[hashv] = chptr;
}*/

/*void
hash_add_userhost(struct UserHost *userhost)
{
  unsigned int hashv = strhash(userhost->host);

  userhost->next = userhostTable[hashv];
  userhostTable[hashv] = userhost;
}*/

void
hash_add_id(struct Client *client)
{
  unsigned int hashv = strhash(client->id);

  client->idhnext = idTable[hashv];
  idTable[hashv] = client;
}

/* hash_del_id()
 *
 * inputs       - pointer to client
 * output       - NONE
 * side effects - Removes an ID from the hash linked list
 */
void
hash_del_id(struct Client *client)
{
  unsigned int hashv = strhash(client->id);
  struct Client *tmp = idTable[hashv];

  if (tmp != NULL)
  {
    if (tmp == client)
    {
      idTable[hashv] = client->idhnext;
      client->idhnext = client;
    }
    else
    {
      while (tmp->idhnext != client)
      {
        if ((tmp = tmp->idhnext) == NULL)
          return;
      }

      tmp->idhnext = tmp->idhnext->idhnext;
      client->idhnext = client;
    }
  }
}

/* hash_del_client()
 *
 * inputs       - pointer to client
 * output       - NONE
 * side effects - Removes a Client's name from the hash linked list
 */
void
hash_del_client(struct Client *client)
{
  unsigned int hashv = strhash(client->name);
  struct Client *tmp = clientTable[hashv];

  if (tmp != NULL)
  {
    if (tmp == client)
    {
      clientTable[hashv] = client->hnext;
      client->hnext = client;
    }
    else
    {
      while (tmp->hnext != client)
      {
        if ((tmp = tmp->hnext) == NULL)
          return;
      }

      tmp->hnext = tmp->hnext->hnext;
      client->hnext = client;
    }
  }
}
#if 0
/* hash_del_userhost()
 *
 * inputs       - pointer to userhost
 * output       - NONE
 * side effects - Removes a userhost from the hash linked list
 */
void
hash_del_userhost(struct UserHost *userhost)
{
  unsigned int hashv = strhash(userhost->host);
  struct UserHost *tmp = userhostTable[hashv];

  if (tmp != NULL)
  {
    if (tmp == userhost)
    {
      userhostTable[hashv] = userhost->next;
      userhost->next = userhost;
    }
    else
    {
      while (tmp->next != userhost)
      {
        if ((tmp = tmp->next) == NULL)
          return;
      }

      tmp->next = tmp->next->next;
      userhost->next = userhost;
    }
  }
}

/* hash_del_channel()
 *
 * inputs       - pointer to client
 * output       - NONE
 * side effects - Removes the channel's name from the corresponding
 *                hash linked list
 */
void
hash_del_channel(struct Channel *chptr)
{
  unsigned int hashv = strhash(chptr->chname);
  struct Channel *tmp = channelTable[hashv];

  if (tmp != NULL)
  {
    if (tmp == chptr)
    {
      channelTable[hashv] = chptr->hnextch;
      chptr->hnextch = chptr;
    }
    else
    {
      while (tmp->hnextch != chptr)
      {
        if ((tmp = tmp->hnextch) == NULL)
          return;
      }

      tmp->hnextch = tmp->hnextch->hnextch;
      chptr->hnextch = chptr;
    }
  }
}
#endif

/* find_client()
 *
 * inputs       - pointer to name
 * output       - NONE
 * side effects - New semantics: finds a client whose name is 'name'
 *                if can't find one returns NULL. If it finds one moves
 *                it to the top of the list and returns it.
 */
struct Client *
find_client(const char *name)
{
  unsigned int hashv = strhash(name);
  struct Client *client;

  if ((client = clientTable[hashv]) != NULL)
  {
    if (irccmp(name, client->name))
    {
      struct Client *prev;

      while (prev = client, (client = client->hnext) != NULL)
      {
        if (!irccmp(name, client->name))
        {
          prev->hnext = client->hnext;
          client->hnext = clientTable[hashv];
          clientTable[hashv] = client;
          break;
        }
      }
    }
  }

  return client;
}

struct Client *
hash_find_id(const char *name)
{
  unsigned int hashv = strhash(name);
  struct Client *client;

  if ((client = idTable[hashv]) != NULL)
  {
    if (irccmp(name, client->id))
    {
      struct Client *prev;

      while (prev = client, (client = client->idhnext) != NULL)
      {
        if (!irccmp(name, client->id))
        {
          prev->idhnext = client->idhnext;
          client->idhnext = idTable[hashv];
          idTable[hashv] = client;
          break;
        }
      }
    }
  }

  return client;
}

/*
 * Whats happening in this next loop ? Well, it takes a name like
 * foo.bar.edu and proceeds to earch for *.edu and then *.bar.edu.
 * This is for checking full server names against masks although
 * it isnt often done this way in lieu of using matches().
 *
 * Rewrote to do *.bar.edu first, which is the most likely case,
 * also made const correct
 * --Bleep
 */
static struct Client *
hash_find_masked_server(const char *name)
{
  char buf[HOSTLEN + 1];
  char *p = buf;
  char *s = NULL;
  struct Client *server = NULL;

  if (*name == '*' || *name == '.')
    return NULL;

  /*
   * copy the damn thing and be done with it
   */
  strlcpy(buf, name, sizeof(buf));

  while ((s = strchr(p, '.')) != NULL)
  {
    *--s = '*';

    /* Dont need to check IsServer() here since nicknames cant
     * have *'s in them anyway.
     */
    if ((server = find_client(s)) != NULL)
      return server;
    p = s + 2;
  }

  return NULL;
}

struct Client *
find_server(const char *name)
{
  unsigned int hashv = strhash(name);
  struct Client *client = NULL;

  if (IsDigit(*name) && strlen(name) == IRC_MAXSID)
    client = hash_find_id(name);

  if ((client == NULL) && (client = clientTable[hashv]) != NULL)
  {
    if ((!IsServer(client) && !IsMe(client)) ||
        irccmp(name, client->name))
    {
      struct Client *prev;

      while (prev = client, (client = client->hnext) != NULL)
      {
        if ((IsServer(client) || IsMe(client)) &&
            !irccmp(name, client->name))
        {
          prev->hnext = client->hnext;
          client->hnext = clientTable[hashv];
          clientTable[hashv] = client;
          break;
        }
      }
    }
  }

  return (client != NULL) ? client : hash_find_masked_server(name);
}

/* hash_find_channel()
 *
 * inputs       - pointer to name
 * output       - NONE
 * side effects - New semantics: finds a channel whose name is 'name', 
 *                if can't find one returns NULL, if can find it moves
 *                it to the top of the list and returns it.
 */
/*struct Channel *
hash_find_channel(const char *name)
{
  unsigned int hashv = strhash(name);
  struct Channel *chptr = NULL;

  if ((chptr = channelTable[hashv]) != NULL)
  {
    if (irccmp(name, chptr->chname))
    {
      struct Channel *prev;

      while (prev = chptr, (chptr = chptr->hnextch) != NULL)
      {
        if (!irccmp(name, chptr->chname))
        {
          prev->hnextch = chptr->hnextch;
          chptr->hnextch = channelTable[hashv];
          channelTable[hashv] = chptr;
          break;
        }
      }
    }
  }

  return chptr;
}
*/
/* hash_get_bucket(int type, unsigned int hashv)
 *
 * inputs       - hash value (must be between 0 and HASHSIZE - 1)
 * output       - NONE
 * returns      - pointer to first channel in channelTable[hashv]
 *                if that exists;
 *                NULL if there is no channel in that place;
 *                NULL if hashv is an invalid number.
 * side effects - NONE
 */
void *
hash_get_bucket(int type, unsigned int hashv)
{
  assert(hashv < HASHSIZE);
  if (hashv >= HASHSIZE)
      return NULL;

  switch (type)
  {
    case HASH_TYPE_ID:
      return idTable[hashv];
      break;
    case HASH_TYPE_CHANNEL:
      return channelTable[hashv];
      break;
    case HASH_TYPE_CLIENT:
      return clientTable[hashv];
      break;
    case HASH_TYPE_USERHOST:
      return userhostTable[hashv];
      break;
    default:
      assert(0);
  }

  return NULL;
}

/*struct UserHost *
hash_find_userhost(const char *host)
{
  unsigned int hashv = strhash(host);
  struct UserHost *userhost;

  if ((userhost = userhostTable[hashv]))
  {
    if (irccmp(host, userhost->host))
    {
      struct UserHost *prev;

      while (prev = userhost, (userhost = userhost->next) != NULL)
      {
        if (!irccmp(host, userhost->host))
        {
          prev->next = userhost->next;
          userhost->next = userhostTable[hashv];
          userhostTable[hashv] = userhost;
          break;
        }
      }
    }
  }

  return userhost;
}*/

/* count_user_host()
 *
 * inputs	- user name
 *		- hostname
 *		- int flag 1 if global, 0 if local
 * 		- pointer to where global count should go
 *		- pointer to where local count should go
 *		- pointer to where identd count should go (local clients only)
 * output	- none
 * side effects	-
 */
/*void
count_user_host(const char *user, const char *host, int *global_p,
                int *local_p, int *icount_p)
{
  dlink_node *ptr;
  struct UserHost *found_userhost;
  struct NameHost *nameh;

  if ((found_userhost = hash_find_userhost(host)) == NULL)
    return;

  DLINK_FOREACH(ptr, found_userhost->list.head)
  {
    nameh = ptr->data;

    if (!irccmp(user, nameh->name))
    {
      if (global_p != NULL)
        *global_p = nameh->gcount;
      if (local_p != NULL)
        *local_p  = nameh->lcount;
      if (icount_p != NULL)
        *icount_p = nameh->icount;
      return;
    }
  }
}
*/
/* add_user_host()
 *
 * inputs	- user name
 *		- hostname
 *		- int flag 1 if global, 0 if local
 * output	- none
 * side effects	- add given user@host to hash tables
 */
/*void
add_user_host(const char *user, const char *host, int global)
{
  dlink_node *ptr;
  struct UserHost *found_userhost;
  struct NameHost *nameh;
  int hasident = 1;

  if (*user == '~')
  {
    hasident = 0;
    ++user;
  }

  if ((found_userhost = find_or_add_userhost(host)) == NULL)
    return;

  DLINK_FOREACH(ptr, found_userhost->list.head)
  {
    nameh = ptr->data;

    if (!irccmp(user, nameh->name))
    {
      nameh->gcount++;
      if (!global)
      {
	if (hasident)
	  nameh->icount++;
	nameh->lcount++;
      }
      return;
    }
  }

  nameh = BlockHeapAlloc(namehost_heap);
  strlcpy(nameh->name, user, sizeof(nameh->name));

  nameh->gcount = 1;
  if (!global)
  {
    if (hasident)
      nameh->icount = 1;
    nameh->lcount = 1;
  }

  dlinkAdd(nameh, &nameh->node, &found_userhost->list);
}
*/
/* delete_user_host()
 *
 * inputs	- user name
 *		- hostname
 *		- int flag 1 if global, 0 if local
 * output	- none
 * side effects	- delete given user@host to hash tables
 */
/*void
delete_user_host(const char *user, const char *host, int global)
{
  dlink_node *ptr = NULL, *next_ptr = NULL;
  struct UserHost *found_userhost;
  struct NameHost *nameh;
  int hasident = 1;

  if (*user == '~')
  {
    hasident = 0;
    ++user;
  }

  if ((found_userhost = hash_find_userhost(host)) == NULL)
    return;

  DLINK_FOREACH_SAFE(ptr, next_ptr, found_userhost->list.head)
  {
    nameh = ptr->data;

    if (!irccmp(user, nameh->name))
    {
      if (nameh->gcount > 0)
        nameh->gcount--;
      if (!global)
      {
	if (nameh->lcount > 0)
	  nameh->lcount--;
	if (hasident && nameh->icount > 0)
	  nameh->icount--;
      }

      if (nameh->gcount == 0 && nameh->lcount == 0)
      {
	dlinkDelete(&nameh->node, &found_userhost->list);
	BlockHeapFree(namehost_heap, nameh);
      }

      if (dlink_list_length(&found_userhost->list) == 0)
      {
	hash_del_userhost(found_userhost);
	BlockHeapFree(userhost_heap, found_userhost);
      }

      return;
    }
  }
}
*/
/* find_or_add_userhost()
 *
 * inputs	- host name
 * output	- none
 * side effects	- find UserHost * for given host name
 */
/*static struct UserHost *
find_or_add_userhost(const char *host)
{
  struct UserHost *userhost;

  if ((userhost = hash_find_userhost(host)) != NULL)
    return userhost;

  userhost = BlockHeapAlloc(userhost_heap);
  strlcpy(userhost->host, host, sizeof(userhost->host));
  hash_add_userhost(userhost);

  return userhost;
}*/
