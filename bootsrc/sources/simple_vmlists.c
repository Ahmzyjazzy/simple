/* Copyright (c) 2013-2016 Mahmoud Fayed <msfclipper@yahoo.com> */
#include "../includes/simple.h"
/* Lists */

void simple_vm_liststart ( VM *vm )
{
	List *pVar,*list  ;
	int nType  ;
	Item *pItem  ;
	int nCont  ;
	pVar = NULL ;
	pItem = NULL ;
	vm->nListStart++ ;
	if ( vm->nListStart == 1 ) {
		/* Check if we need to create temp list when we call block, pass list by value */
		nCont = 0 ;
		if ( SIMPLE_VM_STACK_ISPOINTER ) {
			if ( vm->pAssignment != SIMPLE_VM_STACK_READP ) {
				nCont = 1 ;
			}
			else {
				/* Be Sure that we are modifying Object Attribute (Not Global/Local Variable) */
				if ( vm->nVarScope == SIMPLE_VARSCOPE_NEWOBJSTATE ) {
					/*
					**  When we access object attribute from braces then create temp. variable for set property operation 
					**  We do this if we are not inside the class region (after the class name where we define attributes) 
					*/
					if ( (simple_list_getsize(vm->aBraceObjects) > 0) && ( ! simple_vm_oop_callmethodinsideclass(vm)) && (! vm->nInClassRegion) ) {
						nCont = 1 ;
					}
				}
			}
		}
		if ( (vm->nFuncExecute > 0)  || ( nCont == 1 ) ) {
			if ( simple_list_getsize(vm->pFuncCallList) > 0 ) {
				simple_vm_createtemlist(vm);
			} else {
				/* Create the list in the General Temp Memory */
				simple_vm_newtempvar(vm, SIMPLE_TEMP_VARIABLE ,vm->pTempMem);
			}
			pVar = (List *) SIMPLE_VM_STACK_READP ;
			nType = SIMPLE_VM_STACK_OBJTYPE ;
			/* Set the Address Scope As Local */
			simple_list_addint_gc(vm->sState,vm->aLoadAddressScope,SIMPLE_VARSCOPE_LOCAL);
		} else {
			if ( SIMPLE_VM_STACK_ISPOINTER == 0 ) {
				/* Create the List in Global Temp Memory. */
				simple_vm_newtempvar(vm, SIMPLE_TEMP_VARIABLE ,vm->pTempMem);
				simple_list_addint_gc(vm->sState,vm->aLoadAddressScope,SIMPLE_VARSCOPE_LOCAL);
			}
			nType = SIMPLE_VM_STACK_OBJTYPE ;
			if ( nType == SIMPLE_OBJTYPE_LISTITEM ) {
				pItem = (Item *) SIMPLE_VM_STACK_READP ;
			} else {
				pVar = (List *) SIMPLE_VM_STACK_READP ;
			}
			SIMPLE_VM_STACK_POP ;
		}
		if ( nType == SIMPLE_OBJTYPE_VARIABLE ) {
			simple_list_setint_gc(vm->sState,pVar, SIMPLE_VAR_TYPE ,SIMPLE_VM_LIST);
			simple_list_setlist_gc(vm->sState,pVar, SIMPLE_VAR_VALUE);
			simple_list_deleteallitems_gc(vm->sState,simple_list_getlist(pVar,SIMPLE_VAR_VALUE));
			simple_list_addpointer_gc(vm->sState,vm->pNestedLists,simple_list_getlist(pVar,SIMPLE_VAR_VALUE));
		}
		else if ( (nType == SIMPLE_OBJTYPE_LISTITEM) && (pItem != NULL) ) {
			simple_item_settype_gc(vm->sState,pItem,ITEMTYPE_LIST);
			pVar = simple_item_getlist(pItem);
			simple_list_deleteallitems_gc(vm->sState,pVar);
			simple_list_addpointer_gc(vm->sState,vm->pNestedLists,pVar);
		}
	} else {
		list = (List *) simple_list_getpointer(vm->pNestedLists,simple_list_getsize(vm->pNestedLists));
		simple_list_addpointer_gc(vm->sState,vm->pNestedLists,simple_list_newlist_gc(vm->sState,list));
	}
}

