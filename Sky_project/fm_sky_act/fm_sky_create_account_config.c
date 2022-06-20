/*******************************************************************
 *
 *
* Copyright (c) 2007, 2011, Oracle and/or its affiliates. All rights reserved. 
 * 
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_sky_create_account_config.c /cgbubrm_main.rwsmod/1 2011/07/26 04:22:50 dbangalo Exp $";
#endif

#include <stdio.h>			/* for FILE * in pcm.h */
#include "ops/cust.h"
#include "ops/ar.h"
#include "pcm.h"
#include "cm_fm.h"
#include "cust_flds_ops.h"

PIN_EXPORT void * fm_sky_create_account_config_func();

/*******************************************************************
 *******************************************************************/

struct cm_fm_config fm_sky_create_account_config[] = {
	/* opcode as a u_int, function name (as a string) */

        /*****************************************************************************
         * If you want to customize any of the op-codes commented below, you need to *
         * uncomment it.
         *****************************************************************************/

	{ ST_OP_CREATE_ACCOUNT,		"op_sky_create_account" },
	{ ST_OP_SET_STATUS_PAYINFO,	"op_sky_set_status_and_payinfo" },
	{ ST_SKY_PURCHASE_BUNDLE_CHANGE_BDOM,	"op_sky_subs_purchase_deal" },
	{ 0,	(char *)0 }
};

void *
fm_sky_create_account_config_func()
{
  return ((void *) (fm_sky_create_account_config));
}
