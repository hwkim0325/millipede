/***
 * millipede: MeshBuildingData.h
 * Copyright Stuart Golodetz, 2010. All rights reserved.
 ***/

#ifndef H_MILLIPEDE_MESHBUILDINGDATA
#define H_MILLIPEDE_MESHBUILDINGDATA

#include <itkImage.h>

#include <common/jobs/DataHook.h>
#include "CubeFaceTable.h"
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
	CubeFaceTable m_cubeFaceTable;
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
	CubeFaceTable& cube_face_table()
	{
		return m_cubeFaceTable;
	}

	GlobalNodeTableT& global_node_table()
	{
		return m_globalNodeTable;
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
