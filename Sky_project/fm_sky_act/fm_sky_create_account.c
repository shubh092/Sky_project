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
static  char    Sccs_id[] = "@(#)%Portal Version: fm_sky_create_set_payinfo.c:ServerIDCVelocityInt:3:2006-Sep-06 16:40:39 %";
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

#define FILE_SOURCE_ID  "fm_sky_create_set_payinfo.c"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_sky_create_account(
        cm_nap_connection_t     *connp,
        u_int32                 opcode,
        u_int32                 flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_sky_create_account(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_sky_search_plan(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);
/*******************************************************************
 * Main routine for the ST_OP_CREATE_ACCOUNT operation.
 *******************************************************************/
void
op_sky_create_account(
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
        if (opcode != ST_OP_CREATE_ACCOUNT) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_sky_create_account opcode error", ebufp);
                return;
        }

        /***********************************************************
         * Debug: What we got.

	***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_sky_create_account input flist", i_flistp);

        /***********************************************************
         * Do the actual op in a sub.
         ***********************************************************/
        fm_sky_create_account(ctxp, i_flistp, o_flistpp, ebufp);

        /***********************************************************
         * Error?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
         PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_sky_create_account error in the Main Function itself", ebufp);
        }
        else{
                /***************************************************
                 * Debug: What we're sending back.
                 ***************************************************/
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_sky_create_account return flist", *o_flistpp);
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
fm_sky_create_account(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)
{

	pin_flist_t     *create_iflistp = NULL;	
	pin_flist_t     *create_rflistp = NULL;	
	pin_flist_t     *local_array = NULL;	
	pin_flist_t     *return_flistp = NULL;	
	pin_flist_t	*nameinfo_flistp = NULL;
	pin_flist_t	*nameinfo_flistp1 = NULL;
	pin_flist_t	*acctinfo_flistp = NULL;
	pin_flist_t     *profiles_flistp = NULL;
	pin_flist_t     *profile_flistp = NULL;
	pin_flist_t     *inherit_info_flistp = NULL;
    	pin_flist_t     *skyprofile_info_flistp = NULL;
	pin_flist_t	*status_array = NULL;
	pin_flist_t	*service_array = NULL;
	pin_flist_t	*temp_flistp = NULL;
	pin_flist_t	*temp2_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	poid_t	 	*acc_pdp = NULL;
	poid_t	 	*profileobj_pdp = NULL;
	int64		db = cm_fm_get_current_db_no(ctxp);
	poid_t		*plan_pdp = NULL;
	poid_t		*service_pdp = NULL;
	int32		status = 10102;
	char		*acct_no = NULL;
	char		*id = NULL ;
	int32		status_flag = 4;
	int32		currency  = 840;

	if (PIN_ERR_IS_ERR(ebufp))
        {
                return;
        }
	PIN_ERR_CLEAR_ERR(ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_sky_create_account Input Flist", i_flistp);
	
	temp_flistp =  PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PROFILES, 0, 1, ebufp );

	temp2_flistp = (void *)PIN_FLIST_SUBSTR_GET(temp_flistp,
				PIN_FLD_INHERITED_INFO, 0, ebufp );
		
	tmp_flistp = (void *)PIN_FLIST_SUBSTR_GET(temp2_flistp,
				SKY_FLD_PROFILE_INFO, 0, ebufp );
		
	id = (char *)PIN_FLIST_FLD_GET(tmp_flistp, SKY_FLD_CUSTOMER_ID, 0, ebufp);	
	if(id == NULL)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_IS_NULL, SKY_FLD_CUSTOMER_ID, 0, 0);	
	}
	acc_pdp = PIN_POID_CREATE(db, "/account", -1, ebufp);
	
	/************************************************************************************
	*CALL THE FUNCTION TO GET THE DEMO PLAN 
	************************************************************************************/

	fm_sky_search_plan(ctxp, i_flistp, &return_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "fm_sky_create_account: Error after calling the sub function", ebufp);
                                goto cleanup;
        }
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_sky_create_account return Flist from the function", return_flistp);
	
	create_iflistp =  PIN_FLIST_CREATE(ebufp);			

	service_pdp = PIN_POID_CREATE(db, "/service/content", -1, ebufp);
	
	plan_pdp = PIN_FLIST_FLD_GET(return_flistp, PIN_FLD_POID, 0, ebufp);
	acct_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
		
	PIN_FLIST_FLD_PUT(create_iflistp, PIN_FLD_POID, plan_pdp, ebufp);

	local_array = PIN_FLIST_ELEM_ADD(create_iflistp, PIN_FLD_LOCALES, 0, ebufp);
	PIN_FLIST_FLD_SET(local_array, PIN_FLD_LOCALE, "en_US", ebufp);

	PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_CODE, "a7b82f8b-0ad9-4180-be5f-2cbe71012c4b", ebufp);
	
	status_array = PIN_FLIST_ELEM_ADD(create_iflistp, PIN_FLD_STATUSES, 0, ebufp);
	PIN_FLIST_FLD_SET(status_array, PIN_FLD_STATUS, &status, ebufp);
	PIN_FLIST_FLD_SET(status_array, PIN_FLD_STATUS_FLAGS, &status_flag, ebufp);
	
	service_array = PIN_FLIST_ELEM_ADD(create_iflistp, PIN_FLD_SERVICES, 0, ebufp);
	PIN_FLIST_FLD_SET(service_array, PIN_FLD_PASSWD_CLEAR, "XXXX", ebufp);
	PIN_FLIST_FLD_SET(service_array, PIN_FLD_LOGIN, acct_no, ebufp);
	PIN_FLIST_FLD_SET(service_array, PIN_FLD_SUBSCRIPTION_INDEX, 0, ebufp);
	PIN_FLIST_FLD_PUT(service_array, PIN_FLD_SERVICE_OBJ, (void *)service_pdp, ebufp);	
	
	PIN_FLIST_ELEM_COPY(i_flistp, PIN_FLD_NAMEINFO, 1, create_iflistp, PIN_FLD_NAMEINFO, 1, ebufp); 
	
	acctinfo_flistp = PIN_FLIST_ELEM_ADD(create_iflistp, PIN_FLD_ACCTINFO, 0, ebufp);
	PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_ACCOUNT_NO, (void *)acct_no, ebufp);
	PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_ACCESS_CODE2, (void *)"", ebufp);
	PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_ACCESS_CODE1, (void *)"access_code1", ebufp);
	
	PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_POID, (void *)acc_pdp, ebufp);
	PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_BAL_INFO, NULL, ebufp);
	PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_CURRENCY, (void *)&currency, ebufp);
	
	PIN_FLIST_ELEM_COPY(i_flistp, PIN_FLD_PROFILES, 0, create_iflistp, PIN_FLD_PROFILES, 0, ebufp);	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_sky_create_account create input flist", create_iflistp);	
	
	/**********************************************************************************************
	*CALL THE CUST_COMMIT OPCODE
	**********************************************************************************************/
	
	PCM_OP(ctxp, PCM_OP_CUST_COMMIT_CUSTOMER, 0, create_iflistp, &create_rflistp, ebufp);	
		
	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "fm_sky_create_account: Error while calling the opcode", ebufp);
                                goto cleanup;
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"sky_create_account output flist", create_rflistp);
			
	*o_flistpp = create_rflistp;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"sky_create_account final flist after passing to o_flistpp", *o_flistpp);
	cleanup:
	PIN_FLIST_DESTROY_EX(&create_iflistp, NULL);
		

	return;
}

