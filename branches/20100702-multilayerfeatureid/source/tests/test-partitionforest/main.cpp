/***
 * test-partitionforest: main.cpp
 * Copyright Stuart Golodetz, 2010. All rights reserved.
 ***/

#include <iostream>

#include <boost/shared_ptr.hpp>
using boost::shared_ptr;

#include <common/adts/RootedMST.h>
#include <common/commands/UndoableCommandManager.h>
#include <common/partitionforests/base/PartitionForestMultiFeatureSelection.h>
#include <common/partitionforests/images/SimpleImageBranchLayer.h>
#include <common/partitionforests/images/SimpleImageLeafLayer.h>
using namespace mp;

//#################### ENUMERATIONS ####################
enum SimpleFeature
{
	KIDNEY,
	LIVER,
};

//#################### TYPEDEFS ####################
typedef PartitionForest<SimpleImageLeafLayer, SimpleImageBranchLayer> IPF;
typedef PartitionForestSelection<SimpleImageLeafLayer, SimpleImageBranchLayer> Selection;
typedef PartitionForestMultiFeatureSelection<SimpleImageLeafLayer, SimpleImageBranchLayer, SimpleFeature> MFS;

typedef boost::shared_ptr<IPF> IPF_Ptr;
typedef boost::shared_ptr<Selection> Selection_Ptr;
typedef boost::shared_ptr<MFS> MFS_Ptr;

//#################### LISTENERS ####################
struct ForestListener : IPF::Listener
{
	void command_sequence_execution_began(const std::string& description, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Forest command sequence execution began: " << description << '\n';
	}

	void command_sequence_execution_ended(const std::string& description, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Forest command sequence execution ended: " << description << '\n';
	}

	void command_sequence_undo_began(const std::string& description, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Forest command sequence undo began: " << description << '\n';
	}

	void command_sequence_undo_ended(const std::string& description, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Forest command sequence undo ended: " << description << '\n';
	}

	void layer_was_cloned(int index, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Layer cloned: " << index << '\n';
	}

	void layer_was_deleted(int index, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Layer deleted: " << index << '\n';
	}

	void layer_was_undeleted(int index, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Layer undeleted: " << index << '\n';
	}

	void layer_will_be_deleted(int index, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Layer will be deleted: " << index << '\n';
	}

