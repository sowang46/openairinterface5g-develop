/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include "rrc_defs.h"
#include "rrc_extern.h"
#include "rrc_eNB_UE_context.h"
#include "common/ran_context.h"
#include "LTE_DL-DCCH-Message.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

extern RAN_CONTEXT_t RC;
extern mui_t rrc_eNB_mui;

int rrc_eNB_generate_RRCConnectionReconfiguration_endc(protocol_ctxt_t *ctxt,
                                                       rrc_eNB_ue_context_t *ue_context,
                                                       unsigned char *buffer,
                                                       int buffer_size)
{
  asn_enc_rval_t                                          enc_rval;
  LTE_DL_DCCH_Message_t                                   dl_dcch_msg;
  LTE_RRCConnectionReconfiguration_t                      *r;
  int                                                     trans_id;
  LTE_RadioResourceConfigDedicated_t                      rrcd;
  LTE_DRB_ToAddModList_t                                  drb_list;
  struct LTE_DRB_ToAddMod                                 drb;
  long                                                    eps_bearer_id;
  struct LTE_RLC_Config                                   rlc;
  long                                                    lcid;
  struct LTE_LogicalChannelConfig                         lc;
  struct LTE_LogicalChannelConfig__ul_SpecificParameters  ul_params;
  long                                                    lcg;
  struct LTE_RadioResourceConfigDedicated__mac_MainConfig mac;
  struct LTE_MAC_MainConfig__ext4                         mac_ext4;
  struct LTE_MAC_MainConfig__ext4__dualConnectivityPHR    dc_phr;

  memset(&rrcd, 0, sizeof(rrcd));
  memset(&drb_list, 0, sizeof(drb_list));
  memset(&drb, 0, sizeof(drb));
  memset(&rlc, 0, sizeof(rlc));
  memset(&lc, 0, sizeof(lc));
  memset(&ul_params, 0, sizeof(ul_params));
  memset(&mac, 0, sizeof(mac));
  memset(&mac_ext4, 0, sizeof(mac_ext4));
  memset(&dc_phr, 0, sizeof(dc_phr));

  trans_id = rrc_eNB_get_next_transaction_identifier(ctxt->module_id);

  memset(&dl_dcch_msg,0,sizeof(LTE_DL_DCCH_Message_t));

  dl_dcch_msg.message.present           = LTE_DL_DCCH_MessageType_PR_c1;
  dl_dcch_msg.message.choice.c1.present = LTE_DL_DCCH_MessageType__c1_PR_rrcConnectionReconfiguration;

  r = &dl_dcch_msg.message.choice.c1.choice.rrcConnectionReconfiguration;

  r->rrc_TransactionIdentifier = trans_id;
  r->criticalExtensions.present = LTE_RRCConnectionReconfiguration__criticalExtensions_PR_c1;
  r->criticalExtensions.choice.c1.present = LTE_RRCConnectionReconfiguration__criticalExtensions__c1_PR_rrcConnectionReconfiguration_r8;
  r->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated = &rrcd;

  rrcd.drb_ToAddModList = &drb_list;

  eps_bearer_id = 5;
  drb.eps_BearerIdentity = &eps_bearer_id;
  drb.drb_Identity = 5;
  drb.rlc_Config = &rlc;
  lcid = 4;
  drb.logicalChannelIdentity = &lcid;
  drb.logicalChannelConfig = &lc;

  rlc.present = LTE_RLC_Config_PR_am;
  rlc.choice.am.ul_AM_RLC.t_PollRetransmit = LTE_T_PollRetransmit_ms50;
  rlc.choice.am.ul_AM_RLC.pollPDU = LTE_PollPDU_p16;
  rlc.choice.am.ul_AM_RLC.pollByte = LTE_PollByte_kBinfinity;
  rlc.choice.am.ul_AM_RLC.maxRetxThreshold = LTE_UL_AM_RLC__maxRetxThreshold_t8;
  rlc.choice.am.dl_AM_RLC.t_Reordering = LTE_T_Reordering_ms35;
  rlc.choice.am.dl_AM_RLC.t_StatusProhibit = LTE_T_StatusProhibit_ms25;

  lc.ul_SpecificParameters = &ul_params;

  ul_params.priority = 12;
  ul_params.prioritisedBitRate = LTE_LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_kBps8;
  ul_params.bucketSizeDuration = LTE_LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms300;
  lcg = 3;
  ul_params.logicalChannelGroup = &lcg;

  rrcd.mac_MainConfig = &mac;

  mac.present = LTE_RadioResourceConfigDedicated__mac_MainConfig_PR_explicitValue;
  mac.choice.explicitValue.timeAlignmentTimerDedicated = LTE_TimeAlignmentTimer_sf10240;
  mac.choice.explicitValue.ext4 = &mac_ext4;

  mac_ext4.dualConnectivityPHR = &dc_phr;

  dc_phr.present = LTE_MAC_MainConfig__ext4__dualConnectivityPHR_PR_setup;
  dc_phr.choice.setup.phr_ModeOtherCG_r12 = LTE_MAC_MainConfig__ext4__dualConnectivityPHR__setup__phr_ModeOtherCG_r12_virtual;

  /* NR config */
  struct LTE_RRCConnectionReconfiguration_v890_IEs cr_890;
  struct LTE_RRCConnectionReconfiguration_v920_IEs cr_920;
  struct LTE_RRCConnectionReconfiguration_v1020_IEs cr_1020;
  struct LTE_RRCConnectionReconfiguration_v1130_IEs cr_1130;
  struct LTE_RRCConnectionReconfiguration_v1250_IEs cr_1250;
  struct LTE_RRCConnectionReconfiguration_v1310_IEs cr_1310;
  struct LTE_RRCConnectionReconfiguration_v1430_IEs cr_1430;
  struct LTE_RRCConnectionReconfiguration_v1510_IEs cr_1510;
  struct LTE_RRCConnectionReconfiguration_v1510_IEs__nr_Config_r15 nr;

  memset(&cr_890, 0, sizeof(cr_890));
  memset(&cr_920, 0, sizeof(cr_920));
  memset(&cr_1020, 0, sizeof(cr_1020));
  memset(&cr_1130, 0, sizeof(cr_1130));
  memset(&cr_1250, 0, sizeof(cr_1250));
  memset(&cr_1310, 0, sizeof(cr_1310));
  memset(&cr_1430, 0, sizeof(cr_1430));
  memset(&cr_1510, 0, sizeof(cr_1510));
  memset(&nr, 0, sizeof(nr));

  r->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.nonCriticalExtension = &cr_890;
  cr_890.nonCriticalExtension = &cr_920;
  cr_920.nonCriticalExtension = &cr_1020;
  cr_1020.nonCriticalExtension = &cr_1130;
  cr_1130.nonCriticalExtension = &cr_1250;
  cr_1250.nonCriticalExtension = &cr_1310;
  cr_1310.nonCriticalExtension = &cr_1430;
  cr_1430.nonCriticalExtension = &cr_1510;

  cr_1510.nr_Config_r15 = &nr;
  nr.present = LTE_RRCConnectionReconfiguration_v1510_IEs__nr_Config_r15_PR_setup;
  nr.choice.setup.endc_ReleaseAndAdd_r15 = 0;  /* FALSE */

  OCTET_STRING_t scg_conf;
  unsigned char scg_conf_buf[4] = { 0, 0, 0, 0 };
  nr.choice.setup.nr_SecondaryCellGroupConfig_r15 = &scg_conf;
  scg_conf.buf = scg_conf_buf;
  scg_conf.size = 4;

  long sk_counter = 0;
  cr_1510.sk_Counter_r15 = &sk_counter;

  OCTET_STRING_t nr1_conf;
  unsigned char nr1_buf[4] = { 0, 0, 0, 0 };
  cr_1510.nr_RadioBearerConfig1_r15 = &nr1_conf;
  nr1_conf.buf = nr1_buf;
  nr1_conf.size = 4;

  OCTET_STRING_t nr2_conf;
  unsigned char nr2_buf[4] = { 0, 0, 0, 0 };
  cr_1510.nr_RadioBearerConfig2_r15 = &nr2_conf;
  nr2_conf.buf = nr2_buf;
  nr2_conf.size = 4;

  ASN_SEQUENCE_ADD(&drb_list.list, &drb);

  enc_rval = uper_encode_to_buffer(&asn_DEF_LTE_DL_DCCH_Message,
                                   NULL,
                                   (void *)&dl_dcch_msg,
                                   buffer,
                                   buffer_size);

{
int len = (enc_rval.encoded + 7) / 8;
int i;
printf("len = %d\n", len);
for (i = 0; i < len; i++) printf(" %2.2x", buffer[i]);
printf("\n");
}

  return (enc_rval.encoded + 7) / 8;
}

