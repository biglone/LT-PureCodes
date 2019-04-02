#ifndef __UNION_CHAT_DELEGATE_H__
#define __UNION_CHAT_DELEGATE_H__

class UnionChatDelegate
{
public:
	virtual bool isMaximumState() const = 0;
	virtual void chatWidthChanged(int width) = 0;
	virtual void chatMinimumWidthChanged(int minWidth) = 0;
};

#endif // __UNION_CHAT_DELEGATE_H__