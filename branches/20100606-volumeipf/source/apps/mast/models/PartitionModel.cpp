/***
 * millipede: PartitionModel.cpp
 * Copyright Stuart Golodetz, 2009. All rights reserved.
 ***/

#include "PartitionModel.h"

namespace mp {

//#################### CONSTRUCTORS ####################
PartitionModel::PartitionModel(const DICOMVolume_Ptr& dicomVolume, const SliceLocation& loc, SliceOrientation ori)
:	m_dicomVolume(dicomVolume), m_sliceLocation(loc), m_sliceOrientation(ori)
{}

//#################### PUBLIC METHODS ####################
void PartitionModel::add_listener(Listener *listener)
{
	m_listeners.push_back(listener);
}

SliceTextureSet_CPtr PartitionModel::dicom_texture_set() const					{ return m_dicomTextureSet; }
DICOMVolume_CPtr PartitionModel::dicom_volume() const							{ return m_dicomVolume; }

SliceTextureSet_CPtr PartitionModel::partition_texture_set(int layer) const
{
	int n = layer - 1;
	if(0 <= n && n < static_cast<int>(m_partitionTextureSets.size())) return m_partitionTextureSets[n];
	else return SliceTextureSet_CPtr();
}

void PartitionModel::set_dicom_texture_set(const SliceTextureSet_Ptr& dicomTextureSet)
{
	m_dicomTextureSet = dicomTextureSet;
	alert_listeners();
}

void PartitionModel::set_partition_texture_sets(const std::vector<SliceTextureSet_Ptr>& partitionTextureSets)
{
	m_partitionTextureSets = partitionTextureSets;
	alert_listeners();
}

void PartitionModel::set_slice_location(const SliceLocation& loc)
{
	// TODO: Validate location against bounds
	m_sliceLocation = loc;
	alert_listeners();
}

void PartitionModel::set_slice_orientation(SliceOrientation ori)
{
	m_sliceOrientation = ori;
	alert_listeners();
}

void PartitionModel::set_volume_ipf(const VolumeIPF_Ptr& volumeIPF)
{
	m_volumeIPF = volumeIPF;
	m_selection.reset(new IPFSelection(volumeIPF));
	alert_listeners();
}

const SliceLocation& PartitionModel::slice_location() const					{ return m_sliceLocation; }
SliceOrientation PartitionModel::slice_orientation() const					{ return m_sliceOrientation; }
const PartitionModel::VolumeIPF_Ptr& PartitionModel::volume_ipf()			{ return m_volumeIPF; }
PartitionModel::VolumeIPF_CPtr PartitionModel::volume_ipf() const			{ return m_volumeIPF; }

//#################### PRIVATE METHODS ####################
void PartitionModel::alert_listeners()
{
	for(size_t i=0, size=m_listeners.size(); i<size; ++i)
	{
		m_listeners[i]->model_changed();
	}
}

}
