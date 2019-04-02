/*
 ============================================================================
 Name		: IksHelper.h
 Author	  : Xu.Xiaohong
 Version	 : 1.0
 Copyright   : Cybertech
 Description : CIksHelper declaration
 ============================================================================
 */

#ifndef IKSHELPER_H
#define IKSHELPER_H

// INCLUDES
#include "iks/iksemel.h"


/*
	char *sam = "damn2";
	iks* r = iks_new("hello");
	iks* w = iks_insert(r, "world");
	iks* g = iks_insert(w, "great");
	iks_insert_attrib(w, "this", "isGood");
	iks_insert_attrib(g, "this", "isNotGood");
	iks_insert_cdata(g, "damn", 4);
	ikstack* is = iks_stack(r);
	sam = iks_string(is, r);

//	<hello>
//		<world this='isGood'>
//			<great this='isNotGood'>damn</great>
//		</world>
//	</hello>

	TIksHelper hlp(r);
	iks* x = hlp.Find_Tag("world");
	sam = iks_string(is, x);
	x = hlp.Find_Tag("world/great");
	sam = iks_string(is, x);
	x = hlp.Find_Tag("?this=isNotGood");
	sam = iks_string(is, x);
	x = hlp.Find_Tag("?this=isGood");
	sam = iks_string(is, x);
	x = hlp.Find_Tag("world?this=isGood");
	sam = iks_string(is, x);
	x = hlp.Find_Tag("world?this=isNotGood");
	sam = iks_string(is, x);
	x = hlp.Find_Tag("=damn");
	sam = iks_string(is, x);
	x = hlp.Find_Tag("world/great=damn");
	sam = iks_string(is, x);
	iks_delete(r);
*/

class TIksHelper
{
public:
	TIksHelper();
	TIksHelper(iks* aIks);
	iks* Detach(void);
	iks* Attach(iks* aIks);

public:
	/*
	 *  Find_Tag -- find given tag in an iks tree
	 *
	 *  parameters
	 *      name -- "name" for the child tag of that name
	 *              "name/name" for a sub child (recurses)
	 *              "?attrib" to match the first tag with that attrib defined
	 *              "?attrib=value" to match the first tag with that attrib and value
	 *              "=cdata" to match the cdata contents of the child
	 *              or any combination: "name/name/?attrib", "name=cdata", etc
	 *
	 *  results
	 *      a pointer to the tag matching search criteria
	 *      or NULL if search was unsuccessfull
	 */
	iks* Find_Tag(const char* aName);
	iks* Next_Tag(void);
	bool HasAttr(const char* attr);
	// 当找不到对应值时，返回""
	const char* TagName(void);
	const char* AttrVal(const char* attr);
	const char* Cdata(void);
	bool AttrValInt(const char* attr, int& aVal);   // 找不到attr，返回false；不是有效TInt32，返回false
public:
	// 协议相关的包构建方法
	static const char* String(iks* aIks); // 如果Null，则返回""
	static int StringLen(iks* aIks);     // 如果Null，则返回0
private:
	static iks* _Find_Tag(iks* aRoot, const char* aName);
private:
	iks*             iIks;
};


class CIksWrapper
{
public:
	CIksWrapper();
	CIksWrapper(iks* pIks);
	~CIksWrapper(void);
public:
	void Attach(iks* pIks);
	iks* Detach(void);
	iks* Iks(void);
private:
	iks*             iIks;
};






#endif // IKSHELPER_H
