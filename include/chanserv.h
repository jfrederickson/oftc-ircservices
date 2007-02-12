/* TODO: add copyright block */

#ifndef INCLUDED_chanserv_h
#define INCLUDED_chanserv_h

#include "chanserv-lang.h"

struct RegChannel
{
  dlink_node node;
  
  unsigned int id;
  char channel[CHANNELLEN+1];
  char *description;
  char *entrymsg; 
  char *url;
  char *email;
  char *topic;
  char *mlock;
  char forbidden;
  char priv;
  char restricted;
  char topic_lock;
  char verbose;
  char autolimit;
  char expirebans;
};

#endif /* INCLUDED_chanserv_h */