volatile int go_nr;

struct rrc_eNB_ue_context_s *
get_first_ue_context(eNB_RRC_INST *rrc_instance_pP)
{
  struct rrc_eNB_ue_context_s   *ue_context_p = NULL;
  RB_FOREACH(ue_context_p, rrc_ue_tree_s, &(rrc_instance_pP->rrc_ue_head)) {
    return ue_context_p;
  }
  return NULL;
}

void rrc_go_nr(void)
{
  protocol_ctxt_t ctxt;
  rrc_eNB_ue_context_t *ue_context;
  unsigned char buffer[8192];
  int size;

  go_nr = 0;

  ue_context = get_first_ue_context(RC.rrc[0]);

  PROTOCOL_CTXT_SET_BY_INSTANCE(&ctxt,
                                0,
                                ENB_FLAG_YES,
                                ue_context->ue_context.rnti,
                                0, 0);

  size = rrc_eNB_generate_RRCConnectionReconfiguration_endc(&ctxt, ue_context, buffer, 8192);

  rrc_data_req(&ctxt,
               DCCH,
               rrc_eNB_mui++,
               SDU_CONFIRM_NO,
               size,
               buffer,
               PDCP_TRANSMISSION_MODE_CONTROL);
}

static void new_thread(void *(*f)(void *), void *data) {
  pthread_t t;
  pthread_attr_t att;

  if (pthread_attr_init(&att)) {
    fprintf(stderr, "pthread_attr_init err\n");
    exit(1);
  }

  if (pthread_attr_setdetachstate(&att, PTHREAD_CREATE_DETACHED)) {
    fprintf(stderr, "pthread_attr_setdetachstate err\n");
    exit(1);
  }

  if (pthread_attr_setstacksize(&att, 10000000)) {
    fprintf(stderr, "pthread_attr_setstacksize err\n");
    exit(1);
  }

  if (pthread_create(&t, &att, f, data)) {
    fprintf(stderr, "pthread_create err\n");
    exit(1);
  }

  if (pthread_attr_destroy(&att)) {
    fprintf(stderr, "pthread_attr_destroy err\n");
    exit(1);
  }
}

