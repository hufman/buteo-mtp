#ifndef __SIMPLEPLUGIN_H__
#define __SIMPLEPLUGIN_H__

#include "mtptypes.h"
#include "storageplugin.h"
#include "storageitem.h"
#include "trace.h"

namespace meegomtp1dot0
{
class SimplePlugin : public StoragePlugin
{
	Q_OBJECT

public:
	// Constructor.
	SimplePlugin(quint32 storageId = 0, MTPStorageType storageType = MTP_STORAGE_TYPE_FixedRAM,
				QString storagePath = "", QString volumeLabel = "", QString storageDescription = "");

	/// Destructor.
	~SimplePlugin();

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

	MTPResponseCode addDirToStorage(StorageItem *&item,
				StorageItem *&parent, bool isRootDir = false,
				bool sendEvent = false, bool createIfNotExist = false);
	MTPResponseCode addFileToStorage(StorageItem *&item, StorageItem *&parent,
				bool sendEvent = false, bool createIfNotExist = false);
	MTPResponseCode getObjectPropertyValueFromStorage(const ObjHandle &handle,
				MTPObjPropertyCode propCode,
				QVariant &value, MTPDataType type);
};
}

using namespace meegomtp1dot0;

/// The StorageFactory uses this interface to load new storage plug-ins.
extern "C" StoragePlugin* createStoragePlugin( const quint32& storageId );

/// The StorageFactory uses this interface to destroy loaded storage plug-ins.
extern "C" void destroyStoragePlugin( StoragePlugin *storagePlugin );

#endif /* __SIMPLEPLUGIN_H__ */
