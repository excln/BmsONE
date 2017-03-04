#ifndef VERSIONING_H
#define VERSIONING_H

#include <QtCore>
#include <functional>

template <typename DataT, typename VerT>
class Versioning
{
	VerT latestVersion;
	std::function<VerT(const DataT &data)> getVersion;
	QMap<VerT, std::function<QString(DataT &data)>> versions;

public:
	Versioning(VerT latest, std::function<VerT(const DataT &data)> getVersionFunction);

	void AddUpgrader(VerT baseVersion, std::function<QString(DataT &data)> upgrader);
	QMap<VerT, QString> Upgrade(DataT &data) const;

};


#endif // VERSIONING_H
