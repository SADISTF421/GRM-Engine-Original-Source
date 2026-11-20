#pragma once

#include "alife_space.h"
#include "string_table_defs.h"
#include "object_interfaces.h"

#define NOT_SIMULATION_NEWS		ALife::_NEWS_ID(-1)
#define DEFAULT_NEWS_SHOW_TIME	5000


struct GAME_NEWS_DATA : public IPureSerializeObject<IReader,IWriter>
{	
	enum eNewsType{eNews=0, eTalk=1,} m_type;
						GAME_NEWS_DATA		();

	virtual void		load				(IReader&);
	virtual void		save				(IWriter&);


	//id новости из симуляции ALife::_NEWS_ID(-1) то новость - сюжетная
	ALife::_NEWS_ID 	news_id;
	shared_str			news_text;
	int					show_time;

	ALife::_TIME_ID		receive_time;

	shared_str			texture_name;
	Frect				tex_rect;

	LPCSTR				SingleLineText();

private:
	//полный текст новостей, формируется при первом обращении
	xr_string full_news_text;

	// Структурка описывающая ньюс
	typedef struct tagNewsTemplate
	{
		shared_str		str;
	} SNewsTemplate;

	// Array of news templates
	typedef xr_map<u32, SNewsTemplate>	NewsTemplates;
	NewsTemplates							m_NewsTemplates;

	void LoadNewsTemplates();
};

DEFINE_VECTOR(GAME_NEWS_DATA, GAME_NEWS_VECTOR, GAME_NEWS_IT);