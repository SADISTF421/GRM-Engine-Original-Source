///////////////////////////////////////////////////////////////
// game_news.cpp
// реестр новостей: новости симул€ции + сюжетные
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "game_news.h"
#include "object_broker.h"

#include "ai_space.h"
#include "alife_simulator.h"
#include "alife_news_registry.h"
#include "alife_object_registry.h"
#include "game_graph.h"

#include "date_time.h"
#include "xrServer_Objects_ALife_Monsters.h"

#include "string_table.h"
#include "specific_character.h"



//ссылка на string table
const char * const NEWS_TYPE_KILL				= "alife_news_kill";
const char * const NEWS_TYPE_RETREAT			= "alife_news_retreat";

GAME_NEWS_DATA::GAME_NEWS_DATA():news_id(ALife::_NEWS_ID(-1))
{
	m_type			= eNews;
	tex_rect.set	(0.0f,0.0f,0.0f,0.0f);
	show_time		= DEFAULT_NEWS_SHOW_TIME;

	LoadNewsTemplates ();
}

void GAME_NEWS_DATA::save (IWriter& stream)
{
	save_data(news_id, stream);
	save_data(m_type,		stream);
	save_data(news_text,	stream);
	save_data(receive_time, stream);
	save_data(texture_name,	stream);
	save_data(tex_rect,		stream);
}

void GAME_NEWS_DATA::load (IReader& stream)
{
	load_data(news_id, 		stream);
	load_data(m_type,		stream);
	load_data(news_text,	stream);
	load_data(receive_time, stream);
	load_data(texture_name,	stream);
	load_data(tex_rect,		stream);
}



LPCSTR GAME_NEWS_DATA::SingleLineText()
{
	if( xr_strlen(full_news_text.c_str()) )
		return full_news_text.c_str();

	string1024	newsPhrase = "";
	string128	time = "";
	string512	locationName = "";
	string2048	result = "";
	string2048	result2 = "";
	char		ColorMsg[]	= "%c[255,245,118,54]";

	if(news_id != NOT_SIMULATION_NEWS)
	{
		const CALifeNews* pNewsItem  = ai().alife().news().news(news_id); VERIFY(pNewsItem);
		const CALifeNews& newsItem =  *pNewsItem;

		// Get Level name
		const CGameGraph::CVertex	*game_vertex = ai().game_graph().vertex(newsItem.m_game_vertex_id);
		if (ai().game_graph().header().levels().find(game_vertex->level_id()) != ai().game_graph().header().levels().end())
		{
			strconcat	(sizeof(locationName),locationName,*CStringTable().translate(ai().game_graph().header().levels().find(game_vertex->level_id())->second.name()), " ");
		}

		// Substitute placeholders with real names
		LPCSTR name1 = "";
		LPCSTR name2 = "";

		CSE_ALifeDynamicObject	*newsActorOne = ai().alife().objects().object(newsItem.m_object_id[0]); VERIFY(newsActorOne);
		name1 = newsActorOne->name_replace();
		CSE_ALifeTraderAbstract* pTrader = NULL;
		pTrader = smart_cast<CSE_ALifeTraderAbstract*>(newsActorOne);
		if(pTrader)
		{
			if(pTrader->specific_character().size())
			{
				CSpecificCharacter spec_char;
				spec_char.Load(pTrader->specific_character());
				name1 = *CStringTable().translate(spec_char.Name());
			}
		}

		if (newsItem.m_object_id[1] != static_cast<u16>(-1))
		{
			CSE_ALifeDynamicObject	*newsActorTwo = ai().alife().objects().object(newsItem.m_object_id[1]);
			name2 = newsActorTwo->name_replace();
			pTrader = smart_cast<CSE_ALifeTraderAbstract*>(newsActorTwo);
			if(pTrader)
			{
				if(pTrader->specific_character().size())
				{
					CSpecificCharacter spec_char;
					spec_char.Load(pTrader->specific_character());
					name2 = *CStringTable().translate(spec_char.Name());
				}
			}
		}
		sprintf_s(newsPhrase, *m_NewsTemplates[static_cast<u32>(newsItem.m_news_type)].str, name1, name2);
	}

	// Calc current time
	u32 years, months, days, hours, minutes, seconds, milliseconds;
	split_time		(receive_time, years, months, days, hours, minutes, seconds, milliseconds);
#pragma todo("Satan->Satan : insert carry-over")
	if(news_id == NOT_SIMULATION_NEWS)
		sprintf_s		(time, "%02i:%02i ", hours, minutes);
	else
		sprintf_s		(time, "%02i:%02i \\n", hours, minutes);

	strconcat		(sizeof(result),result, locationName, time, newsPhrase);
	sprintf_s		(result2, "%s%s", ColorMsg, result);

	if(news_id == NOT_SIMULATION_NEWS) {
		full_news_text			= time;
		full_news_text			+= news_text.c_str();
	} else {
		full_news_text = result2;
	}

	return full_news_text.c_str();
}

void GAME_NEWS_DATA::LoadNewsTemplates()
{
	if(!m_NewsTemplates.empty()) return;

	m_NewsTemplates[ALife::eNewsTypeKill].str = CStringTable().translate(NEWS_TYPE_KILL);
	m_NewsTemplates[ALife::eNewsTypeRetreat].str = CStringTable().translate(NEWS_TYPE_RETREAT);
}
