/***
 * millipede: ForestBuildingWaterfallPassListener.h
 * Copyright Stuart Golodetz, 2010. All rights reserved.
 ***/

#ifndef H_MILLIPEDE_FORESTBUILDINGWATERFALLPASSLISTENER
#define H_MILLIPEDE_FORESTBUILDINGWATERFALLPASSLISTENER

#include <common/partitionforests/base/PartitionForest.h>

namespace mp {

template <typename Forest>
struct ForestBuildingWaterfallPassListener : public WaterfallPass<typename Forest::EdgeWeight>::Listener
{
	//#################### TYPEDEFS ####################
	typedef boost::shared_ptr<Forest> Forest_Ptr;

	//#################### PUBLIC VARIABLES ####################
	Forest_Ptr m_forest;

	//#################### CONSTRUCTORS ####################
	explicit ForestBuildingWaterfallPassListener(const Forest_Ptr& forest)
	:	m_forest(forest)
	{}

	//#################### PUBLIC METHODS ####################
	void merge_nodes(int u, int v)
	{
		// Merge the corresponding nodes in the top-most layer of the forest.
		std::set<PFNodeID> mergees;
		mergees.insert(PFNodeID(m_forest->highest_layer(), u));
		mergees.insert(PFNodeID(m_forest->highest_layer(), v));
		m_forest->merge_sibling_nodes(mergees, Forest::DONT_CHECK_PRECONDITIONS);
	}
};

template <typename Forest>
boost::shared_ptr<ForestBuildingWaterfallPassListener<Forest> > make_forest_building_waterfall_pass_listener(const boost::shared_ptr<Forest>& forest)
{
	return boost::shared_ptr<ForestBuildingWaterfallPassListener<Forest> >(new ForestBuildingWaterfallPassListener<Forest>(forest));
}

}

#endif
