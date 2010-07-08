/***
 * millipede: MeshBuildingData.h
 * Copyright Stuart Golodetz, 2010. All rights reserved.
 ***/

#ifndef H_MILLIPEDE_MESHBUILDINGDATA
#define H_MILLIPEDE_MESHBUILDINGDATA

#include <itkImage.h>

#include <common/jobs/DataHook.h>
#include "CubeTable.h"
#include "GlobalNodeTable.h"

namespace mp {

template <typename Label>
class MeshBuildingData
{
	//#################### TYPEDEFS ####################
public:
	typedef GlobalNodeTable<Label> GlobalNodeTableT;
	typedef itk::Image<Label,3> LabelImage;
	typedef typename LabelImage::Pointer LabelImagePointer;

	//#################### PRIVATE VARIABLES ####################
private:
	CubeTable m_cubeTable;
	GlobalNodeTableT m_globalNodeTable;
	DataHook<LabelImagePointer> m_labellingHook;
	// TODO: Triangles

	//#################### CONSTRUCTORS ####################
public:
	MeshBuildingData()
	{}

	//#################### COPY CONSTRUCTOR & ASSIGNMENT OPERATOR ####################
private:
	MeshBuildingData(const MeshBuildingData&);
	MeshBuildingData& operator=(const MeshBuildingData&);

	//#################### PUBLIC METHODS ####################
public:
	CubeTable& cube_table()
	{
		return m_cubeTable;
	}

	GlobalNodeTableT& global_node_table()
	{
		return m_globalNodeTable;
	}

	Label label(const Vector3i& pos) const
	{
		itk::Index<3> index = {{pos.x, pos.y, pos.z}};
		return m_labellingHook.get()->GetPixel(index);
	}

	LabelImagePointer labelling() const
	{
		return m_labellingHook.get();
	}

	void set_labelling(const LabelImagePointer& labelling)
	{
		m_labellingHook.set(labelling);
	}

	void set_labelling_hook(const DataHook<LabelImagePointer>& labellingHook)
	{
		m_labellingHook = labellingHook;
	}
};

}

#endif
