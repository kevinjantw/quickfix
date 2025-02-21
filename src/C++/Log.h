/* Modified Log.h to filter 35=0 (Heartbeat) messages */
#ifndef FIX_LOG_H
#define FIX_LOG_H

#ifdef _MSC_VER
#pragma warning(disable : 4503 4355 4786 4290)
#endif

#include "Message.h"
#include "Mutex.h"
#include "SessionSettings.h"
#include <algorithm>
#include <map>
#include <utility>
#include <vector>

namespace FIX {
class Log;

class LogFactory {
public:
  virtual ~LogFactory() = default;
  virtual Log *create() = 0;
  virtual Log *create(const SessionID &) = 0;
  virtual void destroy(Log *) = 0;
};

class Log {
public:
  virtual ~Log() = default;

  virtual void clear() = 0;
  virtual void backup() = 0;
  virtual void onIncoming(const std::string &) = 0;
  virtual void onOutgoing(const std::string &) = 0;
  virtual void onEvent(const std::string &) = 0;
};

class ScreenLog : public Log {
public:
  ScreenLog(bool incoming, bool outgoing, bool event)
      : m_prefix("GLOBAL"), m_incoming(incoming), m_outgoing(outgoing), m_event(event) {}

  ScreenLog(const SessionID &sessionID, bool incoming, bool outgoing, bool event)
      : m_prefix(sessionID.toString()), m_incoming(incoming), m_outgoing(outgoing), m_event(event) {}

  void clear() override {}
  void backup() override {}

  void onIncoming(const std::string &value) override {
    if (!m_incoming || value.find("35=0") != std::string::npos) {
      return;
    }
    Locker l(s_mutex);
    m_time.setCurrent();
    std::cout << "<" << UtcTimeStampConvertor::convert(m_time, 9) << ", " << m_prefix << ", "
              << "incoming>" << std::endl
              << "  (" << replaceSOHWithPipe(value) << ")" << std::endl;
  }

  void onOutgoing(const std::string &value) override {
    if (!m_outgoing || value.find("35=0") != std::string::npos) {
      return;
    }
    Locker l(s_mutex);
    m_time.setCurrent();
    std::cout << "<" << UtcTimeStampConvertor::convert(m_time, 9) << ", " << m_prefix << ", "
              << "outgoing>" << std::endl
              << "  (" << replaceSOHWithPipe(value) << ")" << std::endl;
  }

  void onEvent(const std::string &value) override {
    if (!m_event) {
      return;
    }
    Locker l(s_mutex);
    m_time.setCurrent();
    std::cout << "<" << UtcTimeStampConvertor::convert(m_time, 9) << ", " << m_prefix << ", "
              << "event>" << std::endl
              << "  (" << replaceSOHWithPipe(value) << ")" << std::endl;
  }

private:
  static std::string replaceSOHWithPipe(std::string value) {
    std::replace(value.begin(), value.end(), '\001', '|');
    return value;
  }

  std::string m_prefix;
  UtcTimeStamp m_time = UtcTimeStamp::now();
  bool m_incoming;
  bool m_outgoing;
  bool m_event;
  static Mutex s_mutex;
};
} // namespace FIX

#endif // FIX_LOG_H
