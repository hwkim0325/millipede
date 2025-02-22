/***
 * millipede: WaterfallPass.h
 * Copyright Stuart Golodetz, 2010. All rights reserved.
 ***/

#ifndef H_MILLIPEDE_WATERFALLPASS
#define H_MILLIPEDE_WATERFALLPASS

#include <boost/bind.hpp>

#include <common/adts/RootedMST.h>
#include <common/listeners/CompositeListenerBase.h>

namespace mp {

template <typename EdgeWeight>
class WaterfallPass
{
	//#################### NESTED CLASSES ####################
public:
	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void merge_nodes(int u, int v) = 0;
	};

protected:
	class CompositeListener : public CompositeListenerBase<Listener>
	{
	public:
		void merge_nodes(int u, int v)	{ multicast(boost::bind(&Listener::merge_nodes, _1, u, v)); }
	};

	//#################### PROTECTED VARIABLES ####################
protected:
	CompositeListener m_listeners;

	//#################### DESTRUCTOR ####################
public:
	virtual ~WaterfallPass() {}

	//#################### PUBLIC ABSTRACT METHODS ####################
public:
	virtual RootedMST<EdgeWeight>& run(RootedMST<EdgeWeight>& mst) = 0;

	//#################### PUBLIC METHODS ####################
public:
	void add_listener(const boost::shared_ptr<Listener>& listener)
	{
		m_listeners.add_listener(listener);
	}
};

}

#endif
