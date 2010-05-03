#ifndef INCLUDED_moged_appcontext_HH
#define INCLUDED_moged_appcontext_HH

#include <vector>
#include <string>

enum CanvasType {
	CanvasType_Skeleton = 0,
	CanvasType_Playback,
	CanvasType_MotionGraph,
	CanvasTypeCount,
};
class CanvasController;

namespace Events {
	class EventSystem;
}

class Entity;

class AppContext
{
	int m_run_level;
	std::vector< CanvasController* > m_canvas_controllers;
	Events::EventSystem* m_evsys;
	std::string m_base_folder;
	Entity* m_current_entity;
public:
	AppContext();
	~AppContext();
	
	void Update();
	
	int GetRunLevel() const { return m_run_level; }
	void SetRunLevel(int level);

	CanvasController* GetCanvasController(int type);
	Events::EventSystem* GetEventSystem() { return m_evsys; }

	const char* GetBaseFolder() const { return m_base_folder.c_str(); }
	void SetBaseFolder(const char* folder) ;

	// app context must ALWAYS have an entity.
	void SetEntity(Entity* entity) ;
	Entity* GetEntity() ;
private:
	void InitWiring();
};

#endif
