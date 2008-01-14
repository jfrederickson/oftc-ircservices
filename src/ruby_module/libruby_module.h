#ifndef INCLUDED_libruby_module_h
#define INCLUDED_libruby_module_h

/* Umm, it sucks but ruby defines these and so do we */
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef EXTERN

#include "stdinc.h"

VALUE rb_carray2rbarray(int, char **);

struct Client* value_to_client(VALUE);
VALUE client_to_value(struct Client*);

struct Channel* rb_rbchannel2cchannel(VALUE);
VALUE rb_cchannel2rbchannel(struct Channel*);

struct RegChannel* rb_rbregchan2cregchan(VALUE);
VALUE rb_cregchan2rbregchan(struct RegChannel*);

struct Nick* rb_rbnick2cnick(VALUE);
VALUE rb_cnick2rbnick(struct Nick*);

void Init_ChannelStruct(void);
void Init_RegChannel(void);
void Init_Client(void);
void Init_NickStruct(void);
void Init_ServiceModule(void);

enum Ruby_Hooks
{
  RB_HOOKS_CMODE,
  RB_HOOKS_UMODE,
  RB_HOOKS_NEWUSR,
  RB_HOOKS_PRIVMSG,
  RB_HOOKS_JOIN,
  RB_HOOKS_PART,
  RB_HOOKS_QUIT,
  RB_HOOKS_NICK,
  RB_HOOKS_NOTICE,
  RB_HOOKS_CHAN_CREATED,
  RB_HOOKS_CHAN_DELETED,
  RB_HOOKS_CTCP,
  RB_HOOKS_NICK_REG,
  RB_HOOKS_CHAN_REG,
  RB_HOOKS_DB_INIT,
  RB_HOOKS_EOB,
  RB_HOOKS_COUNT
};

#define RB_CALLBACK(x) (VALUE (*)())(x)

VALUE rb_singleton_call(VALUE);

void rb_do_hook_cb(VALUE, VALUE);
void rb_add_hook(VALUE, VALUE, int);
void rb_add_event(VALUE, VALUE, VALUE);

int ruby_handle_error(int);
VALUE do_ruby(VALUE, ID, int, ...);
VALUE do_ruby_ret(VALUE, ID, int, ...);

char *strupr(char *);

struct ruby_args
{
  VALUE recv;
  ID id;
  int parc;
  VALUE *parv;
};

struct rhook_args
{
  int parc;
  VALUE *parv;
};

void check_our_type(VALUE obj, VALUE type);

extern VALUE cServiceModule;
extern VALUE cNickStruct;
extern VALUE cClient;
extern VALUE cRegChannel;
extern VALUE cChannelStruct;

#define Check_OurType(x, v) check_our_type((VALUE)(x), (VALUE)(v))

#endif /* INCLUDED_ruby_module_h */
