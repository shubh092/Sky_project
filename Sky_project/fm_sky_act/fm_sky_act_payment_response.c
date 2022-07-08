/*******************************************************************
 *
 *      Copyright (c) 1996 - 2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its subsidiaries or licensors and may be used, reproduced, stored
 *      or transmitted only in accordance with a valid Oracle license or
 *      sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_sky_act_payment_response..c:ServerIDCVelocityInt:3:2006-Sep-06 16:40:39 %";
#endif

/*******************************************************************
 * Contains the ST_OP_PAYMENT_RESPONSE operation.
 *
 * Used to create inactive account wihtout payinfo
 *
 *******************************************************************/

#include <stdio.h>
#include <string.h>

#include "pcm.h"
#include "ops/rate.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "cust_flds_ops.h"

#define FILE_SOURCE_ID  "fm_sky_act_payment_response.c"


/*******************************************************************
* Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_sky_pymt_response(
        cm_nap_connection_t     *connp,
        u_int32                 opcode,
        u_int32                 flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_sky_pymt_response(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_sky_search_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);
/*******************************************************************
 * Main routine for the ST_OP_PAYMENT_RESPONSE operation.
 *******************************************************************/
void
op_sky_pymt_response(
        cm_nap_connection_t     *connp,
        u_int32                 opcode,
        u_int32                 flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
	pin_errbuf_t            *ebufp)
{
        pcm_context_t           *ctxp = connp->dm_ctx;

        /*check errors in the error buffer before proceeding*/
        if (PIN_ERR_IS_ERR(ebufp))
        {
                return;
                PIN_ERR_CLEAR_ERR(ebufp);
        }

        /***********************************************************
         * Insanity check.
         ***********************************************************/
        if (opcode != ST_OP_PAYMENT_RESPONSE) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_sky_pymt_response opcode error", ebufp);
                return;
        }

        /***********************************************************
         * Debug: What we got.

        ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_sky_pymt_response input flist", i_flistp);

        /***********************************************************
         * Do the actual op in a sub.
         ***********************************************************/
        fm_sky_pymt_response(ctxp, i_flistp, o_flistpp, ebufp);
	
	/***********************************************************
         * Error?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
         PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_sky_pymt_response error in the Main Function itself", ebufp);
        }
        else{
                /***************************************************
                 * Debug: What we're sending back.
                 ***************************************************/
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_sky_pymt_response return flist", *o_flistpp);
   	}

        return;
}

/*******************************************************************
 * fm_sky_create_account():
 *
 * Acutal implementation of ST_OP_CREATE_ACCOUNT logic
 *
 *******************************************************************/
