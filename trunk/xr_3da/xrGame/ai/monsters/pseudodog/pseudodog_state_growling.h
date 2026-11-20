#pragma once

#include "../state.h"

template<typename _Object>
class	CPseudodogGrowling : public CState<_Object> {
protected:
	typedef CState<_Object>		inherited;

public:
						CPseudodogGrowling		(_Object *obj);
	virtual	void		execute					();

	u32					m_time_last_psy_hit;
	virtual void		reinit					();
	virtual bool 		check_start_conditions	();
};

#include "pseudodog_state_growling_inline.h"