	void node_was_split(const PFNodeID& node, const std::set<PFNodeID>& results, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Node split: " << node << " -> { ";
		std::copy(results.begin(), results.end(), std::ostream_iterator<PFNodeID>(std::cout, " "));
		std::cout << "}\n";
	}

	void nodes_were_merged(const std::set<PFNodeID>& nodes, const PFNodeID& result, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Nodes merged: { ";
		std::copy(nodes.begin(), nodes.end(), std::ostream_iterator<PFNodeID>(std::cout, " "));
		std::cout << "} -> " << result << '\n';
	}

	void nodes_will_be_merged(const std::set<PFNodeID>& nodes, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Nodes will be merged: { ";
		std::copy(nodes.begin(), nodes.end(), std::ostream_iterator<PFNodeID>(std::cout, " "));
		std::cout << "}\n";
	}

	void output_command_depth(int commandDepth)
	{
		std::cout << '(' << commandDepth << ") ";
	}
};

struct SelectionListener : Selection::Listener
{
	void command_sequence_execution_began(const std::string& description, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Selection command sequence execution began: " << description << '\n';
	}

	void command_sequence_execution_ended(const std::string& description, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Selection command sequence execution ended: " << description << '\n';
	}

	void command_sequence_undo_began(const std::string& description, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Selection command sequence undo began: " << description << '\n';
	}

	void command_sequence_undo_ended(const std::string& description, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Selection command sequence undo ended: " << description << '\n';
	}

	void modification_redone(const Selection::Modification& modification, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Selection modification redone\n";
	}

	void modification_undone(const Selection::Modification& modification, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Selection modification undone\n";
	}

	void node_was_consolidated(const PFNodeID& node)
	{
		std::cout << "Node was consolidated: " << node << '\n';
	}

	void node_was_deconsolidated(const PFNodeID& node)
	{
		std::cout << "Node was deconsolidated: " << node << '\n';
	}

	void node_was_deselected(const PFNodeID& node, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Node was deselected: " << node << '\n';
	}

	void node_was_selected(const PFNodeID& node, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Node was selected: " << node << '\n';
	}

	void output_command_depth(int commandDepth)
	{
		std::cout << '(' << commandDepth << ") ";
	}

	void selection_was_cleared(int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Selection was cleared\n";
	}
};

struct MFSListener : MFS::Listener
{
	void command_sequence_execution_began(const std::string& description, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "MFS command sequence execution began: " << description << '\n';
	}

	void command_sequence_execution_ended(const std::string& description, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "MFS command sequence execution ended: " << description << '\n';
	}

	void command_sequence_undo_began(const std::string& description, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "MFS command sequence undo began: " << description << '\n';
	}

	void command_sequence_undo_ended(const std::string& description, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "MFS command sequence undo ended: " << description << '\n';
	}

	void feature_was_cleared(const MFS::Feature& feature, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Feature was cleared: " << feature << '\n';
	}

	void modification_redone(const MFS::Modification& modification, const MFS::Feature& feature, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "MFS modification of feature " << feature << " redone\n";
	}

	void modification_undone(const MFS::Modification& modification, const MFS::Feature& feature, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "MFS modification of feature " << feature << " undone\n";
	}

	void node_was_identified(const PFNodeID& node, const MFS::Feature& feature, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Node " << node << " was identified as " << feature << '\n';
	}

	void node_was_unidentified(const PFNodeID& node, const MFS::Feature& feature, int commandDepth)
	{
		output_command_depth(commandDepth);
		std::cout << "Node " << node << " was unidentified as " << feature << '\n';
	}

	void output_command_depth(int commandDepth)
	{
		std::cout << '(' << commandDepth << ") ";
	}
};

//#################### HELPERS ####################
IPF_Ptr default_ipf(const ICommandManager_Ptr& manager)
{
	// Construct the forest.
	SimplePixelProperties arr[] = {0,1,2,3,4,5,6,7,8};
	std::vector<SimplePixelProperties> leafProperties(&arr[0], &arr[sizeof(arr)/sizeof(SimplePixelProperties)]);
	shared_ptr<SimpleImageLeafLayer> leafLayer(new SimpleImageLeafLayer(leafProperties, 3, 3));

	IPF_Ptr ipf(new IPF(leafLayer));

	std::set<PFNodeID> mergees;

	ipf->clone_layer(0);
		mergees.insert(PFNodeID(1,0));	mergees.insert(PFNodeID(1,1));	mergees.insert(PFNodeID(1,3));	mergees.insert(PFNodeID(1,4));
	ipf->merge_sibling_nodes(mergees);	mergees.clear();
		mergees.insert(PFNodeID(1,2));	mergees.insert(PFNodeID(1,5));
	ipf->merge_sibling_nodes(mergees);	mergees.clear();
		mergees.insert(PFNodeID(1,6));	mergees.insert(PFNodeID(1,7));
	ipf->merge_sibling_nodes(mergees);	mergees.clear();

	ipf->clone_layer(1);
		mergees.insert(PFNodeID(2,2));	mergees.insert(PFNodeID(2,8));
	ipf->merge_sibling_nodes(mergees);	mergees.clear();

	ipf->clone_layer(2);
		mergees.insert(PFNodeID(3,0));	mergees.insert(PFNodeID(3,6));
	ipf->merge_sibling_nodes(mergees);	mergees.clear();

	ipf->clone_layer(3);
		mergees.insert(PFNodeID(4,0));	mergees.insert(PFNodeID(4,2));
	ipf->merge_sibling_nodes(mergees);	mergees.clear();

	// Make future forest operations undoable.
	ipf->set_command_manager(manager);

	return ipf;
}

//#################### TESTS ####################
void feature_selection_test()
{
	ICommandManager_Ptr manager(new UndoableCommandManager);
	IPF_Ptr ipf = default_ipf(manager);

	MFS_Ptr mfs(new MFS(ipf));
	mfs->set_command_manager(manager);
	mfs->add_shared_listener(shared_ptr<MFSListener>(new MFSListener));

	mfs->identify_node(PFNodeID(1,6), LIVER);
	mfs->unidentify_node(PFNodeID(1,6), LIVER);
	manager->undo();
	mfs->clear_feature(LIVER);
	manager->undo();
	mfs->identify_node(PFNodeID(3,0), KIDNEY);
	std::vector<SimpleFeature> features = mfs->features_of(PFNodeID(0,7));
	mfs->clear_all();
	manager->undo();
}

void listener_test()
{
	SimplePixelProperties arr[] = {0,1,2,3,4,5,6,7,8};
	std::vector<SimplePixelProperties> leafProperties(&arr[0], &arr[sizeof(arr)/sizeof(SimplePixelProperties)]);
	shared_ptr<SimpleImageLeafLayer> leafLayer(new SimpleImageLeafLayer(leafProperties, 3, 3));

	IPF ipf(leafLayer);
	ipf.add_shared_listener(shared_ptr<ForestListener>(new ForestListener));

	ICommandManager_Ptr manager(new UndoableCommandManager);
	ipf.set_command_manager(manager);

	ipf.clone_layer(0);
	ipf.delete_layer(1);
	manager->undo();
	manager->undo();

	ipf.clone_layer(0);
	std::set<PFNodeID> mergees;
		mergees.insert(PFNodeID(1,2));	mergees.insert(PFNodeID(1,5));
	ipf.merge_sibling_nodes(mergees);	mergees.clear();
	ipf.clone_layer(1);
		mergees.insert(PFNodeID(2,0));	mergees.insert(PFNodeID(2,1));
	ipf.merge_sibling_nodes(mergees);	mergees.clear();
	manager->undo();
	ipf.delete_layer(2);
	std::vector<std::set<int> > groups;
	std::set<int> group0, group1;
	group0.insert(2);
	group1.insert(5);
	groups.push_back(group0);
	groups.push_back(group1);
	ipf.split_node(PFNodeID(1,2), groups);
	manager->undo();
	ipf.delete_layer(1);
	manager->undo();
	manager->redo();

	ipf.clone_layer(0);
	ipf.clone_layer(1);
	ipf.unzip_node(PFNodeID(0,0), ipf.highest_layer());
	manager->undo();

	ipf.parent_switch(PFNodeID(1,1), 0);
	manager->undo();
}

void lowest_branch_layer_test()
{
	// Construct the leaf layer.
	SimplePixelProperties arr[] = {0,1,2,3,4,5,6,7,8};
	std::vector<SimplePixelProperties> leafProperties(&arr[0], &arr[sizeof(arr)/sizeof(SimplePixelProperties)]);
	shared_ptr<SimpleImageLeafLayer> leafLayer(new SimpleImageLeafLayer(leafProperties, 3, 3));

	// Create some arbitrary groups.
	std::vector<std::set<int> > groups(3);
	for(int i=0; i<=4; ++i) groups[0].insert(i);
	groups[1].insert(5);
	groups[1].insert(8);
	groups[2].insert(6);
	groups[2].insert(7);

	// Construct the lowest branch layer.
	shared_ptr<SimpleImageBranchLayer> lowestBranchLayer = IPF::make_lowest_branch_layer(leafLayer, groups);

	// Construct the forest itself.
	IPF ipf(leafLayer, lowestBranchLayer);

	// Construct a rooted MST from the lowest branch layer.
	RootedMST<SimpleImageBranchLayer::EdgeWeight> mst(*lowestBranchLayer);
}

void nonsibling_node_merging_test()
{
	// Construct initial forest.
	SimplePixelProperties arr[] = {0,1,2,3,4,5,6,7,8};
	std::vector<SimplePixelProperties> leafProperties(&arr[0], &arr[sizeof(arr)/sizeof(SimplePixelProperties)]);
	shared_ptr<SimpleImageLeafLayer> leafLayer(new SimpleImageLeafLayer(leafProperties, 3, 3));

	IPF ipf(leafLayer);

	ICommandManager_Ptr manager(new UndoableCommandManager);
	ipf.set_command_manager(manager);

	std::set<PFNodeID> mergees;

	ipf.clone_layer(0);

	ipf.clone_layer(1);
		mergees.insert(PFNodeID(2,0));	mergees.insert(PFNodeID(2,1));
	ipf.merge_sibling_nodes(mergees);	mergees.clear();
		mergees.insert(PFNodeID(2,2));	mergees.insert(PFNodeID(2,5));
	ipf.merge_sibling_nodes(mergees);	mergees.clear();
		mergees.insert(PFNodeID(2,3));	mergees.insert(PFNodeID(2,4));
	ipf.merge_sibling_nodes(mergees);	mergees.clear();
		mergees.insert(PFNodeID(2,6));	mergees.insert(PFNodeID(2,7));	mergees.insert(PFNodeID(2,8));
	ipf.merge_sibling_nodes(mergees);	mergees.clear();

	ipf.clone_layer(2);
		mergees.insert(PFNodeID(3,0));	mergees.insert(PFNodeID(3,2));
	ipf.merge_sibling_nodes(mergees);	mergees.clear();
		mergees.insert(PFNodeID(3,3));	mergees.insert(PFNodeID(3,6));
	ipf.merge_sibling_nodes(mergees);	mergees.clear();

	ipf.clone_layer(3);
		mergees.insert(PFNodeID(4,0));	mergees.insert(PFNodeID(4,3));
	ipf.merge_sibling_nodes(mergees);	mergees.clear();

	// Perform operations on it.
	mergees.insert(PFNodeID(1,1));
	mergees.insert(PFNodeID(1,2));
	mergees.insert(PFNodeID(1,3));
	mergees.insert(PFNodeID(1,6));
	ipf.merge_nonsibling_nodes(mergees);

	ipf.output(std::cout);
}

void selection_test()
{
	ICommandManager_Ptr manager(new UndoableCommandManager);
	IPF_Ptr ipf = default_ipf(manager);

	// Test forest selection.
	Selection_Ptr selection(new Selection(ipf));
	selection->set_command_manager(manager);
	ipf->add_weak_listener(selection);
	ipf->add_shared_listener(shared_ptr<ForestListener>(new ForestListener));
	selection->add_shared_listener(shared_ptr<SelectionListener>(new SelectionListener));

	selection->select_node(PFNodeID(4,0));
	manager->undo();
	manager->redo();
	selection->deselect_node(PFNodeID(0,3));
	manager->undo();

	std::vector<std::set<int> > splitGroups(2);
	splitGroups[0].insert(0);
	splitGroups[1].insert(2);
	ipf->split_node(PFNodeID(4,0), splitGroups);
	manager->undo();

	selection->deselect_node(PFNodeID(0,3));
	ipf->delete_layer(3);
	ipf->delete_layer(2);
	manager->undo();
	manager->undo();
	selection->replace_with_node(PFNodeID(4,0));
	manager->undo();
}

void switch_parent_test()
{
	ICommandManager_Ptr manager(new UndoableCommandManager);
	IPF_Ptr ipf = default_ipf(manager);

	ipf->parent_switch(PFNodeID(1,6), 2);
	manager->undo();
	manager->redo();
	manager->undo();

	ipf->output(std::cout);
}

void unzip_zip_test()
{
	ICommandManager_Ptr manager(new UndoableCommandManager);
	IPF_Ptr ipf = default_ipf(manager);

	ipf->unzip_node(PFNodeID(0,5), 3);
	ipf->unzip_node(PFNodeID(0,4), 3);

	std::vector<IPF::Chain> chains;
	IPF::Chain chain0;
	chain0.push_back(PFNodeID(3,4));
	chain0.push_back(PFNodeID(2,4));
	chain0.push_back(PFNodeID(1,4));
	chains.push_back(chain0);
	IPF::Chain chain1;
	chain1.push_back(PFNodeID(3,5));
	chain1.push_back(PFNodeID(2,5));
	chain1.push_back(PFNodeID(1,5));
	chains.push_back(chain1);

	ipf->zip_chains(chains);

	ipf->output(std::cout);
}

int main()
{
	feature_selection_test();
	//listener_test();
	//lowest_branch_layer_test();
	//nonsibling_node_merging_test();
	//selection_test();
	//switch_parent_test();
	//unzip_zip_test();
	return 0;
}
