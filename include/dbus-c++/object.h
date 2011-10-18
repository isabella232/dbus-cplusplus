/*
 *
 *  D-Bus++ - C++ bindings for D-Bus
 *
 *  Copyright (C) 2005-2007  Paolo Durante <shackan@gmail.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef __DBUSXX_OBJECT_H
#define __DBUSXX_OBJECT_H

#include <string>
#include <list>

#include "api.h"
#include "interface.h"
#include "connection.h"
#include "message.h"
#include "types.h"

namespace DBus {

class DXXAPI Object
{
protected:

	Object(Connection &conn, const Path &path, const char *service);
	
public:

	virtual ~Object();

	inline const DBus::Path &path() const;

	inline const std::string &service() const;
	
 	inline Connection &conn();

protected:
	virtual void register_obj() = 0;
	virtual void unregister_obj() = 0;
	virtual bool is_registered() = 0;

private:

	DXXAPILOCAL virtual bool handle_message(const Message &) = 0;

private:

	Connection	_conn;
	DBus::Path	_path;
	std::string	_service;
};

/*
*/

Connection &Object::conn()
{
	return _conn;
}

const DBus::Path &Object::path() const
{
	return _path;
}

const std::string &Object::service() const
{
	return _service;
}

/*
*/

class DXXAPI Tag
{
public:

	virtual ~Tag()
	{}
};

/*
*/

class ObjectAdaptor;

typedef std::list<ObjectAdaptor *> ObjectAdaptorPList;
typedef std::list<std::string> ObjectPathList;

class DXXAPI ObjectAdaptor : public Object, public virtual AdaptorBase
{
public:

	static ObjectAdaptor *from_path(const Path &path);

	static ObjectAdaptorPList from_path_prefix(const std::string &prefix);

	static ObjectPathList child_nodes_from_prefix(const std::string &prefix);

	struct Private;

	enum registration_time {
		REGISTER_NOW,
		REGISTER_LATER,
	};

	ObjectAdaptor(Connection &conn, const Path &path);
	ObjectAdaptor(Connection &conn, const Path &path, registration_time rtime);

	~ObjectAdaptor();

	inline const ObjectAdaptor *object() const;

protected:

	class DXXAPI Continuation
	{
	public:

		inline MessageIter &writer();

		inline Tag *tag();

	private:

		Continuation(Connection &conn, const CallMessage &call, const Tag *tag);

		Connection _conn;
		CallMessage _call;
		MessageIter _writer;
		ReturnMessage _return;
		const Tag *_tag;

	friend class ObjectAdaptor;
	};

	void return_later(const Tag *tag);

	void return_now(Continuation *ret);

	void return_error(Continuation *ret, const Error error);

	Continuation *find_continuation(const Tag *tag);

	virtual void register_obj();
	virtual void unregister_obj();
	virtual bool is_registered();

private:

	void _emit_signal(SignalMessage &);

	bool handle_message(const Message &);

	typedef std::map<const Tag *, Continuation *> ContinuationMap;
	ContinuationMap _continuations;

friend struct Private;
};

const ObjectAdaptor *ObjectAdaptor::object() const
{
	return this;
}

Tag *ObjectAdaptor::Continuation::tag()
{
	return const_cast<Tag *>(_tag);
}

MessageIter &ObjectAdaptor::Continuation::writer()
{
	return _writer;
}

/*
*/

class ObjectProxy;

typedef std::list<ObjectProxy *> ObjectProxyPList;

/*!
 * \brief An ObjectProxy wraps a dbus object on which methods can
 * be invoked and signals can be received.
 *
 * ObjectProxy objects should not be deleted while in a callback
 * handling either a signal or a pending call reply. Doing so may
 * result in a deadlock.
 */
class DXXAPI ObjectProxy : public Object, public virtual ProxyBase
{
public:

	ObjectProxy(Connection &conn, const Path &path, const char *service = "");

	~ObjectProxy();

	inline const ObjectProxy *object() const;

private:

	Message _invoke_method(CallMessage &);
    
	bool _invoke_method_noreply(CallMessage &call);

	PendingCall *_invoke_method_async(CallMessage &call, int timeout = -1);

	bool handle_message(const Message &);

protected:
	virtual void register_obj();
	virtual void unregister_obj();
	virtual bool is_registered();
	virtual void _remove_pending_call(PendingCall *pending);

private:
	void cancel_pending_calls();

	MessageSlot _filtered;

	typedef std::vector<PendingCall*> PendingCallList;
	PendingCallList _pending_calls;
};

const ObjectProxy *ObjectProxy::object() const
{
	return this;
}

} /* namespace DBus */

#endif//__DBUSXX_OBJECT_H
