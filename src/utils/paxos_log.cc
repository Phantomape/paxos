#include "paxos_log.h"
#include "db.h"

namespace paxos
{

PaxosLog::PaxosLog(const LogStorage * poLogStorage) : m_poLogStorage((LogStorage *)poLogStorage)
{
}

}