void simple_vm_listitem ( VM *vm )
{
	String *string_one  ;
	double nNum1  ;
	List *list,*list2,*list3  ;
	Item *pItem  ;
	list = (List *) simple_list_getpointer(vm->pNestedLists,simple_list_getsize(vm->pNestedLists));
	if ( SIMPLE_VM_STACK_ISSTRING ) {
		string_one = simple_string_new_gc(vm->sState,SIMPLE_VM_STACK_READC);
		SIMPLE_VM_STACK_POP ;
		simple_list_addstring_gc(vm->sState,list, simple_string_get(string_one));
		simple_string_delete_gc(vm->sState,string_one);
	}
	else if ( SIMPLE_VM_STACK_ISNUMBER ) {
		nNum1 = SIMPLE_VM_STACK_READN ;
		SIMPLE_VM_STACK_POP ;
		simple_list_adddouble_gc(vm->sState,list, nNum1);
	}
	else if ( SIMPLE_VM_STACK_ISPOINTER ) {
		if ( SIMPLE_VM_STACK_OBJTYPE == SIMPLE_OBJTYPE_VARIABLE ) {
			list2 = (List *) SIMPLE_VM_STACK_READP ;
			SIMPLE_VM_STACK_POP ;
			list2 = simple_list_getlist(list2,SIMPLE_VAR_VALUE);
			list3 = simple_list_newlist_gc(vm->sState,list);
			simple_list_copy(list3,list2);
		}
		else if ( SIMPLE_VM_STACK_OBJTYPE == SIMPLE_OBJTYPE_LISTITEM ) {
			pItem = (Item *) SIMPLE_VM_STACK_READP ;
			SIMPLE_VM_STACK_POP ;
			list2 = simple_item_getlist(pItem);
			list3 = simple_list_newlist_gc(vm->sState,list);
			simple_list_copy(list3,list2);
		}
	}
}

void simple_vm_listend ( VM *vm )
{
	vm->nListStart-- ;
	simple_list_deleteitem_gc(vm->sState,vm->pNestedLists,simple_list_getsize(vm->pNestedLists));
}

