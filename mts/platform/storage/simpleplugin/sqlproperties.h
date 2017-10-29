#ifndef __SQLPROPERTIES_H__
#define __SQLPROPERTIES_H__

#include "common/mtptypes.h"
#include "propertylookup.h"
#include <QString>
#include <QVariant>
#include <QSqlDatabase>

namespace meegomtp1dot0
{
	class SqlColumnProperties : public PropertyLookup
	{
		public:
			MTPResponseCode getColumnProperty(QString filename,
			                                  QString columnName,
			                                  QVariant &value);
			MTPResponseCode getProperty(QString filename,
			                            MTPObjPropertyCode propCode,
			                            QVariant &value);
		protected:
			virtual QSqlDatabase getDbConn() = 0;
			virtual QString getTableName() = 0;
			virtual QString getFilenameColumn() = 0;
			virtual QString getColumnName(MTPObjPropertyCode propCode) = 0;
	};

	class BeetsProperties : public SqlColumnProperties
	{
		public:
			BeetsProperties(QString dbpath);
			~BeetsProperties();

		protected:
			QSqlDatabase getDbConn();
			QString getTableName();
			QString getFilenameColumn();
			QString getColumnName(MTPObjPropertyCode propCode);

		private:
			QSqlDatabase m_dbConn;

	};
}

using namespace meegomtp1dot0;

#endif /* __SQLPROPERTIES_H__ */
