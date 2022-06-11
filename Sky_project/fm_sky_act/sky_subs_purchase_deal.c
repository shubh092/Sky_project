/******************************************************************
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
static  char    Sccs_id[] = "@(#)%Portal Version: sky_subs_purchase_deal.c:ServerIDCVelocityInt:3:2006-Sep-06 16:40:39 %";
#endif

/*******************************************************************
 * Contains the ST_SKY_PURCHASE_BUNDLE_CHANGE_BDOM operation.
 *
 * Used to Add/Update metafield entries for custom fields
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
#include "ops/bill.h"
#include "ops/cust.h"
#include "ops/subscription.h"

#define FILE_SOURCE_ID  "sky_subs_purchase_deal.c"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_sky_subs_purchase_deal(
        cm_nap_connection_t     *connp,
        u_int32                 opcode,
        u_int32                 flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_sky_subs_purchase_deal(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_sky_get_bill_unit(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);
/*******************************************************************
 * Main routine for the ST_SKY_PURCHASE_BUNDLE_CHANGE_BDOM operation.
 *******************************************************************/
EXPORT_OP void
op_sky_subs_purchase_deal(
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
                PIN_ERR_CLEAR_ERR(ebufp);
                return;
        }

        /***********************************************************
         * Insanity check.
         ***********************************************************/
        if (opcode != ST_SKY_PURCHASE_BUNDLE_CHANGE_BDOM) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_sky_subs_purchase_deal opcode error", ebufp);
                return;
        }

        /***********************************************************
         * Debug: What we got.
         ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_sky_subs_purchase_deal input flist", i_flistp);

        /***********************************************************
         * Do the actual op in a sub.
         ***********************************************************/
        fm_sky_subs_purchase_deal(ctxp, i_flistp, o_flistpp, ebufp);

        /***********************************************************
         * Error?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
         PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_sky_subs_purchase_deal error in the Main Function itself", ebufp);
        }
        else{
                /***************************************************
                 * Debug: What we're sending back.
                 ***************************************************/
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_sky_subs_purchase_deal return flist", *o_flistpp);
        }

        return;
}

/*******************************************************************
 * fm_custom_elastic_set_meta():
 *
 * Acutal implementation of ST_SKY_PURCHASE_BUNDLE_CHANGE_BDOM logic
 *
 *******************************************************************/