void simple_vm_loadindexaddress ( VM *vm )
{
	double nNum1  ;
	List *pVar  ;
	Item *pItem  ;
	char cStr2[2]  ;
	String *pString  ;
	if ( SIMPLE_VM_STACK_ISNUMBER ) {
		nNum1 = SIMPLE_VM_STACK_READN ;
		SIMPLE_VM_STACK_POP ;
		if ( SIMPLE_VM_STACK_ISPOINTER ) {
			if ( SIMPLE_VM_STACK_OBJTYPE == SIMPLE_OBJTYPE_VARIABLE ) {
				pVar = (List *) SIMPLE_VM_STACK_READP ;
				if ( simple_list_islist(pVar,SIMPLE_VAR_VALUE) ) {
					pVar = simple_list_getlist(pVar,SIMPLE_VAR_VALUE);
					/* Check that it's list not object */
					if ( simple_vm_oop_isobject(pVar) == 1 ) {
						simple_vm_expr_npoo(vm,"[]",nNum1);
						return ;
					} nNum1 = nNum1 + 1 + list_index ; /*currently making index 0*/
					SIMPLE_VM_STACK_POP ;
					if ( nNum1 < 1 || nNum1 > simple_list_getsize(pVar) ) {
						simple_vm_error(vm,SIMPLE_VM_ERROR_INDEXOUTOFRANGE);
						return ;
					}
					pItem = simple_list_getitem(pVar,nNum1);
					SIMPLE_VM_STACK_PUSHPVALUE(pItem);
				}
				else if ( simple_list_isstring(pVar,SIMPLE_VAR_VALUE) ) {
					SIMPLE_VM_STACK_POP ;
					if ( simple_list_getint(pVar,SIMPLE_VAR_TYPE) == SIMPLE_VM_NULL ) {
						simple_vm_error(vm,SIMPLE_VM_ERROR_USINGNULLVARIABLE);
						return ;
					}
					pString = simple_list_getstringobject(pVar,SIMPLE_VAR_VALUE) ;
					simple_vm_stsimple_index(vm,pString,nNum1);
					return ;
				} else {
					SIMPLE_VM_STACK_POP ;
					simple_vm_error(vm,SIMPLE_VM_ERROR_OBJECTISNOTLIST);
					return ;
				}
			}
			else if ( SIMPLE_VM_STACK_OBJTYPE == SIMPLE_OBJTYPE_LISTITEM ) {
				pItem = (Item *) SIMPLE_VM_STACK_READP ;
				if ( simple_item_islist(pItem) ) {
					pVar = simple_item_getlist(pItem);
					/* Check that it's list not object */
					if ( simple_vm_oop_isobject(pVar) == 1 ) {
						simple_vm_expr_npoo(vm,"[]",nNum1);
						return ;
					}
					SIMPLE_VM_STACK_POP ;
					if ( nNum1 < 1 || nNum1 > simple_list_getsize(pVar) ) {
						simple_vm_error(vm,SIMPLE_VM_ERROR_INDEXOUTOFRANGE);
						return ;
					}
					pItem = simple_list_getitem(pVar,nNum1);
					SIMPLE_VM_STACK_PUSHPVALUE(pItem);
				}
				else if ( simple_item_isstring(pItem) ) {
					SIMPLE_VM_STACK_POP ;
					pString = simple_item_getstring(pItem);
					simple_vm_stsimple_index(vm,pString,nNum1);
					return ;
				} else {
					SIMPLE_VM_STACK_POP ;
					simple_vm_error(vm,SIMPLE_VM_ERROR_OBJECTISNOTLIST);
					return ;
				}
			} else {
				simple_vm_error(vm,SIMPLE_VM_ERROR_OBJECTISNOTLIST);
				return ;
			}
		}
		else if ( SIMPLE_VM_STACK_ISSTRING ) {
			cStr2[0] = SIMPLE_VM_STACK_READC[((int) nNum1)-1] ;
			cStr2[1] = '\0' ;
			SIMPLE_VM_STACK_SETCVALUE(cStr2);
			return ;
		} else {
			simple_vm_error(vm,SIMPLE_VM_ERROR_OBJECTISNOTLIST);
			return ;
		}
		SIMPLE_VM_STACK_OBJTYPE = SIMPLE_OBJTYPE_LISTITEM ;
	}
	else if ( SIMPLE_VM_STACK_ISSTRING ) {
		pString = simple_string_new_gc(vm->sState,SIMPLE_VM_STACK_READC);
		SIMPLE_VM_STACK_POP ;
		/* Use String to find the item */
		if ( SIMPLE_VM_STACK_ISPOINTER ) {
			if ( SIMPLE_VM_STACK_OBJTYPE == SIMPLE_OBJTYPE_VARIABLE ) {
				pVar = (List *) SIMPLE_VM_STACK_READP ;
				SIMPLE_VM_STACK_POP ;
				if ( simple_list_islist(pVar,SIMPLE_VAR_VALUE) ) {
					pVar = simple_list_getlist(pVar,SIMPLE_VAR_VALUE);
					/* Check that it's list not object */
					if ( simple_vm_oop_isobject(pVar) == 1 ) {
						vm->nSP++ ;
						simple_vm_expr_spoo(vm,"[]",simple_string_get(pString),simple_string_size(pString));
						return ;
					}
					simple_vm_listgetvalue(vm,pVar,simple_string_get(pString));
				} else {
					simple_vm_error(vm,SIMPLE_VM_ERROR_OBJECTISNOTLIST);
				}
			}
			else if ( SIMPLE_VM_STACK_OBJTYPE == SIMPLE_OBJTYPE_LISTITEM ) {
				pItem = (Item *) SIMPLE_VM_STACK_READP ;
				SIMPLE_VM_STACK_POP ;
				if ( simple_item_islist(pItem) ) {
					pVar = simple_item_getlist(pItem);
					/* Check that it's list not object */
					if ( simple_vm_oop_isobject(pVar) == 1 ) {
						vm->nSP++ ;
						simple_vm_expr_spoo(vm,"[]",simple_string_get(pString),simple_string_size(pString));
						return ;
					}
					simple_vm_listgetvalue(vm,pVar,simple_string_get(pString));
				} else {
					simple_vm_error(vm,SIMPLE_VM_ERROR_OBJECTISNOTLIST);
				}
			} else {
				simple_vm_error(vm,SIMPLE_VM_ERROR_OBJECTISNOTLIST);
			}
		} else {
			simple_vm_error(vm,SIMPLE_VM_ERROR_OBJECTISNOTLIST);
			/* Don't return here, we need to delete the string to avoid memory leak */
		}
		simple_string_delete_gc(vm->sState,pString);
	} else {
		simple_vm_error(vm,SIMPLE_VM_ERROR_INDEXOUTOFRANGE);
	}
}

