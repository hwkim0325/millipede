/***
 * millipede: DICOMRegionProperties.h
 * Copyright Stuart Golodetz, 2010. All rights reserved.
 ***/

#ifndef H_MILLIPEDE_DICOMREGIONPROPERTIES
#define H_MILLIPEDE_DICOMREGIONPROPERTIES

#include <vector>

#include "DICOMPixelProperties.h"

namespace mp {

class DICOMRegionProperties
{
	//#################### PRIVATE VARIABLES ####################
private:
	size_t m_area;
	double m_meanGreyValue;

	//#################### CONSTRUCTORS ####################
public:
	DICOMRegionProperties();

	//#################### PUBLIC METHODS ####################
public:
	int area() const;
	static DICOMRegionProperties combine_branch_properties(const std::vector<DICOMRegionProperties>& properties);
	static DICOMRegionProperties combine_leaf_properties(const std::vector<DICOMPixelProperties>& properties);
	double mean_grey_value() const;
};

//#################### GLOBAL OPERATORS ####################
std::ostream& operator<<(std::ostream& os, const DICOMRegionProperties& rhs);

}

#endif
