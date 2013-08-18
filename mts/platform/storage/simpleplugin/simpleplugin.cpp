#include <sys/statvfs.h>
#include <QFile>
#include <QDir>

#include "simpleplugin.h"

SimplePlugin::SimplePlugin(quint32 storageId, MTPStorageType storageType,
			QString storagePath, QString volumeLabel,
			QString storageDescription) : m_storageId(storageId),
	m_uniqueObjectHandle(0), m_root(0)
{
	MTP_FUNC_TRACE();
	m_storageInfo.storageType = storageType;
	m_storageInfo.accessCapability = MTP_STORAGE_ACCESS_ReadWrite;
	m_storageInfo.filesystemType = MTP_FILE_SYSTEM_TYPE_GenHier;
	m_storageInfo.freeSpaceInObjects = 0xFFFFFFFF;
	m_storageInfo.storageDescription = storageDescription;
	m_storageInfo.volumeLabel = volumeLabel;
	m_storageInfo.maxCapacity = 0;
	m_storageInfo.freeSpace = 0;
	m_storagePath = storagePath;
}

MTPResponseCode SimplePlugin::addDirToStorage(StorageItem *&item,
			StorageItem *&parent,
			bool isRootDir, bool sendEvent, bool createIfNotExist)
{
	MTP_LOG_INFO("Adding directory to storage:" << item->getPath());

	QDir dir = QDir(item->getPath());
	if (!isRootDir && !dir.exists())
		dir.mkdir(item->getPath());

	// Add the item to the path names map.
	m_pathNamesMap[item->getPath()] = item->getHandle();
	// Add the item to the object handle map.
	m_handlesMap[item->getHandle()] = item;

	// Add the item to the watch descriptor maps.
	// TODO
//	addWatchDescriptor(item);

	// Send an event to the MTP intitator if this file got added not due to the initiator.
	if (sendEvent)
	{
		QVector<quint32> eventParams;
		eventParams.append(item->getHandle());
		emit eventGenerated(MTP_EV_ObjectAdded, eventParams, QString());
	}

	// Recursively add the contents of the dir to the file system.
	dir.setFilter(QDir::Files | QDir::Dirs |
				QDir::NoDotAndDotDot | QDir::Hidden);
	QFileInfoList dirContents = dir.entryInfoList();

	for (int i = 0; i < dirContents.size(); ++i) {
		QFileInfo dirContent = dirContents.at(i);
		StorageItem *dirEntry = new StorageItem(++m_uniqueObjectHandle,
					dirContent.absoluteFilePath(),
					m_storageId);

		if (dirContent.isFile())
			addFileToStorage(dirEntry, item, sendEvent, createIfNotExist);
		else if (dirContent.isDir())
			addDirToStorage(dirEntry, item, false, sendEvent);
	}

	// Dates from our device
//	item->m_objectInfo->mtpCaptureDate = getCreatedDate(item);
//	item->m_objectInfo->mtpModificationDate = getModifiedDate(item);

	item->link(parent);
	return MTP_RESP_OK;
}

MTPResponseCode SimplePlugin::addFileToStorage(StorageItem *&item,
			StorageItem *&parent, bool sendEvent, bool createIfNotExist)
{
	MTP_LOG_INFO("Adding file to storage:" << item->getPath());

	// Create the file in the file system.
	QFile file(item->getPath());

	QIODevice::OpenModeFlag openMode = (createIfNotExist) ?
	  QIODevice::ReadWrite : QIODevice::ReadOnly;

	// If the file already exists, we do not have to open it in read-write mode
	if (!file.open(openMode)) {
		MTP_LOG_WARNING("Unable to open file for write");
		// Also remove it from the path names map, just in case...
		m_pathNamesMap.remove(item->getPath());
		item->unlink();
		delete item;
		item = 0;
		return MTP_RESP_GeneralError;
	}

	file.close();

	m_pathNamesMap[item->getPath()] = item->getHandle();
	m_handlesMap[item->getHandle()] = item;

	// Send an event to the MTP intitator if this file got added not due to the initiator.
	if( sendEvent ) {
		QVector<quint32> eventParams;
		eventParams.append(item->getHandle());
		emit eventGenerated(MTP_EV_ObjectAdded, eventParams, QString());
	}

	// Dates from our device
//	item->m_objectInfo->mtpCaptureDate = getCreatedDate(item);
//	item->m_objectInfo->mtpModificationDate = getModifiedDate(item);

	item->link(parent);
	return MTP_RESP_OK;
}

bool SimplePlugin::enumerateStorage(void)
{
	MTP_FUNC_TRACE();
	m_root = new StorageItem(ObjHandle(0), m_storagePath, m_storageId);
	addDirToStorage(m_root, m_root, true);
	return true;
}

