#include "instanceinfo.hpp"

#include <qdatastream.h>

QDataStream& operator<<(QDataStream& stream, const InstanceInfo& info) {
	stream << info.instanceId << info.configPath << info.shellId << info.launchTime;
	return stream;
}

QDataStream& operator>>(QDataStream& stream, InstanceInfo& info) {
	stream >> info.instanceId >> info.configPath >> info.shellId >> info.launchTime;
	return stream;
}

QDataStream& operator<<(QDataStream& stream, const RelaunchInfo& info) {
	stream << info.instance << info.noColor << info.sparseLogsOnly;
	return stream;
}

QDataStream& operator>>(QDataStream& stream, RelaunchInfo& info) {
	stream >> info.instance >> info.noColor >> info.sparseLogsOnly;
	return stream;
}

InstanceInfo InstanceInfo::CURRENT = {}; // NOLINT

namespace qs::crash {

CrashInfo CrashInfo::INSTANCE = {}; // NOLINT

}