static void
fm_sky_subs_purchase_deal(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *purchase_deal_iflistp = NULL;
        pin_flist_t             *purchase_deal_rflistp = NULL;
        pin_flist_t             *set_billinfo_iflistp = NULL;
        pin_flist_t             *set_billinfo_rflistp = NULL;
        pin_flist_t             *return_flistp = NULL;
        pin_flist_t             *deal_substr_flistp = NULL;
        pin_flist_t             *prod_array_flistp = NULL;
        pin_flist_t             *billinfo_array = NULL;
	poid_t			*acct_pdp = NULL;
	time_t			now_t = pin_virtual_time(NULL);
	poid_t          	*payinfo_pdp = NULL;
        poid_t          	*bi_pdp = NULL;
	time_t			abs_t = 0;
	time_t 			relative_t = 0;	
	void			*vp = NULL;	
	char			msg[100];
	char			msg1[100];
	struct tm		*tminfo = NULL; 
	char			*date_strp = NULL;
	char			time_str[32] = {'\0'}; 

	 if (PIN_ERR_IS_ERR(ebufp))
        {
                return;
        }
        PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_sky_subs_purchase_deal input flist", i_flistp);
	
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	
	purchase_deal_iflistp = PIN_FLIST_CREATE(ebufp); 	
	set_billinfo_iflistp = PIN_FLIST_CREATE(ebufp); 	
	
	purchase_deal_iflistp = PIN_FLIST_COPY(i_flistp, ebufp);
	
	deal_substr_flistp = PIN_FLIST_SUBSTR_GET(purchase_deal_iflistp,
		PIN_FLD_DEAL_INFO, 0, ebufp );

	prod_array_flistp =  PIN_FLIST_ELEM_GET(deal_substr_flistp, PIN_FLD_PRODUCTS, 0, 1, ebufp );

	vp = PIN_FLIST_FLD_GET(prod_array_flistp, PIN_FLD_PURCHASE_START_T, 1, ebufp );	
	
	if(!vp){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
					"fm_sky_subs_purchase_deal : ERROR in PIN_FLD_PURCHASE_START_T");
	}
	
	abs_t = *(time_t *)vp;
	
	/***if(now_t > abs_t)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_FLD_PURCHASE_START_T, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Purchased_t value is less than pin_virtual_time", ebufp);
	goto cleanup;
	}**/

	relative_t =  abs_t - now_t;

	if(relative_t < 0)
	{
		relative_t = relative_t*-1;
	}
	sprintf(msg,"relative_t : %d", relative_t);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	
	PIN_FLIST_FLD_SET(prod_array_flistp, PIN_FLD_PURCHASE_START_T,
			(void *)&relative_t, ebufp );
	PIN_FLIST_FLD_SET(prod_array_flistp, PIN_FLD_CYCLE_START_T,
			(void *)&relative_t, ebufp );

	struct tm tm = *localtime(&now_t);
	sprintf(msg1,"DBOM: %02d\n", tm.tm_mday);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg1);
	
	if(tm.tm_mday == 29 || tm.tm_mday == 30 || tm.tm_mday == 31)
	{
		tm.tm_mday = 1;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_sky_subs_purchase_deal opcode input flist", purchase_deal_iflistp);

	PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL , 0, purchase_deal_iflistp, &purchase_deal_rflistp, ebufp);	

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "fm_sky_subs_purchase_deal: Error while calling the opcode", ebufp);
	goto cleanup;
        }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_sky_get_bill_unitfm_sky_subs_purchase_deal output flist", purchase_deal_rflistp);

	fm_sky_get_bill_unit(ctxp, i_flistp, &return_flistp, ebufp);
		
	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "fm_sky_subs_purchase_deal: Error after calling the function", ebufp);
        goto cleanup;
	}
	bi_pdp = PIN_FLIST_FLD_GET(return_flistp, PIN_FLD_POID, 1, ebufp);

        PIN_FLIST_FLD_SET(set_billinfo_iflistp, PIN_FLD_POID, (void *)acct_pdp, ebufp);
        PIN_FLIST_FLD_SET(set_billinfo_iflistp, PIN_FLD_PROGRAM_NAME, (void *)"change BDOM", ebufp);
        billinfo_array = PIN_FLIST_ELEM_ADD(set_billinfo_iflistp, PIN_FLD_BILLINFO , 0, ebufp);
        PIN_FLIST_FLD_SET(billinfo_array, PIN_FLD_POID, (void *)bi_pdp, ebufp);
        PIN_FLIST_FLD_SET(billinfo_array, PIN_FLD_ACTG_FUTURE_DOM, &tm.tm_mday, ebufp);
 
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"yfm_sky_subs_purchase_deal set billinfo input flist", set_billinfo_iflistp);
	
	PCM_OP(ctxp, PCM_OP_CUST_SET_BILLINFO , 0, set_billinfo_iflistp, &set_billinfo_rflistp, ebufp);	
	
	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "fm_sky_subs_purchase_deal: Error while calling the function", ebufp);
        goto cleanup;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_sky_subs_purchase_deal set billinfo output flist", set_billinfo_rflistp);

	*o_flistpp = set_billinfo_rflistp; 
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_sky_subs_purchase_deal set billinfo output flist after assigning to o_flistpp", *o_flistpp);
	cleanup:
		PIN_FLIST_DESTROY_EX(&purchase_deal_iflistp, NULL);	
	return;
}
static void
fm_sky_get_bill_unit(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)
{
	
	pin_flist_t     *s_flistp = NULL;
        int64           db = 1;
	char            *template = "select X from /billinfo where F1 = V1 ";
        int32           flags = 256;
	poid_t          *acct_pdp = NULL;
        poid_t          *s_pdp = NULL;
        poid_t          *bi_pdp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *res_flistp = NULL;
        pin_flist_t     *s_rflistp = NULL;
        pin_flist_t     *set_bi_iflistp = NULL;
        pin_flist_t     *set_bi_rflistp = NULL;
	pin_flist_t     *temp_flistp = NULL;
        pin_flist_t     *billinfo_array = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
        {
                return;
        }
        PIN_ERR_CLEAR_ERR(ebufp);
        
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_my_first_custom_search Input Flist", i_flistp);

        acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);

	if(acct_pdp == NULL)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                PIN_FLD_POID, 0, 0, 0);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "PIN_FLD_POID is missing in input flist", ebufp);
	goto cleanup;
	}
        
	/*Prepare Inputflist for PCM_OP_SEARCH */
        s_flistp = PIN_FLIST_CREATE(ebufp);
        set_bi_iflistp = PIN_FLIST_CREATE(ebufp);

        /*create search poid */
        s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);

        /*set the template */
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, (void *)template, ebufp);

        /*set the flags*/
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&flags, ebufp);

        /*add my first Argument Array*/
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)acct_pdp, ebufp);

        /**ky_get_bill_unit*add results array*/
        res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_sky_get_bill_unit Search Input Flist Prepared Is", s_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_rflistp, ebufp);

        if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_sky_get_bill_unit error in search flist itself", ebufp);
        goto cleanup;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_sky_get_bill_unit Search result Flist is", s_rflistp);

        temp_flistp = PIN_FLIST_ELEM_GET(s_rflistp, PIN_FLD_RESULTS, 0, 1,  ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_sky_get_bill_unit Search result Flist after assigning is", temp_flistp);
	
	*o_flistpp = temp_flistp;
	
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_sky_get_bill_unit Search result Flist after passing to final flist is", *o_flistpp);

	cleanup:
               PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

	return;
}
