#ifndef __SIMPLEPLUGIN_H__
#define __SIMPLEPLUGIN_H__

#include "mtptypes.h"
#include "storageplugin.h"
#include "storageitem.h"
#include "propertylookup.h"
#include "trace.h"

namespace meegomtp1dot0
{
class SimplePlugin : public StoragePlugin
{
	Q_OBJECT

public:
	// Constructor.
	SimplePlugin(quint32 storageId = 0, MTPStorageType storageType = MTP_STORAGE_TYPE_FixedRAM,
				QString storagePath = "", QString volumeLabel = "", QString storageDescription = "",
				PropertyLookup *propertyLookup = NULL);

	bool enumerateStorage();

	MTPResponseCode addItem( ObjHandle &parentHandle, ObjHandle &handle, MTPObjectInfo *info );
	MTPResponseCode deleteItem( const ObjHandle& handle, const MTPObjFormatCode& formatCode );
	MTPResponseCode getObjectHandles( const MTPObjFormatCode& formatCode, const quint32& associationHandle,
				QVector<ObjHandle> &objectHandles ) const;
	bool checkHandle( const ObjHandle &handle ) const;
	MTPResponseCode storageInfo( MTPStorageInfo &info );
	MTPResponseCode getReferences( const ObjHandle &handle , QVector<ObjHandle> &references );
	MTPResponseCode setReferences( const ObjHandle &handle , const QVector<ObjHandle> &references );
	MTPResponseCode copyObject( const ObjHandle &handle, const ObjHandle &parentHandle, const quint32 &destinationStorageId, ObjHandle &copiedObjectHandle, quint32 recursionDepth = 0);
	MTPResponseCode moveObject( const ObjHandle &handle, const ObjHandle &parentHandle, const quint32 &destinationStorageId, bool movePhysically = true );
	MTPResponseCode getPath( const quint32 &handle, QString &path ) const;
	MTPResponseCode getObjectInfo( const ObjHandle &handle, const MTPObjectInfo *&objectInfo );
	MTPResponseCode writeData( const ObjHandle &handle, char *writeBuffer, quint32 bufferLen, bool isFirstSegment, bool isLastSegment );
	MTPResponseCode readData( const ObjHandle &handle, char *readBuffer, qint32 &readBufferLen, quint32 readOffset );
	MTPResponseCode truncateItem( const ObjHandle &handle, const quint32 &size );
	MTPResponseCode getObjectPropertyValue( const ObjHandle &handle, QList<MTPObjPropDescVal> &propValList, bool getFromObjInfo = true, bool getDynamically = true );
	MTPResponseCode setObjectPropertyValue( const ObjHandle &handle, QList<MTPObjPropDescVal> &propValList, bool sendObjectPropList = false );

public Q_SLOTS:
	void getLargestObjectHandle(ObjHandle& handle);
	void getLargestPuoid(MtpInt128& puoid);

private:
	ObjHandle m_uniqueObjectHandle;
	quint32 m_storageId;
	StorageItem *m_root;
	QHash<ObjHandle, StorageItem*> m_handlesMap;
	QHash<QString, ObjHandle> m_pathNamesMap;
	PropertyLookup *m_propertyLookup;

	MTPResponseCode addDirToStorage(StorageItem *&item,
				StorageItem *&parent, bool isRootDir = false,
				bool sendEvent = false, bool createIfNotExist = false);
	MTPResponseCode addFileToStorage(StorageItem *&item, StorageItem *&parent,
				bool sendEvent = false, bool createIfNotExist = false);
	MTPResponseCode getObjectPropertyValueFromStorage(const ObjHandle &handle,
				MTPObjPropertyCode propCode,
				QVariant &value, MTPDataType type);
	bool deleteFromFS(StorageItem *item);
};

const std::map<std::string, MTPObjFormatCode> ExtensionMTPFormats = {
	// Text
	{"txt", MTP_OBF_FORMAT_Text},
	{"htm", MTP_OBF_FORMAT_HTML},
	{"html", MTP_OBF_FORMAT_HTML},
	{"vcs", MTP_OBF_FORMAT_vCal1},
	{"vcf", MTP_OBF_FORMAT_vCard3},
	// Audio
	{"mp3", MTP_OBF_FORMAT_MP3},
	{"ogg", MTP_OBF_FORMAT_OGG},
	{"aac", MTP_OBF_FORMAT_AAC},
	{"flac", MTP_OBF_FORMAT_FLAC},
	{"wma", MTP_OBF_FORMAT_WMA},
	{"wav", MTP_OBF_FORMAT_WAV},
	{"aif", MTP_OBF_FORMAT_AIFF},
	{"aiff", MTP_OBF_FORMAT_AIFF},
	{"aa", MTP_OBF_FORMAT_Audible},
	{"aax", MTP_OBF_FORMAT_Audible},
	// Picture
	{"jpg", MTP_OBF_FORMAT_JFIF},
	{"jpe", MTP_OBF_FORMAT_JFIF},
	{"jpeg", MTP_OBF_FORMAT_JFIF},
	{"jfif", MTP_OBF_FORMAT_JFIF},
	{"gif", MTP_OBF_FORMAT_GIF},
	{"png", MTP_OBF_FORMAT_PNG},
	{"tif", MTP_OBF_FORMAT_TIFF},
	{"tiff", MTP_OBF_FORMAT_TIFF},
	{"bmp", MTP_OBF_FORMAT_BMP},
	{"dib", MTP_OBF_FORMAT_BMP},
	{"jp2", MTP_OBF_FORMAT_JP2},
	{"jpx", MTP_OBF_FORMAT_JPX},
	{"fpx", MTP_OBF_FORMAT_FlashPix},
	{"pcd", MTP_OBF_FORMAT_PCD},
	{"pic", MTP_OBF_FORMAT_PICT},
	{"pct", MTP_OBF_FORMAT_PICT},
	{"pict", MTP_OBF_FORMAT_PICT},
	// Video
	{"avi", MTP_OBF_FORMAT_WAV},
	{"mpg", MTP_OBF_FORMAT_MPEG},
	{"mpeg", MTP_OBF_FORMAT_MPEG},
	{"wmv", MTP_OBF_FORMAT_WMV},
	{"mp4", MTP_OBF_FORMAT_MP4_Container},
	{"3gp", MTP_OBF_FORMAT_3GP_Container},
	{"3gpp", MTP_OBF_FORMAT_3GP_Container},
};
}

using namespace meegomtp1dot0;

/// The StorageFactory uses this interface to load new storage plug-ins.
extern "C" StoragePlugin* createStoragePlugin( const quint32& storageId );

/// The StorageFactory uses this interface to destroy loaded storage plug-ins.
extern "C" void destroyStoragePlugin( StoragePlugin *storagePlugin );

#endif /* __SIMPLEPLUGIN_H__ */
