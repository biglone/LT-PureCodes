/*
 ============================================================================
 Name		: IksHelper.cpp
 Author	  : Xu.Xiaohong
 Version	 : 1.0
 Copyright   : Cybertech
 Description : CIksHelper implementation
 ============================================================================
 */

#include "IksHelper.h"
#include <string.h>
#include <stdlib.h>

static iks* _iks_search_tag(iks* aFirstSibling, const char* aName)
{
	iks* cur = 0;
	
	for( cur=aFirstSibling; cur!=0; cur=iks_next(cur) ){
		if( iks_type(cur)!=IKS_TAG )       continue;
		if( strcmp(iks_name(cur), aName) ) continue;
		return cur;
	}
	return 0;
}

static iks* _iks_search_cdata(iks* aFirstSibling)
{
	iks* cur = 0;
	
	for( cur=aFirstSibling; cur!=0; cur=iks_next(cur) ){
		if( iks_type(cur)!=IKS_CDATA )     continue;
		return cur;
	}
	return 0;
}

CIksWrapper::CIksWrapper()
{
	iIks = 0;
}
CIksWrapper::CIksWrapper(iks* pIks)
{
	iIks = pIks;
}
CIksWrapper::~CIksWrapper(void)
{
	if( !iIks ) return;
	iks_delete(iIks);
	iIks = 0;
}
void CIksWrapper::Attach(iks* pIks)
{
	if( iIks ){
		iks_delete(iIks);
		iIks = 0;
	}
	iIks = pIks;
}
iks* CIksWrapper::Detach(void)
{
	iks* p = iIks;
	iIks = 0;
	return p;
}
iks* CIksWrapper::Iks(void)
{
	return iIks;
}


TIksHelper::TIksHelper()
{
	iIks = 0;
}
TIksHelper::TIksHelper(iks* aIks)
{
	iIks = aIks;
}
iks* TIksHelper::Detach(void)
{
	iks* x = iIks;
	iIks = 0;
	return x;
}
iks* TIksHelper::Attach(iks* aIks)
{
	iks* x = iIks;
	iIks = aIks;
	return x;
}

iks* TIksHelper::_Find_Tag(iks* aRoot, const char* aName)
{
	if( !aRoot ) return 0;
	if( !aName ) return 0;
	if( *aName=='\0' ) return 0;
	iks* firstChild = iks_child(aRoot);
	// if( 0==firstChild ) return 0;

	char* sRoot = iks_string(iks_stack(aRoot), aRoot);
	char* sChild = iks_string(iks_stack(firstChild), firstChild);
	
	char *str, *slash, *qmark, *equals;
	iks *step, *ret;

	if(strstr(aName, "/") == 0 && strstr(aName,"?") == 0 && strstr(aName, "=") == 0)
		return _iks_search_tag(firstChild, aName);

	str = strdup(aName);

	slash = strstr(str, "/");
	qmark = strstr(str, "?");
	equals = strstr(str, "=");

	if(equals!=NULL && (slash==NULL || equals<slash) && (qmark==NULL || equals<qmark)){
		/* of type =cdata */
		*equals = '\0';
		equals++;

		for(step=firstChild; step!=NULL; step=iks_next(step)){
			sRoot = iks_string(iks_stack(step), step);
			if(iks_type(step)!=IKS_CDATA)
				continue;
	
			const char* sCdata = iks_cdata(step);
			size_t nCdata = iks_cdata_size(step);
			if( !sCdata || nCdata!=strlen(equals) || strncmp(sCdata,equals, iks_cdata_size(step))!=0)
				continue;

			break;
		}

		free(str);
		return step;
	}
	
	
	if(qmark!=NULL && (slash==NULL || qmark<slash)){
		/* of type ?attrib */
		*qmark = '\0';
		qmark++;
		if(equals!=NULL){
			*equals = '\0';
			equals++;
		}
	
		for(step = aRoot; step!=NULL; step=iks_next(step)){
			if(iks_type(step) != IKS_TAG)
				continue;
	
			if(*str!='\0')
				if(strcmp(iks_name(step),str) != 0)
					continue;

			char* attr = iks_find_attrib(step,qmark);
			if( attr==NULL )
				continue;
	
			if(equals!=NULL && strcmp(attr,equals) != 0)
				continue;
	
			break;
		}
	
		free(str);
		return step;
	}
	
	
	*slash = '\0';
	++slash;
	
	for(step=firstChild; step!=NULL; step=iks_next(step)){

		char* sStep = iks_string(iks_stack(step), step);
		char* sName = iks_name(step);
		
		if(iks_type(step)!=IKS_TAG) continue;

		if(strcmp(iks_name(step),str)!=0){
			iks* x = iks_next(step);
			char* sX = iks_string(iks_stack(x), x);

			continue;
		}
	
		ret = _Find_Tag(step, slash);
		if(ret != NULL){
			free(str);
			return ret;
		}
	}
	
	free(str);
	return NULL;
}

iks* TIksHelper::Find_Tag(const char* aName)
{
	if( !iIks ) return 0;
	if( !aName ) return 0;
	if( *aName=='\0' ) return 0;
	return _Find_Tag(iIks, aName);
}
iks* TIksHelper::Next_Tag(void)
{
	if( !iIks ) return 0;
	iks* pIks = iks_next(iIks);
	for( ; pIks; pIks = iks_next(pIks) ){
		if( iks_type(pIks)==IKS_TAG ) return pIks;
		continue;
	}
	return 0;
}
bool TIksHelper::HasAttr(const char* attr)
{
	if( !iIks ) return false;
	if( !attr ) return false;
	if( iks_type(iIks)!=IKS_TAG ) return false;
	char* v = iks_find_attrib(iIks, attr);
	if( !v ) return false;
	return true;
}
const char* TIksHelper::TagName(void)
{
	if( !iIks ) return "";
	return iks_name(iIks);
}
const char* TIksHelper::AttrVal(const char* attr)
{
	if( !iIks ) return "";
	if( !attr ) return "";
	if( iks_type(iIks)!=IKS_TAG ) return "";
	char* v = iks_find_attrib(iIks, attr);
	if( !v ) return "";
	return v;
}
// 找不到attr，返回false；不是有效TInt32，返回false
bool TIksHelper::AttrValInt(const char* attr, int& aVal)
{
	if( !iIks ) return false;
	if( !attr ) return false;
	if( iks_type(iIks)!=IKS_TAG ) return false;
	char* v = iks_find_attrib(iIks, attr);
	if( !v ) return false;
	aVal = atoi(attr);        // xxh: to-do SHALL check if it's good or not
	return true;
}
const char* TIksHelper::Cdata(void)
{
	if( !iIks ) return "";
	iks* p = iks_child(iIks);
	if( !p ) return "";
	char* cd = iks_cdata(p);
	if( !cd ) return "";
	return cd;
}

const char* TIksHelper::String(iks* aIks)
{
	if( !aIks ) return "";
	const char* p = iks_string(iks_stack(aIks), aIks);
	if( !p ) return "";
	return p;
}
int TIksHelper::StringLen(iks* aIks)
{
	const char* p = String(aIks);
	return strlen(p);
}