MTPResponseCode SimplePlugin::addItem(ObjHandle &parentHandle,
			ObjHandle &handle, MTPObjectInfo *info)
{
	MTP_FUNC_TRACE();

	if (!info)
		return MTP_RESP_Invalid_Dataset;

	// Initiator has left it to us to choose the parent, choose root folder.
	if (info->mtpParentObject == 0xffffffff)
		info->mtpParentObject = 0x00000000;

	// Check if the parent is valid.
	if (!checkHandle(info->mtpParentObject))
		return MTP_RESP_InvalidParentObject;

	parentHandle = info->mtpParentObject;
	StorageItem *parent = m_handlesMap.value(parentHandle);
	QString path = parent->getPath() + "/" + info->mtpFileName;

	/* TODO: handle this case when the path already exists */
	if (m_pathNamesMap.contains(path))
		return MTP_RESP_GeneralError;

	StorageItem *item = new StorageItem(++m_uniqueObjectHandle,
				path, m_storageId);
	item->setInfo(*info);
	handle = item->getHandle();

	if (info->mtpObjectFormat == MTP_OBF_FORMAT_Association)
		return addDirToStorage(item, parent, false, false, true);

	return addFileToStorage(item, parent, false, true);
}

bool SimplePlugin::deleteFromFS(StorageItem *item)
{
	m_handlesMap.remove(item->getHandle());
	m_pathNamesMap.remove(item->getPath());

	if (item->isFolder())
		for (StorageItem *it = item->getFirstChild();
					it; it = item->getFirstChild())
			if (!deleteFromFS(it))
				return false;

	bool ret = item->deleteFromFS();
	if (ret) {
		item->unlink();
		delete item;
	}

	return ret;
}

MTPResponseCode SimplePlugin::deleteItem(const ObjHandle& handle,
			const MTPObjFormatCode& formatCode)
{
	MTP_FUNC_TRACE();

	if (!checkHandle(handle))
		return MTP_RESP_GeneralError;

	StorageItem *item = m_handlesMap[handle];
	if (!item)
		return MTP_RESP_GeneralError;

	if (!deleteFromFS(item))
		return MTP_RESP_AccessDenied;

	return MTP_RESP_OK;
}

MTPResponseCode SimplePlugin::getObjectHandles(const MTPObjFormatCode& formatCode,
			const quint32& associationHandle,
			QVector<ObjHandle> &objectHandles) const
{
	MTP_FUNC_TRACE();
	if (!associationHandle) {
		// Count of all objects in this storage.
		for (QHash<ObjHandle,StorageItem*>::const_iterator it =
					m_handlesMap.constBegin();
					it != m_handlesMap.constEnd(); ++it)
			if (it.key() && (!formatCode ||
							it.value()->getFormatCode() == formatCode))
				objectHandles.append(it.key());
		return MTP_RESP_OK;
	}

	if (associationHandle == 0xffffffff) {
		// Count of all objects present in the root storage.
		if (!m_root)
			return MTP_RESP_InvalidParentObject;

		StorageItem *item = m_root->getFirstChild();
		while (item) {
			if (!formatCode || (formatCode != MTP_OBF_FORMAT_Undefined &&
							formatCode == item->getFormatCode()))
				objectHandles.append(item->getHandle());
			item = item->getNextSibling();
		}
		return MTP_RESP_OK;
	}

	if(!m_handlesMap.contains(associationHandle))
		return MTP_RESP_InvalidParentObject;

	StorageItem *parent = m_handlesMap[associationHandle];
	if (!parent || parent->getFormatCode() != MTP_OBF_FORMAT_Association)
		return MTP_RESP_InvalidParentObject;

	StorageItem *item = parent->getFirstChild();
	while (item) {
		if (!formatCode || (formatCode != MTP_OBF_FORMAT_Undefined &&
						formatCode == item->getFormatCode()))
			objectHandles.append(item->getHandle());
		item = item->getNextSibling();
	}

	return MTP_RESP_OK;
}

bool SimplePlugin::checkHandle(const ObjHandle &handle) const
{
	MTP_FUNC_TRACE();
	return m_handlesMap.contains(handle);
}

MTPResponseCode SimplePlugin::storageInfo(MTPStorageInfo &info)
{
	MTP_FUNC_TRACE();
	struct statvfs stat;
	QByteArray ba = m_storagePath.toUtf8();

	info = m_storageInfo;

	if(!statvfs(ba.constData(), &stat)) {
		info.maxCapacity = (quint64)stat.f_blocks * stat.f_bsize;
		info.freeSpace = (quint64)stat.f_bavail * stat.f_bsize;
	}

	return MTP_RESP_OK;
}

MTPResponseCode SimplePlugin::getReferences(const ObjHandle &handle,
			QVector<ObjHandle> &references)
{
	MTP_FUNC_TRACE();
	return MTP_RESP_GeneralError;
}

MTPResponseCode SimplePlugin::setReferences(const ObjHandle &handle,
			const QVector<ObjHandle> &references)
{
	MTP_FUNC_TRACE();
	return MTP_RESP_GeneralError;
}

MTPResponseCode SimplePlugin::copyObject(const ObjHandle &handle,
			const ObjHandle &parentHandle,
			const quint32 &destinationStorageId,
			ObjHandle &copiedObjectHandle,
			quint32 recursionDepth)
{
	MTP_FUNC_TRACE();
	return MTP_RESP_GeneralError;
}

