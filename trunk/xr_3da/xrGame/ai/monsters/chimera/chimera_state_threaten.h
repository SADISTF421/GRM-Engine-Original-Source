#pragma once

#include "../state.h"

template<typename _Object>
class	CStateChimeraThreaten : public CState<_Object> {
protected:
	typedef CState<_Object>		inherited;

public:
						CStateChimeraThreaten		(_Object *obj);
	virtual	void		execute					();

	u32					m_last_time_threaten;
	virtual void		reinit					();
	virtual bool 		check_start_conditions	();
};

#include "chimera_state_threaten_inline.h"