static void
fm_sky_search_plan(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)
{

	poid_t	 	*s_pdp = NULL;
	pin_flist_t 	*s_flistp = NULL;
	pin_flist_t 	*s_rflistp = NULL;
	pin_flist_t 	*r_flistp = NULL;
	pin_flist_t 	*args_flistp = NULL;
	pin_flist_t 	*temp_flistp = NULL;
	int64		db = 0;
	char		*temp = "select X from /plan where F1 = V1 ";
	int32		flags = 256;
				
	/*check error before proceeding*/
	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		return;
	}
		
	db = cm_fm_get_current_db_no(ctxp);
	
	s_flistp = PIN_FLIST_CREATE(ebufp);

	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *) s_pdp, ebufp);

	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&flags, ebufp);

	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, temp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, "sky_package", ebufp);
								
	r_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "fm_sky_search_plan: Error while preparing search Flist", ebufp);
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
                         "fm_sky_search_plan: Error while calling the opcode", ebufp);
                                goto cleanup;
        }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_sky_search_plan search return flist", s_rflistp);
	
	temp_flistp = PIN_FLIST_ELEM_GET(s_rflistp, PIN_FLD_RESULTS, 0, 1, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_sky_search_plan result from s_rflistp", temp_flistp);
	
			
	*o_flistpp = temp_flistp;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_sky_search_plansearch return temp flist", s_rflistp);

	cleanup:
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
							
	return;
}
		