static int create_listen_socket(char *addr, int port) {
  struct sockaddr_in a;
  int s;
  int v;
  s = socket(AF_INET, SOCK_STREAM, 0);

  if (s == -1) {
    perror("socket");
    exit(1);
  }

  v = 1;

  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(int))) {
    perror("setsockopt");
    exit(1);
  }

  a.sin_family = AF_INET;
  a.sin_port = htons(port);
  a.sin_addr.s_addr = inet_addr(addr);

  if (bind(s, (struct sockaddr *)&a, sizeof(a))) {
    perror("bind");
    exit(1);
  }

  if (listen(s, 5)) {
    perror("listen");
    exit(1);
  }

  return s;
}

static int socket_accept(int s) {
  struct sockaddr_in a;
  socklen_t alen;
  alen = sizeof(a);
  return accept(s, (struct sockaddr *)&a, &alen);
}

static int fullread(int fd, void *_buf, int count) {
  char *buf = _buf;
  int ret = 0;
  int l;

  while (count) {
    l = read(fd, buf, count);

    if (l <= 0) return -1;

    count -= l;
    buf += l;
    ret += l;
  }

  return ret;
}

void *nr_hack(void *_)
{
  int s = create_listen_socket("0.0.0.0", 9001);
  int t;

over:
  t = socket_accept(s);

  while (1) {
    char c;

    if (fullread(t, &c, 1) != 1) {
      close(t);
      goto over;
    }

    if (c != '\n') continue;

    printf("setting go_nr to 1\n");
    go_nr = 1;
  }

  return 0;
}

void rrc_endc_hack_init(void)
{
  new_thread(nr_hack, NULL);
}
