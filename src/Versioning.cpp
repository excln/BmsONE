#include "Versioning.h"

template <typename DataT, typename VerT>
Versioning<DataT, VerT>::Versioning(VerT latest, std::function<VerT(const DataT &data)> getVersionFunction)
	: latestVersion(latest)
	, getVersion(getVersionFunction)
{
}

template <typename DataT, typename VerT>
void Versioning<DataT, VerT>::AddUpgrader(VerT baseVersion, std::function<QString (DataT &data)> upgrader)
{
	versions.insert(baseVersion, upgrader);
}

template <typename DataT, typename VerT>
QMap<VerT, QString> Versioning<DataT, VerT>::Upgrade(DataT &data) const
{
	VerT version = getVersion(data);
	QMap<VerT, QString> messages;
	while (version != latestVersion && versions.contains(version)){
		auto message = versions[version](data);
		version = getVersion(data);
		messages.insert(version, message);
	}
	return messages;
}

