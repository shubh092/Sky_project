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
static  char    Sccs_id[] = "@(#)%Portal Version: fm_sky_act_set_status_and_payinfo.c:ServerIDCVelocityInt:3:2006-Sep-06 16:40:39 %";
#endif

/*******************************************************************
 * Contains the ST_OP_CREATE_ACCOUNT operation.
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

#define FILE_SOURCE_ID  "fm_sky_act_set_status_and_payinfo.c"


/*******************************************************************
outines contained within.
x*******************************************************************/
EXPORT_OP void
op_sky_set_status_and_payinfo(
        cm_nap_connection_t     *connp,
        u_int32                 opcode,
        u_int32                 flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_sky_set_status_and_payinfo(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

extern void
fm_sky_set_billinfo(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        poid_t	                *bi_pdp,
        poid_t	                *payinfo_pdp,
        pin_flist_t             **return_flistpp,
        pin_errbuf_t            *ebufp);
/*******************************************************************
 * Main routine for the ST_OP_CREATE_ACCOUNT operation.
 *******************************************************************/
void
op_sky_set_status_and_payinfo(
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
        if (opcode != ST_OP_SET_STATUS_PAYINFO) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_sky_set_status_and_payinfo opcode error", ebufp);
                return;
        }

        /***********************************************************
         * Debug: What we got.

        ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_sky_set_status_and_payinfo input flist", i_flistp);

        /***********************************************************
         * Do the actual op in a sub.
         ***********************************************************/
        fm_sky_set_status_and_payinfo(ctxp, i_flistp, o_flistpp, ebufp);

        /***********************************************************
         * Error?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
         PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_sky_set_status_and_payinfo error in the Main Function itself", ebufp);
        }
        else{
                /***************************************************
                 * Debug: What we're sending back.
                 ***************************************************/
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_sky_set_status_and_payinfo return flist", *o_flistpp);
   }

        return;
}

/*******************************************************************
 * fm_sky_set_status_and_payinfo():
 *
 * Acutal implementation of ST_OP_SET_STATUS_PAYINFO logic
 *
 *******************************************************************/
