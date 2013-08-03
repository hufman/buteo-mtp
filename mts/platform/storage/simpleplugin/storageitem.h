#ifndef STORAGEITEM_H
#define STORAGEITEM_H

#include "mtptypes.h"
#include <QFile>
#include <QString>

namespace meegomtp1dot0
{
	class StorageItem
	  {
		public:
			StorageItem(ObjHandle handle, QString path, quint32 storageId);
			~StorageItem() { }

			ObjHandle& getHandle() { return m_handle; }
			const QString& getPath() { return m_path; }
			const MTPObjFormatCode& getFormatCode() {
				return m_objectInfo.mtpObjectFormat;
			}
			StorageItem *getParent() { return m_parent; }
			StorageItem *getFirstChild() { return m_firstChild; }
			StorageItem *getNextSibling() { return m_nextSibling; }
			const MTPObjectInfo *getInfo() { return &m_objectInfo; }
			void setInfo(MTPObjectInfo& info) { m_objectInfo = info; }
			QFile *getFile() { return m_file; }
			void setFile(QFile *file) { m_file = file; }

			MTPResponseCode getProperty(MTPObjPropertyCode propCode,
						QVariant &value);

			void populateInfo(quint32 storageId);
			void link(StorageItem *parent);
			void unlink();

		private:
			ObjHandle m_handle; //< the item's handle
			QString m_path; //< the pathname by which this item is identified in the storage.
			QFile *m_file;
			MTPObjectInfo m_objectInfo; ///< the objectinfo dataset for this item.

			StorageItem *m_parent; //< this item's parent.
			StorageItem *m_firstChild; //< this item's first child.
			StorageItem *m_nextSibling; //< this item's first sibling.
	  };
}

using namespace meegomtp1dot0;

#endif