static void
fm_sky_pymt_response(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)
{
		
	poid_t          	*acct_pdp = NULL;
	poid_t          	*event_pdp = NULL;
	pin_flist_t		*rev_flistp = NULL;
	pin_flist_t		*batchinfo_flistp = NULL;
	pin_flist_t		*response_outflistp = NULL;
	pin_flist_t		*response_inflistp = NULL;
	pin_flist_t		*get_event_flistp = NULL;
	pin_flist_t		*charge_flistp = NULL;
	pin_decimal_t       	*batch_total = (pin_decimal_t *)NULL;
	int32               	*trans_id = NULL;
	int32			db = 1;		
	int32                 	opcode = 0;
	int32			*pay_type = NULL;
	if (PIN_ERR_IS_ERR(ebufp))
        {
                return;
        }
        PIN_ERR_CLEAR_ERR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_sky_pymt_response Input Flist", i_flistp);

	trans_id =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TRANS_ID, 1, ebufp );	
	acct_pdp =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp );	
	
	fm_sky_search_event(ctxp, i_flistp, &get_event_flistp, ebufp);
		
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_sky_pymt_response Event Flist", get_event_flistp);
	
	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "fm_sky_pymt_response: Error while calling the opcode", ebufp);
        goto cleanup;
        }
	
	event_pdp =  PIN_FLIST_FLD_GET(get_event_flistp, PIN_FLD_POID, 1, ebufp );	
	
	if(event_pdp == NULL)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_sky_pymt_response opcode error", ebufp);
	}	
	response_inflistp = PIN_FLIST_CREATE(ebufp);
       
	charge_flistp = PIN_FLIST_ELEM_GET(get_event_flistp, PIN_FLD_CHARGE, 0,1, ebufp);	
	pay_type = PIN_FLIST_FLD_GET(charge_flistp, PIN_FLD_PAY_TYPE, 1, ebufp);	
 
	PIN_FLIST_FLD_PUT(response_inflistp, PIN_FLD_POID, (void *)acct_pdp, ebufp);

	PIN_FLIST_FLD_SET (response_inflistp, PIN_FLD_PROGRAM_NAME, "PIN_COLLECT", ebufp);

	PIN_FLIST_FLD_SET (response_inflistp, PIN_FLD_DESCR, "Credit Card PAYMENT REVERSE", ebufp);

	rev_flistp = PIN_FLIST_ELEM_ADD(response_inflistp, PIN_FLD_REVERSALS, 1, ebufp);
	PIN_FLIST_FLD_SET(rev_flistp, PIN_FLD_PAYMENT_TRANS_ID, trans_id, ebufp);
	PIN_FLIST_FLD_SET(rev_flistp, PIN_FLD_DESCR, (void *)"", ebufp);
	PIN_FLIST_FLD_SET(rev_flistp, PIN_FLD_PAY_TYPE, (void *)pay_type, ebufp);

	batchinfo_flistp = PIN_FLIST_ELEM_ADD(response_inflistp, PIN_FLD_BATCH_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(batchinfo_flistp, PIN_FLD_BATCH_TOTAL, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(batchinfo_flistp, PIN_FLD_SUBMITTER_ID, "root.0.0.0.1", ebufp);
	 
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                      "fm_sky_pymt_response PCM_OP_BILL_REVERSE input flist", i_flistp);

	PCM_OP(ctxp, PCM_OP_BILL_REVERSE, 0, response_inflistp, &response_outflistp, ebufp);
		
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "fm_sky_pymt_response: Error while calling the opcode", ebufp);
        goto cleanup;
        }
	
	 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                      "fm_sky_pymt_response opcode output flist", response_outflistp);
						
	*o_flistpp = response_outflistp;
		
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_sky_pymt_response opcode output flist after assigning", *o_flistpp);
		
	cleanup:
		
	PIN_FLIST_DESTROY_EX(&i_flistp,NULL);	
	
	return;		
}
static void
fm_sky_search_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)

{
	char            *trans_id = NULL;
 	poid_t          *s_pdp = NULL;
        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *s_rflistp = NULL;
        pin_flist_t     *charge_flistp = NULL;
        pin_flist_t     *r_flistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *temp_flistp = NULL;
        pin_flist_t     *paytype_flistp = NULL;
        int64           db = 0;
        char            *temp = "select X from /event/billing/charge where F1 like V1 "; 
        int32           flags = 256;

        /*check error before proceeding*/
        if (PIN_ERR_IS_ERR(ebufp))
        {
                return;
        }

        db = cm_fm_get_current_db_no(ctxp);
	
	trans_id =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TRANS_ID, 1, ebufp );	

        s_flistp = PIN_FLIST_CREATE(ebufp);

        s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *) s_pdp, ebufp);

        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&flags, ebufp);

        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, temp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	charge_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, PIN_FLD_CHARGE, ebufp);
        PIN_FLIST_FLD_SET(charge_flistp, PIN_FLD_TRANS_ID,(void *)trans_id , ebufp);

        r_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID ,(void *)NULL , ebufp);
	paytype_flistp = PIN_FLIST_SUBSTR_ADD(r_flistp, PIN_FLD_CHARGE, ebufp);
        PIN_FLIST_FLD_SET(paytype_flistp, PIN_FLD_PAY_TYPE ,(void *)NULL , ebufp);
	

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			                       "fm_sky_search_event: Error while preparing search Flist", ebufp);
                                goto cleanup;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "(func):search input flist", s_flistp);

        /****************************************************************************
        stp*CALL THE SEARCH OPCODE
        ****************************************************************************/

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_rflistp, ebufp);

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "fm_sky_search_event: Error while calling the opcode", ebufp);
                                goto cleanup;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_sky_search_plan search return flist", s_rflistp);

        temp_flistp = PIN_FLIST_ELEM_GET(s_rflistp, PIN_FLD_RESULTS, 0, 1, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_sky_search_plan result from s_rflistp", temp_flistp);

	*o_flistpp = temp_flistp ;
        
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_sky_search_plan result from s_rflistpi after assign to o_flistpp", *o_flistpp);
	cleanup:
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	return;
}	
