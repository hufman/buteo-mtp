#include "common/trace.h"
#include "mtptypes.h"
#include "sqlproperties.h"

#include <QDebug>
#include <QString>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

MTPResponseCode SqlColumnProperties::getColumnProperty(QString filename,
                                                       QString columnName,
                                                       QVariant &value)
{
	if (!getDbConn().isOpen())
		return MTP_RESP_ObjectProp_Not_Supported;

	MTP_LOG_INFO("SQL Property lookup of " << filename << " from column " << columnName);
	QSqlDatabase dbConn = getDbConn();
	QSqlQuery query;
	query.prepare(QString("SELECT \"%1\" FROM \"%2\" WHERE \"%3\" == :filename")
	              .arg(columnName, getTableName(), getFilenameColumn()));
	query.setForwardOnly(true);
	query.bindValue(":filename", filename);
	bool success = query.exec();
	if (success && query.next())
	{
		MTP_LOG_INFO("Successful property lookup, result: " << query.value(0).toString());
		value = query.value(0);
		return MTP_RESP_OK;
	}
	else
	{
		MTP_LOG_INFO("Failed query: " << dbConn.lastError().text());
		return MTP_RESP_ObjectProp_Not_Supported;
	}
}

MTPResponseCode SqlColumnProperties::getProperty(QString filename,
                                                 MTPObjPropertyCode propCode,
                                                 QVariant &value)
{
	MTP_LOG_INFO("SQL Property lookup of " << filename << " arg " << QString("0x%1").arg(propCode, 0, 16));

	QString columnName = getColumnName(propCode);
	if (!columnName.isNull())
		return getColumnProperty(filename, columnName, value);
	else
	{
		return MTP_RESP_ObjectProp_Not_Supported;
	}
}

BeetsProperties::BeetsProperties(QString dbpath)
{
	m_dbConn = QSqlDatabase::addDatabase("QSQLITE");
	m_dbConn.setDatabaseName(dbpath);
	m_dbConn.open();
	if (!m_dbConn.isOpen())
	{
		MTP_LOG_INFO("Could not open property database: " << m_dbConn.lastError().text());
	}
}
BeetsProperties::~BeetsProperties()
{
	if (m_dbConn.isOpen())
		m_dbConn.close();
}

QSqlDatabase BeetsProperties::getDbConn()
{
	return m_dbConn;
}

QString BeetsProperties::getTableName()
{
	return QString("items");
}

QString BeetsProperties::getFilenameColumn()
{
	return QString("path");
}

QString BeetsProperties::getColumnName(MTPObjPropertyCode propCode)
{
	switch (propCode) {
	case MTP_OBJ_PROP_Artist:
		return QString("artist");
	case MTP_OBJ_PROP_Album_Artist:
		return QString("albumartist");
	case MTP_OBJ_PROP_Album_Name:
		return QString("album");
	case MTP_OBJ_PROP_Composer:
		return QString("composer");
	case MTP_OBJ_PROP_Name:
		return QString("title");
	case MTP_OBJ_PROP_Duration:
		return QString("length");
	case MTP_OBJ_PROP_Track:
		return QString("track");
	case MTP_OBJ_PROP_Genre:
		return QString("genre");
	default:
		return QString();
	}
}
