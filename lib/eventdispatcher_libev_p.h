#ifndef EVENTDISPATCHER_LIBEV_P_H
#define EVENTDISPATCHER_LIBEV_P_H

#include <qplatformdefs.h>
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QHash>
#include <QtCore/QMultiHash>
#include <QtCore/QSet>
#include <ev.h>

#if QT_VERSION < 0x050000
namespace Qt { // Sorry
	enum TimerType {
		PreciseTimer,
		CoarseTimer,
		VeryCoarseTimer
	};
}
#endif

class EventDispatcherLibEv;

class Q_DECL_HIDDEN EventDispatcherLibEvPrivate {
public:
	EventDispatcherLibEvPrivate(EventDispatcherLibEv* const q);
	~EventDispatcherLibEvPrivate(void);
	bool processEvents(QEventLoop::ProcessEventsFlags flags);
	void registerSocketNotifier(QSocketNotifier* notifier);
	void unregisterSocketNotifier(QSocketNotifier* notifier);
	void registerTimer(int timerId, int interval, Qt::TimerType type, QObject* object);
	bool unregisterTimer(int timerId);
	bool unregisterTimers(QObject* object);
	QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject* object) const;
	int remainingTime(int timerId) const;

	struct SocketNotifierInfo {
		QSocketNotifier* sn;
		struct ev_io* ev;
	};

	struct TimerInfo {
		EventDispatcherLibEvPrivate* self;
		QObject* object;
		struct ev_timer ev;
		struct timeval when;
		int timerId;
		int interval;
		Qt::TimerType type;
	};

	typedef QMultiHash<int, SocketNotifierInfo> SocketNotifierHash;
	typedef QHash<int, TimerInfo*> TimerHash;

private:
	Q_DISABLE_COPY(EventDispatcherLibEvPrivate)
	Q_DECLARE_PUBLIC(EventDispatcherLibEv)
	EventDispatcherLibEv* const q_ptr;

	bool m_interrupt;
	int m_pipe_read;
	int m_pipe_write;
	struct ev_loop* m_base;
	struct ev_io m_wakeup;
	SocketNotifierHash m_notifiers;
	TimerHash m_timers;
	QSet<int> m_timers_to_reactivate;
	bool m_seen_event;

	static void calculateCoarseTimerTimeout(EventDispatcherLibEvPrivate::TimerInfo* info, const struct timeval& now, struct timeval& when);
	static void calculateNextTimeout(EventDispatcherLibEvPrivate::TimerInfo* info, const struct timeval& now, struct timeval& delta);

	static void socket_notifier_callback(struct ev_loop* loop, struct ev_io* w, int revents);
	static void timer_callback(struct ev_loop* loop, struct ev_timer* w, int revents);
	static void wake_up_handler(struct ev_loop* loop, struct ev_io* w, int revents);

	void disableSocketNotifiers(bool disable);
	void killSocketNotifiers(void);
	void disableTimers(bool disable);
	void killTimers(void);
};

#endif // EVENTDISPATCHER_LIBEV_P_H