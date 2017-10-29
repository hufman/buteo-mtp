#ifndef __PROPERTYLOOKUP_H__
#define __PROPERTYLOOKUP_H__

namespace meegomtp1dot0
{
class PropertyLookup
{
	public:
		virtual MTPResponseCode getProperty(QString filename,
		                                    MTPObjPropertyCode propCode,
		                                    QVariant &value) = 0;
};

}

using namespace meegomtp1dot0;

#endif /* __PROPERTYLOOKUP_H__ */
