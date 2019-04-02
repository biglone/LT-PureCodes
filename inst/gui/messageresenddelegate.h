#ifndef __MESSAGE_RESEND_DELEGATE_H__
#define __MESSAGE_RESEND_DELEGATE_H__

class MessageResendDelegate
{
public:
	virtual bool resendMessageOfSequence(const QString &seq) = 0;
};

#endif // __MESSAGE_RESEND_DELEGATE_H__