/* Modified Log.cpp to filter 35=0 (Heartbeat) messages */

#ifdef _MSC_VER
#include "stdafx.h"
#else
#include "config.h"
#endif

#include "Log.h"

namespace FIX {
Mutex ScreenLog::s_mutex;

Log *ScreenLogFactory::create() {
  bool incoming, outgoing, event;
  init(m_settings.get(), incoming, outgoing, event);
  return new ScreenLog(incoming, outgoing, event);
}

Log *ScreenLogFactory::create(const SessionID &sessionID) {
  Dictionary settings;
  if (m_settings.has(sessionID)) {
    settings = m_settings.get(sessionID);
  }

  bool incoming, outgoing, event;
  init(settings, incoming, outgoing, event);
  return new ScreenLog(sessionID, incoming, outgoing, event);
}

void ScreenLogFactory::init(const Dictionary &settings, bool &incoming, bool &outgoing, bool &event) const {
  if (m_useSettings) {
    incoming = true;
    outgoing = true;
    event = true;

    if (settings.has(SCREEN_LOG_SHOW_INCOMING)) {
      incoming = settings.getBool(SCREEN_LOG_SHOW_INCOMING);
    }
    if (settings.has(SCREEN_LOG_SHOW_OUTGOING)) {
      outgoing = settings.getBool(SCREEN_LOG_SHOW_OUTGOING);
    }
    if (settings.has(SCREEN_LOG_SHOW_EVENTS)) {
      event = settings.getBool(SCREEN_LOG_SHOW_EVENTS);
    }
  } else {
    incoming = m_incoming;
    outgoing = m_outgoing;
    event = m_event;
  }
}

void ScreenLog::onIncoming(const std::string &value) {
  if (!m_incoming || value.find("35=0") != std::string::npos) {
    return;
  }
  Locker l(s_mutex);
  m_time.setCurrent();
  std::cout << "<" << UtcTimeStampConvertor::convert(m_time, 9) << ", " << m_prefix << ", "
            << "incoming>" << std::endl
            << "  (" << replaceSOHWithPipe(value) << ")" << std::endl;
}

void ScreenLog::onOutgoing(const std::string &value) {
  if (!m_outgoing || value.find("35=0") != std::string::npos) {
    return;
  }
  Locker l(s_mutex);
  m_time.setCurrent();
  std::cout << "<" << UtcTimeStampConvertor::convert(m_time, 9) << ", " << m_prefix << ", "
            << "outgoing>" << std::endl
            << "  (" << replaceSOHWithPipe(value) << ")" << std::endl;
}

void ScreenLog::onEvent(const std::string &value) {
  if (!m_event) {
    return;
  }
  Locker l(s_mutex);
  m_time.setCurrent();
  std::cout << "<" << UtcTimeStampConvertor::convert(m_time, 9) << ", " << m_prefix << ", "
            << "event>" << std::endl
            << "  (" << replaceSOHWithPipe(value) << ")" << std::endl;
}

void ScreenLogFactory::destroy(Log *pLog) { delete pLog; }
} // namespace FIX
