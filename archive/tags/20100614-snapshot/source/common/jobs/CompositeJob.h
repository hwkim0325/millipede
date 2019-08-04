/***
 * millipede: CompositeJob.h
 * Copyright Stuart Golodetz, 2010. All rights reserved.
 ***/

#ifndef H_MILLIPEDE_COMPOSITEJOB
#define H_MILLIPEDE_COMPOSITEJOB

#include <utility>
#include <vector>

#include "Job.h"

namespace mp {

class CompositeJob : public Job
{
	//#################### PRIVATE VARIABLES ####################
private:
	Job_Ptr m_currentJob;
	std::vector<std::pair<Job_Ptr,bool> > m_jobs;
	int m_length;
	int m_progress;

	//#################### CONSTRUCTORS ####################
public:
	CompositeJob();

	//#################### PUBLIC METHODS ####################
public:
	void abort();
	void add_subjob(Job *job);
	void add_subjob(const Job_Ptr& job);
	void add_main_thread_subjob(Job *job);
	void add_main_thread_subjob(const Job_Ptr& job);
	void execute();
	int length() const;
	int progress() const;
	std::string status() const;
};

//#################### TYPEDEFS ####################
typedef boost::shared_ptr<CompositeJob> CompositeJob_Ptr;

}

#endif