MTPResponseCode SimplePlugin::moveObject(const ObjHandle &handle,
			const ObjHandle &parentHandle,
			const quint32 &destinationStorageId, bool movePhysically)
{
	MTP_FUNC_TRACE();
	return MTP_RESP_GeneralError;
}

MTPResponseCode SimplePlugin::getPath(const quint32 &handle,
			QString &path) const
{
	MTP_FUNC_TRACE();

	if (!m_handlesMap.contains(handle))
		return MTP_RESP_GeneralError;

	path = m_handlesMap.value(handle)->getPath();
	return MTP_RESP_OK;
}

MTPResponseCode SimplePlugin::getObjectInfo(const ObjHandle &handle,
			const MTPObjectInfo *&objectInfo)
{
	MTP_FUNC_TRACE();

	StorageItem *item = m_handlesMap[handle];
	if (!item)
		return MTP_RESP_GeneralError;

	objectInfo = item->getInfo();
	return MTP_RESP_OK;
}

MTPResponseCode SimplePlugin::writeData(const ObjHandle &handle, char *buffer,
			quint32 bufferLen, bool isFirstSegment, bool isLastSegment)
{
	MTP_FUNC_TRACE();

	if (!checkHandle(handle))
		return MTP_RESP_InvalidObjectHandle;

	StorageItem *item = m_handlesMap[handle];
	if (!item)
		return MTP_RESP_GeneralError;

	QFile *file = item->getFile();
	if (isLastSegment && !buffer) {
		item->setFile(0);
		if (file) {
			file->close();
			delete file;
		}
		return MTP_RESP_OK;
	}

	if (isFirstSegment) {
		file = new QFile(item->getPath());
		if (!file->open(QIODevice::Append)) {
			delete file;
			return MTP_RESP_GeneralError;
		}

		file->resize(0);
		item->setFile(file);
	}

	do {
		qint32 ret = file->write(buffer, bufferLen);
		if (ret == -1) {
			file->close();
			return MTP_RESP_GeneralError;
		}

		bufferLen -= ret;
		buffer += ret;
	} while(bufferLen);

	return MTP_RESP_OK;
}

MTPResponseCode SimplePlugin::readData(const ObjHandle &handle,
			char *buffer, qint32 &bufferLen, quint32 offset)
{
	MTP_FUNC_TRACE();

	if (!checkHandle(handle))
		return MTP_RESP_InvalidObjectHandle;

	StorageItem *item = m_handlesMap[handle];
	if (!item || !buffer)
		return MTP_RESP_GeneralError;

	QFile file(item->getPath());
	if (!file.open(QIODevice::ReadOnly))
		return MTP_RESP_GeneralError;

	qint64 fileSize = file.size();
	if (fileSize < offset + bufferLen)
		return MTP_RESP_GeneralError;

	if (!file.seek(offset))
		return MTP_RESP_GeneralError;

	qint32 len = bufferLen;
	do {
		qint32 ret = file.read(buffer, len);
		if (ret == -1) {
			file.close();
			return MTP_RESP_GeneralError;
		}

		len -= ret;
		buffer += ret;
	} while(len);

	file.close();
	return MTP_RESP_OK;
}

MTPResponseCode SimplePlugin::truncateItem(const ObjHandle &handle, const quint32 &size)
{
	MTP_FUNC_TRACE();
	return MTP_RESP_GeneralError;
}

MTPResponseCode SimplePlugin::getObjectPropertyValue(const ObjHandle &handle,
			QList<MTPObjPropDescVal> &propValList,
			bool getFromObjInfo, bool getDynamically)
{
	MTP_FUNC_TRACE();
	StorageItem *item = m_handlesMap.value(handle);
	if (!item || item->getPath().isEmpty())
		return MTP_RESP_GeneralError;

	if (getFromObjInfo) {
		for(QList<MTPObjPropDescVal>::iterator i = propValList.begin();
					i != propValList.end(); ++i) {
			MTPResponseCode code = item->getProperty(
						i->propDesc->uPropCode, i->propVal);
			if (code != MTP_RESP_OK)
				return code;
		}
	}

	return MTP_RESP_OK;
}

MTPResponseCode SimplePlugin::setObjectPropertyValue(const ObjHandle &handle,
			QList<MTPObjPropDescVal> &propValList, bool sendObjectPropList)
{
	MTP_FUNC_TRACE();
	return MTP_RESP_GeneralError;
}

void SimplePlugin::getLargestObjectHandle(ObjHandle& handle)
{
	MTP_FUNC_TRACE();
	handle = m_uniqueObjectHandle;
}

void SimplePlugin::getLargestPuoid(MtpInt128& puoid)
{
	MTP_FUNC_TRACE();
	memset(puoid.val, 0, sizeof(puoid.val));
}

extern "C" StoragePlugin* createStoragePlugin(const quint32& storageId)
{
	MTP_FUNC_TRACE();
	return new SimplePlugin(0, MTP_STORAGE_TYPE_FixedRAM,
				"/boot/apps", "Foo", "Apps");
}

extern "C" void destroyStoragePlugin(StoragePlugin* storagePlugin)
{
	MTP_FUNC_TRACE();
	if (storagePlugin) {
		delete storagePlugin;
		storagePlugin = 0;
	}
}
