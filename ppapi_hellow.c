/*
 * Copyright (c) 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This is a minimal NaCl "hello world" program.  It uses NaCl's
 * stable IRT ABI.
 */

#include <stdint.h>

#include "nacl_irt_interfaces.h"
#include "irt_ppapi.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/c/ppb_core.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"


struct nacl_irt_basic __libnacl_irt_basic;
struct nacl_irt_fdio __libnacl_irt_fdio;
TYPE_nacl_irt_query __nacl_irt_query;

static const PPB_Core *ppb_core;
static const struct PPB_Var_1_0 *ppb_var;


static void grok_auxv(const Elf_auxv_t *auxv) {
  const Elf_auxv_t *av;
  for (av = auxv; av->a_type != AT_NULL; ++av) {
    if (av->a_type == AT_SYSINFO) {
      __nacl_irt_query = (TYPE_nacl_irt_query) av->a_un.a_val;
    }
  }
}

static void __libnacl_mandatory_irt_query(const char *interface,
                                          void *table, size_t table_size) {
  __nacl_irt_query(interface, table, table_size);
}

#define DO_QUERY(ident, name) \
    __libnacl_mandatory_irt_query(ident, &name, sizeof(name))

static void do_write(const char *string, size_t size) {
  size_t written;
  __libnacl_irt_fdio.write(1, string, size, &written);
}

#define LOG(string) do_write(string, sizeof(string) - 1)


static size_t my_strlen(const char *s) {
  size_t n = 0;
  while (*s++ != '\0')
    ++n;
  return n;
}

static int my_strcmp(const char *a, const char *b) {
  while (*a == *b) {
    if (*a == '\0')
      return 0;
    ++a;
    ++b;
  }
  return (int) (unsigned char) *a - (int) (unsigned char) *b;
}

static void log_string(const char *string) {
  do_write(string, my_strlen(string));
}


static PP_Bool DidCreate(PP_Instance instance,
                         uint32_t argc,
                         const char* argn[],
                         const char* argv[]) {
  LOG("In DidCreate\n");
  return 1;
}

static void DidDestroy(PP_Instance instance) {
  LOG("In DidDestroy\n");
}

static void DidChangeView(PP_Instance instance,
                          const struct PP_Rect* position,
                          const struct PP_Rect* clip) {
  LOG("In DidChangeView\n");
}

static void DidChangeFocus(PP_Instance instance, PP_Bool has_focus) {
  LOG("In DidChangeFocus\n");
}

static PP_Bool HandleDocumentLoad(PP_Instance instance,
                                  PP_Resource url_loader) {
  LOG("In HandleDocumentLoad\n");
  return 0;
}

static struct PPP_Instance_1_0 ppp_instance;

static void HandleMessage(PP_Instance instance, struct PP_Var message) {
  LOG("In HandleMessage\n");
  if (message.type == PP_VARTYPE_STRING && ppb_var != NULL) {
    LOG("Received: ");
    uint32_t len;
    const char *string = ppb_var->VarToUtf8(message, &len);
    do_write(string, len);
  }
}

static struct PPP_Messaging_1_0 ppp_messaging;


static void Callback(void *data, int32_t result) {
  LOG("In Callback(), called via CallOnMainThread()\n");
  ppb_core->CallOnMainThread(
      1000, PP_MakeCompletionCallback(Callback, NULL), 0);
}

static int32_t MyPPP_InitializeModule(PP_Module module_id,
                                      PPB_GetInterface get_browser_interface) {
  LOG("In PPP_InitializeModule\n");

  ppb_core = get_browser_interface(PPB_CORE_INTERFACE);
  ppb_var = get_browser_interface(PPB_VAR_INTERFACE_1_0);
  ppb_core->CallOnMainThread(
      1000, PP_MakeCompletionCallback(Callback, NULL), 0);
  return PP_OK;
}

static void MyPPP_ShutdownModule(void) {
  LOG("In PPP_ShutdownModule\n");
}

static const void *MyPPP_GetInterface(const char *interface_name) {
  LOG("In PPP_GetInterface\n");
  log_string(interface_name);
  log_string("\n");
  if (my_strcmp(interface_name, PPP_INSTANCE_INTERFACE_1_0) == 0)
    return &ppp_instance;
  if (my_strcmp(interface_name, PPP_MESSAGING_INTERFACE_1_0) == 0)
    return &ppp_messaging;
  return NULL;
}

void _start(uintptr_t *info) {
  Elf_auxv_t *auxv = nacl_startup_auxv(info);
  grok_auxv(auxv);
  DO_QUERY(NACL_IRT_BASIC_v0_1, __libnacl_irt_basic);
  DO_QUERY(NACL_IRT_DEV_FDIO_v0_1, __libnacl_irt_fdio);

  LOG("Hello world!\n");

  struct nacl_irt_ppapihook ppapihook;
  DO_QUERY(NACL_IRT_PPAPIHOOK_v0_1, ppapihook);
  /* This is local as a workaround to avoid having to apply
   * relocations to global variables. */
  struct PP_StartFunctions start_funcs = {
    MyPPP_InitializeModule,
    MyPPP_ShutdownModule,
    MyPPP_GetInterface,
  };
  /* Similarly, initialise some global variables, avoiding relocations. */
  struct PPP_Instance_1_0 local_ppp_instance = {
    DidCreate,
    DidDestroy,
    DidChangeView,
    DidChangeFocus,
    HandleDocumentLoad,
  };
  ppp_instance = local_ppp_instance;
  struct PPP_Messaging_1_0 local_ppp_messaging = {
    HandleMessage,
  };
  ppp_messaging = local_ppp_messaging;

  ppapihook.ppapi_start(&start_funcs);

  LOG("ppapi_start() returned\n");

  __libnacl_irt_basic.exit(0);
}
