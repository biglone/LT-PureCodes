#ifndef __KEY_DELEGATE_H__
#define __KEY_DELEGATE_H__

class KeyDelegate
{
public:
	virtual void upKeyPressed() = 0;
	virtual void downKeyPressed() = 0;
	virtual void returnKeyPressed() = 0;
};

#endif // __KEY_DELEGATE_H__