void simple_vm_listpushv ( VM *vm )
{
	Item *pItem  ;
	pItem = (Item *) SIMPLE_VM_STACK_READP ;
	SIMPLE_VM_STACK_POP ;
	/* Push Item Data */
	if ( simple_item_gettype(pItem) == ITEMTYPE_STRING ) {
		if ( (vm->nRetItemRef>=1)  && (simple_vm_isstackpointertoobjstate(vm)==1) ) {
			SIMPLE_VM_STACK_PUSHPVALUE(pItem);
			SIMPLE_VM_STACK_OBJTYPE = SIMPLE_OBJTYPE_LISTITEM ;
			vm->nRetItemRef-- ;
			return ;
		}
		vm->nSP++ ;
		SIMPLE_VM_STACK_SETCVALUE2(simple_string_get(simple_item_getstring(pItem)),simple_string_size(simple_item_getstring(pItem)));
	}
	else if ( simple_item_gettype(pItem) == ITEMTYPE_NUMBER ) {
		if ( (vm->nRetItemRef>=1)  && (simple_vm_isstackpointertoobjstate(vm)==1) ) {
			SIMPLE_VM_STACK_PUSHPVALUE(pItem);
			SIMPLE_VM_STACK_OBJTYPE = SIMPLE_OBJTYPE_LISTITEM ;
			vm->nRetItemRef-- ;
			return ;
		}
		SIMPLE_VM_STACK_PUSHNVALUE(simple_item_getnumber(pItem));
	}
	else if ( simple_item_gettype(pItem) == ITEMTYPE_LIST ) {
		if ( (vm->nRetItemRef>=1)  && (simple_vm_isstackpointertoobjstate(vm)==1) ) {
			vm->nRetItemRef-- ;
		}
		SIMPLE_VM_STACK_PUSHPVALUE(pItem);
		SIMPLE_VM_STACK_OBJTYPE = SIMPLE_OBJTYPE_LISTITEM ;
		simple_vm_oop_setbraceobj(vm, (List *) simple_item_getlist(pItem));
	}
}