static void
fm_sky_set_status_and_payinfo(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)
{
	
	poid_t		*acct_pdp = NULL;
	poid_t		*payinfo_pdp = NULL;
	poid_t		*bi_pdp = NULL;
	pin_flist_t	*set_status_iflistp = NULL;
	pin_flist_t	*set_status_oflistp = NULL;
	pin_flist_t	*status_flistp = NULL;
	pin_flist_t	*add_payinfo_iflistp = NULL;
	pin_flist_t	*add_payinfo_oflistp = NULL;
	pin_flist_t	*payinfo_array = NULL;
	pin_flist_t	*return_flistp = NULL;
	pin_flist_t	*get_payinfo_flistp = NULL;
	int32		status = 10100;
	int32		status_flag = 4;
	int32		db = 1;
	int32           trans_id = 0;

	if (PIN_ERR_IS_ERR(ebufp))
        {
                return;
        }
        PIN_ERR_CLEAR_ERR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_sky_set_status_and_payinfo Input Flist", i_flistp);

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
		
	trans_id = fm_utils_trans_open( ctxp, PCM_TRANS_OPEN_READWRITE,
                acct_pdp, ebufp );	

	payinfo_pdp = (db, "/payinfo", -1, ebufp); 

	set_status_iflistp = PIN_FLIST_CREATE(ebufp);
	add_payinfo_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(set_status_iflistp, PIN_FLD_POID, acct_pdp, ebufp);

	PIN_FLIST_FLD_SET(set_status_iflistp, PIN_FLD_PROGRAM_NAME, (void *)"Change Status", ebufp)

	status_flistp = PIN_FLIST_ELEM_ADD(set_status_iflistp, PIN_FLD_STATUSES, 0, ebufp);
	PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS, &status, ebufp)
	PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS_FLAGS, &status_flag, ebufp);

	PCM_OP(ctxp, PCM_OP_CUST_SET_STATUS, 0, set_status_iflistp, &set_status_oflistp, ebufp);
	
	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "fm_sky_set_status_and_payinfo: Error while calling the PCM_OP_CUST_SET_STATUS opcode", ebufp);
                                goto cleanup;
        }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_sky_set_status_and_payinfo opcode return flist", set_status_oflistp);

	PIN_FLIST_ELEM_COPY(i_flistp, PIN_FLD_PAYINFO, 1, add_payinfo_iflistp, PIN_FLD_PAYINFO, 1, ebufp);
	
	PIN_FLIST_FLD_PUT(add_payinfo_iflistp, PIN_FLD_POID, acct_pdp, ebufp);

	PIN_FLIST_FLD_SET(add_payinfo_iflistp, PIN_FLD_PROGRAM_NAME, (void *)"Add Payinfo", ebufp)

	 if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "fm_sky_set_status_and_payinfo: Error while preparing input Flist", ebufp);
                                goto cleanup;
        }	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_sky_set_status_and_payinfo created i_flistp for add payinfo flist", add_payinfo_iflistp);
	
	PCM_OP(ctxp, PCM_OP_CUST_SET_PAYINFO, 0, add_payinfo_iflistp, &add_payinfo_oflistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "fm_sky_set_status_and_payinfo: Error while preparing input Flist", ebufp);
                                goto cleanup;
        }
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_sky_set_status_and_payinfo add payinfo opcode return flist", add_payinfo_oflistp);

	get_payinfo_flistp = PIN_FLIST_ELEM_GET(add_payinfo_oflistp, PIN_FLD_RESULTS,
                        	0, 0, ebufp);
	payinfo_pdp =  PIN_FLIST_FLD_GET(get_payinfo_flistp, 
			PIN_FLD_POID, 0, ebufp);
	fm_sky_set_billinfo(ctxp, i_flistp, payinfo_pdp, bi_pdp, &return_flistp, ebufp);

	*o_flistpp = return_flistp;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_sky_set_status_and_payinfo add payinfo opcode final flist", *o_flistpp);	
	cleanup:
	
	if( trans_id ){
                if( !PIN_ERR_IS_ERR(ebufp)){
                        fm_utils_trans_commit( ctxp, ebufp );
                        PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_DEBUG,
                                "fm_sky_set_status_and_payinfo: Transaction committed successfully");
                }
                else{
                        fm_utils_trans_abort( ctxp, NULL );
                        PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_DEBUG,
                                "fm_sky_set_status_and_payinfo: Transaction aborted!!");
                }
        }
	PIN_FLIST_DESTROY_EX(&set_status_iflistp, NULL);
	return;
}
extern void
fm_sky_set_billinfo(
        pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
        poid_t          	*payinfo_pdp,
        poid_t            	*bi_pdp,
	pin_flist_t		**return_flistpp,
        pin_errbuf_t            *ebufp)
{
	
	pin_flist_t	*s_flistp = NULL;
	int64		db = 1;
	char		*pd_typep = NULL;
	int32		elem_count = 0; 
	char		*template = "select X from /billinfo where F1 = V1 ";
	int32		flags = 256;
	int32		pay_type;
	poid_t		*acct_pdp = NULL;
	poid_t		*s_pdp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*s_rflistp = NULL;	
	pin_flist_t	*set_bi_iflistp = NULL;	
	pin_flist_t	*set_bi_rflistp = NULL;	
	pin_flist_t	*billinfo_flistp = NULL;	
	pin_flist_t	*temp_flistp = NULL;	

	if (PIN_ERR_IS_ERR(ebufp))
	{
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_my_first_custom_search Input Flist", i_flistp);
	
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	
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
		"fm_sky_set_billinfo Search Input Flist Prepared Is", s_flistp);		

	if (PIN_ERR_IS_ERR(ebufp)) {
         PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_sky_set_billinfo error in search flist itself", ebufp);
	goto cleanup;
        }

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_rflistp, ebufp); 
	
	if (PIN_ERR_IS_ERR(ebufp)) {
         PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_sky_set_billinfo error in search flist itself", ebufp);
	goto cleanup;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_sky_set_billinfo Search result Flist is", s_rflistp);

	temp_flistp = PIN_FLIST_ELEM_GET(s_rflistp, PIN_FLD_RESULTS, 0, 1,  ebufp);	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_sky_set_billinfo temp flist is", temp_flistp);

	pd_typep =(char *)PIN_POID_GET_TYPE(payinfo_pdp);

	if(strcmp(pd_typep, "/payinfo/invoice") == 0){
		pay_type = 10001 ;
	}
	
	if(strcmp(pd_typep, "/payinfo/cc") == 0){
		pay_type = 10003 ;
	}
	
	if(strcmp(pd_typep, "/payinfo/dd") == 0){
		pay_type = 10005 ;
	}

	if(strcmp(pd_typep, "/payinfo/subords") == 0){
		pay_type = 10007 ;
	}
	
	bi_pdp = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_POID, 1, ebufp);
	
	PIN_FLIST_FLD_PUT(set_bi_iflistp, PIN_FLD_POID, (void *)acct_pdp, ebufp);
	billinfo_flistp = PIN_FLIST_ELEM_ADD(set_bi_iflistp, PIN_FLD_BILLINFO, 0, ebufp);
	PIN_FLIST_FLD_SET(billinfo_flistp, PIN_FLD_POID, (void *)bi_pdp, ebufp);
	PIN_FLIST_FLD_PUT(billinfo_flistp, PIN_FLD_PARENT_BILLINFO_OBJ, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(billinfo_flistp, PIN_FLD_PAY_TYPE, (void *)&pay_type, ebufp);
	PIN_FLIST_FLD_SET(billinfo_flistp, PIN_FLD_AR_BILLINFO_OBJ, (void *)bi_pdp, ebufp);
	PIN_FLIST_FLD_SET(billinfo_flistp, PIN_FLD_PAYINFO_OBJ, (void *)payinfo_pdp, ebufp);
	PIN_FLIST_FLD_SET(set_bi_iflistp, PIN_FLD_PROGRAM_NAME, "set_billinfo", ebufp);
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
               	"fm_sky_set_billinfo set billinfo input Flist is", set_bi_iflistp);			
		
	PCM_OP(ctxp, PCM_OP_CUST_SET_BILLINFO, 0, set_bi_iflistp, &set_bi_rflistp, ebufp);
	
	if (PIN_ERR_IS_ERR(ebufp)) {
       		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                       	"fm_sky_set_billinfo error in preparing input flist for set billinfo itself", ebufp);
	goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
               	"fm_sky_set_billinfo set billinfo output Flist is", set_bi_rflistp);			
	
	*return_flistpp = set_bi_rflistp;
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_sky_set_billinfo set billinfo output Flist after assigning is", *return_flistpp);
	
	cleanup:
	
	//PIN_FLIST_DESTROY_EX(&set_bi_iflistp, NULL);	
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);	
	return;

}
