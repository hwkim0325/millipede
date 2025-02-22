/***
 * millipede: IPFOverlayTools.h
 * Copyright Stuart Golodetz, 2010. All rights reserved.
 ***/

#ifndef H_MILLIPEDE_IPFOVERLAYTOOLS
#define H_MILLIPEDE_IPFOVERLAYTOOLS

#include <common/partitionforests/images/VolumeIPF.h>
#include <common/slices/SliceLocation.h>
#include <common/slices/SliceOrientation.h>
#include <common/textures/RGBA32ImageTexture.h>
#include <common/util/ITKImageUtil.h>

namespace mp {

namespace IPFOverlayTools {

//#################### FUNCTIONS ####################
/**
@brief	Calculates the bounding indices, width and height of a volume slice.

The bounding indices are 3D positions sliceBegin and sliceEnd such that precisely the pixels in the slice lie in [sliceBegin,sliceEnd).

@param[in]	volumeSize			The size of the volume
@param[in]	sliceLocation		The slice's location within the volume (only one of the coordinates is relevant, namely that specified by the slice orientation)
@param[in]	sliceOrientation	The slice's orientation within the volume (XY, XZ or YZ)
@param[out]	sliceBegin			The lower bounding index of the slice
@param[out]	sliceEnd			The upper bounding index of the slice
@param[out]	width				The width of the slice
@param[out]	height				The height of the slice
*/
void calculate_slice_parameters(const itk::Size<3>& volumeSize, const SliceLocation& sliceLocation, SliceOrientation sliceOrientation,
								itk::Index<3>& sliceBegin, itk::Index<3>& sliceEnd, int& width, int& height);

//#################### TEMPLATE FUNCTIONS ####################
/**
@brief	Draws a node in a volume IPF onto an image corresponding to a slice through the volume the IPF represents.

@param[in]	volumeIPF			The volume IPF
@param[in]	node				The node to draw
@param[in]	image				The image onto which to draw it
@param[in]	sliceBegin			The lower bounding index of the slice
@param[in]	sliceEnd			The upper bounding index of the slice
@param[in]	sliceOrientation	The orientation of the slice (XY, XZ or YZ)
@param[in]	colour				The colour with which to draw the node
@param[in]	boundariesOnly		Whether or not to draw just the boundary of the node
*/
template <typename VolumeIPF_CPtr>
void draw_node(const VolumeIPF_CPtr& volumeIPF, const PFNodeID& node, RGBA32Image::Pointer image,
			   const itk::Index<3>& sliceBegin, const itk::Index<3>& sliceEnd, SliceOrientation sliceOrientation,
			   const RGBA32& colour, bool boundariesOnly)
{
	std::list<int> receptiveRegion = volumeIPF->receptive_region_of(node);
	for(std::list<int>::const_iterator it=receptiveRegion.begin(), iend=receptiveRegion.end(); it!=iend; ++it)
	{
		// Calculate the position of the leaf.
		itk::Index<3> volumePos = volumeIPF->position_of_leaf(*it);

		// Ignore the leaf if it's not within [sliceBegin,sliceEnd).
		if(volumePos[0] < sliceBegin[0] || volumePos[1] < sliceBegin[1] || volumePos[2] < sliceBegin[2] ||
		   volumePos[0] >= sliceEnd[0] || volumePos[1] >= sliceEnd[1] || volumePos[2] >= sliceEnd[2])
		{
			continue;
		}

		if(boundariesOnly)
		{
			bool boundary = false;
			std::vector<itk::Offset<3> > offsets = ITKImageUtil::make_4_connected_offsets(sliceOrientation);
			for(size_t i=0, size=offsets.size(); i<size; ++i)
			{
				if(volumeIPF->node_of(node.layer(), volumePos + offsets[i]) != node)
				{
					boundary = true;
					break;
				}
			}
			if(!boundary) continue;
		}

		// Project the position into image coordinates.
		itk::Index<2> imagePos = {{-1, -1}};
		switch(sliceOrientation)
		{
			case ORIENT_XY:	imagePos[0] = volumePos[0]; imagePos[1] = volumePos[1]; break;
			case ORIENT_XZ:	imagePos[0] = volumePos[0]; imagePos[1] = volumePos[2]; break;
			case ORIENT_YZ:	imagePos[0] = volumePos[1]; imagePos[1] = volumePos[2]; break;
		}

		// Draw the pixel.
		image->SetPixel(imagePos, colour);
	}
}

}

}

#endif
