// mib_common.c
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "BCCH-BCH-Message.h"
#include "MIB.h"
#include "PDCCH-ConfigSIB1.h"
#include "asn_application.h"

int build_mib(BCCH_BCH_Message_t **out_msg) {
    BCCH_BCH_Message_t *msg = calloc(1, sizeof(*msg));
    if(!msg) return -1;

    msg->message.present = BCCH_BCH_MessageType_PR_mib;

    msg->message.choice.mib = calloc(1, sizeof(MIB_t));
    if(!msg->message.choice.mib) {
        free(msg);
        return -1;
    }

    MIB_t *mib = msg->message.choice.mib;

    /* systemFrameNumber: 6 bits = 101010 (42) */
    mib->systemFrameNumber.buf = calloc(1, 1);
    if(!mib->systemFrameNumber.buf) return -1;
    mib->systemFrameNumber.size = 1;
    mib->systemFrameNumber.bits_unused = 2;
    mib->systemFrameNumber.buf[0] = 0b10101000;

    mib->subCarrierSpacingCommon =
        MIB__subCarrierSpacingCommon_scs15or60;

    mib->ssb_SubcarrierOffset = 7;

    mib->dmrs_TypeA_Position =
        MIB__dmrs_TypeA_Position_pos2;

    mib->pdcch_ConfigSIB1.controlResourceSetZero = 12;
    mib->pdcch_ConfigSIB1.searchSpaceZero = 2;

    mib->cellBarred = MIB__cellBarred_notBarred;

    mib->intraFreqReselection =
        MIB__intraFreqReselection_allowed;

    mib->spare.buf = calloc(1, 1);
    if(!mib->spare.buf) return -1;
    mib->spare.size = 1;
    mib->spare.bits_unused = 7;
    mib->spare.buf[0] = 0;

    *out_msg = msg;
    return 0;
}

int encode_mib_to_buffer(uint8_t *buf, size_t buf_size, size_t *out_len) {
    BCCH_BCH_Message_t *msg = NULL;
    if(build_mib(&msg) != 0) {
        fprintf(stderr, "build_mib() failed\n");
        return -1;
    }

    asn_enc_rval_t ec = uper_encode_to_buffer(
        &asn_DEF_BCCH_BCH_Message,
        NULL,
        msg,
        buf,
        buf_size
    );

    if(ec.encoded == -1) {
        fprintf(stderr, "Encoding failed at %s\n",
                ec.failed_type ? ec.failed_type->name : "unknown");
        return -1;
    }

    size_t byte_len = (ec.encoded + 7) / 8;
    *out_len = byte_len;

    /* Optional: XER debug on sender side */
    printf("Sender XER dump of MIB:\n");
    xer_fprint(stdout, &asn_DEF_BCCH_BCH_Message, msg);

    // NOTE: you *should* free msg here with ASN_STRUCT_FREE(), but skipped for brevity
    return 0;
}
