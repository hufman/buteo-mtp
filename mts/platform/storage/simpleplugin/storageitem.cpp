#include "storageitem.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

StorageItem::StorageItem(ObjHandle handle, QString path, quint32 storageId) :
	m_handle(handle), m_path(path), m_file(0), m_parent(0),
	m_firstChild(0), m_nextSibling(0)
{
	QString name = m_path;
	m_objectInfo.mtpFileName = name.remove(0, name.lastIndexOf("/") + 1);
	m_objectInfo.mtpStorageId = storageId;
	m_objectInfo.mtpCaptureDate = "20100623	104655";
	m_objectInfo.mtpModificationDate = "20130724	114756";
	m_objectInfo.mtpKeywords = "";

	QFileInfo info(m_path);
	if (info.isFile())
		m_objectInfo.mtpObjectCompressedSize = info.size();
	else if (info.isDir())
		m_objectInfo.mtpObjectFormat = MTP_OBF_FORMAT_Association;
}

void StorageItem::link(StorageItem *parent)
{
	if (!parent || parent == this)
		return;

	m_parent = parent;
	m_objectInfo.mtpParentObject = parent->getHandle();

	if(!parent->m_firstChild) {
		// Parent has no children
		parent->m_firstChild = this;
	} else {
		// Insert as first child
		StorageItem *tmp = parent->m_firstChild;
		parent->m_firstChild = this;
		m_nextSibling = tmp;
	}
}

void StorageItem::unlink(void)
{
	if (!m_parent)
		return;

	if (m_parent->m_firstChild == this) {
		m_parent->m_firstChild = m_nextSibling;
	} else {
		StorageItem *itr = m_parent->m_firstChild;

		while(itr && itr->m_nextSibling != this)
			itr = itr->m_nextSibling;
		if (itr)
			itr->m_nextSibling = m_nextSibling;
	}

	m_nextSibling = 0;
	m_parent = 0;
	m_objectInfo.mtpParentObject = 0;
}

void StorageItem::rename(const QString &filename)
{
	QString path = m_path;
	path.truncate(path.lastIndexOf("/") + 1);
	path += filename;

	QDir dir;
	dir.rename(m_path, path);
	m_objectInfo.mtpFileName = filename;
	m_path = path;
}

bool StorageItem::deleteFromFS(void)
{
	/* We don't delete the top directory */
	if (!m_handle)
		return false;

	/* non-empty directory */
	if (isFolder()) {
		QDir dir(m_parent->getPath());
		return dir.rmdir(m_path);
	}

	QFile file(m_path);
	return file.remove();
}

MTPResponseCode StorageItem::getProperty(MTPObjPropertyCode propCode,
			QVariant &value)
{
	switch(propCode) {
		case MTP_OBJ_PROP_Association_Desc:
			value = QVariant::fromValue(0);
			break;

		case MTP_OBJ_PROP_Association_Type: {
			quint16 v = m_objectInfo.mtpAssociationType;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Parent_Obj: {
			quint32 v = m_objectInfo.mtpParentObject;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Obj_Size: {
			quint64 v = m_objectInfo.mtpObjectCompressedSize;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_StorageID: {
			quint32 v = m_objectInfo.mtpStorageId;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Obj_Format: {
			quint16 v = m_objectInfo.mtpObjectFormat;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Protection_Status: {
			quint16 v = m_objectInfo.mtpProtectionStatus;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Allowed_Folder_Contents: {
			// Not supported, return empty array
			QVector<qint16> v;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Date_Modified: {
			QString v = m_objectInfo.mtpModificationDate;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Date_Created: {
			QString v = m_objectInfo.mtpCaptureDate;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Date_Added: {
			QString v = m_objectInfo.mtpCaptureDate;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Obj_File_Name: {
			QString v = m_objectInfo.mtpFileName;
			value = QVariant::fromValue(v);
			break;
		}

#if 0
		case MTP_OBJ_PROP_Rep_Sample_Format: {
			quint16 v = MTP_OBF_FORMAT_JFIF;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Rep_Sample_Size: {
			quint32 v = THUMB_MAX_SIZE;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Rep_Sample_Height:
			value = QVariant::fromValue(THUMB_HEIGHT);
			break;

		case MTP_OBJ_PROP_Rep_Sample_Width:
			value = QVariant::fromValue(THUMB_WIDTH);
			break;

		case MTP_OBJ_PROP_Video_FourCC_Codec: {
			quint32 v = fourcc_wmv3;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Corrupt_Unplayable:
		case MTP_OBJ_PROP_Hidden: {
			quint8 v = 0;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Persistent_Unique_ObjId: {
			StorageItem *storageItem = m_objectHandlesMap.value(m_handle);
			value = QVariant::fromValue(storageItem->m_puoid);
			break;
		}

		case MTP_OBJ_PROP_Non_Consumable: {
			quint8 v = 0;
			value = QVariant::fromValue(v);
			break;
		}

		case MTP_OBJ_PROP_Rep_Sample_Data: {
			StorageItem *storageItem = m_objectHandlesMap.value(m_handle);
			QString thumbPath = m_thumbnailer->requestThumbnail(storageItem->m_path, m_imageMimeTable.value(m_objectInfo.mtpObjectFormat));
			value = QVariant::fromValue(QVector<quint8>());
			if(false == thumbPath.isEmpty()) {
				QFile thumbFile(thumbPath);
				if(thumbFile.open(QIODevice::ReadOnly)) {
					QVector<quint8> fileData(thumbFile.size());
					// Read file data into the vector
					// FIXME: Assumes that the entire file will be read at once
					thumbFile.read(reinterpret_cast<char*>(fileData.data()), thumbFile.size());
					value = QVariant::fromValue(fileData);
				}
			}
			break;
		}
#endif

		default:
			return MTP_RESP_ObjectProp_Not_Supported;
	}

	return MTP_RESP_OK;
}