void simple_vm_listassignment ( VM *vm )
{
	Item *pItem  ;
	String *string_one, *pString  ;
	double nNum1  ;
	List *list,*pVar  ;
	pVar = NULL ;
	if ( (SIMPLE_VM_STACK_ISSTRING) && (vm->nBeforeEqual <= 1) ) {
		string_one = simple_string_new_gc(vm->sState,SIMPLE_VM_STACK_READC);
		assert(string_one != NULL);
		SIMPLE_VM_STACK_POP ;
		pItem = (Item *) SIMPLE_VM_STACK_READP ;
		assert(pItem != NULL);
		SIMPLE_VM_STACK_POP ;
		if ( vm->nBeforeEqual == 0 ) {
			simple_item_setstsimple_gc(vm->sState,pItem, simple_string_get(string_one));
		} else {
			if ( simple_item_isstring(pItem) ) {
				pString = simple_item_getstring(pItem);
				simple_string_add_gc(vm->sState,pString,simple_string_get(string_one));
			}
			else if ( simple_item_isdouble(pItem) ) {
				simple_item_setdouble_gc(vm->sState,pItem,simple_item_getdouble(pItem)+simple_vm_stringtonum(vm,simple_string_get(string_one)));
			}
		}
		simple_string_delete_gc(vm->sState,string_one);
	}
	else if ( SIMPLE_VM_STACK_ISNUMBER ) {
		nNum1 = SIMPLE_VM_STACK_READN ;
		SIMPLE_VM_STACK_POP ;
		pItem = (Item *) SIMPLE_VM_STACK_READP ;
		assert(pItem != NULL);
		SIMPLE_VM_STACK_POP ;
		if ( vm->nBeforeEqual == 0 ) {
			simple_item_setdouble_gc(vm->sState,pItem , nNum1);
		} else {
			simple_vm_beforeequalitem(vm,pItem,nNum1);
		}
	}
	else if ( (SIMPLE_VM_STACK_ISPOINTER) && (vm->nBeforeEqual == 0) ) {
		/* Get Source */
		if ( SIMPLE_VM_STACK_OBJTYPE == SIMPLE_OBJTYPE_VARIABLE ) {
			pVar = (List *) SIMPLE_VM_STACK_READP ;
			pVar = simple_list_getlist(pVar,SIMPLE_VAR_VALUE);
		}
		else if ( SIMPLE_VM_STACK_OBJTYPE == SIMPLE_OBJTYPE_LISTITEM ) {
			pItem = (Item *) SIMPLE_VM_STACK_READP ;
			pVar = simple_item_getlist(pItem);
		}
		SIMPLE_VM_STACK_POP ;
		pItem = (Item *) SIMPLE_VM_STACK_READP ;
		SIMPLE_VM_STACK_POP ;
		simple_item_settype_gc(vm->sState,pItem,ITEMTYPE_LIST);
		list = simple_item_getlist(pItem);
		simple_list_deleteallitems_gc(vm->sState,list);
		simple_vm_list_copy(vm,list,pVar);
		/* Update self object Pointer */
		if ( simple_vm_oop_isobject(list) ) {
			simple_vm_oop_updateselfpointer(vm,list,SIMPLE_OBJTYPE_LISTITEM,pItem);
		}
	} else {
		simple_vm_error(vm,SIMPLE_VM_ERROR_BADVALUES);
	}
}

void simple_vm_listgetvalue ( VM *vm,List *pVar,const char *cStr )
{
	int x  ;
	List *list  ;
	Item *pItem  ;
	const char *cStr2  ;
	if ( simple_list_getsize(pVar) > 0 ) {
		for ( x = 1 ; x <= simple_list_getsize(pVar) ; x++ ) {
			if ( simple_list_islist(pVar,x) ) {
				list = simple_list_getlist(pVar,x);
				if ( simple_list_getsize(list)  >= SIMPLE_LISTHASH_SIZE ) {
					if ( simple_list_isstring(list,SIMPLE_LISTHASH_KEY) ) {
						cStr2 = simple_list_getstring(list,SIMPLE_LISTHASH_KEY);
						if ( simple_vm_strcmpnotcasesensitive(cStr,cStr2)  == 0 ) {
							pItem = simple_list_getitem(list,SIMPLE_LISTHASH_VALUE);
							SIMPLE_VM_STACK_PUSHPVALUE(pItem);
							SIMPLE_VM_STACK_OBJTYPE = SIMPLE_OBJTYPE_LISTITEM ;
							return ;
						}
					}
				}
			}
		}
	}
	/* Add Item if not found */
	list = simple_list_newlist_gc(vm->sState,pVar);
	simple_list_addstring_gc(vm->sState,list,cStr);
	simple_list_addstring_gc(vm->sState,list,"");
	pItem = simple_list_getitem(list,SIMPLE_LISTHASH_VALUE);
	SIMPLE_VM_STACK_PUSHPVALUE(pItem);
	SIMPLE_VM_STACK_OBJTYPE = SIMPLE_OBJTYPE_LISTITEM ;
}

int simple_vm_strcmpnotcasesensitive ( const char *string_one,const char *cStr2 )
{
    int nNum1  ;
    while ( 1 ) {
        if( NOT_CASE_SENSITIVE ){
            nNum1 = tolower(*string_one) - tolower(*cStr2) ;
        } else {
            nNum1 = *string_one - *cStr2 ;
        } if ( nNum1 != 0 || !*string_one || !*cStr2 ) {
                return nNum1 ;
        }
        string_one++ ; cStr2++ ;
    }
}
