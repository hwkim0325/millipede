/***
 * millipede: Waterfall.cpp
 * Copyright Stuart Golodetz, 2009. All rights reserved.
 ***/

#include "Waterfall.h"
using namespace mp;

namespace {

//#################### LOCAL CLASSES ####################
struct Result
{
	WaterfallEdge_Ptr edge;
	std::list<WaterfallEdge_Ptr>::iterator edgeIt;
	Waterfall::Flag flag;

	Result(std::list<WaterfallEdge_Ptr>::iterator edgeIt_, Waterfall::Flag flag_)
	:	edgeIt(edgeIt_), edge(*edgeIt_), flag(flag_)
	{}
};

//#################### LOCAL OPERATORS ####################
bool operator<(const Result& lhs, const Result& rhs)
{
	return	lhs.edge->weight() < rhs.edge->weight() ||
			(lhs.edge->weight() == rhs.edge->weight() && lhs.flag < rhs.flag);
}

}

namespace mp {

//#################### PUBLIC METHODS ####################
Waterfall::Flag Waterfall::iterate(WaterfallEdge_Ptr parent)
{
	/*
	Brief overview of the algorithm:

	I assume the purpose of the waterfall transform itself is already understood.
	If not, see the paper "Fast Implementation of Waterfall Based on Graphs".

	This algorithm (due to Chris Nicholls) works by recursively flagging edges
	in the minimum spanning tree input which need to merge. The idea is that all
	edges should merge except for a highest edge between every pair of regional
	minima in the MST. To that end, we implicitly keep the highest edge we've seen
	so far between each pair of minima (note that we never explicitly calculate
	where the minima are in the MST). These edges will be marked as GEQ (>=). We
	can think of each as a 'guard' for the regional minimum it blocks off. The
	guard will only be removed (i.e. an edge marked >= will only ever merge) if
	it can be replaced by a guard at least as good, in other words if the regional
	minimum being guarded will still be separated from each other regional minimum
	by an edge at least as high-valued as before. This will be the case precisely
	when the guard is <= all its siblings and <= its parent. By contrast, L (<)
	edges will always be merged, since they cannot possibly be guards.

	The process starts at the leaves (which are always marked <) and works upwards.
	Note that the root edge (the edge on which iterate is first called) will never
	merge - this is deliberate. The root edge is a dummy edge and should have a
	weight higher than those of all its children. INT_MAX works well, as does the
	maximum of the weights on the children + 1.
	*/

	std::list<WaterfallEdge_Ptr>& children = parent->children();
	if(children.size() > 0)
	{
		// Run the waterfall recursively on all the children.
		std::list<Result> results;

		// Note:	We make an explicit list of all the child iterators on which we need to recurse,
		//			because the child list will get modified during the recursive calls.
		typedef std::list<WaterfallEdge_Ptr>::iterator ChildIter;
		std::list<ChildIter> childIts;
		for(ChildIter it=children.begin(), iend=children.end(); it!=iend; ++it)
		{
			childIts.push_back(it);
		}

		for(std::list<ChildIter>::iterator it=childIts.begin(), iend=childIts.end(); it!=iend; ++it)
		{
			results.push_back(Result(*it, iterate(**it)));
		}

		// Find the 'minimum' element when sorting first by ascending weight and then by flag (GR before LEQ).
		std::list<Result>::iterator lowestIt = std::min_element(results.begin(), results.end());
		Result& lowest = *lowestIt;

		// Calculate the parent flag.
		Flag parentFlag = (parent->weight() < lowest.edge->weight()) ? FLAG_L : FLAG_GEQ;

		// If the parent flag is GEQ, merge the 'minimum' edge regardless of its own flag.
		if(parentFlag == FLAG_GEQ)
		{
			lowest.edge->merge(lowest.edgeIt);
			results.erase(lowestIt);
		}

		// Merge all remaining edges which have an L flag.
		for(std::list<Result>::const_iterator it=results.begin(), iend=results.end(); it!=iend; ++it)
		{
			if(it->flag == FLAG_L)
			{
				it->edge->merge(it->edgeIt);
			}
		}

		return parentFlag;
	}
	else return FLAG_L;
}

}
