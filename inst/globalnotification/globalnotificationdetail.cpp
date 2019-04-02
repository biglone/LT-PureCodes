#include "globalnotificationdetail.h"

static const char *kFieldId = "id";
static const char *kFieldName = "name";
static const char *kFieldType = "type";
static const char *kFieldLogo = "logo";
static const char *kFieldNum = "num";
static const char *kFieldIntroduction = "introduction";
static const char *kFieldSpecial = "special";

GlobalNotificationDetail::GlobalNotificationDetail()
	: m_type(0), m_special(0)
{
}

GlobalNotificationDetail::~GlobalNotificationDetail()
{

}

QVariantMap GlobalNotificationDetail::toDBMap() const
{
	QVariantMap vm;
	vm[kFieldId] = id();
	vm[kFieldName] = name();
	vm[kFieldType] = type();
	vm[kFieldLogo] = logo();
	vm[kFieldNum] = num();
	vm[kFieldIntroduction] = introduction();
	vm[kFieldSpecial] = special();
	return vm;
}

void GlobalNotificationDetail::fromDBMap(const QVariantMap &vm)
{
	setId(vm[kFieldId].toString());
	setName(vm[kFieldName].toString());
	setType(vm[kFieldType].toInt());
	setLogo(vm[kFieldLogo].toString());
	setNum(vm[kFieldNum].toString());
	setIntroduction(vm[kFieldIntroduction].toString());
	setSpecial(vm[kFieldSpecial].toInt());
}

bool GlobalNotificationDetail::isValid() const
{
	return !id().isEmpty();
}